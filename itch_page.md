# itch.io page copy — Survivor

## Title
Survivor — Top Down Shooter

## Short description (tagline, ~120 chars max)
Wave-based top-down survival shooter with 4 weapons and rotating bosses. Plays on desktop and mobile.

## Genre / tags
action, shooter, top-down, survival, arcade, singleplayer, 2D, pixel-art, raylib, c-plus-plus, android, windows

## Custom noun
install

(This is the field under Pricing that swaps the word "downloads" on your page's stats/counter. "install" reads better than the default "downloads" here since one of the two uploads is an Android APK people install rather than a file they just download and unzip.)

## Download & install instructions
(itch.io field under the Uploads section — shown to players above the download button)

```
Windows: download and unzip SURVIVOR-windows.zip anywhere, then run bttf_shooter.exe — no installer needed. Everything the game needs (the raylib DLLs, assets) is already in the zip.

Android: download Survivor.apk, open it on your phone, and tap install. You'll likely need to allow "install from unknown sources" for your browser/file manager the first time — Android will prompt you for this automatically when you open the file. The game runs fullscreen in landscape with on-screen twin-stick controls (see the Controls section above).
```

## Full description

**Survive the waves. Master four very different weapons.**

Survivor is a top-down survival shooter: enemies spawn in escalating waves, elites get tougher stat lines, and a boss shows up every 4th wave. You've got four weapons to lean on, each with a completely different feel:

- **Assault Rifle** — your bread-and-butter, hold to fire.
- **Charge Laser** — hold to charge through 3 power tiers, release for a beam that hits harder the longer you waited.
- **Frag Grenade** — arc it over obstacles, aim the trajectory yourself.
- **Cryo Orb** — throw a slow field to buy yourself room to breathe.

Pick up ammo and health drops between fights, dodge around the arena's obstacles, and see how many waves you can clear.

**Controls**

Desktop: WASD to move, mouse to aim, left click to fire the rifle, right click to charge/throw your other weapons, R to reload, 1-4 or scroll to switch weapons.

Mobile / Android: twin-stick controls — left stick moves, right stick independently aims and fires/charges/throws (push it out to aim, let go to release), with dedicated reload and weapon-switch buttons.

Built in C++ with [raylib](https://www.raylib.com/).

## Suggested itch.io settings
- Kind of project: Executable / HTML
- Release status: Released (or "In development" if you plan to keep iterating)
- Pricing: your call — "No payments" / "$0 or donate" both work fine for a first release
- Platforms: Windows, Android, HTML5 (check boxes to match the uploads)
- App store metadata (Android): mark the APK's upload as "Android" platform — itch.io will show an install button on mobile and can be played via the itch.io app
- For the HTML5 build zip: after uploading, tick **"This file will be played in the browser"** on that specific file, and set the embed viewport to at least 1280x720 (see note in web_build/README below once built)
