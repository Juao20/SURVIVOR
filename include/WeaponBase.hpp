#pragma once

#include <iostream>
#include <string>
#include <utility>
#include "Ressource.hpp"

class Weapon
{
protected:
    std::string name;
    int currentAmmo;
    int maxAmmo;
    int magazineSize;
    float fireRate;
    float reloadTime;
    float lastFireTime;
    bool isReloading;
    float reloadStartTime;
    float reloadTimer;    // temps restant avant fin du reload (0.2s)
    std::string spritePath;
    Texture2D sprite;
    Texture2D icon;
    std::string playerSkin;
    int damage;
    std::pair<int, int> pos;
    Color col;

public:
    Weapon(std::string weaponName = "Default", int magSize = 10, int maxAmmoCount = 50,
           float rate = 0.5f, float reloadDuration = 0.2f, int dmg = 10);
    virtual ~Weapon();

    virtual void fire(std::pair<int, int> position, std::pair<float, float> direction) = 0;
    virtual void update(float deltaTime) = 0;
    virtual std::string getWeaponType() const = 0;

    void reload(std::unique_ptr<Ressource>& spriteManager);
    void updateReload(float deltaTime);
    bool canFire(float currentTime) const;
    void decrementAmmo();

    int getCurrentAmmo() const { return currentAmmo; }
    int getMaxAmmo() const { return maxAmmo; }
    int getMagazineSize() const { return magazineSize; }
    std::string getName() const { return name; }
    bool getIsReloading() const { return isReloading; }
    float getReloadTimer() const { return reloadTimer; }
    float getReloadProgress(float currentTime) const;
    std::string getSpritePath() const { return spritePath; }
    int getDamage() const { return damage; }
    std::pair<int, int> getPosition() const { return pos; }
    Color getColor() const { return col; }
    std::string getPlayerSkin() const { return playerSkin; }
    float getFireRate() const { return fireRate; }
    float getLastFireTime() const { return lastFireTime; }
    Texture2D getIcon() const { return icon; }
    void setLastFireTime(float t) { lastFireTime = t; }

    void setCurrentAmmo(int ammo) { currentAmmo = ammo; }
    void setSpritePath(const std::string& path) { spritePath = path; }
    void setSpriteTexture(const Texture2D sp) { sprite = sp; }
    void setPosition(std::pair<int, int> position) { pos = position; }
    void addAmmo(int amount);
};

Weapon::Weapon(std::string weaponName, int magSize, int maxAmmoCount,
               float rate, float reloadDuration, int dmg)
    : name(weaponName), currentAmmo(magSize), maxAmmo(maxAmmoCount),
      magazineSize(magSize), fireRate(rate), reloadTime(reloadDuration),
      lastFireTime(0.0f), isReloading(false), reloadStartTime(0.0f),
      damage(dmg), pos({0, 0}), playerSkin("default_rifle"), reloadTimer(reloadDuration), icon(LoadTexture("assets/sprites/rifle.png"))
{
}

Weapon::~Weapon() {}

void Weapon::reload(std::unique_ptr<Ressource>& spriteManager)
{
    // Démarre le reload (0.2s) si pas déjà en cours et qu'on a besoin de munitions
    if (!isReloading && currentAmmo < magazineSize && maxAmmo > 0)
    {
        PlaySound(spriteManager->getSound("reload"));
        isReloading = true;
        reloadTimer = reloadTime;
        reloadStartTime = (float)GetTime();
    }
}

void Weapon::updateReload(float deltaTime)
{
    if (!isReloading) return;
    reloadTimer -= deltaTime;
    if (reloadTimer <= 0.0f)
    {
        reloadTimer = 0.0f;
        isReloading = false;
        int ammoNeeded = magazineSize - currentAmmo;
        int ammoToReload = (maxAmmo >= ammoNeeded) ? ammoNeeded : maxAmmo;
        maxAmmo -= ammoToReload;
        currentAmmo += ammoToReload;
    }
}

bool Weapon::canFire(float currentTime) const
{
    if (isReloading) return false;
    if (currentAmmo <= 0) return false;
    if (currentTime - lastFireTime < fireRate) return false;
    return true;
}

void Weapon::decrementAmmo()
{
    if (currentAmmo > 0)
        currentAmmo--;
}

float Weapon::getReloadProgress(float currentTime) const
{
    if (!isReloading) return 0.0f;
    float elapsed = currentTime - reloadStartTime;
    return (elapsed / reloadTime) * 100.0f;
}

void Weapon::addAmmo(int amount)
{
    maxAmmo += amount;
    if (maxAmmo > 999) maxAmmo = 999;
}
