# AGENTS.md

Repository-specific instructions for AI coding agents.

## Mandatory Build Rule

Do not use MSYS, MSYS2, UCRT64, MinGW, or `build-msys2` from an AI/assistant terminal in this repository.

Forbidden commands and routes:

- `cmake --build build-msys2`
- `cmake -S . -B build-msys2 ...`
- `.\run-game.bat mingw`
- `mingw32-make`
- MSYS2 `ninja`
- `pacman`
- Direct tools under `D:\msys2\...`
- Direct tools under `C:\x86_64-*-mingw64\...`

The user may build successfully with MSYS/MinGW manually, but AI terminal sessions for this project should not retry that path.

Use this Windows verification command by default:

```powershell
cmake --build build --config Debug
```

If the Visual Studio build directory is missing or stale, use the Visual Studio route:

```powershell
.\run-game.bat vs
```

Only use another toolchain if the user explicitly asks for it in the current conversation.
