# Witch-Potion-Hunt

Witch Potion Hunt is a small 2D action-platformer built with C++ and SFML. You play as a witch, collect coins, fight enemies with spells, unlock movement abilities, and enter the portal to advance through the game.

## Gameplay Overview

- There are 3 levels.
- Each level contains 4 coins to collect.
- The portal opens only after all 4 coins are collected.
- Enemies damage your HP on contact or by shooting projectiles.
- Spells consume mana, and mana restores over time.
- Level 2 unlocks double jump.
- Level 3 unlocks climbing on green vines.

## Controls

- Move: `A` / `D` or `Left` / `Right`
- Jump: `Space`, `W`, or `Up`
- Climb: `W` / `S` or `Up` / `Down` while touching a green climb wall
- Select spells: `1`, `2`, `3`
- Cast spell: `Left Click`
- Restart after win or loss: `R` or `Enter`
- Leave settings / confirm some overlays: `Enter` or `Escape`

## Tech Notes

- The project uses CMake and automatically downloads SFML `3.0.2` during configuration.
- The game uses generated audio, so no external sound asset pack is required.
- The codebase targets C++17.

## Build And Run

### Clone

```bash
git clone <your-repo-url>
cd Witch-Potion-Hunt
```

## Run On macOS

### Requirements

- Xcode Command Line Tools
- CMake 3.24 or newer
- Git

Install the Apple toolchain:

```bash
xcode-select --install
```

If you do not already have `git` and `cmake`, you can install them with Homebrew:

```bash
brew install git cmake
```

### Configure From Scratch

If you want a completely fresh build, remove the old build folder first:

```bash
rm -rf build
```

Configure the project:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
```

The first configure can take a little longer because CMake downloads SFML and builds its dependencies.

### Build

```bash
cmake --build build -j
```

### Run

```bash
./build/WitchPotionHunt
```

## Run On Windows 11

### Requirements

- Git
- CMake 3.24 or newer
- Visual Studio 2022 with the C++ workload installed

### Configure

```bash
cmake -S . -B build -G "Visual Studio 17 2022"
```

### Build

```bash
cmake --build build --config Release
```

### Run

From PowerShell:

```powershell
.\build\Release\WitchPotionHunt.exe
```

You can also open the generated Visual Studio solution from the `build` folder and run `WitchPotionHunt` there.
