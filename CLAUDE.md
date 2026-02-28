# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build

Open `CyNickal Software EFT.slnx` in Visual Studio 2022 and build from there. The project uses MSVC v145 toolset with C++20 (`stdcpp20`).

Build configurations:
- **Debug|x64** / **Release|x64** — builds a standalone console EXE (`EFT_DMA.exe`)
- **DLL|x64** — builds a DLL with `DLL_FORM` preprocessor define enabled; `DllMain` injects into a host process and calls `main()` on a new thread

To build from command line:
```
msbuild "CyNickal Software EFT.slnx" /p:Configuration=Release /p:Platform=x64
```

## Updating the Game Database

`TarkovDevAPI/TarkovDevAPI.py` populates `CyNickal Software EFT/EFT_Data.db` by querying the tarkov.dev GraphQL API. Run it whenever game data (items, ammo, containers, exfils) needs refreshing:
```
python TarkovDevAPI/TarkovDevAPI.py
```

## Architecture

### Threading Model

The application runs two threads:

1. **Main thread** — initializes and runs the ImGui/DirectX 11 window (`MainWindow`). Calls `OnFrame()` each tick to render all GUI panels.
2. **DMA thread** (`DMA_Thread_Main`) — all game memory reads happen here. Uses `CTimer<Interval, Fn>` instances to schedule work at different rates:
   - 2ms: camera view matrix update
   - 25ms: player quick-update, response data (JSON from game memory)
   - 50ms: keybind processing
   - 5s: player allocation scan, DMA light-refresh
   - 10s: raid presence check / world creation

The `EFT::m_GameWorldMutex` guards access to `EFT::pGameWorld` between threads.

### Memory Access Layer

`DMA_Connection` (singleton) wraps a `VMM_HANDLE` from MemProcFS (`vmm.dll`/`leechcore.dll`). All physical/virtual memory reads flow through this handle. `Process` (in `DMA/Process.h`) wraps the target process handle within `DMA_Connection`.

### Game Object Discovery

`GOM` (Game Object Manager) scans the Unity `CGameObjectManager` linked-list at `Offsets::pGOM` inside `UnityPlayer.dll`. It walks `ActiveNodes`/`LastActiveNode` linked entries, reads `CObjectInfo` for each, and finds addresses matching `GameWorld`. `EFT::CreateWorldIfNeeded` calls into GOM to locate the current `CLocalGameWorld`.

### EFT Game Layer

`EFT` is a static class that owns `pGameWorld` (a `CLocalGameWorld`). Subordinate classes accessed through it:
- `CRegisteredPlayers` — array of player pointers; quick-updated every 25ms
- `CLootList` — loose loot items on the map
- `CExfilController` — extraction point list

All memory offsets are centralized in `Offsets.h` as `constexpr std::ptrdiff_t` inside named namespaces (one per game class). Dynamic offsets resolved at startup by `Offsets::ResolveOffsets()` via signature scanning (`Signature Scanner.cpp`).

### GUI Layer

All GUI panels are ImGui-based and rendered from the main thread:
- `GUI/Main Window` — D3D11 device/swap chain, WndProc, frame loop
- `GUI/Main Menu` — tab navigation between feature panels
- `GUI/Fuser` — 3D world-space ESP overlay
- `GUI/Radar` — 2D top-down radar
- `GUI/Aimbot` — aimbot settings; hardware mouse movement via Makcu (`Makcu/MyMakcu`)
- `GUI/Flea Bot` — automated flea market purchase logic
- `GUI/Player Table` / `GUI/Item Table` — sortable entity detail tables
- `GUI/Color Picker` — shared color selection widget
- `GUI/Keybinds` — runtime key binding management
- `GUI/Config` — JSON config save/load via nlohmann/json (`configs/<name>.json`)

### Database Layer

`Database` opens `EFT_Data.db` (SQLite) at startup. Lookup helpers (`TarkovItemData`, `TarkovAmmoData`, `TarkovContainerData`, `TarkovExfilData`) are defined as static inline functions in `Database.h` and used throughout GUI and game layers to resolve BSG item IDs to human-readable names and prices.

### ResponseData

`ResponseData` reads a large JSON buffer (up to 500 KB) directly from game memory at a fixed address resolved at startup. It is polled every 25ms and exposes `ResponseData::LatestJson` for features that consume game-provided structured data (Flea Bot, etc.).

## Dependencies (in `Dependencies/`)

| Library | Purpose |
|---|---|
| `MemProcFS` | DMA hardware memory access (VMM + LeechCore) |
| `ImGui` | Immediate-mode GUI (DX11 + Win32 backends) |
| `nlohmann/json` | JSON config serialization |
| `sqlite3` | Embedded game data database |
| `libcurl` | HTTP for network requests |
| `Makcu` | Hardware HID device for aimbot mouse movement |

All runtime DLLs (`vmm.dll`, `leechcore.dll`, `FTD3XX.dll`, `FTD3XXWU.dll`, `leechcore_driver.dll`, `makcu-cpp.dll`, `libcurl-x64.dll`) must be placed alongside the built executable.
