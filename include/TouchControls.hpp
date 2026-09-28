#pragma once

#include <raylib.h>
#include <cmath>

// On-screen touch controls for the Android build: twin analog sticks — the
// LEFT stick only moves the character, the RIGHT stick independently aims
// AND fires (push out to aim+charge/hold-fire, let go back to center to
// release/throw), so you can e.g. back away while still firing forward.
// Stick deflection also drives thrown-weapon trajectories (distance scales
// with how far the stick is pushed). Plus RELOAD and weapon-switch buttons,
// and menu/back. Entirely inert on non-Android builds (all queries return
// neutral values, update()/draw() are no-ops), so it is always safe to call
// unconditionally from Game.hpp.
class TouchControls
{
private:
    // Left stick — movement only.
    Vector2 moveCenter = {0,0}, moveKnob = {0,0};
    float   stickRadius = 65.0f, stickRange = 85.0f;
    int     moveTouchId = -1;
    Vector2 moveDir = {0,0}; // normalised, length in [0,1]

    // Right stick — aim + fire/charge/throw trigger.
    Vector2 aimCenter = {0,0}, aimKnob = {0,0};
    int     aimTouchId = -1;
    float   curAimMag = 0.0f;         // this frame's raw deflection, 0 when centered/released
    bool    aimDownFlag = false, aimWasDown = false, aimReleasedFlag = false;
    Vector2 lastAimDir = {1,0};       // sticky: last direction while deflected past the deadzone
    float   lastAimMag = 0.0f;        // sticky: last magnitude while deflected — read at release time
    static constexpr float maxThrowDistance = 700.0f;

    bool    bWasDown = false, bTapThisFrame = false;   // RELOAD
    bool    cWasDown = false, cTapThisFrame = false;   // SWAP weapon
    Vector2 btnBCenter = {0,0}, btnCCenter = {0,0};
    float   btnRadiusBC = 46.0f;

    Rectangle menuRect = {0,0,0,0}, backRect = {0,0,0,0};
    bool    menuWasDown = false, backWasDown = false;
    bool    menuTapThisFrame = false, backTapThisFrame = false;

    Texture2D iconC = {0,0,0,0,0}, iconAim = {0,0,0,0,0};

#if defined(PLATFORM_ANDROID)
    static bool pointInCircle(Vector2 p, Vector2 c, float r) {
        float dx = p.x - c.x, dy = p.y - c.y;
        return (dx*dx + dy*dy) <= r*r;
    }
    static void drawIconOrLabel(Texture2D tex, Vector2 center, float radius, const char* label) {
        if (tex.id != 0) {
            float scale = (radius * 1.1f) / (float)((tex.width > tex.height) ? tex.width : tex.height);
            float w = tex.width * scale, h = tex.height * scale;
            DrawTexturePro(tex, {0,0,(float)tex.width,(float)tex.height},
                {center.x - w/2, center.y - h/2, w, h}, {0,0}, 0.0f, WHITE);
        } else {
            int tw = MeasureText(label, 20);
            DrawText(label, (int)center.x - tw/2, (int)center.y - 10, 20, DARKGRAY);
        }
    }
#endif

public:
    void update()
    {
#if defined(PLATFORM_ANDROID)
        int sw = GetScreenWidth(), sh = GetScreenHeight();
        moveCenter = { 150.0f, sh - 170.0f };
        aimCenter  = { (float)sw - 190.0f, sh - 170.0f };

        btnCCenter = { (float)sw - 190.0f, sh - 380.0f };
        btnBCenter = { (float)sw - 190.0f, sh - 490.0f };

        menuRect = { (float)sw - 140.0f, 20.0f, 120.0f, 50.0f };
        backRect = { 20.0f, 20.0f, 120.0f, 50.0f };

        int count = GetTouchPointCount();

        bool moveTouchStillActive = false;
        bool aimTouchStillActive = false;
        bool bSeen = false, cSeen = false, menuSeen = false, backSeen = false;

        for (int i = 0; i < count; i++) {
            int id = GetTouchPointId(i);
            Vector2 p = GetTouchPosition(i);

            if (moveTouchId == -1 && aimTouchId != id && pointInCircle(p, moveCenter, stickRadius * 1.6f)) {
                moveTouchId = id;
            }
            if (id == moveTouchId) {
                moveTouchStillActive = true;
                Vector2 d = { p.x - moveCenter.x, p.y - moveCenter.y };
                float len = sqrtf(d.x*d.x + d.y*d.y);
                if (len > stickRange) { d.x = d.x / len * stickRange; d.y = d.y / len * stickRange; len = stickRange; }
                moveKnob = { moveCenter.x + d.x, moveCenter.y + d.y };
                moveDir  = (len > 1.0f) ? Vector2{ d.x / stickRange, d.y / stickRange } : Vector2{0,0};
            }

            if (aimTouchId == -1 && moveTouchId != id && pointInCircle(p, aimCenter, stickRadius * 1.6f)) {
                aimTouchId = id;
            }
            if (id == aimTouchId) {
                aimTouchStillActive = true;
                Vector2 d = { p.x - aimCenter.x, p.y - aimCenter.y };
                float len = sqrtf(d.x*d.x + d.y*d.y);
                if (len > stickRange) { d.x = d.x / len * stickRange; d.y = d.y / len * stickRange; len = stickRange; }
                aimKnob = { aimCenter.x + d.x, aimCenter.y + d.y };
                curAimMag = len / stickRange;
                // Sticky direction/magnitude: only updated while meaningfully
                // deflected, so they still hold the last real aim when the
                // stick snaps back to center on release (needed to compute
                // the throw on that very same frame).
                if (curAimMag > 0.12f) {
                    lastAimDir = { d.x / len, d.y / len };
                    lastAimMag = curAimMag;
                }
            }

            // Ignore touches already captured by a stick so a drag that sweeps
            // over B/C/menu/back doesn't also trigger them.
            if (id != moveTouchId && id != aimTouchId) {
                if (pointInCircle(p, btnBCenter, btnRadiusBC)) bSeen = true;
                if (pointInCircle(p, btnCCenter, btnRadiusBC)) cSeen = true;
                if (CheckCollisionPointRec(p, menuRect)) menuSeen = true;
                if (CheckCollisionPointRec(p, backRect)) backSeen = true;
            }
        }

        if (!moveTouchStillActive) { moveTouchId = -1; moveKnob = moveCenter; moveDir = {0,0}; }
        if (!aimTouchStillActive) { aimTouchId = -1; aimKnob = aimCenter; curAimMag = 0.0f; }

        aimWasDown = aimDownFlag;
        aimDownFlag = curAimMag > 0.12f;
        aimReleasedFlag = aimWasDown && !aimDownFlag;

        bTapThisFrame = bSeen && !bWasDown;
        bWasDown = bSeen;
        cTapThisFrame = cSeen && !cWasDown;
        cWasDown = cSeen;

        menuTapThisFrame = menuSeen && !menuWasDown;
        backTapThisFrame = backSeen && !backWasDown;
        menuWasDown = menuSeen;
        backWasDown = backSeen;
#endif
    }

    // Icons: aim = current weapon (shown faintly under the aim stick knob),
    // c = next weapon (shown on the SWAP button). Call once per frame before
    // draw(). Safe to pass a zero-id texture (falls back to text).
    void setIcons(Texture2D aim, Texture2D c) { iconAim = aim; iconC = c; }

    void draw() const
    {
#if defined(PLATFORM_ANDROID)
        DrawCircleV(moveCenter, stickRadius + stickRange, Fade(GRAY, 0.25f));
        DrawCircleV(moveCenter, stickRadius, Fade(LIGHTGRAY, 0.35f));
        DrawCircleV(moveKnob, stickRadius * 0.6f, Fade(WHITE, 0.55f));

        DrawCircleV(aimCenter, stickRadius + stickRange, Fade(MAROON, 0.20f));
        DrawCircleV(aimCenter, stickRadius, Fade(ORANGE, 0.30f));
        drawIconOrLabel(iconAim, aimCenter, stickRadius * 0.9f, "AIM");
        DrawCircleV(aimKnob, stickRadius * 0.6f, Fade(aimDownFlag ? WHITE : ORANGE, 0.65f));

        DrawCircleV(btnBCenter, btnRadiusBC, Fade(bWasDown ? WHITE : LIGHTGRAY, bWasDown ? 0.6f : 0.35f));
        drawIconOrLabel({0,0,0,0,0}, btnBCenter, btnRadiusBC, "RLD");

        DrawCircleV(btnCCenter, btnRadiusBC, Fade(cWasDown ? WHITE : LIGHTGRAY, cWasDown ? 0.6f : 0.35f));
        drawIconOrLabel(iconC, btnCCenter, btnRadiusBC, "SWAP");

        DrawRectangleRounded(menuRect, 0.3f, 4, Fade(LIGHTGRAY, 0.35f));
        DrawText("MENU", (int)menuRect.x + 18, (int)menuRect.y + 14, 18, DARKGRAY);
        DrawRectangleRounded(backRect, 0.3f, 4, Fade(LIGHTGRAY, 0.35f));
        DrawText("BACK", (int)backRect.x + 18, (int)backRect.y + 14, 18, DARKGRAY);
#endif
    }

    // Movement delta (already scaled by speed*dt), zero when the left stick
    // is idle/inactive. Fully independent of the aim stick, so you can move
    // one way while aiming/firing another.
    Vector2 getMoveDelta(float speed, float dt) const
    {
#if defined(PLATFORM_ANDROID)
        return { moveDir.x * speed * dt, moveDir.y * speed * dt };
#else
        (void)speed; (void)dt;
        return {0,0};
#endif
    }

    bool isActive() const
    {
#if defined(PLATFORM_ANDROID)
        return true;
#else
        return false;
#endif
    }

    // Synthesises a "virtual mouse" world point in the aim stick's direction —
    // used for hitscan weapons (rifle/laser).
    Vector2 computeAimWorldPos(Vector2 playerPos) const
    {
        return { playerPos.x + lastAimDir.x * 400.0f, playerPos.y + lastAimDir.y * 400.0f };
    }

    // Aim point for thrown weapons (bomb/slow orb): how far the aim stick was
    // pushed sets the throw distance, its direction sets the arc — read at
    // release time via the sticky lastAimDir/lastAimMag (still valid even
    // though the stick has already snapped back to center this frame).
    Vector2 getThrowAimWorldPos(Vector2 playerPos) const
    {
        float dist = lastAimMag * maxThrowDistance;
        if (dist < 150.0f) dist = 200.0f;
        return { playerPos.x + lastAimDir.x * dist, playerPos.y + lastAimDir.y * dist };
    }

    bool aimDown()          const { return aimDownFlag; }
    bool aimReleased()      const { return aimReleasedFlag; }
    bool reloadTapped()     const { return bTapThisFrame; }
    bool buttonCTapped()    const { return cTapThisFrame; }
    bool menuTapped()       const { return menuTapThisFrame; }
    bool backTapped()       const { return backTapThisFrame; }
};
