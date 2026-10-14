# Opto Glow by ROHIT JI

Vintage-style opto compressor VST3 by ROHIT JI.

## Controls

| Section | Control | What it does |
|---|---|---|
| Opto Compressor | SMOOTH / PUNCH | SMOOTH = slow, program-dependent opto (LA-2A style). PUNCH = faster, more forward opto (LA-3A style). |
| | Peak Reduction | How much it compresses. |
| | Gain | Make-up gain after compression. |
| | Compress / Limit | Limit = higher ratio, firmer knee. |
| Tone | Warmth | Low shelf + low-mid body. |
| | Bite | Presence bell + high shelf for sharpness. |
| | SC HPF | Sidechain high-pass so bass doesn't over-trigger compression. |
| Saturation | Clean / Warm / Triode / Retro / Tape | Saturation character (4x oversampled). |
| | Drive, Sat Mix | Amount and blend of saturation. |
| Output | Mix, Output | Parallel (New York) compression blend and final level. |

Includes 10 factory presets (Vocal Silk, Vocal Bite, Devotional Lead, Drum Bus Glue, Bass Thick, Retro Radio, ...).

## Building the Windows VST3

**Easiest (no software needed):** upload this folder to a GitHub repository. The included
workflow (`.github/workflows/build-windows.yml`) builds the plugin automatically. Open the
**Actions** tab → latest run → download **Opto-Glow-Windows**.

**On your own PC:** install Visual Studio 2022 (Desktop development with C++) and CMake, then:

```
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release --target BKPOptoGlow_VST3
```

The plugin appears in `build/BKPOptoGlow_artefacts/Release/VST3/`. See `INSTALL-WINDOWS.txt`.
