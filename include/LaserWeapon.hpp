#pragma once

#include "WeaponBase.hpp"
#include <raylib.h>
#include <cmath>

enum class LaserStage { NONE, PARTIAL, STRONG, FULL };

class LaserWeapon : public Weapon
{
private:
    float range;

    bool  showBeam;
    float beamTimer;
    float beamDuration;
    Vector2 beamStart;
    Vector2 beamEnd;
    Vector2 lastHitPoint;
    bool  didHit;
    int   beamDamage;

    float chargeLevel;
    float chargeRate;
    bool  isCharging;

    float cooldownTimer;
    float cooldownDuration;

public:
    LaserWeapon();
    ~LaserWeapon() override = default;

    void fire(std::pair<int, int> position, std::pair<float, float> direction) override;
    void update(float deltaTime) override;
    std::string getWeaponType() const override { return "Laser"; }

    void startCharge(float deltaTime);
    void cancelCharge();
    bool releaseBeam(Vector2 playerPos, Vector2 direction);

    bool isBeamVisible() const { return showBeam; }
    Vector2 getBeamStart() const { return beamStart; }
    Vector2 getBeamEnd() const { return beamEnd; }
    bool didLastShotHit() const { return didHit; }
    Vector2 getLastHitPoint() const { return lastHitPoint; }
    float getChargeLevel() const { return chargeLevel; }
    bool getIsCharging() const { return isCharging; }
    bool isOnCooldown() const { return cooldownTimer > 0.0f; }
    float getCooldownPct() const { return cooldownTimer / cooldownDuration; }
    LaserStage getStage() const;
    int getBeamDamage() const { return beamDamage; }
    float getRange() const { return range; }
    void setLastHitPoint(Vector2 p) { lastHitPoint = p; didHit = true; }
};

LaserWeapon::LaserWeapon()
    : Weapon("Charge Laser", 10, 40, 0.0f, 2.0f, 0),
      range(900.0f), showBeam(false), beamTimer(0.0f), beamDuration(0.12f),
      didHit(false), beamDamage(0),
      chargeLevel(0.0f), chargeRate(0.65f), isCharging(false),
      cooldownTimer(0.0f), cooldownDuration(2.0f)
{
    icon = LoadTexture("assets/sprites/laser.png");
    beamStart = {0, 0};
    beamEnd   = {0, 0};
    lastHitPoint = {0, 0};
    col = {100, 180, 255, 255};
    playerSkin = "default_laser";
}

LaserStage LaserWeapon::getStage() const
{
    if (chargeLevel >= 1.0f) return LaserStage::FULL;
    if (chargeLevel >= 0.66f) return LaserStage::STRONG;
    if (chargeLevel >= 0.33f) return LaserStage::PARTIAL;
    return LaserStage::NONE;
}

void LaserWeapon::startCharge(float deltaTime)
{
    if (cooldownTimer > 0.0f || currentAmmo <= 0 || showBeam) return;
    isCharging = true;
    chargeLevel += chargeRate * deltaTime;
    if (chargeLevel > 1.0f) chargeLevel = 1.0f;
}

void LaserWeapon::cancelCharge()
{
    chargeLevel = 0.0f;
    isCharging = false;
}

bool LaserWeapon::releaseBeam(Vector2 playerPos, Vector2 direction)
{
    if (chargeLevel < 0.33f || cooldownTimer > 0.0f || currentAmmo <= 0) {
        cancelCharge();
        return false;
    }

    LaserStage stage = getStage();
    switch (stage) {
        case LaserStage::PARTIAL: beamDamage = 25;  cooldownDuration = 0.8f; break;
        case LaserStage::STRONG:  beamDamage = 60;  cooldownDuration = 1.5f; break;
        case LaserStage::FULL:    beamDamage = 120; cooldownDuration = 2.5f; break;
        default: break;
    }

    // Normalise direction
    float len = sqrtf(direction.x * direction.x + direction.y * direction.y);
    if (len > 0) { direction.x /= len; direction.y /= len; }

    beamStart = playerPos;
    beamEnd   = { playerPos.x + direction.x * range, playerPos.y + direction.y * range };

    showBeam  = true;
    beamTimer = beamDuration;
    didHit    = false;

    decrementAmmo();
    lastFireTime = (float)GetTime();

    cooldownTimer = cooldownDuration;
    chargeLevel   = 0.0f;
    isCharging    = false;

    return true;
}

void LaserWeapon::fire(std::pair<int, int> position, std::pair<float, float> direction)
{
    // Legacy interface — unused; charge mechanic uses releaseBeam()
    (void)position; (void)direction;
}

void LaserWeapon::update(float deltaTime)
{
    if (showBeam) {
        beamTimer -= deltaTime;
        if (beamTimer <= 0.0f) {
            showBeam = false;
            didHit   = false;
        }
    }
    if (cooldownTimer > 0.0f) {
        cooldownTimer -= deltaTime;
        if (cooldownTimer < 0.0f) cooldownTimer = 0.0f;
    }
}
