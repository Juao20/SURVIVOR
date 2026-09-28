#pragma once

#include <raylib.h>
#include <utility>
#include <string>


enum class AmmoType { RIFLE, LASER, BOMB, SLOW};

struct AmmoPickup
{
    std::pair<float, float> pos;
    AmmoType type;
    int      amount;
    float    bobTimer;
    bool     active;

    AmmoPickup(std::pair<float, float> position, AmmoType t, int amt)
        : pos(position), type(t), amount(amt), bobTimer(0.0f), active(true)
    {}

    Color getColor() const
    {
        switch (type) {
            case AmmoType::RIFLE: return {255, 220, 30, 255};
            case AmmoType::LASER: return {80, 180, 255, 255};
            case AmmoType::BOMB:  return {255, 120, 20, 255};
            case AmmoType::SLOW:  return {120, 200, 255, 255};
        }
        return WHITE;
    }

    const char* getLabel() const
    {
        switch (type) {
            case AmmoType::RIFLE: return "AMMO";
            case AmmoType::LASER: return "NRG";
            case AmmoType::BOMB:  return "BOMB";
            case AmmoType::SLOW:  return "CRYO";
        }
        return "?";
    }

    char getIcon() const
    {
        switch (type) {
            case AmmoType::RIFLE: return 'R';
            case AmmoType::LASER: return 'L';
            case AmmoType::BOMB:  return 'B';
            case AmmoType::SLOW:  return 'S';
        }
        return '?';
    }

    void update(float deltaTime)
    {
        bobTimer += deltaTime * 3.0f;
    }

    float getBobOffset() const
    {
        return sinf(bobTimer) * 3.0f;
    }

    void draw() const
    {
        if (!active) return;

        float bx = pos.first;
        float by = pos.second + getBobOffset();
        Color col = getColor();

        // Outer glow ring
        DrawCircleV({bx, by}, 14.0f, Fade(col, 0.18f));
        // Body
        DrawCircleV({bx, by}, 10.0f, Fade(col, 0.85f));
        // Border
        DrawCircleLines((int)bx, (int)(by), 10.0f, col);

        // Icon letter
        const char* lbl = getLabel();
        int tw = MeasureText(lbl, 9);
        DrawText(lbl, (int)(bx - tw / 2), (int)(by - 5), 9, {10, 10, 10, 255});
    }
};
