#pragma once

#include <raylib.h>
#include <string>
#include <functional>

struct Slider
{
    Rectangle bounds;
    float value;
    float minValue, maxValue;
    std::string label;
    bool isDragging;
    std::function<void(float)> onChange;

    Slider(Rectangle rect, const std::string& lbl, float min, float max, float initial, std::function<void(float)> cb)
        : bounds(rect), value(initial), minValue(min), maxValue(max),
          label(lbl), isDragging(false), onChange(cb) {}

    void update(Vector2 mp)
    {
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(mp, bounds)) isDragging = true;
        if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) isDragging = false;
        if (isDragging) {
            float nx = (mp.x - bounds.x) / bounds.width;
            value = (nx < 0) ? 0 : (nx > 1) ? 1 : nx;
            if (onChange) onChange(minValue + (maxValue - minValue) * value);
        }
    }

    void draw() const
    {
        DrawText(label.c_str(), (int)bounds.x, (int)(bounds.y - 26), 18, WHITE);
        DrawRectangleRec(bounds, {40, 40, 50, 255});
        Rectangle filled = bounds; filled.width *= value;
        DrawRectangleRec(filled, {80, 150, 255, 255});
        DrawRectangleLinesEx(bounds, 1.5f, WHITE);
        DrawCircle((int)(bounds.x + bounds.width * value), (int)(bounds.y + bounds.height / 2), 9, WHITE);
        float actual = minValue + (maxValue - minValue) * value;
        DrawText(TextFormat("%.0f%%", actual * 100), (int)(bounds.x + bounds.width + 16), (int)(bounds.y + 2), 16, LIGHTGRAY);
    }
};

struct Toggle
{
    Rectangle bounds;
    std::string label;
    bool value;
    std::function<void(bool)> onChange;
    bool isHovered;

    Toggle(Rectangle rect, const std::string& lbl, bool initial, std::function<void(bool)> cb)
        : bounds(rect), label(lbl), value(initial), onChange(cb), isHovered(false) {}

    void update(Vector2 mp)
    {
        isHovered = CheckCollisionPointRec(mp, bounds);
        if (isHovered && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            value = !value;
            if (onChange) onChange(value);
        }
    }

    void draw() const
    {
        DrawText(label.c_str(), (int)bounds.x, (int)(bounds.y - 26), 18, WHITE);
        Color bg = isHovered ? (Color){60, 80, 140, 255} : (Color){30, 35, 55, 255};
        DrawRectangleRec(bounds, bg);
        DrawRectangleLinesEx(bounds, 1.5f, isHovered ? WHITE : (Color){80, 100, 140, 255});
        const char* txt = value ? "ON" : "OFF";
        Color tc = value ? GREEN : RED;
        int tw = MeasureText(txt, 22);
        DrawText(txt, (int)(bounds.x + (bounds.width - tw) / 2), (int)(bounds.y + (bounds.height - 22) / 2), 22, tc);
    }
};

class SettingsMenu
{
private:
    int screenWidth, screenHeight;
    Slider volumeSlider;
    Toggle musicToggle;
    Toggle fullscreenToggle;

public:
    SettingsMenu(int width, int height,
                 std::function<void(float)> volCb,
                 std::function<void(bool)>  fsCb);
    ~SettingsMenu() = default;

    void update(Vector2 virtualMouse);
    void draw() const;
    void setVolume(float v)      { volumeSlider.value = v; }
    void setFullscreen(bool fs)  { fullscreenToggle.value = fs; }
    void setMusicEnabled(bool m) { musicToggle.value = m; }
};

SettingsMenu::SettingsMenu(int width, int height,
                           std::function<void(float)> volCb,
                           std::function<void(bool)>  fsCb)
    : screenWidth(width), screenHeight(height),
      volumeSlider({(float)(width / 2 - 200), 280, 400, 28}, "Master Volume", 0.0f, 1.0f, 0.5f, volCb),
      musicToggle({(float)(width / 2 - 200), 370, 120, 46}, "Music", true, nullptr),
      fullscreenToggle({(float)(width / 2 - 200), 460, 160, 46}, "Fullscreen", false, fsCb)
{}

void SettingsMenu::update(Vector2 virtualMouse)
{
    volumeSlider.update(virtualMouse);
    musicToggle.update(virtualMouse);
    fullscreenToggle.update(virtualMouse);
}

void SettingsMenu::draw() const
{
    ClearBackground({8, 10, 18, 255});
    const char* title = "SETTINGS";
    int tw = MeasureText(title, 48);
    DrawText(title, screenWidth / 2 - tw / 2, 120, 48, {120, 180, 255, 255});
    volumeSlider.draw();
    musicToggle.draw();
    fullscreenToggle.draw();
    DrawText("BACKSPACE to return", screenWidth / 2 - 70, screenHeight - 60, 16, {80, 100, 140, 255});
}