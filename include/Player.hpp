#pragma once

#include <iostream>
#include <vector>
#include <memory>
#include <map>
#include <cmath>
#include "WeaponBase.hpp"
#include "RifleWeapon.hpp"
#include "LaserWeapon.hpp"
#include "BombWeapon.hpp"
#include "Ressource.hpp"
#include <raylib.h>

class Projectile;


struct PoisonEffect {
    float damage;
    float duration;
    float tickRate;
    float tickTimer;
};

class Player
{
private:
    int hp;
    int maxHp;
    std::vector<std::shared_ptr<Weapon>> weapons;
    int currentWeaponIndex;
    std::pair<float, float> pos;
    Texture2D sprite;
    float speed;
    float rotation;
    Rectangle collisionBox;
    std::unique_ptr<Ressource>& spriteManager;

    std::vector<Rectangle>* obstacles;

    std::vector<PoisonEffect> poisonEffects;

    float invincTimer;
    static constexpr float INVINC_DURATION = 0.5f;

    bool showThrowPreview;

public:
    Player(std::unique_ptr<Ressource>& sM, std::pair<float, float> startPos = {640.0f, 360.0f});
    ~Player() = default;

    void update(float deltaTime, Vector2 worldMouse);
    void draw(Vector2 worldMouse) const;
    void handleInput(float deltaTime);
    void takeDamage(int damage);
    void heal(int amount);
    void applyPoison(float damage, float duration, float tickRate = 1.0f);

    // Weapons
    void addWeapon(std::shared_ptr<Weapon> weapon);
    void switchWeapon(int index);
    void nextWeapon();
    void previousWeapon();
    void reload();

    // Returns new projectile if fired (rifle), nullptr otherwise
    std::shared_ptr<Projectile> tryFire(Vector2 mousePos);
    // Laser charge
    void chargeLaser(float deltaTime);
    bool releaseLaser(Vector2 mousePos);
    // Bomb
    std::shared_ptr<Projectile> throwBomb(Vector2 mousePos);

    void move(float dx, float dy);
    void setObstacles(std::vector<Rectangle>* obs) { obstacles = obs; }

    // Getters
    int getHp() const { return hp; }
    int getMaxHp() const { return maxHp; }
    void setHp(int h) { hp = (h > maxHp) ? maxHp : (h < 0) ? 0 : h; }
    std::pair<float, float> getPosition() const { return pos; }
    Rectangle getCollisionBox() const { return collisionBox; }
    std::shared_ptr<Weapon> getCurrentWeapon() const;
    int getCurrentWeaponIndex() const { return currentWeaponIndex; }
    int getWeaponCount() const { return (int)weapons.size(); }
    float getRotation() const { return rotation; }
    bool isShowingThrowPreview() const { return showThrowPreview; }
    void setShowThrowPreview(bool v) { showThrowPreview = v; }
    std::shared_ptr<Weapon> getWeapon(int i) const { return (i < (int)weapons.size()) ? weapons[i] : nullptr; }

    void setSpriteTexture(const Texture2D& sp) { sprite = sp; }
    void setPosition(std::pair<float, float> position);

private:
    void resolveObstacleCollision();
    void drawThrowPreview(Vector2 mousePos) const;
};


Player::Player(std::unique_ptr<Ressource>& sM, std::pair<float, float> startPos)
    : hp(100), maxHp(100), currentWeaponIndex(0), pos(startPos),
      speed(200.0f), rotation(0.0f), spriteManager(sM),
      obstacles(nullptr), invincTimer(0.0f), showThrowPreview(false)
{
    collisionBox = {pos.first - 16, pos.second - 16, 32, 32};
}

void Player::addWeapon(std::shared_ptr<Weapon> weapon)
{
    weapons.push_back(weapon);
}

void Player::switchWeapon(int index)
{
    if (index >= 0 && index < (int)weapons.size()) {
        currentWeaponIndex = index;
        setSpriteTexture(spriteManager->getTexture(weapons[currentWeaponIndex]->getPlayerSkin()));
    }
}

void Player::nextWeapon()
{
    if (weapons.empty()) return;
    currentWeaponIndex = (currentWeaponIndex + 1) % weapons.size();
    setSpriteTexture(spriteManager->getTexture(weapons[currentWeaponIndex]->getPlayerSkin()));
}

void Player::previousWeapon()
{
    if (weapons.empty()) return;
    currentWeaponIndex = (currentWeaponIndex - 1 + weapons.size()) % weapons.size();
    setSpriteTexture(spriteManager->getTexture(weapons[currentWeaponIndex]->getPlayerSkin()));
}

void Player::reload()
{
    if (!weapons.empty() && currentWeaponIndex < (int)weapons.size())
        weapons[currentWeaponIndex]->reload(spriteManager);
}

std::shared_ptr<Weapon> Player::getCurrentWeapon() const
{
    if (!weapons.empty() && currentWeaponIndex < (int)weapons.size())
        return weapons[currentWeaponIndex];
    return nullptr;
}

void Player::setPosition(std::pair<float, float> position)
{
    pos = position;
    collisionBox.x = pos.first  - 16;
    collisionBox.y = pos.second - 16;
}

void Player::move(float dx, float dy)
{
    pos.first  += dx;
    pos.second += dy;
    collisionBox.x = pos.first  - 16;
    collisionBox.y = pos.second - 16;
    resolveObstacleCollision();
}

void Player::resolveObstacleCollision()
{
    if (!obstacles) return;
    for (const auto& obs : *obstacles) {
        if (CheckCollisionRecs(collisionBox, obs)) {
            // Push player out of obstacle (find smallest overlap axis)
            float overlapLeft  = (collisionBox.x + collisionBox.width)  - obs.x;
            float overlapRight = (obs.x + obs.width)  - collisionBox.x;
            float overlapTop   = (collisionBox.y + collisionBox.height) - obs.y;
            float overlapBot   = (obs.y + obs.height) - collisionBox.y;

            float minX = (overlapLeft < overlapRight) ? -overlapLeft : overlapRight;
            float minY = (overlapTop  < overlapBot)   ? -overlapTop  : overlapBot;

            if (fabsf(minX) < fabsf(minY))
                pos.first += minX;
            else
                pos.second += minY;

            collisionBox.x = pos.first  - 16;
            collisionBox.y = pos.second - 16;
        }
    }
}

void Player::takeDamage(int damage)
{
    if (invincTimer > 0) return;
    hp -= damage;
    if (hp < 0) hp = 0;
    invincTimer = INVINC_DURATION;
    PlaySound(spriteManager->getSound("hit_player"));
}

void Player::heal(int amount)
{
    hp += amount;
    if (hp > maxHp) hp = maxHp;
}

void Player::applyPoison(float damage, float duration, float tickRate)
{
    poisonEffects.push_back({damage, duration, tickRate, tickRate});
}

std::shared_ptr<Projectile> Player::tryFire(Vector2 mousePos)
{
    if (weapons.empty()) return nullptr;
    auto weapon = getCurrentWeapon();
    if (!weapon) return nullptr;

    auto rifle = std::dynamic_pointer_cast<RifleWeapon>(weapon);
    if (!rifle) return nullptr;

    float currentTime = (float)GetTime();
    if (!rifle->canFire(currentTime)) return nullptr;

    std::pair<float, float> dir = {
        mousePos.x - pos.first,
        mousePos.y - pos.second
    };

    // Normalise
    float len = sqrtf(dir.first * dir.first + dir.second * dir.second);
    if (len < 1.0f) return nullptr;
    dir.first /= len; dir.second /= len;

    // Decrement ammo and record fire time
    rifle->fire({(int)pos.first, (int)pos.second}, dir);

    Vector2 spawnPos = { pos.first, pos.second };
    Vector2 vel = { dir.first * rifle->getProjectileSpeed(), dir.second * rifle->getProjectileSpeed() };
    auto proj = rifle->createProjectile(spawnPos, vel);
    proj->setSpriteTexture(spriteManager->getTexture("bullet"));
    return proj;
}

void Player::chargeLaser(float deltaTime)
{
    if (weapons.empty()) return;
    auto laser = std::dynamic_pointer_cast<LaserWeapon>(getCurrentWeapon());
    if (laser) laser->startCharge(deltaTime);
}

bool Player::releaseLaser(Vector2 mousePos)
{
    if (weapons.empty()) return false;
    auto laser = std::dynamic_pointer_cast<LaserWeapon>(getCurrentWeapon());
    if (!laser) return false;

    Vector2 pPos = { pos.first, pos.second };
    Vector2 dir  = { mousePos.x - pos.first, mousePos.y - pos.second };
    float len = sqrtf(dir.x * dir.x + dir.y * dir.y);
    if (len < 1.0f) return false;
    dir.x /= len; dir.y /= len;

    return laser->releaseBeam(pPos, dir);
}

std::shared_ptr<Projectile> Player::throwBomb(Vector2 mousePos)
{
    if (weapons.empty()) return nullptr;
    auto bomb = std::dynamic_pointer_cast<BombWeapon>(getCurrentWeapon());
    if (!bomb) return nullptr;

    Vector2 pPos = { pos.first, pos.second };
    return bomb->createGrenade(pPos, mousePos);
}

void Player::update(float deltaTime, Vector2 worldMouse)
{
    // Update current weapon + reload timer
    if (!weapons.empty() && currentWeaponIndex < (int)weapons.size()) {
        auto& w = weapons[currentWeaponIndex];
        w->update(deltaTime);
        bool wasReloading = w->getIsReloading();
        w->updateReload(deltaTime);
        bool nowReloading = w->getIsReloading();

        // Pendant le reload : sprite "default_reload"
        if (nowReloading) {
            setSpriteTexture(spriteManager->getTexture("default_reload"));
        } else if (wasReloading && !nowReloading) {
            // Reload terminé — revient au skin de l'arme
            setSpriteTexture(spriteManager->getTexture(w->getPlayerSkin()));
        }
    }

    if (invincTimer > 0) invincTimer -= deltaTime;

    // Poison DoT
    for (auto it = poisonEffects.begin(); it != poisonEffects.end();) {
        it->tickTimer -= deltaTime;
        it->duration  -= deltaTime;
        if (it->tickTimer <= 0.0f) {
            hp -= (int)it->damage;
            if (hp < 0) hp = 0;
            it->tickTimer = it->tickRate;
        }
        if (it->duration <= 0.0f)
            it = poisonEffects.erase(it);
        else
            ++it;
    }

    // Rotation toward world mouse (virtual coords)
    rotation = atan2f(worldMouse.y - pos.second, worldMouse.x - pos.first) * RAD2DEG;
}

void Player::drawThrowPreview(Vector2 mousePos) const
{
    float dx = mousePos.x - pos.first;
    float dy = mousePos.y - pos.second;
    float dist = sqrtf(dx * dx + dy * dy);
    if (dist < 1.0f) return;
    float nx = dx / dist;
    float ny = dy / dist;
    float power = std::min(dist * 0.85f, 700.0f);
    if (power < 200.0f) power = 200.0f;

    Vector2 vel = { nx * power, ny * power - 280.0f };
    Vector2 simPos = { pos.first, pos.second };
    float gravity = 420.0f;
    float dt = 0.05f;
    for (int i = 0; i < 24; i++) {
        vel.y += gravity * dt;
        simPos.x += vel.x * dt;
        simPos.y += vel.y * dt;
        if (i % 2 == 0)
            DrawCircleV(simPos, 3.0f, Fade({255, 180, 0, 255}, 0.6f - i * 0.025f));
    }
}

void Player::draw(Vector2 worldMouse) const
{
    Vector2 center = { pos.first, pos.second };

    bool isPoisoned = !poisonEffects.empty();

    bool flashFrame = (invincTimer > 0) && ((int)(invincTimer * 10) % 2 == 0);
    if (flashFrame) return;

    auto laser = std::dynamic_pointer_cast<LaserWeapon>(
        (!weapons.empty() && currentWeaponIndex < (int)weapons.size()) ? weapons[currentWeaponIndex] : nullptr);
    if (laser && laser->getIsCharging()) {
        float charge = laser->getChargeLevel();
        float ringR = 18.0f + charge * 22.0f;
        Color ringCol = (charge >= 1.0f) ? WHITE :
                        (charge >= 0.66f) ? ORANGE : (Color){255, 80, 80, 255};
        DrawCircleLines((int)center.x, (int)center.y, ringR, Fade(ringCol, 0.7f));
        DrawCircleLines((int)center.x, (int)center.y, ringR + 3, Fade(ringCol, 0.25f));
    }

    if (sprite.id != 0) {
        Vector2 origin = { sprite.width / 2.0f, sprite.height / 2.0f };
        Color tint = isPoisoned ? (Color){150, 255, 150, 255} : WHITE;
        DrawTexturePro(sprite,
            {0, 0, (float)sprite.width, (float)sprite.height},
            {pos.first, pos.second, (float)sprite.width, (float)sprite.height},
            origin, rotation, tint);
    } else {
        float rad = rotation * DEG2RAD;
        Vector2 tip   = { center.x + cosf(rad) * 20.0f, center.y + sinf(rad) * 20.0f };
        Vector2 left  = { center.x + cosf(rad + 2.4f) * 14.0f, center.y + sinf(rad + 2.4f) * 14.0f };
        Vector2 right = { center.x + cosf(rad - 2.4f) * 14.0f, center.y + sinf(rad - 2.4f) * 14.0f };
        Color fillCol = isPoisoned ? (Color){150, 255, 150, 255} : (Color){60, 120, 200, 255};
        DrawTriangle(tip, left, right, fillCol);
        DrawTriangleLines(tip, left, right, WHITE);
    }

    // Aim line (faint) — drawn to world mouse position (virtual coords)
    // if (!weapons.empty() && currentWeaponIndex < (int)weapons.size()) {
    //     Color aimCol = Fade(weapons[currentWeaponIndex]->getColor(), 0.25f);
    //     DrawLineEx(center, worldMouse, 1.0f, aimCol);
    // }

    // Laser beam render
    if (laser && laser->isBeamVisible()) {
        Vector2 bs = laser->getBeamStart();
        Vector2 be = laser->getBeamEnd();
        LaserStage stage = laser->getStage();

        Color beamCol = (stage == LaserStage::FULL)   ? WHITE :
                        (stage == LaserStage::STRONG)  ? ORANGE : RED;
        float beamW   = (stage == LaserStage::FULL)   ? 12.0f :
                        (stage == LaserStage::STRONG)  ? 7.0f  : 4.0f;

        DrawLineEx(bs, be, beamW + 6.0f, Fade(beamCol, 0.2f));
        DrawLineEx(bs, be, beamW, beamCol);
        DrawLineEx(bs, be, 1.5f, WHITE);

        if (laser->didLastShotHit()) {
            Vector2 hp2 = laser->getLastHitPoint();
            DrawCircleV(hp2, 10.0f, Fade(ORANGE, 0.6f));
            DrawCircleV(hp2, 5.0f, YELLOW);
        }
    }

    // Bomb throw preview
    auto bomb = std::dynamic_pointer_cast<BombWeapon>(
        (!weapons.empty() && currentWeaponIndex < (int)weapons.size()) ? weapons[currentWeaponIndex] : nullptr);
    if (bomb && showThrowPreview)
        drawThrowPreview(worldMouse);
}

void Player::handleInput(float deltaTime)
{
    float dx = 0.0f, dy = 0.0f;
    if (IsKeyDown(KEY_LEFT_SHIFT)) speed = 350.0;
    if (IsKeyDown(KEY_W) || IsKeyDown(KEY_Z)) dy -= speed * deltaTime;
    if (IsKeyDown(KEY_S))                      dy += speed * deltaTime;
    if (IsKeyDown(KEY_A) || IsKeyDown(KEY_Q))  dx -= speed * deltaTime;
    if (IsKeyDown(KEY_D))                      dx += speed * deltaTime;
    if (!IsKeyDown(KEY_W) && !IsKeyDown(KEY_S) && !IsKeyDown(KEY_A) && !IsKeyDown(KEY_D) &&
        !IsKeyDown(KEY_Z) && !IsKeyDown(KEY_Q))
        StopSound(spriteManager->getSound("walking"));
    else
        PlaySound(spriteManager->getSound("walking"));

    move(dx, dy);
    speed = 200.0;

    if (IsKeyPressed(KEY_R)) reload();
    float wheel = GetMouseWheelMove();
    if (wheel > 0) nextWeapon();
    if (wheel < 0) previousWeapon();

    for (int i = 0; i < (int)weapons.size() && i < 9; i++) {
        if (IsKeyPressed(KEY_ONE + i)) switchWeapon(i);
    }

    auto bomb = std::dynamic_pointer_cast<BombWeapon>(getCurrentWeapon());
    showThrowPreview = (bomb != nullptr) && IsMouseButtonDown(MOUSE_RIGHT_BUTTON);
}