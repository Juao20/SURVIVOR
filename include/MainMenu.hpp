#pragma once

#include <raylib.h>
#include <vector>
#include <string>
#include <functional>
#include <memory>

class Ressource;

struct MenuButton
{
    Rectangle bounds;
    std::string text;
    Color normalColor;
    Color hoverColor;
    Color textColor;
    bool isHovered;
    std::function<void()> onClick;
    Ressource* spriteManager;
    bool soundPlayed;

    MenuButton(Rectangle rect, std::string txt, std::function<void()> callback, Ressource* sm = nullptr)
        : bounds(rect), text(txt),
          normalColor({30, 35, 55, 220}), hoverColor({60, 100, 180, 255}), textColor(WHITE),
          isHovered(false), onClick(callback), spriteManager(sm), soundPlayed(false) {}

    void update(Vector2 virtualMouse)
    {
        isHovered = CheckCollisionPointRec(virtualMouse, bounds);
        if (isHovered && !soundPlayed && spriteManager) {
            PlaySound(spriteManager->getSound("navigate"));
            soundPlayed = true;
        }
        if (!isHovered) soundPlayed = false;
        if (isHovered && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            if (spriteManager) PlaySound(spriteManager->getSound("select"));
            if (onClick) onClick();
        }
    }

    void draw() const
    {
        Color color = isHovered ? hoverColor : normalColor;
        DrawRectangleRec(bounds, color);
        DrawRectangleLinesEx(bounds, 2, isHovered ? WHITE : (Color){80, 100, 140, 255});
        int textWidth = MeasureText(text.c_str(), 24);
        DrawText(text.c_str(),
                 (int)(bounds.x + (bounds.width - textWidth) / 2),
                 (int)(bounds.y + (bounds.height - 24) / 2),
                 24, textColor);
    }
};

class MainMenu
{
private:
    int screenWidth;
    int screenHeight;
    std::vector<MenuButton> buttons;
    float animTime;
    Ressource* spriteManager;

public:
    MainMenu(int width = 1280, int height = 720, Ressource* sm = nullptr);
    ~MainMenu() = default;

    void addButton(Rectangle bounds, const std::string& text, std::function<void()> callback);
    void update(float deltaTime, Vector2 virtualMouse);
    void draw() const;
    void clear();
};

MainMenu::MainMenu(int width, int height, Ressource* sm)
    : screenWidth(width), screenHeight(height), animTime(0.0f), spriteManager(sm) {}

void MainMenu::addButton(Rectangle bounds, const std::string& text, std::function<void()> callback)
{
    buttons.emplace_back(bounds, text, callback, spriteManager);
}

void MainMenu::update(float deltaTime, Vector2 virtualMouse)
{
    animTime += deltaTime;
    for (auto& b : buttons) b.update(virtualMouse);
}

void MainMenu::draw() const
{
    ClearBackground({8, 10, 18, 255});

    for (int y = 0; y < screenHeight; y += 4)
        DrawRectangle(0, y, screenWidth, 1, Fade(BLACK, 0.12f));

    float pulse = 0.85f + 0.15f * sinf(animTime * 1.5f);
    const char* title = "SURVIVOR";
    int tw = MeasureText(title, 72);
    DrawText(title, screenWidth / 2 - tw / 2 + 2, 102, 72, Fade({40, 60, 120, 255}, pulse));
    DrawText(title, screenWidth / 2 - tw / 2, 100, 72, Fade({120, 180, 255, 255}, pulse));

    const char* sub = "TOP-DOWN SHOOTER";
    int sw = MeasureText(sub, 20);
    DrawText(sub, screenWidth / 2 - sw / 2, 180, 20, {160, 180, 220, 200});

    for (const auto& b : buttons) b.draw();

    const char* credits = "Made with Raylib";
    int cw = MeasureText(credits, 13);
    DrawText(credits, screenWidth / 2 - cw / 2, screenHeight - 30, 13, {60, 70, 90, 255});
}

void MainMenu::clear() { buttons.clear(); }