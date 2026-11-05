-- 블록 월드의 Steve. 액터 이름으로 역할을 고르고, 시간표에 맞춰 일하다가 올마이트 착지에 환호한다.
-- 팔다리 각도는 차렷(0)에서 앞(+)으로 벌어진 정도다. 메시의 정면은 로컬 +Y다.

local T_ALERT = 14.0
local T_IMPACT = 18.0
local T_CHEER = 20.2
local LAND_X, LAND_Y = 0.0, -3.0
local STAND_Z = 1.01 -- 차렷 자세의 발바닥이 메시 원점 아래 6.76이다 (× 스케일 0.15)

-- 본 축과 기준 자세 각도는 메시의 RefBonePose로 계산한 값이다
local Limbs = {
    RightArm = { Bone = "Bone.003", Axis = { 0.9326, 0.0, -0.3610 }, Bind = -43.3 },
    LeftArm = { Bone = "Bone.002", Axis = { 0.9297, 0.0, 0.3684 }, Bind = 44.5 },
    RightLeg = { Bone = "Bone.006", Axis = { 1.0, 0.0, 0.0 }, Bind = 44.5 },
    LeftLeg = { Bone = "Bone.008", Axis = { 1.0, 0.0, 0.0 }, Bind = -43.3 },
    Head = { Bone = "Bone.010", Axis = { 1.0, 0.0, 0.0 }, Bind = 0.0 },
}

-- Path: { 시각, X, Y }, Work: 팔을 휘두르는 구간, Face: 멈췄을 때 바라볼 곳
local Roles = {
    Steve_Builder = {
        Path = { { 0, 3, 3.5 }, { 4, 10, 3.5 }, { 6.5, 11.5, 8 }, { 9, 9, 12.5 }, { 12, 3, 12.5 }, { 14, 1.5, 8 } },
        Work = { { 0.0, 14.0 } }, Reach = 110.0,
    },
    Steve_Lumber = {
        Path = { { 0, -9.5, -7.6 }, { 10.5, -9.5, -7.6 }, { 13, -5.5, -7 } },
        Work = { { 1.5, 10.0 } }, Face = { -9.5, -5.5 }, Reach = 80.0,
    },
    Steve_Miner = {
        Path = { { 0, 11.5, -8.4 }, { 7.5, 11.5, -8.4 }, { 8.2, 11.5, -9.4 } },
        Work = { { 1.2, 13.0 } }, Face = { 11.5, -12 }, Reach = 75.0,
    },
    Steve_Tower = {
        Path = { { 0, -5.5, 4.4 }, { 14, -5.5, 4.4 } },
        Work = { { 1.2, 13.0 } }, Face = { -5.5, 6.5 }, Reach = 135.0,
    },
    Steve_Walker = {
        Path = { { 0, -7, 3.5 }, { 3.5, -7, 14 }, { 6.5, -15, 15.5 }, { 9.5, -20.5, 9 }, { 11, -20, 4 }, { 12.5, -14, 2.5 }, { 14, -8, 3 } },
        Work = {}, Reach = 0.0,
    },
    Steve_Fisher = {
        Path = { { 0, -8.8, 9 }, { 14, -8.8, 9 } },
        Work = {}, Face = { -14, 9 }, Reach = 0.0, Hold = 62.0,
    },
}

local Role = nil
local Elapsed = 0.0
local Yaw = 0.0
local WalkPhase = 0.0
local Rotations = {}

-- 축과 각도(도) → FRotator. 엔진의 FQuat::ToFMatrix, MatrixToRotator와 같은 식이다
local function AxisRotator(Axis, Degrees)
    local Half = math.rad(Degrees) * 0.5
    local S = math.sin(Half)
    local X, Y, Z, W = Axis[1] * S, Axis[2] * S, Axis[3] * S, math.cos(Half)
    local M00 = 1.0 - 2.0 * (Y * Y + Z * Z)
    local M01 = 2.0 * (X * Y + W * Z)
    local M02 = 2.0 * (X * Z - W * Y)
    local M12 = 2.0 * (Y * Z + W * X)
    local M22 = 1.0 - 2.0 * (X * X + Y * Y)
    return FRotator.new(
        math.deg(math.atan2(-M02, math.sqrt(M12 * M12 + M22 * M22))),
        math.deg(math.atan2(M01, M00)),
        math.deg(math.atan2(M12, M22)))
end

local function SetLimb(Limb, Degrees)
    Rotations[Limb.Bone] = AxisRotator(Limb.Axis, Degrees - Limb.Bind)
end

local function IsWorking(Time)
    for _, Range in ipairs(Role.Work) do
        if Time >= Range[1] and Time <= Range[2] then
            return true
        end
    end
    return false
end

-- 위치와 이동 방향, 이동 여부, 속도를 돌려준다
local function SamplePath(Time)
    local Path = Role.Path
    for Index = 1, #Path - 1 do
        local A, B = Path[Index], Path[Index + 1]
        if Time <= B[1] then
            local Alpha = (Time - A[1]) / (B[1] - A[1])
            local Dx, Dy = B[2] - A[2], B[3] - A[3]
            local Length = math.sqrt(Dx * Dx + Dy * Dy)
            return A[2] + Dx * Alpha, A[3] + Dy * Alpha, Dx, Dy, Length > 0.001, Length / (B[1] - A[1])
        end
    end
    local Last = Path[#Path]
    return Last[2], Last[3], 0.0, 0.0, false, 0.0
end

local function TurnTo(TargetYaw, DeltaTime)
    local Delta = (TargetYaw - Yaw + 180.0) % 360.0 - 180.0
    Yaw = Yaw + Delta * math.min(DeltaTime * 8.0, 1.0)
end

-- 정면이 +Y이므로 (Dx, Dy)를 보는 Yaw는 atan2(-Dx, Dy)다
local function YawToward(Dx, Dy)
    return math.deg(math.atan2(-Dx, Dy))
end

function BeginPlay()
    Role = Roles[self:GetName()]
    if Role then
        Yaw = self:GetActorRotation().Yaw
    end
end

function Tick(DeltaTime)
    if not Role then
        return
    end
    Elapsed = Elapsed + DeltaTime
    local Time = Elapsed

    local X, Y, Dx, Dy, bMoving, Speed = SamplePath(math.min(Time, T_ALERT))
    local Z = STAND_Z
    local RightArm, LeftArm, RightLeg, LeftLeg, Head = 0.0, 0.0, 0.0, 0.0, 0.0

    if Time < T_ALERT then
        if bMoving then
            TurnTo(YawToward(Dx, Dy), DeltaTime)
            WalkPhase = WalkPhase + DeltaTime * Speed * 3.2
            local Swing = math.sin(WalkPhase) * 38.0
            RightLeg, LeftLeg = Swing, -Swing
            RightArm, LeftArm = -Swing * 0.8, Swing * 0.8
        elseif Role.Face then
            TurnTo(YawToward(Role.Face[1] - X, Role.Face[2] - Y), DeltaTime)
        end

        if IsWorking(Time) then
            RightArm = Role.Reach + math.sin(Time * 9.0) * 42.0
        elseif Role.Hold then
            RightArm, LeftArm = Role.Hold, Role.Hold
        end
    else
        TurnTo(YawToward(LAND_X - X, LAND_Y - Y), DeltaTime)

        if Time < T_IMPACT then
            -- 하늘을 올려다본다
            Head = math.min((Time - T_ALERT) / 0.8, 1.0) * 34.0
            RightArm, LeftArm = 12.0, 12.0
        elseif Time < T_CHEER then
            -- 충격파에 튀어 오른다
            local Hop = Time - T_IMPACT
            Z = Z + math.max(0.0, Hop * (2.3 - Hop * 4.2))
            RightArm, LeftArm = 150.0, 150.0
            RightLeg, LeftLeg = 22.0, -22.0
        else
            -- 환호
            local Jump = (Time - T_CHEER) * 7.0 + X
            Z = Z + math.abs(math.sin(Jump)) * 0.55
            local Wave = math.sin(Jump * 2.0) * 14.0
            RightArm, LeftArm = 160.0 + Wave, 160.0 - Wave
            RightLeg, LeftLeg = 10.0, -10.0
            Head = 8.0
        end
    end

    self:SetActorLocation(FVector.new(X, Y, Z))
    self:SetActorRotation(FRotator.new(0.0, Yaw, 0.0))

    SetLimb(Limbs.RightArm, RightArm)
    SetLimb(Limbs.LeftArm, LeftArm)
    SetLimb(Limbs.RightLeg, RightLeg)
    SetLimb(Limbs.LeftLeg, LeftLeg)
    SetLimb(Limbs.Head, Head)
    self:SetBoneRotations(Rotations)
end
