#pragma once
#include <raylib.h>
#include <utility>
#include <cmath>

struct HealthPickup {
    std::pair<float,float> pos;
    int   amount;
    float bobTimer;
    float lifetime;
    bool  active;

    HealthPickup(std::pair<float,float> p, int amt = 5)
        : pos(p), amount(amt), bobTimer(0.f), lifetime(15.f), active(true) {}

    void update(float dt) {
        bobTimer += dt * 2.8f;
        lifetime -= dt;
        if (lifetime <= 0.f) active = false;
    }

    float bobY() const { return sinf(bobTimer) * 4.f; }

    void draw() const {
        if (!active) return;
        float bx = pos.first, by = pos.second + bobY();
        float alpha = (lifetime < 3.f) ? lifetime / 3.f : 1.f;
        DrawCircleV({bx,by}, 15.f, Fade({255,60,60,255}, alpha*0.22f));
        DrawCircleV({bx,by}, 11.f, Fade({220,50,50,255}, alpha*0.95f));
        DrawCircleLines((int)bx,(int)by, 11.f, Fade({255,140,140,255}, alpha));
        // Cross blanc
        int ix=(int)bx, iy=(int)by;
        DrawRectangle(ix-7, iy-2, 14, 4, Fade(WHITE, alpha));
        DrawRectangle(ix-2, iy-7, 4, 14, Fade(WHITE, alpha));
        // "+20" label
        DrawText("+20", ix-8, iy+14, 11, Fade({255,200,200,255}, alpha));
    }
};
