-- 올마이트: 하늘에서 떨어져 웅크려 착지한 뒤, 일어나 주먹을 치켜든다.
-- 메시의 원점은 발밑이고 정면은 로컬 +Y다. 모든 본의 축이 컴포넌트 축과 같다.

local T_FALL = 15.0
local T_IMPACT = 18.0
local T_RISE = 19.2
local T_POSE = 20.4
local LAND_X, LAND_Y = 0.0, -3.0
local FALL_HEIGHT = 70.0
local HIDE_HEIGHT = 400.0
local MESH_SCALE = 0.45
local THIGH, SHIN = 1.2, 1.201 -- 엉덩이-무릎, 무릎-발목 길이

-- 다리: +가 앞, 무릎: -가 굽힘, 허리: -가 앞으로 숙임, 머리: +가 위,
-- 왼팔: +가 아래, 오른팔: -가 아래, 팔꿈치: +가 앞으로 굽힘
local Poses = {
    Fall = { Leg = 6, Knee = -14, Waist = 4, Head = 12, LeftArm = 58, RightArm = -58, LeftElbow = 14, RightElbow = 14 },
    Crouch = { Leg = 60, Knee = -120, Waist = -34, Head = 22, LeftArm = 72, RightArm = -72, LeftElbow = 22, RightElbow = 22 },
    Stand = { Leg = 0, Knee = 0, Waist = 0, Head = 8, LeftArm = 72, RightArm = -6, LeftElbow = 62, RightElbow = 8 },
    Hero = { Leg = 0, Knee = 0, Waist = 3, Head = 12, LeftArm = 66, RightArm = 58, LeftElbow = 78, RightElbow = 16 },
}

local Elapsed = 0.0
local Rotations = {}

local function Smooth(Alpha)
    Alpha = math.max(0.0, math.min(Alpha, 1.0))
    return Alpha * Alpha * (3.0 - 2.0 * Alpha)
end

local function BlendPose(A, B, Alpha)
    local Pose = {}
    for Key, Value in pairs(A) do
        Pose[Key] = Value + (B[Key] - Value) * Alpha
    end
    return Pose
end

local function GetPose(Time)
    if Time < T_IMPACT then
        return Poses.Fall
    elseif Time < T_RISE then
        return BlendPose(Poses.Fall, Poses.Crouch, Smooth((Time - T_IMPACT) / 0.07))
    elseif Time < T_POSE then
        return BlendPose(Poses.Crouch, Poses.Stand, Smooth((Time - T_RISE) / (T_POSE - T_RISE)))
    end
    return BlendPose(Poses.Stand, Poses.Hero, Smooth((Time - T_POSE) / 0.3))
end

local function Pitch(Degrees) return FRotator.new(Degrees, 0.0, 0.0) end
local function Yaw(Degrees) return FRotator.new(0.0, Degrees, 0.0) end
local function Roll(Degrees) return FRotator.new(0.0, 0.0, Degrees) end

local function ApplyPose(Pose)
    -- _EX는 근육 보조 본이라 같은 각도로 함께 돌려야 메시가 찢어지지 않는다
    Rotations["L_leg"] = Roll(Pose.Leg)
    Rotations["L_leg_EX"] = Roll(Pose.Leg)
    Rotations["R_leg"] = Roll(Pose.Leg)
    Rotations["R_leg_EX"] = Roll(Pose.Leg)
    Rotations["L_knee"] = Roll(Pose.Knee)
    Rotations["L_knee_EX"] = Roll(Pose.Knee)
    Rotations["R_knee"] = Roll(Pose.Knee)
    Rotations["R_knee_EX"] = Roll(Pose.Knee)
    -- 발바닥이 땅과 평행하도록 정강이 기울기를 되돌린다
    local Ankle = Roll(-(Pose.Leg + Pose.Knee))
    Rotations["L_ankle"] = Ankle
    Rotations["L_ankle_EX"] = Ankle
    Rotations["R_ankle"] = Ankle
    Rotations["R_ankle_EX"] = Ankle
    Rotations["waist"] = Roll(Pose.Waist)
    Rotations["head"] = Roll(Pose.Head)
    Rotations["L_arm"] = Pitch(Pose.LeftArm)
    Rotations["L_arm_EX"] = Pitch(Pose.LeftArm)
    Rotations["R_arm"] = Pitch(Pose.RightArm)
    Rotations["R_arm_EX"] = Pitch(Pose.RightArm)
    Rotations["L_elbow"] = Yaw(Pose.LeftElbow)
    Rotations["R_elbow"] = Yaw(-Pose.RightElbow)
    self:SetBoneRotations(Rotations)
end

function BeginPlay()
    ApplyPose(Poses.Fall)
    self:SetActorLocation(FVector.new(LAND_X, LAND_Y, HIDE_HEIGHT))
end

function Tick(DeltaTime)
    Elapsed = Elapsed + DeltaTime
    local Time = Elapsed
    if Time < T_FALL then
        return
    end

    local Pose = GetPose(Time)
    ApplyPose(Pose)

    local Z = 0.0
    if Time < T_IMPACT then
        local Alpha = (Time - T_FALL) / (T_IMPACT - T_FALL)
        Z = FALL_HEIGHT * (1.0 - Alpha * Alpha)
    else
        -- 다리를 굽힌 만큼 몸을 내려 발이 땅에 붙어 있게 한다
        local LegRad, ShinRad = math.rad(Pose.Leg), math.rad(Pose.Leg + Pose.Knee)
        Z = -((1.0 - math.cos(LegRad)) * THIGH + (1.0 - math.cos(ShinRad)) * SHIN) * MESH_SCALE
    end
    self:SetActorLocation(FVector.new(LAND_X, LAND_Y, Z))
end
