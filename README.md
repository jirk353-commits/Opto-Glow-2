# ROHIT JI Plugins

VST3 audio plugins for Windows (64-bit and 32-bit), built automatically with MSVC on GitHub Actions.

| Plugin | What it is | Download |
|---|---|---|
| **Opto Glow** | Vintage opto compressor (LA-2A / LA-3A style) with tone and saturation | [Opto-Glow-Setup.exe](../../releases/tag/opto-glow) |
| **Glow Verb** | Algorithmic reverb: 9 environments, reverb EQ, saturation + air, sidechain ducking | [Glow-Verb-Setup.exe](../../releases/tag/glow-verb) |

Run the setup, click **Next > Install > Finish**. The installer lets you choose 64-bit, 32-bit or both.

## Layout

- `OptoGlow/`, `GlowVerb/` - one JUCE/CMake project per plugin (`Source/`, `Tests/`, `Installer/` art)
- `installer/plugin-setup.iss` - shared Inno Setup wizard
- `.github/workflows/build-windows.yml` - builds x64 + Win32 for each plugin, makes the installers and publishes releases
