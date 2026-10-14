; Shared setup wizard for ROHIT JI plugins (Inno Setup 6).
; Called by GitHub Actions with:
;   /DAppName="Glow Verb" /DAppIdGuid=... /DArtDir=...\Installer /DDir64=... /DDir32=... /DOutName=Glow-Verb-Setup
; Installs the VST3 bundle for 64-bit hosts and/or 32-bit hosts (the user can choose).

#define AppVersion   "1.0.0"
#define AppPublisher "ROHIT JI"
#define AppIdFull    "{{" + AppIdGuid + "}"

[Setup]
AppId={#AppIdFull}
AppName={#AppName}
AppVersion={#AppVersion}
AppVerName={#AppName} {#AppVersion}
AppPublisher={#AppPublisher}
VersionInfoVersion={#AppVersion}
VersionInfoCompany={#AppPublisher}
VersionInfoDescription={#AppName} Setup
DefaultDirName={commoncf}\VST3
DisableDirPage=yes
DisableProgramGroupPage=yes
Uninstallable=yes
UninstallDisplayName={#AppName} (VST3) by {#AppPublisher}
UninstallDisplayIcon={uninstallexe}
UninstallFilesDir={commonpf}\{#AppPublisher}\{#AppName}
ArchitecturesAllowed=x86compatible x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
PrivilegesRequired=admin
OutputDir=Output
OutputBaseFilename={#OutName}
SetupIconFile={#ArtDir}\icon.ico
WizardStyle=modern
WizardImageFile={#ArtDir}\wizard-large-100.bmp,{#ArtDir}\wizard-large.bmp
WizardSmallImageFile={#ArtDir}\wizard-small-100.bmp,{#ArtDir}\wizard-small.bmp
WizardImageStretch=yes
Compression=lzma2/max
SolidCompression=yes
CloseApplications=yes
RestartApplications=no

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Messages]
WelcomeLabel1=Welcome to the [name] Setup
WelcomeLabel2=This will install [name/ver] by ROHIT JI on your computer.%n%nPlease close FL Studio, Cubase and any other music software before continuing.
SelectComponentsDesc=Which versions of the plugin should be installed?
SelectComponentsLabel2=Most people only need the 64-bit version. Also tick 32-bit if you use an older 32-bit DAW.
FinishedHeadingLabel=[name] is installed!
FinishedLabelNoIcons=The plugin is ready to use.%n%nFL STUDIO: Options > Manage plugins > Find installed plugins.%n%nCUBASE: restart Cubase (or Studio > VST Plug-in Manager > Rescan).

[Types]
Name: "full";   Description: "64-bit + 32-bit"
Name: "only64"; Description: "64-bit only"
Name: "custom"; Description: "Custom"; Flags: iscustom

[Components]
Name: "vst3_64"; Description: "VST3 plugin - 64-bit  (FL Studio, Cubase and other 64-bit DAWs)"; Types: full only64 custom
Name: "vst3_32"; Description: "VST3 plugin - 32-bit  (older 32-bit DAWs)"; Types: full

[InstallDelete]
Type: filesandordirs; Name: "{commoncf}\VST3\{#AppName}.vst3";   Components: vst3_64; Check: Is64BitInstallMode
Type: filesandordirs; Name: "{commoncf32}\VST3\{#AppName}.vst3"; Check: Install32
#ifdef OldName
Type: filesandordirs; Name: "{commoncf}\VST3\{#OldName}.vst3"
Type: filesandordirs; Name: "{commoncf32}\VST3\{#OldName}.vst3"
#endif

[Files]
Source: "{#Dir64}\*"; DestDir: "{commoncf}\VST3\{#AppName}.vst3";   Components: vst3_64; Check: Is64BitInstallMode; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "{#Dir32}\*"; DestDir: "{commoncf32}\VST3\{#AppName}.vst3"; Check: Install32; Flags: ignoreversion recursesubdirs createallsubdirs

[UninstallDelete]
Type: filesandordirs; Name: "{commoncf}\VST3\{#AppName}.vst3"
Type: filesandordirs; Name: "{commoncf32}\VST3\{#AppName}.vst3"

[Code]
{ 32-bit Windows can only use the 32-bit plugin, so it is always installed there. }
function Install32: Boolean;
begin
  Result := WizardIsComponentSelected('vst3_32') or (not Is64BitInstallMode);
end;
