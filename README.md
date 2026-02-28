# CyNickal Software EFT DMA

> **Educational Disclaimer**
> This project is intended strictly for **educational and research purposes** — specifically to demonstrate DMA (Direct Memory Access) hardware techniques, low-level Windows memory internals, and Unity game engine object layouts. The author does not endorse or encourage the use of this software in any live online game environment. Usage in a manner that violates any game's Terms of Service or applicable laws is solely the responsibility of the end user. No warranty is provided.
<img width="1919" height="793" alt="Showcase image" src="https://raw.githubusercontent.com/CyN1ckal/CyNickal-Software-EFT/refs/heads/master/Images/EFT_1.0.2.0.png"/>

# Current Features
- 3D Fuser ESP including player skeleton, player type (PMC/Scav/Boss), held weapon name, held weapon ammo, lootable items, lootable containers, and exfils
- 2D Radar including players (with view direction ray), lootable items, lootable containers, and exfils
- Hardware level aimbot using Makcu
- Hardware level Flea Market bot that automatically buys certain items at given price thresholds
- A variety of entity tables that display fine details of each entity
- Many customization options/toggles for each feature; too many to list here
- Customizable colors for all entities on both Fuser and Radar
- Multiple user config support

Currently all features are **READ ONLY**

# Runtime Requirements

This application requires the following DLL dependencies to run:

### MemProcFS Dependencies
Download from [MemProcFS Releases](https://github.com/ufrisk/MemProcFS/releases):
- `FTD3XX.dll`
- `FTD3XXWU.dll`
- `leechcore.dll`
- `leechcore_driver.dll`
- `vmm.dll`

### Makcu Dependencies
Download from [makcu-cpp Releases](https://github.com/K4HVH/makcu-cpp/releases):
- `makcu-cpp.dll`

### libcurl Dependencies
Download from [curl for Windows](https://curl.se/windows/):
- `libcurl-x64.dll`

**Note:** All DLL files must be placed in the same directory as the executable or in your system PATH.

# Contribute!
This project is open to pull requests, please contribute!
