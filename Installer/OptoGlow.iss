; Opto Glow by ROHIT JI - standard Windows setup wizard (Inno Setup 6)
; Built automatically by GitHub Actions. PluginDir is passed on the command line.

#define AppName      "Opto Glow"
#define AppVersion   "1.0.0"
#define AppPublisher "ROHIT JI"
#ifndef PluginDir
  #define PluginDir "..\build\BKPOptoGlow_artefacts\Release\VST3\Opto Glow.vst3"
#endif

[Setup]
AppId={{6F2C1A8E-4B7D-4E59-9C1A-0B7A5D3E8F21}
AppName={#AppName}
AppVersion={#AppVersion}
AppVerName={#AppName} {#AppVersion}
AppPublisher={#AppPublisher}
VersionInfoVersion={#AppVersion}
VersionInfoCompany={#AppPublisher}
VersionInfoDescription={#AppName} Setup
DefaultDirName={commoncf64}\VST3
DisableDirPage=yes
DisableProgramGroupPage=yes
DisableReadyPage=no
Uninstallable=yes
CreateUninstallRegKey=yes
UninstallDisplayName={#AppName} (VST3) by {#AppPublisher}
UninstallDisplayIcon={uninstallexe}
UninstallFilesDir={commonpf64}\{#AppPublisher}\{#AppName}
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
PrivilegesRequired=admin
OutputDir=Output
OutputBaseFilename=Opto-Glow-Setup
SetupIconFile=icon.ico
WizardStyle=modern
WizardImageFile=wizard-large-100.bmp,wizard-large.bmp
WizardSmallImageFile=wizard-small-100.bmp,wizard-small.bmp
WizardImageStretch=yes
Compression=lzma2/max
SolidCompression=yes
CloseApplications=yes
RestartApplications=no

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Messages]
WelcomeLabel1=Welcome to the [name] Setup
WelcomeLabel2=This will install [name/ver] - a vintage opto compressor VST3 plugin by ROHIT JI - on your computer.%n%nPlease close FL Studio, Cubase and any other music software before continuing.
FinishedHeadingLabel=Opto Glow is installed!
FinishedLabelNoIcons=The plugin is ready to use.%n%nFL STUDIO: Options > Manage plugins > Find installed plugins, then add "Opto Glow" from the Mixer.%n%nCUBASE: restart Cubase - you will find it under Insert > Dynamics > Opto Glow.%n(If it was blocked before: Studio > VST Plug-in Manager > reactivate it.)

[InstallDelete]
; remove older builds of the plugin before copying the new one
Type: filesandordirs; Name: "{commoncf64}\VST3\Opto Glow.vst3"
Type: filesandordirs; Name: "{commoncf64}\VST3\BKP Opto Glow.vst3"

[Files]
Source: "{#PluginDir}\*"; DestDir: "{commoncf64}\VST3\Opto Glow.vst3"; Flags: ignoreversion recursesubdirs createallsubdirs

[UninstallDelete]
Type: filesandordirs; Name: "{commoncf64}\VST3\Opto Glow.vst3"
