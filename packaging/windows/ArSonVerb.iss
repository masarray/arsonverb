; ArSonVerb x64 per-user installer: no elevation, no services, no drivers.
; Same native VST3 files as the transparent VST3-only ZIP.
#define AppVersion "1.0.1"
#define AppName "ArSonVerb"
[Setup]
AppId={{A4E930F2-BD67-4BE8-AFF9-5A11367A9101}
AppName={#AppName}
AppVersion={#AppVersion}
AppVerName=ArSonVerb {#AppVersion}
AppPublisher=Mas Ari - SonKuPik
AppPublisherURL=https://www.youtube.com/@sonkupik
AppSupportURL=https://github.com/masarray/arsonverb/issues
AppUpdatesURL=https://github.com/masarray/arsonverb/releases
LicenseFile=..\..\LICENSE
DefaultDirName={localappdata}\Programs\ArSonVerb
DefaultGroupName=ArSonVerb
DisableProgramGroupPage=yes
PrivilegesRequired=lowest
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
MinVersion=10.0
OutputDir=..\..\release-assets
OutputBaseFilename=ArSonVerb-Windows-x64-Setup-v{#AppVersion}
Compression=lzma2/max
SolidCompression=yes
WizardStyle=modern
UninstallDisplayName=ArSonVerb {#AppVersion}
CloseApplications=yes
RestartApplications=no
ChangesAssociations=no
DisableDirPage=yes
[Tasks]
Name: "vst3"; Description: "VST3 plugin (DAW) — user install"; GroupDescription: "Install components"; Flags: checkedonce
Name: "standalone"; Description: "Standalone application and Start Menu shortcut"; GroupDescription: "Install components"; Flags: checkedonce
[Files]
Source: "..\..\package-stage\ArSonVerb.vst3\*"; DestDir: "{localappdata}\Programs\Common\VST3\ArSonVerb.vst3"; Tasks: vst3; Flags: recursesubdirs createallsubdirs ignoreversion
Source: "..\..\package-stage\ArSonVerb.exe"; DestDir: "{app}"; Tasks: standalone; Flags: ignoreversion
Source: "..\..\LICENSE"; DestDir: "{app}"; DestName: "LICENSE.txt"; Flags: ignoreversion
Source: "..\..\assets\fonts\OFL.txt"; DestDir: "{app}"; DestName: "INTER-OFL.txt"; Flags: ignoreversion
Source: "..\..\docs\RELEASE_V1.0.1.md"; DestDir: "{app}"; DestName: "RELEASE_NOTES.txt"; Flags: ignoreversion
[Icons]
Name: "{group}\ArSonVerb"; Filename: "{app}\ArSonVerb.exe"; Tasks: standalone
Name: "{group}\Uninstall ArSonVerb"; Filename: "{uninstallexe}"
[Run]
Filename: "{app}\ArSonVerb.exe"; Description: "Open ArSonVerb"; Flags: nowait postinstall skipifsilent unchecked; Tasks: standalone
