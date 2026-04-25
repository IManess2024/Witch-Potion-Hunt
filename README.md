# Witch-Potion-Hunt

This is where we work on the game Witch Potion Hunt.

## Build And Run

This project uses CMake and downloads SFML 3 automatically during configuration, so you do not need to install SFML manually.

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
