#pragma once

#include "WeaponBase.hpp"
#include "Projectile.hpp"
#include <memory>
#include <cmath>

class BombWeapon : public Weapon
{
private:
    float throwCooldown;
    float cooldownTimer;
    float explosionRadius;
    float fuseTime;

public:
    BombWeapon();
    ~BombWeapon() override = default;

    void fire(std::pair<int, int> position, std::pair<float, float> direction) override;
    void update(float deltaTime) override;
    std::string getWeaponType() const override { return "Bomb"; }

    float getExplosionRadius() const { return explosionRadius; }
    bool isOnCooldown() const { return cooldownTimer > 0.0f; }
    float getCooldownPct() const { return cooldownTimer / throwCooldown; }
    float getFuseTime() const { return fuseTime; }

    std::shared_ptr<Projectile> createGrenade(Vector2 playerPos, Vector2 targetPos);

    bool canThrow() const { return cooldownTimer <= 0.0f && currentAmmo > 0; }
};

BombWeapon::BombWeapon()
    : Weapon("Frag Bomb", 4, 12, 0.0f, 0.5f, 0),
      throwCooldown(1.5f), cooldownTimer(0.0f),
      explosionRadius(200.0f), fuseTime(1.4f)
{
    icon = LoadTexture("assets/sprites/machine.png");
    col = {255, 180, 0, 255};
    playerSkin = "default_bomb";
}

void BombWeapon::fire(std::pair<int, int> position, std::pair<float, float> direction)
{
    (void)position; (void)direction;
}

void BombWeapon::update(float deltaTime)
{
    if (cooldownTimer > 0.0f) {
        cooldownTimer -= deltaTime;
        if (cooldownTimer < 0.0f) cooldownTimer = 0.0f;
    }
}

std::shared_ptr<Projectile> BombWeapon::createGrenade(Vector2 playerPos, Vector2 targetPos)
{
    if (!canThrow()) return nullptr;

    float dx = targetPos.x - playerPos.x;
    float dy = targetPos.y - playerPos.y;
    float dist = sqrtf(dx * dx + dy * dy);
    if (dist < 1.0f) dist = 1.0f;
    float nx = dx / dist;
    float ny = dy / dist;

    // Trajectoir de l'arc
    float throwPower = dist * 0.85f;
    if (throwPower > 700.0f) throwPower = 700.0f;
    if (throwPower < 200.0f) throwPower = 200.0f;

    Vector2 vel = {
        nx * throwPower,
        ny * throwPower - 280.0f
    };

    auto grenade = std::make_shared<Projectile>(playerPos, vel, 80.0f, fuseTime + 0.5f, ProjectileType::GRENADE);
    grenade->setIsExplosive(true);
    grenade->setExplosionRadius(explosionRadius);
    grenade->setFuse(fuseTime);
    grenade->setColor(col);

    decrementAmmo();
    cooldownTimer = throwCooldown;
    lastFireTime  = (float)GetTime();

    return grenade;
}
