# Survivor — Top Down Shooter

A top-down survivor-style shooter built in C++17 with [raylib](https://www.raylib.com/). Fight off waves of enemies and bosses with four weapons, each with its own feel: an assault rifle, a chargeable laser, throwable grenades, and a slow-field orb.

Runs on **Windows** and **Android**.

## Features

- 4 weapons: Assault Rifle, Charge Laser (3 power tiers), Frag Grenade (arc trajectory), Cryo Orb (area slow field)
- Wave-based survival loop with elite enemy variants and rotating bosses every 4th wave
- Ammo and health pickups, obstacles, and a camera that follows and zooms with the action
- Twin-stick touch controls on Android (see below) — independent from the desktop mouse/keyboard scheme
- Settings menu (volume, fullscreen), pause menu, main menu

## Controls

### PC (keyboard + mouse)

| Action | Input |
|---|---|
| Move | `WASD` / `ZQSD` |
| Aim | Mouse |
| Fire (Rifle) | Left click (hold) |
| Charge / release Laser, throw Grenade or Cryo Orb | Right click (hold to charge/aim, release to fire/throw) |
| Reload | `R` |
| Switch weapon | `1`-`4`, mouse wheel |
| Pause / resume | `Backspace` |

### Android (touch)

Twin-stick layout:
- **Left stick** — movement only.
- **Right stick (AIM)** — independent aim + fire/charge/throw trigger. Push it in a direction to aim and fire (Rifle) or charge (Laser); let it snap back to center to release the laser beam or throw a grenade/orb. How far it's pushed sets the throw distance, so you can back away with the left stick while firing forward with the right one.
- **RLD** button — reload.
- **SWAP** button — cycle weapon.
- **MENU** / **BACK** buttons (top corners) — return to main menu / pause-resume toggle, also active in the Settings screen.

## Project structure

```
src/main.cpp        Entry point
include/*.hpp        Header-only game code (Game, Player, Enemy, weapons, HUD, ...)
assets/               Sprites, sounds, music
Makefile              Desktop build (Windows/Linux)
android_build/        Android build script + generated output (gitignored)
```

## Building on Windows

Requires a MinGW-w64 toolchain and raylib. The easiest way is [MSYS2](https://www.msys2.org/):

```bash
pacman -S mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-raylib make
```

Then, from an MSYS2 UCRT64 shell in the project root:

```bash
make        # build
make run    # build and launch
```

This produces `bttf_shooter.exe`. It's dynamically linked against raylib — to distribute it to someone without MSYS2 installed, bundle these DLLs from `ucrt64/bin` alongside the exe and the `assets/` folder: `libraylib.dll`, `glfw3.dll`, `libgcc_s_seh-1.dll`, `libstdc++-6.dll`, `libwinpthread-1.dll`.

## Building on Android

No Gradle — the game is built and packaged into an installable APK directly with the Android NDK and command-line build-tools, via `android_build/build_android.sh`.

Prerequisites (edit the paths at the top of the script if yours differ):
- Android SDK with `build-tools` and a `platforms/android-<N>` (any recent one — used only to compile resources)
- Android NDK r23+ (tested with r27c)
- A JDK (the one bundled with Android Studio at `.../Android Studio/jbr` works)

```bash
cd android_build
bash build_android.sh
```

This compiles raylib for `arm64-v8a`, compiles the game into `libmain.so`, packages assets/resources/manifest into an APK, and signs it with a locally-generated debug keystore. Output: `android_build/Survivor.apk`.

Install on a connected device via ADB:

```bash
adb install -r android_build/Survivor.apk
```

### Notes on the Android port

- The game requests the device's native resolution (`InitWindow(0,0,...)`) instead of a fixed size, so it fills the screen edge-to-edge instead of being letterboxed at a smaller logical resolution.
- Touch input handling lives entirely in `include/TouchControls.hpp`, gated behind `#if defined(PLATFORM_ANDROID)` — it's a no-op on desktop, so the same `main.cpp` and `Game.hpp` build unmodified for both platforms.
