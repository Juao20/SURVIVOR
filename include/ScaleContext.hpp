#pragma once

#include <raylib.h>
#include <cmath>

// ─────────────────────────────────────────────────────────────────────────────
//  ScaleContext — Virtual resolution system
//
//  The game world is designed at a fixed "virtual" resolution (VIRT_W × VIRT_H).
//  At runtime, we compute a uniform scale factor so the virtual canvas fits
//  inside the real window without distortion, with letterbox bars if needed.
//
//  Usage:
//    ScaleContext sc;
//    sc.update(GetScreenWidth(), GetScreenHeight());
//
//    // Game draw loop:
//    BeginDrawing();
//      ClearBackground(BLACK);
//      BeginMode2D(sc.getCamera());   // applies offset + scale
//        // draw everything in virtual coords
//      EndMode2D();
//      // draw HUD in virtual coords with sc.s() scale multiplier
//    EndDrawing();
//
//  World coords are always in [0..VIRT_W] x [0..VIRT_H].
//  Mouse coords must be transformed: sc.worldMouse()
// ─────────────────────────────────────────────────────────────────────────────

struct ScaleContext
{
    // ── Virtual (design) resolution ──────────────────────────────────────────
    static constexpr float VIRT_W = 1920.0f;
    static constexpr float VIRT_H = 1080.0f;

    float scale         = 1.0f;    // uniform scale applied to the virtual canvas
    float offsetX       = 0.0f;    // left letterbox offset in real pixels
    float offsetY       = 0.0f;    // top  letterbox offset in real pixels
    int   realW         = 1280;
    int   realH         = 720;
    
    // ── Camera follow ────────────────────────────────────────────────────────
    float cameraPosX    = VIRT_W / 2.0f;  // current camera position
    float cameraPosY    = VIRT_H / 2.0f;
    float cameraZoom    = 1.0f;           // applied on top of base scale
    float targetCameraX = VIRT_W / 2.0f;  // target position for smooth follow
    float targetCameraY = VIRT_H / 2.0f;
    float targetZoom    = 1.0f;           // target zoom
    float followSpeed   = 6.0f;           // smoothing speed
    float zoomSpeed     = 4.0f;           // zoom smoothing speed

    // Call once per frame after potential resize / fullscreen toggle
    void update(int screenW, int screenH)
    {
        realW = screenW;
        realH = screenH;

        float scaleX = (float)screenW / VIRT_W;
        float scaleY = (float)screenH / VIRT_H;
        scale   = (scaleX < scaleY) ? scaleX : scaleY; // fit inside, keep ratio

        offsetX = ((float)screenW - VIRT_W * scale) * 0.5f;
        offsetY = ((float)screenH - VIRT_H * scale) * 0.5f;
    }
    
    // Update camera to follow player position with zoom
    void updateCamera(float playerX, float playerY, float deltaTime)
    {
        // Smooth follow to player position
        cameraPosX += (targetCameraX - cameraPosX) * followSpeed * deltaTime;
        cameraPosY += (targetCameraY - cameraPosY) * followSpeed * deltaTime;
        
        // Smooth zoom
        cameraZoom += (targetZoom - cameraZoom) * zoomSpeed * deltaTime;
        
        // Clamp camera to arena bounds (with zoom consideration)
        float viewWidth  = VIRT_W / cameraZoom;
        float viewHeight = VIRT_H / cameraZoom;
        
        float minX = viewWidth / 2.0f;
        float maxX = VIRT_W - viewWidth / 2.0f;
        float minY = viewHeight / 2.0f;
        float maxY = VIRT_H - viewHeight / 2.0f;
        
        if (cameraPosX < minX) cameraPosX = minX;
        if (cameraPosX > maxX) cameraPosX = maxX;
        if (cameraPosY < minY) cameraPosY = minY;
        if (cameraPosY > maxY) cameraPosY = maxY;
    }
    
    // Set camera target (call with player position each frame)
    void setCameraTarget(float playerX, float playerY, float zoomLevel = 1.0f)
    {
        targetCameraX = playerX;
        targetCameraY = playerY;
        targetZoom    = zoomLevel;
    }

    // Reset camera to default state (centered, no zoom)
    void resetCamera()
    {
        targetCameraX = VIRT_W / 2.0f;
        targetCameraY = VIRT_H / 2.0f;
        targetZoom    = 1.0f;
        cameraPosX    = VIRT_W / 2.0f;
        cameraPosY    = VIRT_H / 2.0f;
        cameraZoom    = 1.0f;
    }

    // Raylib Camera2D that maps virtual → real screen
    Camera2D getCamera() const
    {
        Camera2D cam = {0};
        cam.offset = { (float)realW / 2.0f, (float)realH / 2.0f };  // center of screen
        cam.target = { cameraPosX, cameraPosY };
        cam.zoom   = scale * cameraZoom;
        cam.rotation = 0.0f;
        return cam;
    }

    // Convert real mouse position → virtual world coords
    Vector2 worldMouse() const
    {
        Vector2 mp = GetMousePosition();
        Vector2 screenPos = {
            (mp.x - offsetX) / scale,
            (mp.y - offsetY) / scale
        };
        // Apply inverse camera transform
        float viewWidth  = VIRT_W / cameraZoom;
        float viewHeight = VIRT_H / cameraZoom;
        float camLeft    = cameraPosX - viewWidth / 2.0f;
        float camTop     = cameraPosY - viewHeight / 2.0f;
        
        return {
            camLeft + screenPos.x / cameraZoom,
            camTop + screenPos.y / cameraZoom
        };
    }

    // Scale a scalar value (e.g. font size) for HUD drawing in real pixels
    // HUD is drawn OUTSIDE BeginMode2D, so it must use real-pixel coords.
    float s(float virtualSize) const { return virtualSize * scale; }

    // Convert virtual position → real screen position (for HUD elements)
    float rx(float vx) const { return vx * scale + offsetX; }
    float ry(float vy) const { return vy * scale + offsetY; }

    // Check if real point is inside the virtual canvas
    bool insideCanvas(Vector2 realPt) const
    {
        return realPt.x >= offsetX && realPt.x <= offsetX + VIRT_W * scale
            && realPt.y >= offsetY && realPt.y <= offsetY + VIRT_H * scale;
    }

    // Draw letterbox bars (call outside BeginMode2D)
    void drawLetterbox(Color barColor = BLACK) const
    {
        if (offsetX > 0.5f) {
            DrawRectangle(0, 0, (int)offsetX, realH, barColor);
            DrawRectangle((int)(offsetX + VIRT_W * scale), 0, realW, realH, barColor);
        }
        if (offsetY > 0.5f) {
            DrawRectangle(0, 0, realW, (int)offsetY, barColor);
            DrawRectangle(0, (int)(offsetY + VIRT_H * scale), realW, realH, barColor);
        }
    }
};
