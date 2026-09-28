#pragma once

#include <raylib.h>
#include <string>

enum class ProjectileType {
    BULLET,
    ROCKET,
    LASER_BOLT,
    PLASMA,
    GRENADE,
    POISON_SPIT
};

class Projectile {
private:
    Vector2 position;
    Vector2 velocity;
    float rotation;
    float damage;
    float lifetime;
    float maxLifetime;
    bool active;
    ProjectileType type;
    Rectangle hitbox;
    Texture2D sprite;
    std::string spritePath;
    Color trailColor;

    bool isExplosive;
    float explosionRadius;
    bool isPiercing;

    float fuseTimer;
    bool  fuseActive;

    float dotDamage;
    float dotDuration;
    bool Player_Own = true;

public:
    Projectile(Vector2 pos, Vector2 vel, float dmg, float life, ProjectileType projType, bool pown = true);
    ~Projectile() = default;

    void update(float deltaTime);
    void draw();
    void deactivate();

    // Getters
    Vector2 getPosition() const { return position; }
    Vector2 getVelocity() const { return velocity; }
    float getDamage() const { return damage; }
    bool isActive() const { return active; }
    Rectangle getHitbox() const { return hitbox; }
    ProjectileType getType() const { return type; }
    bool getIsExplosive() const { return isExplosive; }
    float getExplosionRadius() const { return explosionRadius; }
    bool getIsPiercing() const { return isPiercing; }
    bool isFuseActive() const { return fuseActive; }
    float getFuseTimer() const { return fuseTimer; }
    float getDotDamage() const { return dotDamage; }
    float getDotDuration() const { return dotDuration; }
    bool getOwn() const {return Player_Own;}

    // Setters
    void setPosition(Vector2 pos);
    void setVelocity(Vector2 vel) { velocity = vel; }
    void setActive(bool a) { active = a; }
    void setIsExplosive(bool explosive) { isExplosive = explosive; }
    void setExplosionRadius(float radius) { explosionRadius = radius; }
    void setIsPiercing(bool piercing) { isPiercing = piercing; }
    void setFuse(float fuseTime) { fuseTimer = fuseTime; fuseActive = true; }
    void setDot(float dmg, float dur) { dotDamage = dmg; dotDuration = dur; }
    void loadSprite(const std::string& path);
    void setTrailColor(Color color) { trailColor = color; }
    void setColor(Color color) { trailColor = color; }
    void setSpriteTexture(const Texture2D sp) { sprite = sp; }

private:
    void updateHitbox();
};

Projectile::Projectile(Vector2 pos, Vector2 vel, float dmg, float life, ProjectileType projType, bool pown)
    : position(pos), velocity(vel), rotation(0), damage(dmg),
      lifetime(life), maxLifetime(life), active(true),
      type(projType), trailColor(YELLOW),
      isExplosive(false), explosionRadius(0), isPiercing(false),
      fuseTimer(0.0f), fuseActive(false), dotDamage(0.0f), dotDuration(0.0f), Player_Own(pown)
{
    float radius = 4.0f;
    hitbox = {pos.x - radius, pos.y - radius, radius * 2, radius * 2};
    sprite = {0, 0, 0, 0, 0};

    if (Player_Own == false)
        trailColor = BLUE;
    if (projType == ProjectileType::GRENADE) {
        trailColor = {255, 200, 0, 255};
    } else if (projType == ProjectileType::POISON_SPIT) {
        trailColor = {0, 220, 60, 255};
    }
}

void Projectile::update(float deltaTime)
{
    if (!active) return;

    // Arc gravity for grenades and poison spit
    if (type == ProjectileType::GRENADE) {
        velocity.y += 420.0f * deltaTime;
        // Grenade with fuse: count down fuse
        if (fuseActive) {
            fuseTimer -= deltaTime;
            if (fuseTimer <= 0.0f) {
                active = false; // triggers explosion in game
                return;
            }
        }
    } else if (type == ProjectileType::POISON_SPIT) {
        velocity.y += 160.0f * deltaTime;
    }

    position.x += velocity.x * deltaTime;
    position.y += velocity.y * deltaTime;

    // Rotate along velocity direction
    if (velocity.x != 0 || velocity.y != 0)
        rotation = atan2f(velocity.y, velocity.x) * RAD2DEG;

    lifetime -= deltaTime;
    if (lifetime <= 0)
        active = false;

    updateHitbox();
}

void Projectile::draw()
{
    if (!active) return;

    float radius = (type == ProjectileType::GRENADE) ? 8.0f :
                   (type == ProjectileType::POISON_SPIT) ? 6.0f : 4.0f;

    if (sprite.id != 0) {
        DrawTextureEx(sprite, position, rotation, 1.0f, WHITE);
    } else {
        DrawCircleV(position, radius, trailColor);
        DrawCircleV(position, radius + 2, Fade(trailColor, 0.3f));
    }

    if (type == ProjectileType::GRENADE && fuseActive && fuseTimer > 0) {
        float pct = fuseTimer / 1.4f;
        DrawCircleLines((int)position.x, (int)position.y, 14.0f * pct + 4, {255, 100, 0, 200});
    }
}

void Projectile::deactivate()
{
    active = false;
}

void Projectile::setPosition(Vector2 pos)
{
    position = pos;
    updateHitbox();
}

void Projectile::updateHitbox()
{
    float radius = 4.0f;
    hitbox = {position.x - radius, position.y - radius, radius * 2, radius * 2};
}

void Projectile::loadSprite(const std::string& path)
{
    spritePath = path;
    sprite = LoadTexture(path.c_str());
}
