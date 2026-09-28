#pragma once

#include "WeaponBase.hpp"
#include "Projectile.hpp"
#include <vector>
#include <memory>
#include <cmath>

class RifleWeapon : public Weapon
{
private:
    float projectileSpeed;
    float projectileLifetime;
    float spread;

public:
    RifleWeapon();
    ~RifleWeapon() override = default;

    void fire(std::pair<int, int> position, std::pair<float, float> direction) override;
    void update(float deltaTime) override;
    std::string getWeaponType() const override { return "Rifle"; }

    float getProjectileSpeed() const { return projectileSpeed; }
    float getProjectileLifetime() const { return projectileLifetime; }

    std::shared_ptr<Projectile> createProjectile(Vector2 pos, Vector2 velocity);
};

RifleWeapon::RifleWeapon()
    : Weapon("Assault Rifle", 30, 120, 0.09f, 1.8f, 18),
      projectileSpeed(520.0f), projectileLifetime(2.5f), spread(3.0f)
{
    icon = LoadTexture("assets/sprites/rifle.png");
    col = YELLOW;
    playerSkin = "default_rifle";
}

void RifleWeapon::fire(std::pair<int, int> position, std::pair<float, float> direction)
{
    (void)position;
    (void)direction;
    
    decrementAmmo();
    lastFireTime = (float)GetTime();
}

void RifleWeapon::update(float deltaTime)
{
    (void)deltaTime;
}

std::shared_ptr<Projectile> RifleWeapon::createProjectile(Vector2 pos, Vector2 velocity)
{
    // Apply spread
    float angle = atan2f(velocity.y, velocity.x);
    float spreadRad = spread * DEG2RAD;
    float offset = ((float)rand() / RAND_MAX - 0.5f) * 2.0f * spreadRad;
    angle += offset;

    float speed = sqrtf(velocity.x * velocity.x + velocity.y * velocity.y);
    velocity.x = cosf(angle) * speed;
    velocity.y = sinf(angle) * speed;

    auto projectile = std::make_shared<Projectile>(pos, velocity, damage, projectileLifetime, ProjectileType::BULLET);
    projectile->setColor(col);
    return projectile;
}
