#pragma once

#include <raylib.h>
#include <string>
#include <vector>
#include "Player.hpp"
#include "LaserWeapon.hpp"
#include "BombWeapon.hpp"
#include "ScaleContext.hpp"


class HUD
{
private:
    int virtW, virtH;

public:
    HUD(int vw = 1920, int vh = 1080) : virtW(vw), virtH(vh) {}
    ~HUD() = default;

    void drawScaled(const Player* player, int wave,
                    bool bossAlert, float bossAlertTimer,
                    const ScaleContext& sc) const;

    void draw(const Player* player, int wave, bool bossAlert, float bossAlertTimer) const;

private:
    void drawHealthBar(int hp, int maxHp, int rx, int ry, int rw, int rh) const;
    void drawAmmoInfo(const Player* player, int rx, int ry, float fs) const;
    void drawWeaponSlots(const Player* player, const ScaleContext& sc) const;
    void drawLaserCharge(const Player* player, const ScaleContext& sc) const;
    void drawBombCooldown(const Player* player, const ScaleContext& sc) const;
    void drawWaveInfo(int wave, bool bossAlert, float bossAlertTimer,
                      int screenW, const ScaleContext& sc) const;
};


void HUD::drawScaled(const Player* player, int wave,
                     bool bossAlert, float bossAlertTimer,
                     const ScaleContext& sc) const
{
    if (!player) return;

    int sw = GetScreenWidth();
    // int sh = GetScreenHeight();
    float s = sc.scale;

    // ── Top bar ───────────────────────────────────────────────────────────────
    int barH = (int)(64 * s);
    DrawRectangle(0, 0, sw, barH, Fade(BLACK, 0.65f));

    // Health bar
    int bx = (int)(16 * s), by = (int)(12 * s);
    int bw = (int)(220 * s), bh = (int)(18 * s);
    drawHealthBar(player->getHp(), player->getMaxHp(), bx, by, bw, bh);
    int fs = (int)(13 * s);
    if (fs < 8) fs = 8;
    DrawText(TextFormat("%d / %d", player->getHp(), player->getMaxHp()),
             bx + bw + (int)(8 * s), by + 1, fs, WHITE);

    // Ammo info
    drawAmmoInfo(player, bx, by + (int)(26 * s), s);

    // Wave + boss alert centred
    drawWaveInfo(wave, bossAlert, bossAlertTimer, sw, sc);

    // Weapon slots (right side)
    drawWeaponSlots(player, sc);

    // Bottom charge / cooldown bars
    drawLaserCharge(player, sc);
    drawBombCooldown(player, sc);
}


void HUD::draw(const Player* player, int wave, bool bossAlert, float bossAlertTimer) const
{
    ScaleContext fake; fake.scale = 1.0f; fake.offsetX = 0; fake.offsetY = 0;
    drawScaled(player, wave, bossAlert, bossAlertTimer, fake);
}


void HUD::drawHealthBar(int hp, int maxHp, int rx, int ry, int rw, int rh) const
{
    float pct = (maxHp > 0) ? (float)hp / maxHp : 0.0f;
    Color hcol = (pct > 0.6f) ? (Color){50, 200, 60, 255}
               : (pct > 0.3f) ? (Color){230, 160, 20, 255}
               :                (Color){220, 40, 40, 255};

    // Background
    DrawRectangle(rx, ry, rw, rh, {40, 40, 40, 255});
    // Fill
    DrawRectangle(rx, ry, (int)(rw * pct), rh, hcol);
    // Border
    DrawRectangleLinesEx({(float)rx, (float)ry, (float)rw, (float)rh}, 1.5f, {180, 180, 180, 255});

    int lblW = MeasureText("HP", 10);
    DrawText("HP", rx - lblW - 4, ry + rh / 2 - 5, 10, {180, 180, 180, 255});
}

void HUD::drawAmmoInfo(const Player* player, int rx, int ry, float s) const
{
    auto w = player->getCurrentWeapon();
    if (!w) return;

    int fs = (int)(13 * s); if (fs < 8) fs = 8;
    int ammo    = w->getCurrentAmmo();
    int mag     = w->getMagazineSize();
    int reserve = w->getMaxAmmo();
    Color col   = (ammo == 0) ? RED : (ammo <= 5) ? ORANGE : WHITE;

    DrawText(w->getName().c_str(),  rx, ry, fs, SKYBLUE);
    DrawText(TextFormat("%d / %d  [%d]", ammo, mag, reserve),
             rx + (int)(120 * s), ry, fs, col);

    if (w->getIsReloading())
        DrawText("RELOADING...", rx + (int)(290 * s), ry, fs, YELLOW);
}

void HUD::drawWeaponSlots(const Player* player, const ScaleContext& sc) const
{
    int n = player->getWeaponCount();
    if (n == 0) return;

    float s   = sc.scale;
    int slotW = (int)(58 * s), slotH = (int)(52 * s), gap = (int)(6 * s);
    int totalW = n * slotW + (n - 1) * gap;
    int startX = GetScreenWidth() - totalW - (int)(14 * s);
    int startY = (int)(6 * s);

    for (int i = 0; i < n; i++) {
        auto w = player->getWeapon(i);
        if (!w) continue;

        int  sx     = startX + i * (slotW + gap);
        bool active = (i == player->getCurrentWeaponIndex());
        Color border = active ? WHITE : (Color){70, 80, 100, 200};
        Color bg     = active ? (Color){50, 55, 90, 220} : (Color){20, 22, 34, 180};

        DrawRectangle(sx, startY, slotW, slotH, bg);
        DrawRectangleLinesEx({(float)sx, (float)startY, (float)slotW, (float)slotH},
                             active ? 2.0f : 1.0f, border);

        // Colour dot
        // DrawCircle(sx + slotW / 2, startY + (int)(18 * s), (int)(7 * s), w->getColor());

        // Weapon icon
        DrawTextureEx(w->getIcon(), { (float)(sx + slotW / 2 - w->getIcon().width / 2), (float)(startY + slotH / 2 - w->getIcon().height / 2) }, 0.0f, 1.0f, w->getColor());
        // Key number hint
        int fs = (int)(11 * s); if (fs < 7) fs = 7;
        DrawText(TextFormat("%d", i + 1), sx + (int)(3 * s), startY + (int)(2 * s), fs, {160, 160, 180, 255});

        // Ammo mini-bar
        float apct = (w->getMagazineSize() > 0) ? (float)w->getCurrentAmmo() / w->getMagazineSize() : 0.0f;
        int bw = slotW - (int)(8 * s);
        DrawRectangle(sx + (int)(4 * s), startY + slotH - (int)(10 * s), bw, (int)(4 * s), {40, 40, 40, 255});
        DrawRectangle(sx + (int)(4 * s), startY + slotH - (int)(10 * s),
                      (int)(bw * apct), (int)(4 * s), (apct < 0.2f) ? RED : w->getColor());
    }
}

void HUD::drawLaserCharge(const Player* player, const ScaleContext& sc) const
{
    auto w = player->getCurrentWeapon();
    if (!w) return;
    auto laser = std::dynamic_pointer_cast<LaserWeapon>(w);
    if (!laser) return;

    float s   = sc.scale;
    int sw    = GetScreenWidth();
    int sh    = GetScreenHeight();
    int barW  = (int)(200 * s);
    int barH  = (int)(12 * s); if (barH < 6) barH = 6;
    int bx    = sw / 2 - barW / 2;
    int by    = sh - (int)(28 * s);
    int fs    = (int)(12 * s); if (fs < 8) fs = 8;

    if (laser->isOnCooldown()) {
        float pct = laser->getCooldownPct();
        DrawRectangle(bx, by, barW, barH, {40, 40, 40, 200});
        DrawRectangle(bx, by, (int)(barW * (1.0f - pct)), barH, {100, 180, 255, 255});
        DrawRectangleLinesEx({(float)bx, (float)by, (float)barW, (float)barH}, 1.0f, WHITE);
        int lw = MeasureText("COOLDOWN", fs);
        DrawText("COOLDOWN", sw / 2 - lw / 2, by - fs - 4, fs, {100, 180, 255, 200});
        return;
    }

    if (!laser->getIsCharging()) return;

    float charge = laser->getChargeLevel();
    Color chargeCol = (charge >= 1.0f)  ? WHITE
                    : (charge >= 0.66f) ? ORANGE
                    :                     (Color){255, 80, 80, 255};

    DrawRectangle(bx, by, barW, barH, {40, 40, 40, 200});
    DrawRectangle(bx, by, (int)(barW * charge), barH, chargeCol);
    DrawRectangleLinesEx({(float)bx, (float)by, (float)barW, (float)barH}, 1.5f, WHITE);

    const char* lbl = (charge >= 1.0f)  ? "FULL CHARGE"
                    : (charge >= 0.66f) ? "STRONG"
                    : (charge >= 0.33f) ? "PARTIAL"
                    :                    "CHARGING...";
    int lw = MeasureText(lbl, fs);
    DrawText(lbl, sw / 2 - lw / 2, by - fs - 4, fs, chargeCol);
}

void HUD::drawBombCooldown(const Player* player, const ScaleContext& sc) const
{
    auto w = player->getCurrentWeapon();
    if (!w) return;
    auto bomb = std::dynamic_pointer_cast<BombWeapon>(w);
    if (!bomb || !bomb->isOnCooldown()) return;

    float s   = sc.scale;
    int sw    = GetScreenWidth();
    int sh    = GetScreenHeight();
    int barW  = (int)(160 * s);
    int barH  = (int)(12 * s); if (barH < 6) barH = 6;
    int bx    = sw / 2 - barW / 2;
    int by    = sh - (int)(28 * s);
    int fs    = (int)(12 * s); if (fs < 8) fs = 8;

    float pct = bomb->getCooldownPct();
    DrawRectangle(bx, by, barW, barH, {40, 40, 40, 200});
    DrawRectangle(bx, by, (int)(barW * (1.0f - pct)), barH, {255, 180, 0, 255});
    DrawRectangleLinesEx({(float)bx, (float)by, (float)barW, (float)barH}, 1.0f, WHITE);
    int lw = MeasureText("BOMB COOLDOWN", fs);
    DrawText("BOMB COOLDOWN", sw / 2 - lw / 2, by - fs - 4, fs, {255, 180, 0, 200});
}

void HUD::drawWaveInfo(int wave, bool bossAlert, float bossAlertTimer,
                        int sw, const ScaleContext& sc) const
{
    float s  = sc.scale;
    int  fs1 = (int)(24 * s); if (fs1 < 12) fs1 = 12;
    int  by  = (int)(12 * s);

    const char* wt = TextFormat("WAVE  %d", wave);
    int tw = MeasureText(wt, fs1);
    DrawText(wt, sw / 2 - tw / 2, by, fs1, {220, 220, 100, 255});

    if (bossAlert) {
        float alpha  = 0.5f + 0.5f * sinf(bossAlertTimer * 6.0f);
        int   fs2    = (int)(22 * s); if (fs2 < 11) fs2 = 11;
        const char* at = "! BOSS !";
        int   aw = MeasureText(at, fs2);
        DrawRectangle(sw / 2 - aw / 2 - (int)(10 * s), by + fs1 + 2, aw + (int)(20 * s), fs2 + 4,
                      Fade({180, 0, 0, 255}, alpha * 0.5f));
        DrawText(at, sw / 2 - aw / 2, by + fs1 + 4, fs2, Fade({255, 60, 60, 255}, alpha));
    }
}
