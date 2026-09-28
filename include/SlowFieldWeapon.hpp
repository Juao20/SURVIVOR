#pragma once
#include "WeaponBase.hpp"
#include "Projectile.hpp"
#include <memory>
#include <cmath>


class SlowFieldWeapon : public Weapon {
private:
    float cooldownTimer;
    static constexpr float COOLDOWN   = 6.0f;
    static constexpr float ORB_RADIUS = 120.0f;
    static constexpr float ZONE_DUR   = 5.0f;
    static constexpr float SLOW_MULT  = 0.28f;

public:
    SlowFieldWeapon()
        : Weapon("Cryo Orb", 4, 12, 0.f, 0.2f, 0), cooldownTimer(0.f)
    {
        icon = LoadTexture("assets/sprites/machine.png");
        col = {80, 210, 255, 255};
        playerSkin = "default_rifle";
    }
    ~SlowFieldWeapon() override = default;

    void fire(std::pair<int,int>, std::pair<float,float>) override {}
    void update(float dt) override {
        if (cooldownTimer > 0.f) {
            cooldownTimer -= dt;
            if (cooldownTimer < 0.f) cooldownTimer = 0.f;
        }
    }
    std::string getWeaponType() const override { return "SlowField"; }

    bool  canThrow()        const { return cooldownTimer <= 0.f && currentAmmo > 0; }
    bool  isOnCooldown()    const { return cooldownTimer > 0.f; }
    float getCooldownPct()  const { return cooldownTimer / COOLDOWN; }
    float getOrbRadius()    const { return ORB_RADIUS; }
    float getZoneDuration() const { return ZONE_DUR; }
    float getSlowMult()     const { return SLOW_MULT; }

    std::shared_ptr<Projectile> createOrb(Vector2 playerPos, Vector2 targetPos) {
        if (!canThrow()) return nullptr;

        float dx = targetPos.x - playerPos.x;
        float dy = targetPos.y - playerPos.y;
        float dist = sqrtf(dx*dx + dy*dy);
        if (dist < 1.f) dist = 1.f;

        float power = std::min(dist * 0.70f, 620.f);
        if (power < 160.f) power = 160.f;

        Vector2 vel = { dx/dist * power, dy/dist * power - 240.f };

        auto orb = std::make_shared<Projectile>(playerPos, vel, 0.f, 2.4f, ProjectileType::GRENADE);
        orb->setIsExplosive(false);   // pas d'explosion classique
        orb->setExplosionRadius(ORB_RADIUS);
        orb->setFuse(1.8f);
        orb->setColor({80, 210, 255, 255});

        decrementAmmo();
        cooldownTimer = COOLDOWN;
        return orb;
    }
};
