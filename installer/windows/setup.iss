; Whetstone Editor - Inno Setup Script
; Requires Inno Setup 6.x (https://jrsoftware.org/isinfo.php)
;
; Build the editor first using build.ps1, then compile this script.

#define MyAppName "Whetstone Editor"
#define MyAppVersion "0.1.0"
#define MyAppPublisher "Whetstone DSL Project"
#define MyAppURL "https://github.com/billthemaker/Whetstone_DSL"
#define MyAppExeName "whetstone_editor.exe"

[Setup]
AppId={{B7E3F2A1-4D5C-6E7F-8A9B-0C1D2E3F4A5B}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
DefaultDirName={autopf}\{#MyAppName}
DefaultGroupName={#MyAppName}
AllowNoIcons=yes
OutputDir=..\..\editor\build\installer
OutputBaseFilename=WhetstoneEditor-{#MyAppVersion}-setup
Compression=lzma2
SolidCompression=yes
WizardStyle=modern
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
; LicenseFile=..\..\LICENSE
; Uncomment the following line to require admin privileges:
; PrivilegesRequired=admin

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked
Name: "fileassoc_whet"; Description: "Associate .whet files with Whetstone Editor"; GroupDescription: "File Associations:"
Name: "fileassoc_semanno"; Description: "Associate .semanno files with Whetstone Editor"; GroupDescription: "File Associations:"

[Files]
; Main executables
Source: "staging\whetstone_editor.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "staging\orchestrator.exe"; DestDir: "{app}"; Flags: ignoreversion

; Runtime DLLs from vcpkg
Source: "staging\SDL2.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "staging\*.dll"; DestDir: "{app}"; Flags: ignoreversion skipifsourcedoesntexist

; Data files (if any)
Source: "staging\data\*"; DestDir: "{app}\data"; Flags: ignoreversion recursesubdirs createallsubdirs skipifsourcedoesntexist

[Icons]
Name: "{group}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"
Name: "{group}\Whetstone Orchestrator"; Filename: "{app}\orchestrator.exe"
Name: "{group}\{cm:UninstallProgram,{#MyAppName}}"; Filename: "{uninstallexe}"
Name: "{autodesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Tasks: desktopicon

[Registry]
; .whet file association
Root: HKA; Subkey: "Software\Classes\.whet"; ValueType: string; ValueName: ""; ValueData: "WhetstoneFile"; Flags: uninsdeletevalue; Tasks: fileassoc_whet
Root: HKA; Subkey: "Software\Classes\WhetstoneFile"; ValueType: string; ValueName: ""; ValueData: "Whetstone Source File"; Flags: uninsdeletekey; Tasks: fileassoc_whet
Root: HKA; Subkey: "Software\Classes\WhetstoneFile\DefaultIcon"; ValueType: string; ValueName: ""; ValueData: "{app}\{#MyAppExeName},0"; Tasks: fileassoc_whet
Root: HKA; Subkey: "Software\Classes\WhetstoneFile\shell\open\command"; ValueType: string; ValueName: ""; ValueData: """{app}\{#MyAppExeName}"" ""%1"""; Tasks: fileassoc_whet

; .semanno file association
Root: HKA; Subkey: "Software\Classes\.semanno"; ValueType: string; ValueName: ""; ValueData: "SemannoFile"; Flags: uninsdeletevalue; Tasks: fileassoc_semanno
Root: HKA; Subkey: "Software\Classes\SemannoFile"; ValueType: string; ValueName: ""; ValueData: "Semantic Annotation File"; Flags: uninsdeletekey; Tasks: fileassoc_semanno
Root: HKA; Subkey: "Software\Classes\SemannoFile\DefaultIcon"; ValueType: string; ValueName: ""; ValueData: "{app}\{#MyAppExeName},0"; Tasks: fileassoc_semanno
Root: HKA; Subkey: "Software\Classes\SemannoFile\shell\open\command"; ValueType: string; ValueName: ""; ValueData: """{app}\{#MyAppExeName}"" ""%1"""; Tasks: fileassoc_semanno

[Run]
Filename: "{app}\{#MyAppExeName}"; Description: "{cm:LaunchProgram,{#StringChange(MyAppName, '&', '&&')}}"; Flags: nowait postinstall skipifsilent

[UninstallDelete]
Type: filesandordirs; Name: "{app}\data"
