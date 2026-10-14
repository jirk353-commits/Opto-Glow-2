# Opto Glow by ROHIT JI

Vintage opto compressor VST3 for Windows - LA-2A style **SMOOTH** and LA-3A style **PUNCH** modes,
Warmth / Bite tone controls, 5 saturation types (Clean, Warm, Triode, Retro, Tape), analog VU meter
and 10 presets.

## Download

Go to **[Releases](../../releases/latest)** and pick:

| File | For |
|---|---|
| **Opto-Glow-Setup-64bit.exe** | 64-bit FL Studio, Cubase and other modern DAWs (most people) |
| **Opto-Glow-Setup-32bit.exe** | Older 32-bit DAWs only |

Run it and click **Next > Install > Finish**.

## How it is built

Every push to `main` builds the plugin with MSVC (x64 and Win32) on GitHub Actions, packages each
into its own setup wizard (`Installer/setup.iss`, Inno Setup) and publishes them as the latest release.
