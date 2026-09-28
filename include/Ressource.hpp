#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <raylib.h>

class Ressource
{
private:
    std::map<std::string, std::string> playerSprites;
    std::map<std::string, std::string> weaponSprites;
    std::map<std::string, std::string> enemySprites;
    std::map<std::string, std::string> projectileSprites;
    std::map<std::string, std::string> uiSprites;
    std::map<std::string, std::string> menuSounds;
    std::map<std::string, std::string> menuMusics;
    std::map<std::string, std::string> inGameSounds;
    std::map<std::string, std::string> inGameMusics;

    std::map<std::string, Texture2D> loadedTextures;
    std::map<std::string, Sound>     loadedSounds;
    std::map<std::string, Music>     loadedMusics;

public:
    Ressource();
    ~Ressource();

    void addPlayerSprite(const std::string& name, const std::string& path);
    void addWeaponSprite(const std::string& name, const std::string& path);
    void addEnemySprite(const std::string& name, const std::string& path);
    void addProjectileSprite(const std::string& name, const std::string& path);
    void addUISprite(const std::string& name, const std::string& path);
    void addMenuSound(const std::string& name, const std::string& path);
    void addMenuMusic(const std::string& name, const std::string& path);
    void addInGameSound(const std::string& name, const std::string& path);
    void addInGameMusic(const std::string& name, const std::string& path);

    Texture2D loadTexture(const std::string& path);
    Texture2D getTexture(const std::string& spriteName);
    Sound loadSound(const std::string& path);
    Sound getSound(const std::string& soundName);
    Music loadMusic(const std::string& path);
    Music getMusic(const std::string& musicName);

    std::string getPlayerSpritePath(const std::string& name) const;
    std::string getWeaponSpritePath(const std::string& name) const;
    std::string getEnemySpritePath(const std::string& name) const;
    std::string getProjectileSpritePath(const std::string& name) const;
    std::string getUISpritePath(const std::string& name) const;

    void unloadAllTextures();
    void unloadTexture(const std::string& spriteName);
    void unloadAllSounds();
    void unloadSound(const std::string& soundName);
    void unloadAllMusics();
    void unloadMusic(const std::string& musicName);
};

Ressource::Ressource()
{
    // Sprites
    addPlayerSprite("default_rifle",  "assets/sprites/Soldier/gun.png");
    addPlayerSprite("default_laser",  "assets/sprites/Soldier/silencer.png");
    addPlayerSprite("default_bomb",  "assets/sprites/Soldier/machine.png");
    addPlayerSprite("default_reload",  "assets/sprites/Soldier/reload.png");
    addWeaponSprite("rifle",          "assets/sprites/pistol.png");
    addWeaponSprite("laser",          "assets/sprites/laser.png");
    addEnemySprite ("basic",          "assets/sprites/Soldier/gun.png");
    addProjectileSprite("bullet",     "assets/sprites/bullet.png");
    // Musics
    addMenuMusic("menu_theme", "assets/musics/su_MainTheme.mp3");
    addInGameMusic("boss_theme", "assets/musics/su_BossTheme.mp3");
    addInGameMusic("game_over", "assets/musics/su_GameOver.mp3");
    addInGameMusic("victory", "assets/musics/su_Victory.mp3");
    // Sounds
    addMenuSound("navigate", "assets/sounds/su_Navigate.mp3");
    addMenuSound("select",   "assets/sounds/su_Select.mp3");
    addInGameSound("shoot_ar",   "assets/sounds/su_Ar.mp3");
    addInGameSound("shoot_laser",   "assets/sounds/su_Laser.mp3");
    addInGameSound("shoot_Frag",   "assets/sounds/su_FragGrenade.mp3");
    addInGameSound("Frag",   "assets/sounds/su_ExGrenade.mp3");
    addInGameSound("reload",  "assets/sounds/su_Reload.mp3");
    addInGameSound("hit_enemy",   "assets/sounds/su_HitEnemy.mp3");
    addInGameSound("hit_enemy",   "assets/sounds/su_HitEnemy.mp3");
    addInGameSound("hit_player",   "assets/sounds/su_HitPlayer.mp3");
    addInGameSound("poison_throw",   "assets/sounds/su_PoisonThrow.mp3");
    addInGameSound("walking",   "assets/sounds/su_Walking.mp3");
}

Ressource::~Ressource()
{
    // std::cout << "dede1" << std::endl;
    // unloadAllTextures();
    // std::cout << "dede2" << std::endl;
    // unloadAllSounds();
    // std::cout << "dede3" << std::endl;
    // unloadAllMusics();
    // std::cout << "dede4" << std::endl;
}

void Ressource::addPlayerSprite(const std::string& name, const std::string& path) { playerSprites[name] = path; }
void Ressource::addWeaponSprite(const std::string& name, const std::string& path) { weaponSprites[name] = path; }
void Ressource::addEnemySprite(const std::string& name, const std::string& path)  { enemySprites[name] = path; }
void Ressource::addProjectileSprite(const std::string& name, const std::string& path) { projectileSprites[name] = path; }
void Ressource::addUISprite(const std::string& name, const std::string& path)     { uiSprites[name] = path; }
void Ressource::addMenuSound(const std::string& name, const std::string& path)    { menuSounds[name] = path; }
void Ressource::addMenuMusic(const std::string& name, const std::string& path)    { menuMusics[name] = path; }
void Ressource::addInGameSound(const std::string& name, const std::string& path)  { inGameSounds[name] = path; }
void Ressource::addInGameMusic(const std::string& name, const std::string& path)  { inGameMusics[name] = path; }

Texture2D Ressource::loadTexture(const std::string& path)
{
    if (loadedTextures.find(path) != loadedTextures.end())
        return loadedTextures[path];
    Texture2D texture = LoadTexture(path.c_str());
    if (texture.id != 0)
        loadedTextures[path] = texture;
    return texture;
}

Texture2D Ressource::getTexture(const std::string& spriteName)
{
    std::string path;
    if (playerSprites.count(spriteName))     path = playerSprites.at(spriteName);
    else if (weaponSprites.count(spriteName))     path = weaponSprites.at(spriteName);
    else if (enemySprites.count(spriteName))      path = enemySprites.at(spriteName);
    else if (projectileSprites.count(spriteName)) path = projectileSprites.at(spriteName);
    else if (uiSprites.count(spriteName))         path = uiSprites.at(spriteName);
    if (!path.empty()) return loadTexture(path);
    return Texture2D{0, 0, 0, 0, 0};
}

Sound Ressource::loadSound(const std::string& path)
{
    if (loadedSounds.find(path) != loadedSounds.end())
        return loadedSounds[path];
    Sound sound = LoadSound(path.c_str());
    loadedSounds[path] = sound;
    return sound;
}

Sound Ressource::getSound(const std::string& soundName)
{
    std::string path;
    if (menuSounds.count(soundName))   path = menuSounds.at(soundName);
    else if (inGameSounds.count(soundName)) path = inGameSounds.at(soundName);
    if (!path.empty()) return loadSound(path);
    std::cout << "Sound not found: " << soundName << std::endl;
    return Sound{0, 0, 0, 0, 0};
}

Music Ressource::loadMusic(const std::string& path)
{
    if (loadedMusics.find(path) != loadedMusics.end())
        return loadedMusics[path];
    Music music = LoadMusicStream(path.c_str());
    loadedMusics[path] = music;
    return music;
}

Music Ressource::getMusic(const std::string& musicName)
{
    std::string path;
    if (menuMusics.count(musicName))   path = menuMusics.at(musicName);
    else if (inGameMusics.count(musicName)) path = inGameMusics.at(musicName);
    if (!path.empty()) return loadMusic(path);
    Music m = {0};
    return m;
}

std::string Ressource::getPlayerSpritePath(const std::string& name) const {
    auto it = playerSprites.find(name); return (it != playerSprites.end()) ? it->second : "";
}
std::string Ressource::getWeaponSpritePath(const std::string& name) const {
    auto it = weaponSprites.find(name); return (it != weaponSprites.end()) ? it->second : "";
}
std::string Ressource::getEnemySpritePath(const std::string& name) const {
    auto it = enemySprites.find(name); return (it != enemySprites.end()) ? it->second : "";
}
std::string Ressource::getProjectileSpritePath(const std::string& name) const {
    auto it = projectileSprites.find(name); return (it != projectileSprites.end()) ? it->second : "";
}
std::string Ressource::getUISpritePath(const std::string& name) const {
    auto it = uiSprites.find(name); return (it != uiSprites.end()) ? it->second : "";
}

void Ressource::unloadAllTextures() {
    for (auto& p : loadedTextures) UnloadTexture(p.second);
    loadedTextures.clear();
}
void Ressource::unloadTexture(const std::string& n) {
    auto it = loadedTextures.find(n);
    if (it != loadedTextures.end()) { UnloadTexture(it->second); loadedTextures.erase(it); }
}
void Ressource::unloadAllSounds() {
    for (auto& p : loadedSounds) UnloadSound(p.second);
    loadedSounds.clear();
}
void Ressource::unloadSound(const std::string& n) {
    auto it = loadedSounds.find(n);
    if (it != loadedSounds.end()) { UnloadSound(it->second); loadedSounds.erase(it); }
}
void Ressource::unloadAllMusics() {
    for (auto& p : loadedMusics) UnloadMusicStream(p.second);
    loadedMusics.clear();
}
void Ressource::unloadMusic(const std::string& n) {
    auto it = loadedMusics.find(n);
    if (it != loadedMusics.end()) { UnloadMusicStream(it->second); loadedMusics.erase(it); }
}
