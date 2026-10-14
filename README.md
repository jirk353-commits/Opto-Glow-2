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

## Download

Get **Opto-Glow-Setup.exe** from the [latest release](../../releases/tag/latest), run it and click Next > Install > Finish.

## How it is built

Every push to `main` builds the plugin with MSVC on GitHub Actions and packages it into a
standard setup wizard (Inno Setup, `Installer/OptoGlow.iss`), published as the `latest` release.
