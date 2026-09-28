#pragma once
#include <raylib.h>
#include <utility>
#include <cmath>
#include <memory>
#include <vector>
#include "WeaponBase.hpp"
#include "Projectile.hpp"

enum class EnemyType {
    ZOMBIE, ZOMBIE_E, ZOMBIE_BOSS,
    SHOOTER, SHOOTER_E,
    BRUTE, BRUTE_E, BRUTE_BOSS,
    ASSASSIN_BOSS
};

enum class EnemyState {
    WANDER, CHASE, RANGED_ATTACK, MELEE_ATTACK,
    TELEPORTING,       // ASSASSIN_BOSS : invisible en transit
    STOMP_WINDUP,      // BRUTE/BRUTE_BOSS : charge du stomp
    BERSERK_CHARGE,    // ZOMBIE_BOSS P3 / BRUTE_BOSS P3 : charge en ligne
    HURT, DEAD
};

class Enemy {
private:
    std::pair<float,float> pos;
    int   hp, maxHp;
    float baseSpeed, speed;
    float slowMult;
    float scale;
    Rectangle collisionBox;
    Texture2D sprite;

    EnemyType  type;
    EnemyState aiState;

    int   currentPhase, maxPhases;
    bool  phaseJustChanged; // pour effets de transition

    float fireTimer, fireInterval;
    float attackRange, meleeRange;

    float wanderTimer, wanderAngle;
    float hurtTimer;

    Vector2 knockbackVel;
    std::vector<std::shared_ptr<Projectile>> pending;

    float meleeCooldown;
    int   meleeDamage;

    // ── ASSASSIN_BOSS ─────────────────────────────────────────────────────────
    float teleportCooldown;
    bool  invisible;
    float invisTimer;
    std::pair<float,float> teleportDest;

    // ── BRUTE / BRUTE_BOSS ────────────────────────────────────────────────────
    float stompCooldown;
    float stompWindup;
    bool  stompFired;     // Game lit pour AoE
    float stompRadiusMult; // 1.0 normal, 1.8 boss

    // ── ZOMBIE_BOSS : berserk charge ─────────────────────────────────────────
    float chargeTimer;   // durée restante de la charge berserk
    float chargeCooldown;
    Vector2 chargeDir;

    // ── BRUTE_BOSS : spin AoE continu ────────────────────────────────────────
    float spinTimer;
    float spinCooldown;
    bool  spinActive;

    const std::vector<Rectangle>* obstacles;

public:
    Enemy(std::pair<float,float> p, EnemyType t = EnemyType::ZOMBIE);
    ~Enemy() = default;

    void update(float dt, std::pair<float,float> playerPos);
    void draw() const;
    void takeDamage(int dmg);
    void applyKnockback(Vector2 dir, float force);
    void setSlowMult(float m) { slowMult = m; }
    void setObstacles(const std::vector<Rectangle>* o) { obstacles = o; }

    std::vector<std::shared_ptr<Projectile>> flushProjectiles();

    // ── Getters ───────────────────────────────────────────────────────────────
    std::pair<float,float> getPosition()  const { return pos; }
    int   getHp()                         const { return hp; }
    int   getMaxHp()                      const { return maxHp; }
    int   getMeleeDamage()                const { return meleeDamage; }
    float getMeleeCooldown()              const { return meleeCooldown; }
    void  setMeleeCooldown(float t)             { meleeCooldown = t; }
    Rectangle getCollisionBox()           const { return collisionBox; }
    EnemyType  getType()                  const { return type; }
    bool isDead()                         const { return hp <= 0; }
    bool isBossEnemy() const {
        return type==EnemyType::ZOMBIE_BOSS ||
               type==EnemyType::ASSASSIN_BOSS ||
               type==EnemyType::BRUTE_BOSS;
    }
    bool isInvisible()  const { return invisible; }
    bool isSpinActive() const { return spinActive; }
    int  getPhase()     const { return currentPhase; }

    float getStompRadius() const { return 130.f * stompRadiusMult; }
    int   getStompDamage() const {
        return (type==EnemyType::BRUTE_BOSS) ? 55 : 35;
    }
    bool wasStompFired() { bool r=stompFired; stompFired=false; return r; }

    bool wasSpinHit()   { return spinActive; }
    float getSpinRadius() const { return 180.f; }
    int   getSpinDamage() const { return 6; }

    void setPosition(std::pair<float,float> p);
    void setSpriteTexture(const Texture2D& s) { sprite = s; }
    void applyWaveScaling(int wave, int cycle);

private:
    void updateAI(float dt, std::pair<float,float> pp);
    void updatePhase();

    void spawnBullet(std::pair<float,float> pp, float speedMult=1.f, int count=1, float spread=0.f);
    void spawnSpit  (std::pair<float,float> pp, int n=1);

    // AI par type
    void aiZombie   (float dt, std::pair<float,float> pp, float d, float nx, float ny);
    void aiShooter  (float dt, std::pair<float,float> pp, float d, float nx, float ny);
    void aiBrute    (float dt, std::pair<float,float> pp, float d, float nx, float ny);
    void aiZombieBoss (float dt, std::pair<float,float> pp, float d, float nx, float ny);
    void aiAssassinBoss(float dt, std::pair<float,float> pp, float d, float nx, float ny);
    void aiBruteBoss  (float dt, std::pair<float,float> pp, float d, float nx, float ny);

    void moveWith(float dx, float dy);
    bool resolveRect(const Rectangle& r);
    float dist(std::pair<float,float> t) const;
    Color bodyColor() const;
    void  updateHB(); 
};

Enemy::Enemy(std::pair<float,float> p, EnemyType t)
    : pos(p), type(t), aiState(EnemyState::WANDER),
      slowMult(1.f), currentPhase(1), maxPhases(1),
      phaseJustChanged(false),
      fireTimer(0.f), wanderTimer(0.f), wanderAngle(0.f),
      hurtTimer(0.f), meleeCooldown(0.f),
      teleportCooldown(0.f), invisible(false), invisTimer(0.f),
      stompCooldown(0.f), stompWindup(0.f), stompFired(false), stompRadiusMult(1.f),
      chargeTimer(0.f), chargeCooldown(0.f),
      spinTimer(0.f), spinCooldown(0.f), spinActive(false),
      obstacles(nullptr)
{
    knockbackVel={0,0};
    teleportDest={0,0};
    chargeDir={0,0};

    switch(type) {
        // Normaux
        case EnemyType::ZOMBIE:
            hp=maxHp=100;  baseSpeed=90.f;  fireInterval=2.5f;
            meleeDamage=12; scale=1.0f;
            attackRange=220.f; meleeRange=46.f; break;

        case EnemyType::ZOMBIE_E:
            hp=maxHp=180;  baseSpeed=115.f; fireInterval=1.8f;
            meleeDamage=18; scale=1.2f;
            attackRange=280.f; meleeRange=50.f; break;

        case EnemyType::SHOOTER:
            hp=maxHp=130;  baseSpeed=70.f;  fireInterval=1.5f;
            meleeDamage=6;  scale=1.1f;
            attackRange=440.f; meleeRange=55.f; break;

        case EnemyType::SHOOTER_E:
            hp=maxHp=200;  baseSpeed=100.f; fireInterval=0.9f;
            meleeDamage=8;  scale=1.3f;
            attackRange=490.f; meleeRange=60.f; break;

        case EnemyType::BRUTE:
            hp=maxHp=300;  baseSpeed=50.f;  fireInterval=9999.f;
            meleeDamage=25; scale=1.7f;
            attackRange=220.f; meleeRange=70.f;
            stompCooldown=5.f; break;

        case EnemyType::BRUTE_E:
            hp=maxHp=420;  baseSpeed=75.f;  fireInterval=9999.f;
            meleeDamage=40; scale=1.9f;
            attackRange=250.f; meleeRange=80.f;
            stompCooldown=3.f; stompRadiusMult=1.3f; break;

        // BOSS
        case EnemyType::ZOMBIE_BOSS:
            // Phase 1: rush + spit
            // Phase 2: spit storm + plus rapide
            // Phase 3: berserk charge + spit continu
            hp=maxHp=1200; baseSpeed=95.f;  fireInterval=1.2f;
            meleeDamage=22; scale=2.2f; maxPhases=3;
            attackRange=350.f; meleeRange=58.f;
            chargeCooldown=5.f; break;

        case EnemyType::ASSASSIN_BOSS:
            // Phase 1: chasse normale + balles
            // Phase 2: téléportation + burst de balles
            // Phase 3: invisible quasi-permanent + salve rapide
            hp=maxHp=1500; baseSpeed=105.f; fireInterval=1.1f;
            meleeDamage=28; scale=1.8f; maxPhases=3;
            attackRange=420.f; meleeRange=58.f;
            teleportCooldown=9999.f;
            break;

        case EnemyType::BRUTE_BOSS:
            // Phase 1: charge lente + stomp
            // Phase 2: stomp géant + plus rapide
            // Phase 3: rage spin AoE + stomp en continu
            hp=maxHp=1800; baseSpeed=52.f;  fireInterval=9999.f;
            meleeDamage=35; scale=2.0f; maxPhases=3;
            attackRange=280.f; meleeRange=80.f;
            stompCooldown=4.f; stompRadiusMult=1.8f;
            spinCooldown=8.f; break;
    }
    speed=baseSpeed;
    updateHB();
    sprite={0,0,0,0,0};
}

void Enemy::applyWaveScaling(int wave, int cycle) {
    if (isBossEnemy()) {
        if (cycle < 3) return;
        float hm = 1.f + 0.25f * (cycle - 2);
        hp = maxHp = (int)(maxHp * hm);
        baseSpeed = std::min(baseSpeed + 5.f * (cycle - 2), 200.f);
        speed = baseSpeed;
        return;
    }
    float hm = 1.f + 0.18f * cycle;
    hp = maxHp = (int)(maxHp * hm);
    baseSpeed = std::min(baseSpeed + 4.f*wave, 200.f);
    speed = baseSpeed;
    if (type==EnemyType::SHOOTER || type==EnemyType::SHOOTER_E)
        fireInterval = std::max(0.4f, fireInterval / (1.f + 0.07f*wave));
}

void Enemy::updateHB() {
    float r=20.f*scale;
    collisionBox={pos.first-r, pos.second-r, r*2.f, r*2.f};
}
void Enemy::setPosition(std::pair<float,float> p) { pos=p; updateHB(); }

bool Enemy::resolveRect(const Rectangle& o) {
    if(!CheckCollisionRecs(collisionBox,o)) return false;
    float oL=(collisionBox.x+collisionBox.width)-o.x;
    float oR=(o.x+o.width)-collisionBox.x;
    float oT=(collisionBox.y+collisionBox.height)-o.y;
    float oB=(o.y+o.height)-collisionBox.y;
    float mX=(oL<oR)?-oL:oR;
    float mY=(oT<oB)?-oT:oB;
    if(fabsf(mX)<fabsf(mY)) pos.first+=mX; else pos.second+=mY;
    updateHB(); return true;
}
void Enemy::moveWith(float dx, float dy) {
    pos.first+=dx; updateHB();
    if(obstacles) for(auto& o:*obstacles) resolveRect(o);
    pos.second+=dy; updateHB();
    if(obstacles) for(auto& o:*obstacles) resolveRect(o);
}
float Enemy::dist(std::pair<float,float> t) const {
    float dx=t.first-pos.first, dy=t.second-pos.second;
    return sqrtf(dx*dx+dy*dy);
}


void Enemy::spawnBullet(std::pair<float,float> pp, float speedMult, int count, float spread) {
    float dx=pp.first-pos.first, dy=pp.second-pos.second;
    float l=sqrtf(dx*dx+dy*dy); if(l<1.f)return;
    float base=atan2f(dy,dx);
    for(int i=0;i<count;i++){
        float angle = base + (count>1 ? (i-count/2)*spread : 0.f);
        Vector2 vel={cosf(angle)*350.f*speedMult, sinf(angle)*350.f*speedMult};
        auto b=std::make_shared<Projectile>(Vector2{pos.first,pos.second}, vel, 14.f, 3.5f, ProjectileType::BULLET, false);
        b->setColor({255,200,60,255});
        pending.push_back(b);
    }
}

void Enemy::spawnSpit(std::pair<float,float> pp, int n) {
    float dx=pp.first-pos.first, dy=pp.second-pos.second;
    float l=sqrtf(dx*dx+dy*dy); if(l<1.f)return;
    float base=atan2f(dy,dx);
    float spr=(n>1)?18.f*DEG2RAD:0.f;
    for(int i=0;i<n;i++){
        float angle=base+(n>1?(i-n/2)*spr:0.f);
        Vector2 v={cosf(angle)*215.f, sinf(angle)*215.f};
        auto s=std::make_shared<Projectile>(Vector2{pos.first,pos.second}, v, 8.f, 3.5f, ProjectileType::POISON_SPIT, false);
        s->setDot(4.f,3.f); s->setColor({0,220,60,255});
        pending.push_back(s);
    }
}

void Enemy::updatePhase() {
    if(maxPhases<2) return;
    float pct=(float)hp/(float)maxHp;
    int prev=currentPhase;
    currentPhase=(pct<=0.33f)?3:(pct<=0.66f)?2:1;
    if(currentPhase!=prev){
        phaseJustChanged=true;
        // Ajustements par boss
        if(type==EnemyType::ZOMBIE_BOSS){
            if(currentPhase==2){ baseSpeed*=1.25f; fireInterval*=0.65f; }
            if(currentPhase==3){ baseSpeed*=1.15f; fireInterval*=0.6f; chargeCooldown=3.f; }
        }
        if(type==EnemyType::ASSASSIN_BOSS){
            if(currentPhase==2){
                teleportCooldown=3.f; // active les téléportations
                fireInterval=0.8f;
            }
            if(currentPhase==3){
                teleportCooldown=1.5f;
                fireInterval=0.45f;
                baseSpeed*=1.2f;
            }
        }
        if(type==EnemyType::BRUTE_BOSS){
            if(currentPhase==2){ baseSpeed*=1.3f; stompCooldown=2.5f; stompRadiusMult=2.2f; }
            if(currentPhase==3){ baseSpeed*=1.2f; stompCooldown=1.8f; spinCooldown=6.f; }
        }
        speed=baseSpeed*slowMult;
    } else {
        phaseJustChanged=false;
    }
}

void Enemy::aiZombie(float dt, std::pair<float,float> pp, float d, float nx, float ny) {
    switch(aiState){
        case EnemyState::WANDER:
            wanderTimer-=dt;
            if(wanderTimer<=0.f){wanderAngle=(float)(rand()%360)*DEG2RAD; wanderTimer=1.3f+(float)(rand()%80)/50.f;}
            moveWith(cosf(wanderAngle)*speed*0.38f*dt, sinf(wanderAngle)*speed*0.38f*dt);
            if(d<attackRange) aiState=EnemyState::CHASE;
            break;
        case EnemyState::CHASE:
            if(d<meleeRange){ aiState=EnemyState::MELEE_ATTACK; break; }
            if(d<attackRange && fireTimer>=fireInterval){ aiState=EnemyState::RANGED_ATTACK; break; }
            moveWith(nx*speed*dt, ny*speed*dt);
            if(d>attackRange+80.f) aiState=EnemyState::WANDER;
            break;
        case EnemyState::RANGED_ATTACK:
            spawnSpit(pp, (type==EnemyType::ZOMBIE_E)?3:1);
            fireTimer=0.f; aiState=EnemyState::CHASE; break;
        case EnemyState::MELEE_ATTACK:
            if(d>meleeRange+14.f) aiState=EnemyState::CHASE;
            moveWith(nx*speed*0.3f*dt, ny*speed*0.3f*dt); break;
        case EnemyState::HURT:
            if(hurtTimer<=0) aiState=EnemyState::CHASE; break;
        default: break;
    }
}

void Enemy::aiShooter(float dt, std::pair<float,float> pp, float d, float nx, float ny) {
    const float PREF=290.f;
    switch(aiState){
        case EnemyState::WANDER:
            wanderTimer-=dt;
            if(wanderTimer<=0.f){wanderAngle=(float)(rand()%360)*DEG2RAD; wanderTimer=1.5f;}
            moveWith(cosf(wanderAngle)*speed*0.4f*dt, sinf(wanderAngle)*speed*0.4f*dt);
            if(d<attackRange) aiState=EnemyState::CHASE;
            break;
        case EnemyState::CHASE:
            if(d<meleeRange){ aiState=EnemyState::MELEE_ATTACK; break; }
            if(d<PREF-40.f)       moveWith(-nx*speed*0.7f*dt, -ny*speed*0.7f*dt);
            else if(d>PREF+60.f)  moveWith(nx*speed*0.7f*dt,   ny*speed*0.7f*dt);
            else {
                float side=((int)(wanderTimer*8)%2==0)?1.f:-1.f;
                moveWith(-ny*speed*0.55f*dt*side, nx*speed*0.55f*dt*side);
                wanderTimer+=dt;
            }
            if(fireTimer>=fireInterval && d<=attackRange) aiState=EnemyState::RANGED_ATTACK;
            if(d>attackRange+80.f) aiState=EnemyState::WANDER;
            break;
        case EnemyState::RANGED_ATTACK:
            spawnBullet(pp, 1.f, (type==EnemyType::SHOOTER_E)?3:1, 10.f*DEG2RAD);
            fireTimer=0.f; aiState=EnemyState::CHASE; break;
        case EnemyState::MELEE_ATTACK:
            if(d>meleeRange+12.f) aiState=EnemyState::CHASE; break;
        case EnemyState::HURT:
            if(hurtTimer<=0) aiState=EnemyState::CHASE; break;
        default: break;
    }
}

void Enemy::aiBrute(float dt, std::pair<float,float> pp, float d, float nx, float ny) {
    (void)pp;
    if(stompCooldown>0) stompCooldown-=dt;
    switch(aiState){
        case EnemyState::WANDER:
            wanderTimer-=dt;
            if(wanderTimer<=0.f){wanderAngle=(float)(rand()%360)*DEG2RAD; wanderTimer=2.f;}
            moveWith(cosf(wanderAngle)*speed*0.3f*dt, sinf(wanderAngle)*speed*0.3f*dt);
            if(d<attackRange) aiState=EnemyState::CHASE;
            break;
        case EnemyState::CHASE:
            if(d<meleeRange){ aiState=EnemyState::MELEE_ATTACK; break; }
            moveWith(nx*speed*dt, ny*speed*dt);
            if(d>attackRange+100.f) aiState=EnemyState::WANDER;
            if(stompCooldown<=0.f && d<getStompRadius()+30.f){
                aiState=EnemyState::STOMP_WINDUP; stompWindup=0.65f;
            }
            break;
        case EnemyState::STOMP_WINDUP:
            stompWindup-=dt;
            if(stompWindup<=0.f){
                stompFired=true;
                stompCooldown=(type==EnemyType::BRUTE_E)?3.5f:5.5f;
                aiState=EnemyState::CHASE;
            }
            break;
        case EnemyState::MELEE_ATTACK:
            if(d>meleeRange+14.f) aiState=EnemyState::CHASE;
            moveWith(nx*speed*0.28f*dt, ny*speed*0.28f*dt); break;
        case EnemyState::HURT:
            if(hurtTimer<=0) aiState=EnemyState::CHASE; break;
        default: break;
    }
}

void Enemy::aiZombieBoss(float dt, std::pair<float,float> pp, float d, float nx, float ny) {
    if(chargeCooldown>0) chargeCooldown-=dt;

    if(currentPhase==3){
        if(chargeTimer>0){
            chargeTimer-=dt;
            moveWith(chargeDir.x*speed*2.2f*dt, chargeDir.y*speed*2.2f*dt);
            if(fireTimer>=fireInterval*0.4f){ spawnSpit(pp,3); fireTimer=0.f; }
            return;
        }
        if(chargeCooldown<=0.f && d>70.f){
            chargeDir={nx,ny};
            chargeTimer=0.55f;
            chargeCooldown=2.5f;
        }
        moveWith(nx*speed*dt, ny*speed*dt);
        if(d<meleeRange) aiState=EnemyState::MELEE_ATTACK;
        if(fireTimer>=fireInterval*0.5f){ spawnSpit(pp,4); fireTimer=0.f; }
        if(aiState==EnemyState::MELEE_ATTACK) return;
        return;
    }

    if(currentPhase==2){
        moveWith(nx*speed*dt, ny*speed*dt);
        if(d<meleeRange){ aiState=EnemyState::MELEE_ATTACK; }
        if(fireTimer>=fireInterval){ spawnSpit(pp,5); fireTimer=0.f; }
        if(aiState==EnemyState::MELEE_ATTACK){
            if(d>meleeRange+14.f) aiState=EnemyState::CHASE;
            moveWith(nx*speed*0.3f*dt, ny*speed*0.3f*dt);
        }
        return;
    }

    switch(aiState){
        case EnemyState::WANDER:
            wanderTimer-=dt;
            if(wanderTimer<=0.f){wanderAngle=(float)(rand()%360)*DEG2RAD; wanderTimer=1.f;}
            moveWith(cosf(wanderAngle)*speed*0.4f*dt, sinf(wanderAngle)*speed*0.4f*dt);
            if(d<attackRange+100.f) aiState=EnemyState::CHASE;
            break;
        case EnemyState::CHASE:
            if(d<meleeRange){ aiState=EnemyState::MELEE_ATTACK; break; }
            moveWith(nx*speed*dt, ny*speed*dt);
            if(fireTimer>=fireInterval){ spawnSpit(pp,2); fireTimer=0.f; }
            break;
        case EnemyState::MELEE_ATTACK:
            if(d>meleeRange+14.f) aiState=EnemyState::CHASE;
            moveWith(nx*speed*0.3f*dt, ny*speed*0.3f*dt); break;
        case EnemyState::HURT:
            if(hurtTimer<=0) aiState=EnemyState::CHASE; break;
        default: break;
    }
}

void Enemy::aiAssassinBoss(float dt, std::pair<float,float> pp, float d, float nx, float ny) {
    if(teleportCooldown>0) teleportCooldown-=dt;

    if(invisible){
        invisTimer-=dt;
        if(invisTimer<=0.f){
            invisible=false;
            pos=teleportDest; updateHB();
            int shots=(currentPhase>=3)?5:(currentPhase>=2)?3:0;
            if(shots>0) spawnBullet(pp, 1.2f, shots, 14.f*DEG2RAD);
            fireTimer=0.f;
        }
        return;
    }

    if(currentPhase==1){
        switch(aiState){
            case EnemyState::WANDER:
                wanderTimer-=dt;
                if(wanderTimer<=0.f){wanderAngle=(float)(rand()%360)*DEG2RAD; wanderTimer=1.f;}
                moveWith(cosf(wanderAngle)*speed*0.35f*dt, sinf(wanderAngle)*speed*0.35f*dt);
                if(d<attackRange+120.f) aiState=EnemyState::CHASE;
                break;
            case EnemyState::CHASE:
                moveWith(nx*speed*dt, ny*speed*dt);
                if(d<meleeRange){ aiState=EnemyState::MELEE_ATTACK; break; }
                if(fireTimer>=fireInterval && d<=attackRange){
                    spawnBullet(pp, 1.f, 2, 8.f*DEG2RAD);
                    fireTimer=0.f;
                }
                break;
            case EnemyState::MELEE_ATTACK:
                if(d>meleeRange+14.f) aiState=EnemyState::CHASE;
                moveWith(nx*speed*0.35f*dt, ny*speed*0.35f*dt); break;
            case EnemyState::HURT:
                if(hurtTimer<=0) aiState=EnemyState::CHASE; break;
            default: break;
        }
        return;
    }

    if(teleportCooldown<=0.f && d>90.f && aiState!=EnemyState::TELEPORTING){
        float behind=50.f+(float)(rand()%55);
        teleportDest={
            pp.first - nx*behind + (float)((rand()%90)-45),
            pp.second- ny*behind + (float)((rand()%90)-45)
        };
        invisible=true;
        float invDur=(currentPhase==3)?0.25f:0.5f;
        invisTimer=invDur;
        teleportCooldown=(currentPhase==3)?1.8f:3.2f;
        aiState=EnemyState::TELEPORTING;
        return;
    }

    if(!invisible){
        moveWith(nx*speed*dt, ny*speed*dt);
        if(d<meleeRange){ aiState=EnemyState::MELEE_ATTACK; }
        if(fireTimer>=fireInterval && d<=attackRange){
            int shots=(currentPhase==3)?4:2;
            spawnBullet(pp, 1.1f, shots, 10.f*DEG2RAD);
            fireTimer=0.f;
        }
        if(aiState==EnemyState::MELEE_ATTACK){
            if(d>meleeRange+14.f) aiState=EnemyState::CHASE;
            moveWith(nx*speed*0.35f*dt, ny*speed*0.35f*dt);
        }
        if(aiState==EnemyState::TELEPORTING) aiState=EnemyState::CHASE;
    }

    if(aiState==EnemyState::HURT && hurtTimer<=0) aiState=EnemyState::CHASE;
}

void Enemy::aiBruteBoss(float dt, std::pair<float,float> pp, float d, float nx, float ny) {
    (void)pp;
    if(stompCooldown>0) stompCooldown-=dt;
    if(spinCooldown>0)  spinCooldown-=dt;

    if(currentPhase==3){
        if(spinCooldown<=0.f){ spinActive=true; spinTimer=3.5f; spinCooldown=7.f; }
        if(spinActive){
            spinTimer-=dt;
            if(spinTimer<=0.f) spinActive=false;
            wanderAngle+=2.5f*dt;
            moveWith(cosf(wanderAngle)*speed*1.8f*dt, sinf(wanderAngle)*speed*0.4f*dt);
        }
        if(stompCooldown<=0.f && d<getStompRadius()+80.f){
            aiState=EnemyState::STOMP_WINDUP; stompWindup=0.3f;
        }
        if(aiState==EnemyState::STOMP_WINDUP){
            stompWindup-=dt;
            if(stompWindup<=0.f){ stompFired=true; stompCooldown=1.8f; aiState=EnemyState::CHASE; }
            return;
        }
        if(!spinActive) moveWith(nx*speed*2*dt, ny*speed*2*dt);
        if(aiState==EnemyState::HURT && hurtTimer<=0) aiState=EnemyState::CHASE;
        return;
    }

    switch(aiState){
        case EnemyState::WANDER:
            wanderTimer-=dt;
            if(wanderTimer<=0.f){wanderAngle=(float)(rand()%360)*DEG2RAD; wanderTimer=2.f;}
            moveWith(cosf(wanderAngle)*speed*1.f*dt, sinf(wanderAngle)*speed*0.3f*dt);
            if(d<attackRange+100.f) aiState=EnemyState::CHASE;
            break;
        case EnemyState::CHASE:
        case EnemyState::BERSERK_CHARGE:
            if(d<meleeRange){ aiState=EnemyState::MELEE_ATTACK; break; }
            moveWith(nx*speed*dt, ny*speed*1.5*dt);
            if(d>attackRange+150.f) aiState=EnemyState::WANDER;
            if(stompCooldown<=0.f && d<getStompRadius()+50.f){
                aiState=EnemyState::STOMP_WINDUP;
                stompWindup=(currentPhase==2)?0.45f:0.65f;
            }
            break;
        case EnemyState::STOMP_WINDUP:
            stompWindup-=dt;
            if(stompWindup<=0.f){
                stompFired=true;
                stompCooldown=(currentPhase==2)?2.5f:4.f;
                aiState=EnemyState::CHASE;
            }
            break;
        case EnemyState::MELEE_ATTACK:
            if(d>meleeRange+14.f) aiState=EnemyState::CHASE;
            moveWith(nx*speed*0.5f*dt, ny*speed*0.5f*dt); break;
        case EnemyState::HURT:
            if(hurtTimer<=0) aiState=EnemyState::CHASE; break;
        default: break;
    }
}

void Enemy::updateAI(float dt, std::pair<float,float> pp) {
    float d=dist(pp);
    if(hurtTimer>0)     hurtTimer-=dt;
    if(meleeCooldown>0) meleeCooldown-=dt;
    fireTimer+=dt;
    speed=baseSpeed*slowMult;

    float dx=pp.first-pos.first, dy=pp.second-pos.second;
    float dl=sqrtf(dx*dx+dy*dy);
    float nx=0,ny=0; if(dl>0){nx=dx/dl;ny=dy/dl;}

    switch(type){
        case EnemyType::ZOMBIE:
        case EnemyType::ZOMBIE_E:      aiZombie(dt,pp,d,nx,ny); break;
        case EnemyType::SHOOTER:
        case EnemyType::SHOOTER_E:     aiShooter(dt,pp,d,nx,ny); break;
        case EnemyType::BRUTE:
        case EnemyType::BRUTE_E:       aiBrute(dt,pp,d,nx,ny); break;
        case EnemyType::ZOMBIE_BOSS:   aiZombieBoss(dt,pp,d,nx,ny); break;
        case EnemyType::ASSASSIN_BOSS: aiAssassinBoss(dt,pp,d,nx,ny); break;
        case EnemyType::BRUTE_BOSS:    aiBruteBoss(dt,pp,d,nx,ny); break;
    }
}

void Enemy::update(float dt, std::pair<float,float> pp) {
    if(hp<=0){aiState=EnemyState::DEAD; return;}

    if(fabsf(knockbackVel.x)>0.5f || fabsf(knockbackVel.y)>0.5f){
        moveWith(knockbackVel.x*dt, knockbackVel.y*dt);
        float fr=1.f-9.f*dt; if(fr<0)fr=0;
        knockbackVel.x*=fr; knockbackVel.y*=fr;
        if(fabsf(knockbackVel.x)<1.f) knockbackVel.x=0;
        if(fabsf(knockbackVel.y)<1.f) knockbackVel.y=0;
    }

    updatePhase();
    updateAI(dt,pp);
    updateHB();
}

Color Enemy::bodyColor() const {
    switch(type){
        case EnemyType::ZOMBIE:        return {55,100,45,255};
        case EnemyType::ZOMBIE_E:      return {35,145,35,255};
        case EnemyType::ZOMBIE_BOSS:   return {0,92,50,255};  // vert foncé
        case EnemyType::SHOOTER:       return {40,80,155,255};
        case EnemyType::SHOOTER_E:     return {20,50,200,255};
        case EnemyType::BRUTE:         return {105,55,25,255};
        case EnemyType::BRUTE_E:       return {140,75,30,255};
        case EnemyType::BRUTE_BOSS:    return {80,35,10,255};  // brun très sombre
        case EnemyType::ASSASSIN_BOSS: return {20,15,38,255};
    }
    return GREEN;
}

void Enemy::draw() const {
    if(hp<=0) return;

    if(invisible){
        DrawCircleV({pos.first,pos.second}, 20.f*scale, Fade({160,80,255,255},0.08f));
        return;
    }

    int cx=(int)pos.first, cy=(int)pos.second;
    float r=20.f*scale;
    Color body=bodyColor();

    if(hurtTimer>0){
        float t=hurtTimer/0.15f;
        body={(unsigned char)(body.r+(255-body.r)*t),
              (unsigned char)(body.g+(255-body.g)*t),
              (unsigned char)(body.b+(255-body.b)*t),255};
    }
    // Slow tint bleuté
    if(slowMult<0.85f){
        float bl=1.f-slowMult;
        body={(unsigned char)(body.r*(1-bl)+50*bl),
              (unsigned char)(body.g*(1-bl)+180*bl),
              (unsigned char)(body.b*(1-bl)+255*bl),255};
    }

    DrawCircleV({(float)cx,(float)cy}, r, body);

    if(type==EnemyType::ZOMBIE){
        DrawCircleV({cx+r*0.30f,cy-r*0.22f},3.5f*scale,{200,40,40,255});
        DrawCircleV({cx-r*0.30f,cy-r*0.22f},3.5f*scale,{200,40,40,255});
    }
    else if(type==EnemyType::ZOMBIE_E){
        DrawCircleLines(cx,cy,r+4,Fade({60,220,60,255},0.55f));
        DrawCircleV({cx+r*0.30f,cy-r*0.22f},4.5f*scale,{255,60,60,255});
        DrawCircleV({cx-r*0.30f,cy-r*0.22f},4.5f*scale,{255,60,60,255});
    }
    else if(type==EnemyType::ZOMBIE_BOSS){
        // Aura violette + phase indicateur
        DrawCircleLines(cx,cy,r+7,  Fade({200,0,160,255},0.6f));
        DrawCircleLines(cx,cy,r+12, Fade({200,0,160,255},0.25f));
        if(currentPhase>=2) DrawCircleLines(cx,cy,r+18,Fade({255,80,200,255},0.12f));
        DrawText(TextFormat("P%d",currentPhase), cx-8, cy-(int)r-24, 14, {255,80,200,255});
        // Yeux rouges intenses
        DrawCircleV({cx+r*0.28f,cy-r*0.20f},5.f*scale,{255,30,30,255});
        DrawCircleV({cx-r*0.28f,cy-r*0.20f},5.f*scale,{255,30,30,255});
        // charge indicator
        if(currentPhase==3 && chargeTimer>0){
            float prog=chargeTimer/0.55f;
            DrawCircleV({(float)cx,(float)cy},r*1.4f,Fade({255,100,0,255},prog*0.4f));
        }
    }
    else if(type==EnemyType::SHOOTER){
        DrawCircleLines(cx,cy,r+3,Fade({100,160,255,255},0.5f));
        DrawLineEx({(float)cx,(float)cy},{cx+r*1.5f,(float)cy},3.f,{120,180,255,200});
        DrawCircleV({cx+r*0.28f,cy-r*0.18f},3.f*scale,{200,230,255,255});
        DrawCircleV({cx-r*0.28f,cy-r*0.18f},3.f*scale,{200,230,255,255});
    }
    else if(type==EnemyType::SHOOTER_E){
        DrawCircleLines(cx,cy,r+4,Fade({120,180,255,255},0.55f));
        DrawCircleLines(cx,cy,r+8,Fade({120,180,255,255},0.25f));
        DrawLineEx({(float)cx,(float)cy},{cx+r*1.5f,(float)cy},4.f,{150,220,255,220});
        DrawCircleV({cx+r*0.28f,cy-r*0.18f},3.5f*scale,{220,240,255,255});
        DrawCircleV({cx-r*0.28f,cy-r*0.18f},3.5f*scale,{220,240,255,255});
    }
    else if(type==EnemyType::BRUTE || type==EnemyType::BRUTE_E){
        float ringAlpha=(type==EnemyType::BRUTE_E)?0.65f:0.55f;
        DrawCircleLines(cx,cy,r+6,Fade({180,100,40,255},ringAlpha));
        if(type==EnemyType::BRUTE_E)
            DrawCircleLines(cx,cy,r+12,Fade({200,120,50,255},0.28f));
        if(aiState==EnemyState::STOMP_WINDUP && stompWindup>0){
            float maxWU=(type==EnemyType::BRUTE_E)?0.65f:0.65f;
            float prog=1.f-stompWindup/maxWU;
            DrawCircleV({(float)cx,(float)cy}, r+prog*getStompRadius()*0.6f,
                         Fade({255,180,0,255},prog*0.28f));
        }
        float eyeS=(type==EnemyType::BRUTE_E)?6.f:5.f;
        DrawCircleV({cx+r*0.28f,cy-r*0.18f},eyeS*scale,{255,70,0,255});
        DrawCircleV({cx-r*0.28f,cy-r*0.18f},eyeS*scale,{255,70,0,255});
    }
    else if(type==EnemyType::BRUTE_BOSS){
        DrawCircleLines(cx,cy,r+8,  Fade({180,80,20,255},0.65f));
        DrawCircleLines(cx,cy,r+15, Fade({180,80,20,255},0.28f));
        if(currentPhase>=2) DrawCircleLines(cx,cy,r+22,Fade({255,120,30,255},0.12f));
        DrawText(TextFormat("P%d",currentPhase), cx-8, cy-(int)r-26, 14, {255,140,30,255});
        if(aiState==EnemyState::STOMP_WINDUP){
            float maxWU=(currentPhase==2)?0.45f:0.65f;
            float prog=1.f-stompWindup/maxWU;
            DrawCircleV({(float)cx,(float)cy}, r+prog*getStompRadius()*0.7f,
                         Fade({255,200,0,255},prog*0.35f));
        }
        if(spinActive){
            float pulse=0.6f+0.4f*sinf((float)GetTime()*8.f);
            DrawCircleV({(float)cx,(float)cy}, getSpinRadius()*pulse,
                         Fade({255,100,0,255},0.18f));
            DrawCircleLines(cx,cy,getSpinRadius(),Fade({255,150,50,255},0.5f));
        }
        DrawCircleV({cx+r*0.28f,cy-r*0.20f},6.f*scale,{255,50,0,255});
        DrawCircleV({cx-r*0.28f,cy-r*0.20f},6.f*scale,{255,50,0,255});
    }
    else if(type==EnemyType::ASSASSIN_BOSS){
        DrawCircleLines(cx,cy,r+6,  Fade({170,40,255,255},0.55f));
        DrawCircleLines(cx,cy,r+10, Fade({170,40,255,255},0.20f));
        if(currentPhase>=2) DrawCircleLines(cx,cy,r+15,Fade({255,80,255,255},0.10f));
        DrawText(TextFormat("P%d",currentPhase), cx-8, cy-(int)r-22, 14, {200,80,255,255});
        DrawCircleV({cx+r*0.28f,cy-r*0.20f},4.f*scale,{255,40,200,255});
        DrawCircleV({cx-r*0.28f,cy-r*0.20f},4.f*scale,{255,40,200,255});
        // Flash pré-téléport
        if(teleportCooldown>0.f && teleportCooldown<0.6f){
            float a=(0.6f-teleportCooldown)/0.6f;
            DrawCircleV({(float)cx,(float)cy},r+4.f,Fade({255,200,255,255},a*0.5f));
        }
    }

    // Barre de vie (plus large pour les boss)
    float pct=(float)hp/(float)maxHp;
    int bw=(int)(52*scale), bx=cx-bw/2, by=cy-(int)r-13;
    DrawRectangle(bx,by,bw,5,DARKGRAY);
    Color hcol=(pct>0.6f)?GREEN:(pct>0.3f)?ORANGE:RED;
    DrawRectangle(bx,by,(int)(bw*pct),5,hcol);
    DrawRectangleLines(bx,by,bw,5,WHITE);
}


void Enemy::takeDamage(int dmg){
    if(invisible) dmg=(int)(dmg*0.55f);
    hp-=dmg; if(hp<0)hp=0;
    hurtTimer=0.15f;
    if(aiState!=EnemyState::DEAD && aiState!=EnemyState::TELEPORTING)
        aiState=EnemyState::HURT;
}
void Enemy::applyKnockback(Vector2 dir, float force){
    if(isBossEnemy()) force*=0.15f;
    else if(type==EnemyType::BRUTE||type==EnemyType::BRUTE_E) force*=0.35f;
    knockbackVel.x+=dir.x*force; knockbackVel.y+=dir.y*force;
}
std::vector<std::shared_ptr<Projectile>> Enemy::flushProjectiles(){
    auto r=pending; pending.clear(); return r;
}
