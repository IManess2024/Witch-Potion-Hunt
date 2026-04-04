# Witch-Potion-Hunt

This is where we work on the game Witch Potion Hunt.

## Run On Windows 11

### Requirements

- Git
- CMake 3.24 or newer
- Visual Studio 2022 with the C++ workload installed

### Clone

```bash
git clone <your-repo-url>
cd Witch-Potion-Hunt
```

### Configure

This project downloads SFML 3 automatically with CMake, so you do not need to install SFML manually.

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
