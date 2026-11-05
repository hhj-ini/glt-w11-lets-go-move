-- 이름으로 동작을 정하는 블록.
-- Pop_<시각>_<번호>: 그 시각에 나타난다
-- Out_<시각>_<번호>: 그 시각에 사라진다
-- Deb_<시각>_<vx>_<vy>_<vz>_<번호>: 그 시각에 날아갔다가 떨어진다

local HIDDEN = 0.01
local POP_TIME = 0.16
local GRAVITY = 18.0
local REST_TIME = 4.0

local Mode = nil
local StartTime = 0.0
local Elapsed = 0.0
local FullScale, Origin
local Velocity = { 0.0, 0.0, 0.0 }

local function SetUniformScale(Alpha)
    self:SetActorScale3D(FVector.new(
        math.max(FullScale.X * Alpha, HIDDEN),
        math.max(FullScale.Y * Alpha, HIDDEN),
        math.max(FullScale.Z * Alpha, HIDDEN)))
end

function BeginPlay()
    local Name = self:GetName()
    FullScale = self:GetActorScale3D()
    Origin = self:GetActorLocation()

    local PopTime = Name:match("^Pop_([%d%.]+)_")
    local OutTime = Name:match("^Out_([%d%.]+)_")
    local DebTime, Vx, Vy, Vz = Name:match("^Deb_([%d%.]+)_(-?[%d%.]+)_(-?[%d%.]+)_(-?[%d%.]+)_")
    if PopTime then
        Mode, StartTime = "Pop", tonumber(PopTime)
        SetUniformScale(0.0)
    elseif OutTime then
        Mode, StartTime = "Out", tonumber(OutTime)
    elseif DebTime then
        Mode, StartTime = "Deb", tonumber(DebTime)
        Velocity = { tonumber(Vx), tonumber(Vy), tonumber(Vz) }
        SetUniformScale(0.0)
    end
end

function Tick(DeltaTime)
    if not Mode then
        return
    end
    Elapsed = Elapsed + DeltaTime
    local Time = Elapsed - StartTime
    if Time < 0.0 then
        return
    end

    if Mode == "Pop" then
        local Alpha = math.min(Time / POP_TIME, 1.0)
        SetUniformScale(Alpha + 0.25 * math.sin(Alpha * math.pi))
        if Alpha >= 1.0 then
            Mode = nil
        end
    elseif Mode == "Out" then
        local Alpha = math.min(Time / POP_TIME, 1.0)
        SetUniformScale(1.0 - Alpha)
        if Alpha >= 1.0 then
            Mode = nil
        end
    else
        -- 바닥에 닿는 시각까지만 포물선을 따라간다
        local Landing = (Velocity[3] + math.sqrt(Velocity[3] * Velocity[3] + 2.0 * GRAVITY * Origin.Z)) / GRAVITY
        local FlyTime = math.min(Time, Landing)
        self:SetActorLocation(FVector.new(
            Origin.X + Velocity[1] * FlyTime,
            Origin.Y + Velocity[2] * FlyTime,
            math.max(Origin.Z + Velocity[3] * FlyTime - 0.5 * GRAVITY * FlyTime * FlyTime, 0.0) + FullScale.Z * 0.5))
        self:SetActorRotation(FRotator.new(FlyTime * 340.0, FlyTime * 210.0, FlyTime * 160.0))

        if Time < REST_TIME then
            SetUniformScale(1.0)
        else
            local Alpha = math.min((Time - REST_TIME) / 0.4, 1.0)
            SetUniformScale(1.0 - Alpha)
            if Alpha >= 1.0 then
                Mode = nil
            end
        end
    end
end
