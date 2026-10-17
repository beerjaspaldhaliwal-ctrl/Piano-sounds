; Keyverse - Windows installer (Inno Setup 6).
; Built by CI:  ISCC.exe /DAppVersion=1.2.3 /DSourceDir=<folder with Keyverse.vst3 + Keyverse.exe> Keyverse.iss

#define AppName "Keyverse"
#ifndef AppVersion
  #define AppVersion "1.0.0"
#endif
#ifndef SourceDir
  #define SourceDir "..\Keyverse-Windows"
#endif

[Setup]
AppId={{6B1F3E2A-9C4D-4B7E-8A15-2F0C9D3E4A71}
AppName={#AppName}
AppVersion={#AppVersion}
AppVerName={#AppName} {#AppVersion}
AppPublisher=Keyverse Audio
AppPublisherURL=https://github.com/beerjaspaldhaliwal-ctrl/Piano-sounds
AppSupportURL=https://github.com/beerjaspaldhaliwal-ctrl/Piano-sounds
DefaultDirName={autopf}\{#AppName}
DefaultGroupName={#AppName}
DisableProgramGroupPage=yes
PrivilegesRequired=admin
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
WizardStyle=modern
SetupIconFile=keyverse.ico
UninstallDisplayIcon={app}\Keyverse.exe
UninstallDisplayName={#AppName}
OutputDir=..\installer-out
OutputBaseFilename=Keyverse-Windows-Setup
Compression=lzma2/max
SolidCompression=yes
CloseApplications=yes

[Messages]
WelcomeLabel2=This will install [name/ver] on your computer.%n%nKeyverse adds hundreds of pianos, electric pianos, organs, synths and guitars to your music software as a VST3 instrument, plus a standalone app you can play without a DAW.%n%nClose your DAW before continuing.
FinishedLabel=Keyverse is installed.%n%nOpen your DAW (FL Studio, Ableton Live, Cubase, Studio One, Reaper, Bitwig...) and rescan plug-ins; Keyverse appears as an instrument from "Keyverse Audio".

[Types]
Name: "full"; Description: "VST3 plug-in and standalone app"
Name: "custom"; Description: "Choose what to install"; Flags: iscustom

[Components]
Name: "vst3"; Description: "VST3 plug-in (installs to Common Files\VST3)"; Types: full custom
Name: "standalone"; Description: "Standalone app"; Types: full custom

[Tasks]
Name: "desktopicon"; Description: "Create a desktop shortcut for the standalone app"; Components: standalone; Flags: unchecked

[Files]
Source: "{#SourceDir}\Keyverse.vst3\*"; DestDir: "{commoncf64}\VST3\Keyverse.vst3"; Components: vst3; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "{#SourceDir}\Keyverse.exe"; DestDir: "{app}"; Components: standalone; Flags: ignoreversion
Source: "{#SourceDir}\INSTALL.md"; DestDir: "{app}"; Flags: ignoreversion

[Icons]
Name: "{autoprograms}\{#AppName}"; Filename: "{app}\Keyverse.exe"; Components: standalone
Name: "{autodesktop}\{#AppName}"; Filename: "{app}\Keyverse.exe"; Components: standalone; Tasks: desktopicon

[Run]
Filename: "{app}\Keyverse.exe"; Description: "Open the Keyverse standalone app"; Components: standalone; Flags: nowait postinstall skipifsilent unchecked
