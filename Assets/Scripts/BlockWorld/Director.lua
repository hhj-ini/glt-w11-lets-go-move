-- 카메라 연출, 슬로 모션, 착지 충격을 맡는다. 재생 중에는 입력을 받지 않는다.

local T_FALL = 15.0
local T_SLOW = 17.45
local T_IMPACT = 18.0
local T_TEXT = 20.6
local T_ORBIT = 21.0
local LAND = { 0.0, -3.0 }
local FALL_HEIGHT = 70.0
local SLOW_SCALE = 0.18

-- { 시각, 카메라 위치, 보는 곳 }
local Shots = {
    { 0.0, { -22, -24, 12 }, { 1, 2, 1 } },
    { 5.0, { 2, -26, 11 }, { 3, 4, 1 } },
    { 9.0, { 21, -16, 9 }, { 5, 5, 2 } },
    { 12.5, { 19, 2, 5 }, { 6, 8, 2 } },
    { 14.5, { 10, -13, 3 }, { 1, 1, 7 } },
}

local Elapsed = 0.0
local bImpacted = false
local bSlowActive = false

local function Smooth(Alpha)
    Alpha = math.max(0.0, math.min(Alpha, 1.0))
    return Alpha * Alpha * (3.0 - 2.0 * Alpha)
end

local function Mix(A, B, Alpha)
    return { A[1] + (B[1] - A[1]) * Alpha, A[2] + (B[2] - A[2]) * Alpha, A[3] + (B[3] - A[3]) * Alpha }
end

local function SampleShots(Time)
    for Index = 1, #Shots - 1 do
        local A, B = Shots[Index], Shots[Index + 1]
        if Time <= B[1] then
            local Alpha = Smooth((Time - A[1]) / (B[1] - A[1]))
            return Mix(A[2], B[2], Alpha), Mix(A[3], B[3], Alpha)
        end
    end
    local Last = Shots[#Shots]
    return Last[2], Last[3]
end

local function GetFallZ(Time)
    local Alpha = (Time - T_FALL) / (T_IMPACT - T_FALL)
    return FALL_HEIGHT * (1.0 - Alpha * Alpha)
end

local function GetShot(Time)
    if Time < T_FALL then
        return SampleShots(Time)
    elseif Time < T_IMPACT then
        -- 낮은 자리로 내려가 떨어지는 올마이트를 올려다본다
        local Low = { 1.5, -15.0, 1.0 }
        local Target = { LAND[1], LAND[2], math.min(GetFallZ(Time) + 1.2, 26.0) }
        local From, FromLook = SampleShots(14.5)
        local Alpha = Smooth((Time - T_FALL) / 1.2)
        return Mix(From, Low, Alpha), Mix(FromLook, Target, Alpha)
    elseif Time < T_ORBIT then
        local Alpha = Smooth((Time - T_IMPACT) / (T_ORBIT - T_IMPACT))
        return Mix({ 1.5, -15.0, 1.0 }, { 3.0, -10.0, 1.3 }, Alpha), Mix({ 0.0, -3.0, 1.0 }, { 0.0, -3.0, 1.9 }, Alpha)
    end

    -- 올마이트 앞을 천천히 돌아 뒤편의 Steve들까지 함께 잡는다
    local Angle = math.rad(-66.8 - math.min(Time - T_ORBIT, 16.0) * 3.5)
    return { LAND[1] + math.cos(Angle) * 7.6, LAND[2] + math.sin(Angle) * 7.6, 1.3 }, { 0.0, -3.0, 1.9 }
end

local function PlayImpactEffects()
    local Manager = GetPlayerCameraManager()
    if Manager and Manager:IsValid() then
        Manager:AddCameraModifier("UCameraModifier_CameraShake", {
            Scale = 1.0,
            Pattern = {
                Type = "Perlin", Duration = 1.3, BlendInTime = 0.02, BlendOutTime = 1.1,
                X = { Amplitude = 0.3, Frequency = 20.0 }, Y = { Amplitude = 0.35, Frequency = 23.0 }, Z = { Amplitude = 0.3, Frequency = 25.0 },
                Pitch = { Amplitude = 5.0, Frequency = 22.0 }, Yaw = { Amplitude = 4.0, Frequency = 19.0 }, Roll = { Amplitude = 3.0, Frequency = 17.0 },
            },
        })
    end

    SpawnDecal({
        Location = FVector.new(LAND[1], LAND[2], 0.0),
        Rotation = FRotator.new(90.0, 0.0, 0.0),
        Size = FVector.new(1.0, 5.5, 5.5),
        Texture = "Assets/Textures/BlockWorld/T_Crater.png",
        SpecularFactor = 0.05,
    })
end

function BeginPlay()
    local Manager = GetPlayerCameraManager()
    if Manager and Manager:IsValid() then
        Manager:SetViewTarget(self)
    end

    local Camera = self:GetCamera()
    if Camera then
        Camera:SetFieldOfView(62.0)
    end
end

function Tick(DeltaTime)
    Elapsed = Elapsed + DeltaTime
    local Time = Elapsed

    if not bSlowActive and Time >= T_SLOW and Time < T_IMPACT then
        SetGlobalTimeDilation(SLOW_SCALE)
        bSlowActive = true
    end
    if not bImpacted and Time >= T_IMPACT then
        bImpacted = true
        SetGlobalTimeDilation(1.0)
        PlayImpactEffects()
    end

    local Position, Look = GetShot(Time)
    local Dx, Dy, Dz = Look[1] - Position[1], Look[2] - Position[2], Look[3] - Position[3]
    self:SetActorLocation(FVector.new(Position[1], Position[2], Position[3]))
    -- 이 엔진의 Pitch는 +가 아래를 본다
    self:SetActorRotation(FRotator.new(
        -math.deg(math.atan2(Dz, math.sqrt(Dx * Dx + Dy * Dy))),
        math.deg(math.atan2(Dy, Dx)),
        0.0))

    if Time >= T_TEXT then
        AddUIText("I AM HERE!", 72.0, FVector2.new(0.5, 0.14), FVector2.new(0.5, 0.5))
    end
end

function EndPlay()
    ResetTimeEffects()
end
