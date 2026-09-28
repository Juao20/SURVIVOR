#pragma once

#include <iostream>
#include <memory>
#include <vector>
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <raylib.h>

#include "ScaleContext.hpp"
#include "AmmoPickup.hpp"
#include "HealthPickup.hpp"
#include "Player.hpp"
#include "Enemy.hpp"
#include "Projectile.hpp"
#include "LaserWeapon.hpp"
#include "BombWeapon.hpp"
#include "SlowFieldWeapon.hpp"
#include "CollisionManager.hpp"
#include "SettingsMenu.hpp"
#include "MainMenu.hpp"
#include "HUD.hpp"
#include "Ressource.hpp"
#include "TouchControls.hpp"

// ETATS
enum class GameState { MAIN_MENU, CHARA_SELECT, SETTINGS, PLAYING, PAUSED, GAME_OVER };
enum class WavePhase { COUNTDOWN, ACTIVE, REST, BOSS_REST };

struct WaveManager {
    int       currentWave      = 0;
    WavePhase phase            = WavePhase::COUNTDOWN;
    float     phaseTimer       = 3.0f;
    int       enemiesRemaining = 0;
    float     spawnTimer       = 0.0f;
    int       enemiesToSpawn   = 0;
    bool      bossAlive        = false;
    float     bossAlertTimer   = 0.0f;
    bool      bossAlert        = false;

    static constexpr float COUNTDOWN_DUR = 3.5f;
    static constexpr float REST_DUR      = 5.0f;
    static constexpr float BOSS_REST_DUR = 9.0f;

    // megaCycle : 0=vagues1-4, 1=vagues5-8, 2=vagues9-12, 3+=mélange
    int megaCycle() const { return (currentWave - 1) / 4; }

    int waveInCycle() const {
        int r = currentWave % 4;
        return (r == 0) ? 4 : r;
    }

    bool isBossWave() const { return currentWave % 4 == 0 && currentWave > 0; }

    int enemyCount() const {
        if (isBossWave()) return 1;
        return 5 + (int)(2.2f * currentWave);
    }

    float spawnInterval() const {
        return 0.55f / (1.f + 0.04f * currentWave);
    }

    EnemyType toElite(EnemyType t) const {
        switch (t) {
            case EnemyType::ZOMBIE:  return EnemyType::ZOMBIE_E;
            case EnemyType::SHOOTER: return EnemyType::SHOOTER_E;
            case EnemyType::BRUTE:   return EnemyType::BRUTE_E;
            default: return t;
        }
    }

    EnemyType pickBossType() const {
        int mc = megaCycle();
        if (mc == 0) return EnemyType::ZOMBIE_BOSS;
        if (mc == 1) return EnemyType::ASSASSIN_BOSS;
        if (mc == 2) return EnemyType::BRUTE_BOSS;
        static const EnemyType bosses[] = {
            EnemyType::ZOMBIE_BOSS, EnemyType::ASSASSIN_BOSS, EnemyType::BRUTE_BOSS
        };
        return bosses[(currentWave / 4 - 1) % 3];
    }

    EnemyType pickEnemyType() const {
        int mc = megaCycle();
        int wc = waveInCycle();

        static const EnemyType mainTypes[] = {
            EnemyType::ZOMBIE, EnemyType::SHOOTER, EnemyType::BRUTE
        };

        if (mc < 3) {
            EnemyType main = mainTypes[mc];
            if (wc == 1) return main;               // pur normal
            if (wc == 3) return toElite(main);      // pur élite
            // wc == 2 : mixte 50/50
            return (rand()%2==0) ? main : toElite(main);
        }

        if (wc == 1) return mainTypes[rand()%3];             // normaux mixtes
        if (wc == 3) return toElite(mainTypes[rand()%3]);    // élites mixtes
        static const EnemyType all[] = {
            EnemyType::ZOMBIE, EnemyType::ZOMBIE_E,
            EnemyType::SHOOTER, EnemyType::SHOOTER_E,
            EnemyType::BRUTE, EnemyType::BRUTE_E
        };
        return all[rand()%6];
    }
};

struct ExplosionEffect {
    Vector2 pos; float radius, maxRadius, timer, duration;
};

struct SlowZone {
    Vector2 center;
    float   radius;
    float   slowMult;
    float   timer;
    float   duration;

    void update(float dt) { timer -= dt; }
    bool expired() const  { return timer <= 0.f; }

    bool contains(std::pair<float,float> p) const {
        float dx=p.first-center.x, dy=p.second-center.y;
        return sqrtf(dx*dx+dy*dy) <= radius;
    }

    void draw() const {
        float alpha = std::min(1.f, timer/duration) * 0.35f;
        DrawCircleV(center, radius, Fade({80,200,255,255}, alpha));
        DrawCircleLines((int)center.x,(int)center.y, radius, Fade({150,230,255,255}, alpha*1.8f));
        float pulse=0.7f+0.3f*sinf(timer*6.f);
        DrawCircleLines((int)center.x,(int)center.y, radius*0.55f*pulse,
                         Fade({200,240,255,255}, alpha));
    }
};

struct SimpleButton {
    Rectangle rect; const char* label; bool isHovered=false;
    void update()  { isHovered=CheckCollisionPointRec(GetMousePosition(),rect); }
    bool clicked() { return isHovered&&IsMouseButtonPressed(MOUSE_LEFT_BUTTON); }
    void draw() const {
        Color bg=isHovered?(Color){70,100,200,230}:(Color){30,40,70,210};
        DrawRectangleRec(rect,bg);
        DrawRectangleLinesEx(rect,1.5f,isHovered?WHITE:(Color){80,100,160,255});
        int tw=MeasureText(label,22);
        DrawText(label,(int)(rect.x+(rect.width-tw)/2),(int)(rect.y+(rect.height-22)/2),22,WHITE);
    }
};

struct AmmoDrop {
    static bool shouldDrop(int wave) {
        float c=0.30f+0.05f*wave; if(c>0.65f)c=0.65f;
        return (rand()%100)<(int)(c*100);
    }
    static AmmoType pickType() {
        int r=rand()%100;
        if(r<50) return AmmoType::RIFLE;
        if(r<78) return AmmoType::LASER;
        if(r<92) return AmmoType::BOMB;
        return AmmoType::RIFLE;
    }
    static int amountForType(AmmoType t, int wave) {
        int b=wave/3;
        switch(t){
            case AmmoType::RIFLE: return 15+b*5;
            case AmmoType::LASER: return 5+b*2;
            case AmmoType::BOMB:  return 1+(wave>=6?1:0);
        }
        return 10;
    }
};

class Game {
private:
    int         baseWidth, baseHeight;
    const char* windowTitle;
    GameState   currentState;
    bool        isRunning;
    ScaleContext scale;

    std::unique_ptr<Player>                  player;
    std::vector<std::shared_ptr<Enemy>>      enemies;
    std::vector<std::shared_ptr<Projectile>> projectiles;
    std::vector<AmmoPickup>                  ammoPickups;
    std::vector<HealthPickup>                healthPickups;
    std::vector<SlowZone>                    slowZones;

    std::unique_ptr<SettingsMenu>     settingsMenu;
    std::unique_ptr<MainMenu>         mainMenu;
    std::unique_ptr<HUD>              hud;
    std::unique_ptr<Ressource>        spriteManager;
    std::unique_ptr<CollisionManager> collisionManager;

    std::vector<Rectangle> obstacles;
    WaveManager waveManager;

    std::vector<ExplosionEffect> explosions;

    SimpleButton btnResume, btnQuitMenu;
    TouchControls touchControls;

    float masterVolume;
    bool  isFullscreen;
    float deltaTime, gameTime;

    // Flags pour arme offerte après cycle 1
    bool slowWeaponGiven;
    
    // Musiques d'état
    std::string currentGameMusic;
    bool bossThemePlaying;

public:
    Game(int w=1280, int h=720, const char* t="Survivor");
    ~Game();
    void init();
    void run();
    void cleanup();
    GameState getState() const { return currentState; }
    Player*   getPlayer() const { return player.get(); }

private:
    void buildArena();
    void setupMainMenu();
    void initNewGame();

    void update();
    void draw();
    void setState(GameState s);

    void updateMainMenu();
    void updateCharaSelect();
    void updateSettings();
    void updateGameplay();
    void updatePaused();
    void updateGameOver();
    void updateWaveManager();

    void drawGameplay();
    void drawArena();
    void drawPickups();
    void drawSlowZones();
    void drawExplosions();
    void drawHUD();
    void drawPaused();
    void drawGameOver();
    void drawCountdown() const;

    void checkCollisions();
    void checkProjectileVsEnemies();
    void checkProjectileVsPlayer();
    void checkProjectileVsWalls();
    void checkAmmoPickupCollisions();
    void checkHealthPickupCollisions();
    void checkSlowOrbLanding();
    void checkEnemyPlayerCollisions();
    void triggerExplosion(Vector2 center, float radius, int maxDamage);
    void triggerStompExplosion(Vector2 center, float radius, int maxDamage);

    void applySlowZonesToEnemies();
    void spawnHealthPickupAtWaveEnd();
    void checkAndGiveSlowWeapon();
    void skipWave();

    std::pair<float,float> getSpawnPosition();
    void dropAmmo(std::pair<float,float> pos);
    void applyAmmoToPlayer(const AmmoPickup& pick);
    void recalcPauseButtons();
    Vector2 v2(std::pair<float,float> p) const { return {p.first,p.second}; }
};

// ═════════════════════════════════════════════════════════════════════════════
// IMPLÉMENTATION
// ═════════════════════════════════════════════════════════════════════════════

Game::Game(int w, int h, const char* t)
    : baseWidth(w), baseHeight(h), windowTitle(t),
      currentState(GameState::MAIN_MENU), isRunning(false),
      masterVolume(0.5f), isFullscreen(false),
      deltaTime(0.f), gameTime(0.f), slowWeaponGiven(false),
      currentGameMusic("none"), bossThemePlaying(false)
{
    btnResume   = {{0,0,260,52},"RESUME"};
    btnQuitMenu = {{0,0,260,52},"QUIT TO MENU"};
}
Game::~Game() { cleanup(); }

void Game::recalcPauseButtons() {
    float cx=(GetScreenWidth()-260)/2.f, cy=GetScreenHeight()/2.f;
    btnResume.rect   = {cx, cy-10, 260, 52};
    btnQuitMenu.rect = {cx, cy+72, 260, 52};
}

void Game::init() {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(baseWidth, baseHeight, windowTitle);
    SetTargetFPS(120);
    InitAudioDevice();

    scale.update(GetScreenWidth(), GetScreenHeight());

    spriteManager    = std::make_unique<Ressource>();
    collisionManager = std::make_unique<CollisionManager>();
    hud = std::make_unique<HUD>((int)ScaleContext::VIRT_W,(int)ScaleContext::VIRT_H);

    setupMainMenu();

    settingsMenu = std::make_unique<SettingsMenu>(
        (int)ScaleContext::VIRT_W, (int)ScaleContext::VIRT_H,
        [this](float v){ masterVolume=v; SetMasterVolume(v); },
        [this](bool){
            if (IsWindowFullscreen()) {
                ToggleFullscreen();
                SetWindowSize(1280, 720);
                SetWindowPosition((GetMonitorWidth(0)-800)/2, (GetMonitorHeight(0)-600)/2);
                isFullscreen = false;
            } else {
                SetWindowSize(GetMonitorWidth(0), GetMonitorHeight(0));
                ToggleFullscreen();
                isFullscreen = true;
            }
        }
    );
    isRunning=true;
}

void Game::setupMainMenu() {
    StopMusicStream(spriteManager->getMusic("game_over"));
    PlayMusicStream(spriteManager->getMusic("menu_theme"));
    mainMenu=std::make_unique<MainMenu>((int)ScaleContext::VIRT_W,(int)ScaleContext::VIRT_H, spriteManager.get());
    float cx=ScaleContext::VIRT_W/2.f-130, cy=ScaleContext::VIRT_H/2.f-60;
    mainMenu->addButton({cx,cy,     260,52},"START GAME",[this](){initNewGame();setState(GameState::PLAYING);});
    mainMenu->addButton({cx,cy+70,  260,52},"SETTINGS",  [this](){setState(GameState::SETTINGS);});
    mainMenu->addButton({cx,cy+140, 260,52},"QUIT",      [this](){isRunning=false;});
}

void Game::initNewGame() {
    StopMusicStream(spriteManager->getMusic("menu_theme"));
    enemies.clear(); projectiles.clear();
    ammoPickups.clear(); healthPickups.clear();
    slowZones.clear(); explosions.clear();
    waveManager=WaveManager{};
    gameTime=0.f;
    slowWeaponGiven=true;

    buildArena();

    float px=ScaleContext::VIRT_W/2.f, py=ScaleContext::VIRT_H/2.f;
    player=std::make_unique<Player>(spriteManager, std::make_pair(px,py));
    player->setSpriteTexture(spriteManager->getTexture("default_rifle"));
    player->setObstacles(&obstacles);

    player->addWeapon(std::make_shared<RifleWeapon>());
    player->addWeapon(std::make_shared<LaserWeapon>());
    player->addWeapon(std::make_shared<BombWeapon>());
    player->addWeapon(std::make_shared<SlowFieldWeapon>());
    player->switchWeapon(0);
}

void Game::buildArena() {
    obstacles.clear();
    const float W=ScaleContext::VIRT_W, H=ScaleContext::VIRT_H, M=32.f;

    // Murs périmètre
    obstacles.push_back({0,0,W,M});
    obstacles.push_back({0,H-M,W,M});
    obstacles.push_back({0,0,M,H});
    obstacles.push_back({W-M,0,M,H});

    // Piliers centraux avec gap
    obstacles.push_back({W/2-60, H/2-180, 48, 148});
    obstacles.push_back({W/2+12, H/2-180, 48, 148});
    obstacles.push_back({W/2-60, H/2+32,  48, 148});
    obstacles.push_back({W/2+12, H/2+32,  48, 148});

    // Couvertures en L par quadrant
    obstacles.push_back({220,160,100,32}); obstacles.push_back({220,160,32,90});
    obstacles.push_back({W-320,160,100,32}); obstacles.push_back({W-252,160,32,90});
    obstacles.push_back({220,H-252,100,32}); obstacles.push_back({220,H-252,32,90});
    obstacles.push_back({W-320,H-252,100,32}); obstacles.push_back({W-252,H-252,32,90});

    // Murs corridors
    obstacles.push_back({M+60,  H/2-16, 160, 32});
    obstacles.push_back({W-M-220,H/2-16,160,32});
    obstacles.push_back({W/2-16, M+60,  32, 140});
    obstacles.push_back({W/2-16, H-M-200,32,140});

    // Caisses quadrant
    obstacles.push_back({490,280,60,60});
    obstacles.push_back({W-550,280,60,60});
    obstacles.push_back({490,H-340,60,60});
    obstacles.push_back({W-550,H-340,60,60});
}

void Game::run() {
    while (!WindowShouldClose() && isRunning) {
        // Mise à jour scale chaque frame — gère resize ET fullscreen toggle
        scale.update(GetScreenWidth(), GetScreenHeight());
        deltaTime=GetFrameTime();
        if(deltaTime>0.05f) deltaTime=0.05f;
        gameTime+=deltaTime;
        
        // Update musiques de fond
        UpdateMusicStream(spriteManager->getMusic("menu_theme"));
        UpdateMusicStream(spriteManager->getMusic("boss_theme"));
        UpdateMusicStream(spriteManager->getMusic("game_over"));
        UpdateMusicStream(spriteManager->getMusic("victory"));
        
        update();
        draw();
    }
}

void Game::setState(GameState s) {
    currentState=s;
    if(s==GameState::MAIN_MENU) setupMainMenu();
    if(s==GameState::PAUSED) {
        recalcPauseButtons();
        scale.resetCamera();
    }
    if(s==GameState::CHARA_SELECT) /* TODO */;
    if(s==GameState::GAME_OVER) {
        scale.resetCamera();
        StopMusicStream(spriteManager->getMusic("boss_theme"));
        PlayMusicStream(spriteManager->getMusic("game_over"));
        currentGameMusic="game_over";
        bossThemePlaying=false;
    }
}

void Game::update() {
    touchControls.update();
    switch(currentState){
        case GameState::MAIN_MENU: updateMainMenu(); break;
        case GameState::SETTINGS:  updateSettings();  break;
        case GameState::PLAYING:   updateGameplay();  break;
        case GameState::PAUSED:    updatePaused();    break;
        case GameState::GAME_OVER: updateGameOver();  break;
    }
}

void Game::updateMainMenu()  { mainMenu->update(deltaTime, scale.worldMouse()); }
void Game::updateCharaSelect()  { /* TODO */ }
void Game::updateSettings()  { settingsMenu->update(scale.worldMouse()); if(IsKeyPressed(KEY_BACKSPACE)||touchControls.backTapped()) setState(GameState::MAIN_MENU); }
void Game::updateGameOver()  { if(IsKeyPressed(KEY_ENTER)||IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) setState(GameState::MAIN_MENU); }

void Game::updatePaused() {
    if(IsKeyPressed(KEY_BACKSPACE)||touchControls.backTapped()) setState(GameState::PLAYING);
    recalcPauseButtons();
    btnResume.update(); btnQuitMenu.update();
    if(btnResume.clicked())   setState(GameState::PLAYING);
    if(btnQuitMenu.clicked()||touchControls.menuTapped()) {StopMusicStream(spriteManager->getMusic("boss_theme")); currentGameMusic="none"; setState(GameState::MAIN_MENU);}
}

void Game::updateGameplay() {
    if(IsKeyPressed(KEY_BACKSPACE)||touchControls.backTapped()) setState(GameState::PAUSED);
    if(touchControls.menuTapped()) {StopMusicStream(spriteManager->getMusic("boss_theme")); currentGameMusic="none"; setState(GameState::MAIN_MENU);}
    if(IsKeyPressed(KEY_KP_SUBTRACT)) skipWave();

    if(player){
        auto pp = player->getPosition();
        float zoomLevel = 1.0f - (waveManager.enemiesRemaining * 0.02f);
        zoomLevel = std::max(0.8f, std::min(1.3f, zoomLevel));
        scale.setCameraTarget(pp.first, pp.second, 1.5f);
        scale.updateCamera(pp.first, pp.second, deltaTime);
    }

    if(player){
        player->handleInput(deltaTime);
        Vector2 touchDelta=touchControls.getMoveDelta(200.f, deltaTime);
        if(touchDelta.x!=0.f||touchDelta.y!=0.f) player->move(touchDelta.x, touchDelta.y);

        bool touchActive = touchControls.isActive();
        Vector2 pp = v2(player->getPosition());
        Vector2 wm = touchActive ? touchControls.computeAimWorldPos(pp) : scale.worldMouse();
        player->update(deltaTime, wm);

        if(touchControls.buttonCTapped()) player->nextWeapon();
        if(touchControls.reloadTapped()) player->reload();
        if(auto cw=player->getCurrentWeapon()){
            Texture2D nextIcon = {0,0,0,0,0};
            if(player->getWeaponCount()>0){
                auto nw=player->getWeapon((player->getCurrentWeaponIndex()+1)%player->getWeaponCount());
                if(nw) nextIcon=nw->getIcon();
            }
            touchControls.setIcons(cw->getIcon(), nextIcon);
        }

        // Rifle — on touch, the right (AIM) stick replaces the left click
        // entirely (raylib emulates a phantom left-click from ANY finger on
        // screen on Android, so mixing the two would fire while just moving).
        if(std::dynamic_pointer_cast<RifleWeapon>(player->getCurrentWeapon())){
            bool fireDown = touchActive ? touchControls.aimDown() : IsMouseButtonDown(MOUSE_LEFT_BUTTON);
            if(fireDown){
                auto p=player->tryFire(wm);
                if(p) {
                    projectiles.push_back(p);
                    PlaySound(spriteManager->getSound("shoot_ar"));
                }
            }
        }
        // Laser
        if(std::dynamic_pointer_cast<LaserWeapon>(player->getCurrentWeapon())){
            bool chargeDown = touchActive ? touchControls.aimDown() : IsMouseButtonDown(MOUSE_RIGHT_BUTTON);
            bool release     = touchActive ? touchControls.aimReleased() : IsMouseButtonReleased(MOUSE_RIGHT_BUTTON);
            if(chargeDown) player->chargeLaser(deltaTime);
            if(release) {
                if(player->releaseLaser(wm))
                    PlaySound(spriteManager->getSound("shoot_laser"));
            }
        }
        // Bomb — pushing the AIM stick out sets the throw direction and
        // distance (how far it's pushed = how far it's thrown), mirroring
        // how far the mouse is moved on desktop.
        if(std::dynamic_pointer_cast<BombWeapon>(player->getCurrentWeapon())){
            bool held    = touchActive ? touchControls.aimDown() : IsMouseButtonDown(MOUSE_RIGHT_BUTTON);
            bool release = touchActive ? touchControls.aimReleased() : IsMouseButtonReleased(MOUSE_RIGHT_BUTTON);
            if(touchActive) player->setShowThrowPreview(held);
            if(release) {
                Vector2 throwPos = touchActive ? touchControls.getThrowAimWorldPos(pp) : wm;
                auto g=player->throwBomb(throwPos);
                if(g) {
                    projectiles.push_back(g);
                    PlaySound(spriteManager->getSound("shoot_Frag"));
                }
            }
        }
        // SlowField
        auto slowW=std::dynamic_pointer_cast<SlowFieldWeapon>(player->getCurrentWeapon());
        if(slowW){
            bool release = touchActive ? touchControls.aimReleased() : IsMouseButtonReleased(MOUSE_RIGHT_BUTTON);
            if(release){
                Vector2 throwPos = touchActive ? touchControls.getThrowAimWorldPos(pp) : wm;
                auto orb=slowW->createOrb(pp, throwPos);
                if(orb) {
                    projectiles.push_back(orb);
                    PlaySound(spriteManager->getSound("poison_throw"));
                }
            }
        }

        if(player->getHp()<=0) setState(GameState::GAME_OVER);
    }

    updateWaveManager();

    auto playerPos=player?player->getPosition():std::make_pair(0.f,0.f);

    applySlowZonesToEnemies();

    for(auto it=enemies.begin(); it!=enemies.end();){
        auto& e=*it;
        e->setObstacles(&obstacles);
        e->update(deltaTime, playerPos);

        for(auto& s:e->flushProjectiles()) projectiles.push_back(s);

        if(e->wasStompFired()){
            Vector2 sc2=v2(e->getPosition());
            triggerStompExplosion(sc2, e->getStompRadius(), e->getStompDamage());
            explosions.push_back({sc2, 0, e->getStompRadius()*1.3f, 0.5f, 0.5f});
        }
        if(e->isSpinActive() && e->getType()==EnemyType::BRUTE_BOSS){
            Vector2 sc2=v2(e->getPosition());
            float dx2=v2(playerPos).x-sc2.x, dy2=v2(playerPos).y-sc2.y;
            float dp=sqrtf(dx2*dx2+dy2*dy2);
            if(player && dp<=e->getSpinRadius())
                player->takeDamage((int)(e->getSpinDamage()*deltaTime*60.f));
        }

        if(e->isDead()){
            waveManager.enemiesRemaining--;
            if(e->isBossEnemy()) { 
                waveManager.bossAlive=false;
                StopMusicStream(spriteManager->getMusic("boss_theme"));
                bossThemePlaying=false;
            }
            dropAmmo(e->getPosition());
            it=enemies.erase(it);
        } else ++it;
    }

    for(auto it=projectiles.begin(); it!=projectiles.end();){
        (*it)->update(deltaTime);

        if((*it)->getType()==ProjectileType::GRENADE && !(*it)->isActive()){
            if((*it)->getIsExplosive()){
                PlaySound(spriteManager->getSound("Frag"));
                triggerExplosion((*it)->getPosition(), (*it)->getExplosionRadius(), 100);
            } else {
                // SlowField orb : crée une slow zone
                slowZones.push_back({
                    (*it)->getPosition(),
                    (*it)->getExplosionRadius(),
                    0.28f,
                    5.f, 5.f
                });
                // Effet visuel splash
                explosions.push_back({(*it)->getPosition(), 0, (*it)->getExplosionRadius()*0.6f, 0.35f, 0.35f});

            }
            it=projectiles.erase(it); continue;
        }

        Vector2 p=(*it)->getPosition();
        if(p.x<-60||p.x>ScaleContext::VIRT_W+60||p.y<-60||p.y>ScaleContext::VIRT_H+60)
            (*it)->deactivate();
        if(!(*it)->isActive()) it=projectiles.erase(it); else ++it;
    }

    for(auto& sz:slowZones) sz.update(deltaTime);
    slowZones.erase(std::remove_if(slowZones.begin(),slowZones.end(),[](const SlowZone& z){return z.expired();}),slowZones.end());

    for(auto& ap:ammoPickups)    ap.update(deltaTime);
    for(auto& hp:healthPickups)  hp.update(deltaTime);
    healthPickups.erase(std::remove_if(healthPickups.begin(),healthPickups.end(),[](const HealthPickup& h){return !h.active;}),healthPickups.end());

    for(auto it=explosions.begin();it!=explosions.end();){
        it->timer-=deltaTime;
        it->radius=it->maxRadius*(1.f-it->timer/it->duration);
        if(it->timer<=0) it=explosions.erase(it); else ++it;
    }

    checkCollisions();
}

void Game::updateWaveManager() {
    auto& wm=waveManager;
    wm.phaseTimer-=deltaTime;
    if(wm.bossAlert) wm.bossAlertTimer+=deltaTime;

    switch(wm.phase){
        case WavePhase::COUNTDOWN:
            if(wm.phaseTimer<=0.f){
                wm.currentWave++;
                wm.bossAlert      = wm.isBossWave();
                wm.bossAlertTimer = 0.f;
                wm.enemiesToSpawn = wm.enemyCount();
                wm.enemiesRemaining=0;
                wm.spawnTimer=0.f;
                wm.phase=WavePhase::ACTIVE;
            }
            break;

        case WavePhase::ACTIVE:
            if(wm.enemiesToSpawn>0){
                wm.spawnTimer-=deltaTime;
                if(wm.spawnTimer<=0.f){
                    auto spos=getSpawnPosition();
                    EnemyType eType=wm.isBossWave() ? wm.pickBossType() : wm.pickEnemyType();
                    auto e=std::make_shared<Enemy>(spos, eType);
                    e->applyWaveScaling(wm.currentWave, wm.megaCycle());
                    e->setSpriteTexture(spriteManager->getTexture("basic"));
                    e->setObstacles(&obstacles);
                    enemies.push_back(e);
                    wm.enemiesRemaining++;
                    wm.enemiesToSpawn--;
                    wm.spawnTimer=wm.spawnInterval();
                    
                    
                    if(wm.isBossWave() && !bossThemePlaying && e->isBossEnemy()){
                        StopMusicStream(spriteManager->getMusic("menu_theme"));
                        PlayMusicStream(spriteManager->getMusic("boss_theme"));
                        currentGameMusic="boss_theme";
                        bossThemePlaying=true;
                    }
                }
            }
            if(wm.enemiesToSpawn==0 && wm.enemiesRemaining<=0){
                wm.bossAlert=false;
                spawnHealthPickupAtWaveEnd();
                checkAndGiveSlowWeapon();

                wm.phase    = wm.isBossWave() ? WavePhase::BOSS_REST : WavePhase::REST;
                wm.phaseTimer=wm.isBossWave() ? WaveManager::BOSS_REST_DUR : WaveManager::REST_DUR;
            }
            break;

        case WavePhase::REST:
        case WavePhase::BOSS_REST:
            if(wm.phaseTimer<=0.f){
                wm.phase=WavePhase::COUNTDOWN;
                wm.phaseTimer=WaveManager::COUNTDOWN_DUR;
            }
            break;
    }
}

std::pair<float,float> Game::getSpawnPosition() {
    static const float pts[][2]={
        {120,80},{480,60},{960,60},{1440,60},{1800,80},
        {80,320},{1840,320},
        {80,760},{1840,760},
        {120,1000},{480,1020},{960,1020},{1440,1020},{1800,1000}
    };
    constexpr int N=14;
    if(!player) return {pts[rand()%N][0],pts[rand()%N][1]};
    auto pp=player->getPosition();
    int best=0; float bestD=-1;
    for(int i=0;i<N;i++){
        float dx=pts[i][0]-pp.first, dy=pts[i][1]-pp.second;
        float d=dx*dx+dy*dy;
        if(d>bestD){bestD=d;best=i;}
    }
    float jx=(float)((rand()%80)-40), jy=(float)((rand()%80)-40);
    return {pts[best][0]+jx, pts[best][1]+jy};
}

void Game::spawnHealthPickupAtWaveEnd() {
    float cx=ScaleContext::VIRT_W/2.f;
    float cy=ScaleContext::VIRT_H/2.f;
    healthPickups.emplace_back(std::make_pair(cx,cy), 10);
}

void Game::checkAndGiveSlowWeapon() {
    if (!slowWeaponGiven && waveManager.currentWave >= 4 && player) {
        player->addWeapon(std::make_shared<SlowFieldWeapon>());
        slowWeaponGiven = true;
    }
}

void Game::skipWave() {
    enemies.clear();
    waveManager.enemiesRemaining = 0;
    waveManager.enemiesToSpawn = 0;
    waveManager.phase = WavePhase::REST;
    waveManager.phaseTimer = WaveManager::REST_DUR;
}

void Game::applySlowZonesToEnemies() {
    for(auto& e:enemies){
        float mult=1.f;
        for(auto& sz:slowZones){
            if(sz.contains(e->getPosition())) { mult=sz.slowMult; break; }
        }
        e->setSlowMult(mult);
    }
}

void Game::dropAmmo(std::pair<float,float> pos) {
    if(!AmmoDrop::shouldDrop(waveManager.currentWave)) return;
    AmmoType t=AmmoDrop::pickType();
    int amt=AmmoDrop::amountForType(t,waveManager.currentWave);
    ammoPickups.emplace_back(pos,t,amt);
}

void Game::applyAmmoToPlayer(const AmmoPickup& pick) {
    if(!player) return;
    for(int i=0;i<player->getWeaponCount();i++){
        auto w=player->getWeapon(i); if(!w) continue;
        bool applied=false;
        switch(pick.type){
            case AmmoType::RIFLE: if(w->getWeaponType()=="Rifle"){w->addAmmo(pick.amount);applied=true;} break;
            case AmmoType::LASER: if(w->getWeaponType()=="Laser"){w->addAmmo(pick.amount);applied=true;} break;
            case AmmoType::BOMB:  if(w->getWeaponType()=="Bomb"||w->getWeaponType()=="SlowField"){w->addAmmo(pick.amount);applied=true;} break;
            case AmmoType::SLOW: if(w->getWeaponType()=="SlowField"){w->addAmmo(pick.amount);applied=true;} break;
        }
        if(applied) break;
    }
}

void Game::checkCollisions() {
    checkProjectileVsEnemies();
    checkProjectileVsPlayer();
    checkProjectileVsWalls();
    checkAmmoPickupCollisions();
    checkHealthPickupCollisions();
    checkEnemyPlayerCollisions();
}

void Game::checkProjectileVsEnemies() {
    for(auto it=projectiles.begin();it!=projectiles.end();){
        auto& proj=*it;
        if(!proj||!proj->isActive()||proj->getOwn()!=true){++it;continue;}
        if(proj->getType()==ProjectileType::GRENADE){
            for(auto& e:enemies){
                if(!e||e->isDead()) continue;
                if(collisionManager->checkProjectileEnemyCollision(proj->getPosition(),5.f,e->getCollisionBox())){
                    PlaySound(spriteManager->getSound("hit_enemy"));
                    if(proj->getIsExplosive()){
                        triggerExplosion(proj->getPosition(), proj->getExplosionRadius(), 100);
                    } else {
                        slowZones.push_back({proj->getPosition(), proj->getExplosionRadius(), 0.28f, 5.f, 5.f});
                        explosions.push_back({proj->getPosition(), 0, proj->getExplosionRadius()*0.6f, 0.35f, 0.35f});
                    }
                    proj->deactivate();
                    break;
                }
            }
            if(!proj->isActive()) it=projectiles.erase(it); else ++it;
            continue;
        }
        bool hit=false;
        for(auto& e:enemies){
            if(!e||e->isDead()) continue;
            if(collisionManager->checkProjectileEnemyCollision(proj->getPosition(),5.f,e->getCollisionBox())){
                PlaySound(spriteManager->getSound("hit_enemy"));
                e->takeDamage((int)proj->getDamage()); proj->deactivate(); hit=true; break;
            }
        }
        if(hit) it=projectiles.erase(it); else ++it;
    }
    if(player){
        auto laser=std::dynamic_pointer_cast<LaserWeapon>(player->getCurrentWeapon());
        if(laser&&laser->isBeamVisible()){
            Vector2 bs=laser->getBeamStart(), be=laser->getBeamEnd();
            for(auto& e:enemies){
                if(!e||e->isDead()) continue;
                Vector2 hp2;
                if(collisionManager->checkHitscanEnemyCollision(bs,be,e->getCollisionBox(),hp2)){
                    PlaySound(spriteManager->getSound("hit_enemy"));
                    e->takeDamage(laser->getBeamDamage());
                    laser->setLastHitPoint(hp2); break;
                }
            }
        }
    }
}

void Game::checkProjectileVsPlayer() {
    if(!player) return;
    Rectangle pb=player->getCollisionBox();
    for(auto it=projectiles.begin();it!=projectiles.end();){
        auto& proj=*it;
        if(!proj||!proj->isActive()||proj->getOwn()!=false){++it;continue;}
        if(CheckCollisionPointRec(proj->getPosition(),pb)){
            player->takeDamage((int)proj->getDamage());
            if(proj->getDotDamage()>0) player->applyPoison(proj->getDotDamage(),proj->getDotDuration());
            proj->deactivate(); it=projectiles.erase(it);
        } else ++it;
    }
    for(auto it=projectiles.begin();it!=projectiles.end();){
        auto& proj=*it;
        if(!proj||!proj->isActive()||proj->getType()!=ProjectileType::BULLET){++it;continue;}
        if((int)proj->getDamage()==14 && CheckCollisionPointRec(proj->getPosition(),pb)){
            player->takeDamage((int)proj->getDamage());
            proj->deactivate(); it=projectiles.erase(it);
        } else ++it;
    }
}

void Game::checkProjectileVsWalls() {
    for(auto& proj:projectiles){
        if(!proj||!proj->isActive()) continue;
        if(proj->getType()==ProjectileType::GRENADE){
            for(const auto& obs:obstacles){
                if(collisionManager->checkProjectileWallCollision(proj->getPosition(),5.f,obs)){
                    if(proj->getIsExplosive()){
                        triggerExplosion(proj->getPosition(), proj->getExplosionRadius(), 100);
                    } else {
                        slowZones.push_back({proj->getPosition(), proj->getExplosionRadius(), 0.28f, 5.f, 5.f});
                        explosions.push_back({proj->getPosition(), 0, proj->getExplosionRadius()*0.6f, 0.35f, 0.35f});
                    }
                    proj->deactivate(); break;
                }
            }
            continue;
        }
        for(const auto& obs:obstacles){
            if(collisionManager->checkProjectileWallCollision(proj->getPosition(),5.f,obs)){
                proj->deactivate(); break;
            }
        }
    }
}

void Game::checkAmmoPickupCollisions() {
    if(!player) return;
    Rectangle pb=player->getCollisionBox();
    for(auto& ap:ammoPickups){
        if(!ap.active) continue;
        Rectangle ab={ap.pos.first-12,ap.pos.second-12,24,24};
        if(CheckCollisionRecs(pb,ab)){ applyAmmoToPlayer(ap); ap.active=false; }
    }
    ammoPickups.erase(std::remove_if(ammoPickups.begin(),ammoPickups.end(),[](const AmmoPickup& a){return !a.active;}),ammoPickups.end());
}

void Game::checkHealthPickupCollisions() {
    if(!player) return;
    Rectangle pb=player->getCollisionBox();
    for(auto& hp:healthPickups){
        if(!hp.active) continue;
        Rectangle hb={hp.pos.first-14,hp.pos.second-14,28,28};
        if(CheckCollisionRecs(pb,hb)){
            int newHp=std::min(player->getHp()+hp.amount, player->getMaxHp());
            player->setHp(newHp);
            hp.active=false;
        }
    }
}

void Game::checkEnemyPlayerCollisions() {
    if(!player) return;
    Rectangle pb=player->getCollisionBox();
    for(auto& e:enemies){
        if(!e||e->isDead()) continue;
        if(collisionManager->checkPlayerEnemyCollision(pb,e->getCollisionBox())){
            if(e->getMeleeCooldown()<=0.f){
                player->takeDamage(e->getMeleeDamage());
                e->setMeleeCooldown(1.f);
            }
        }
    }
}

void Game::triggerExplosion(Vector2 center, float radius, int maxDamage) {
    explosions.push_back({center,0,radius,0.55f,0.55f});
    for(auto& e:enemies){
        if(!e||e->isDead()) continue;
        Vector2 ep=v2(e->getPosition());
        float dx=ep.x-center.x, dy=ep.y-center.y, d=sqrtf(dx*dx+dy*dy);
        if(d<=radius){
            float f=1.f-d/radius;
            e->takeDamage((int)(maxDamage*f));
            if(d>1.f){ Vector2 dir={dx/d,dy/d}; e->applyKnockback(dir,320.f*f); }
        }
    }
    if(player){
        Vector2 pp=v2(player->getPosition());
        float dx=pp.x-center.x, dy=pp.y-center.y, d=sqrtf(dx*dx+dy*dy);
        if(d<=radius*0.6f) player->takeDamage((int)(25*(1.f-d/(radius*0.6f))));
    }
}

void Game::triggerStompExplosion(Vector2 center, float radius, int maxDamage) {
    for(auto& e:enemies){
        if(!e||e->isDead()) continue;
        if(e->getType()==EnemyType::BRUTE || e->getType()==EnemyType::BRUTE_E || e->getType()==EnemyType::BRUTE_BOSS) continue;
        Vector2 ep=v2(e->getPosition());
        float dx=ep.x-center.x, dy=ep.y-center.y, d=sqrtf(dx*dx+dy*dy);
        if(d<=radius){
            float f=1.f-d/radius;
            e->takeDamage((int)(maxDamage*f*0.5f));
        }
    }
    if(player){
        Vector2 pp=v2(player->getPosition());
        float dx=pp.x-center.x, dy=pp.y-center.y, d=sqrtf(dx*dx+dy*dy);
        if(d<=radius){
            float f=1.f-d/radius;
            player->takeDamage((int)(maxDamage*f));
        }
    }
}

void Game::draw() {
    BeginDrawing();
    switch(currentState){
        case GameState::MAIN_MENU:
            BeginMode2D(scale.getCamera()); mainMenu->draw(); EndMode2D();
            scale.drawLetterbox(); break;
        case GameState::SETTINGS:
            BeginMode2D(scale.getCamera()); settingsMenu->draw(); EndMode2D();
            scale.drawLetterbox(); break;
        case GameState::PLAYING:
            BeginMode2D(scale.getCamera()); drawGameplay(); EndMode2D();
            scale.drawLetterbox(); drawHUD(); drawCountdown(); touchControls.draw(); break;
        case GameState::PAUSED:
            BeginMode2D(scale.getCamera()); drawGameplay(); EndMode2D();
            scale.drawLetterbox(); drawHUD(); drawPaused(); touchControls.draw(); break;
        case GameState::GAME_OVER:
            BeginMode2D(scale.getCamera()); drawGameOver(); EndMode2D();
            scale.drawLetterbox(); break;
    }
    EndDrawing();
}

void Game::drawGameplay() {
    ClearBackground({14,18,28,255});
    drawArena();
    drawSlowZones();
    drawPickups();
    for(const auto& p:projectiles) if(p&&p->isActive()) p->draw();
    drawExplosions();
    for(const auto& e:enemies) if(e) e->draw();
    if(player) player->draw(scale.worldMouse());
}

void Game::drawArena() {
    for(float x=0;x<ScaleContext::VIRT_W;x+=80) DrawLine((int)x,0,(int)x,(int)ScaleContext::VIRT_H,{255,255,255,5});
    for(float y=0;y<ScaleContext::VIRT_H;y+=80) DrawLine(0,(int)y,(int)ScaleContext::VIRT_W,(int)y,{255,255,255,5});
    for(const auto& obs:obstacles){
        bool wall=(obs.x<=0||obs.y<=0||obs.x+obs.width>=ScaleContext::VIRT_W-1||obs.y+obs.height>=ScaleContext::VIRT_H-1);
        DrawRectangleRec(obs, wall?(Color){28,34,50,255}:(Color){38,48,68,255});
        DrawRectangleLinesEx(obs,2.f, wall?(Color){55,70,100,255}:(Color){70,95,135,255});
        if(!wall) DrawLine((int)obs.x,(int)obs.y,(int)(obs.x+obs.width),(int)obs.y,{100,140,200,130});
    }
}

void Game::drawSlowZones() {
    for(const auto& sz:slowZones) sz.draw();
}

void Game::drawPickups() {
    for(const auto& ap:ammoPickups) ap.draw();
    for(const auto& hp:healthPickups) hp.draw();
}

void Game::drawExplosions() {
    for(const auto& ex:explosions){
        float alpha=ex.timer/ex.duration;
        DrawCircleV(ex.pos, ex.radius*0.6f, Fade({255,200,50,255},alpha*0.7f));
        DrawCircleV(ex.pos, ex.radius,       Fade({255,80,0,255},  alpha*0.4f));
        DrawCircleLines((int)ex.pos.x,(int)ex.pos.y, ex.radius,       Fade(WHITE,alpha*0.8f));
        DrawCircleLines((int)ex.pos.x,(int)ex.pos.y, ex.radius*1.3f,  Fade({255,150,0,255},alpha*0.3f));
    }
}

void Game::drawHUD() {
    hud->drawScaled(player.get(), waveManager.currentWave,
                    waveManager.bossAlert, waveManager.bossAlertTimer, scale);
}

void Game::drawCountdown() const {
    if(waveManager.phase!=WavePhase::COUNTDOWN&&waveManager.phase!=WavePhase::REST&&waveManager.phase!=WavePhase::BOSS_REST) return;
    const char* msg=nullptr; Color col=WHITE;
    if(waveManager.phase==WavePhase::COUNTDOWN){
        int sec=(int)waveManager.phaseTimer+1;
        int nextWave = waveManager.currentWave + 1;

        bool nextIsBoss = (nextWave % 4 == 0) && (nextWave > 0);
        const char* wtype="";
        if(!nextIsBoss){
            int wc = (nextWave % 4 == 0) ? 4 : (nextWave % 4);
            if(wc==1) wtype=" [NORMAL]";
            else if(wc==2) wtype=" [MIXED]";
            else if(wc==3) wtype=" [ELITE]";
        }
        if(nextIsBoss){
            int tmpWave=nextWave;
            int mc=(tmpWave-1)/4;
            const char* bossName="BOSS";
            EnemyType bt;
            if(mc==0) bt=EnemyType::ZOMBIE_BOSS;
            else if(mc==1) bt=EnemyType::ASSASSIN_BOSS;
            else if(mc==2) bt=EnemyType::BRUTE_BOSS;
            else { int idx=(tmpWave/4-1)%3; if(idx==0)bt=EnemyType::ZOMBIE_BOSS; else if(idx==1)bt=EnemyType::ASSASSIN_BOSS; else bt=EnemyType::BRUTE_BOSS; }
            if(bt==EnemyType::ZOMBIE_BOSS)   bossName="!! ZOMBIE BOSS !!";
            if(bt==EnemyType::ASSASSIN_BOSS) bossName="!! ASSASSIN BOSS !!";
            if(bt==EnemyType::BRUTE_BOSS)    bossName="!! BRUTE BOSS !!";
            msg=TextFormat("%s  in  %d ...", bossName, sec);
        } else {
            msg=TextFormat("Wave %d%s  in  %d ...", nextWave, wtype, sec);
        }
        col=nextIsBoss?(Color){255,60,60,255}:(Color){220,220,80,255};
    } else if(waveManager.phase==WavePhase::REST){
        if(!slowWeaponGiven && waveManager.currentWave==4)
            msg="WAVE CLEARED !  NEW WEAPON: Cryo Orb  (press 4)  [Next wave in...]";
        else
            msg=TextFormat("WAVE CLEARED !   Next in %d ...", (int)waveManager.phaseTimer+1);
        col={80,220,80,255};
    } else {
        msg=TextFormat("BOSS DEFEATED !   Respite...  %d", (int)waveManager.phaseTimer+1);
        col={80,180,255,255};
    }
    if(!msg) return;
    int fs=(int)scale.s(24); if(fs<12)fs=12;
    int tw=MeasureText(msg,fs);
    int tx=(GetScreenWidth()-tw)/2;
    int ty=(int)scale.ry(ScaleContext::VIRT_H/2.f-14);
    DrawRectangle(tx-16,ty-8,tw+32,fs+16,Fade(BLACK,0.65f));
    DrawText(msg,tx,ty,fs,col);
}

void Game::drawPaused() {
    DrawRectangle(0,0,GetScreenWidth(),GetScreenHeight(),Fade(BLACK,0.55f));
    int sz=(int)scale.s(48); const char* t="PAUSED";
    int tw=MeasureText(t,sz);
    DrawText(t,(GetScreenWidth()-tw)/2,(int)(GetScreenHeight()/2.f-scale.s(85)),sz,WHITE);
    btnResume.draw(); btnQuitMenu.draw();
}

void Game::drawGameOver() {
    ClearBackground({10,5,5,255});
    const char* title="GAME OVER"; int sz=72;
    int tw=MeasureText(title,sz);
    DrawText(title,(int)(ScaleContext::VIRT_W/2-tw/2),(int)(ScaleContext::VIRT_H/2-100),sz,{220,40,40,255});
    const char* sub=TextFormat("You survived until Wave %d", waveManager.currentWave);
    int sw=MeasureText(sub,26);
    DrawText(sub,(int)(ScaleContext::VIRT_W/2-sw/2),(int)(ScaleContext::VIRT_H/2+10),26,{180,180,180,255});
    const char* hint="Click or ENTER to return to menu";
    int hw=MeasureText(hint,18);
    DrawText(hint,(int)(ScaleContext::VIRT_W/2-hw/2),(int)(ScaleContext::VIRT_H/2+70),18,Fade(WHITE,0.6f+0.4f*sinf(gameTime*2.5f)));
}

void Game::cleanup() { CloseAudioDevice(); CloseWindow(); }
