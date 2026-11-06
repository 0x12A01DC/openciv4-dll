# Lists what oc4core.exe imports from the game core (CvGameCoreDLL-x64.dll) that a game core does not export.
# The engine resolves these names when it loads the game core and stops if one is missing.
#   tools\check-exports.ps1 -Exe <engine folder>\oc4core.exe -Dll <build>\mod\CvGameCoreDLL-x64.dll
param(
	[Parameter(Mandatory = $true)][string]$Exe,
	[Parameter(Mandatory = $true)][string]$Dll
)
$ErrorActionPreference = "Stop"

$dumpbin = (Get-Command dumpbin.exe -ErrorAction SilentlyContinue).Source
if (-not $dumpbin) {
	$vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
	$vs = & $vswhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
	$dumpbin = Get-ChildItem "$vs\VC\Tools\MSVC\*\bin\Hostx64\x64\dumpbin.exe" | Select-Object -Last 1 -ExpandProperty FullName
}
if (-not $dumpbin) { throw "dumpbin.exe not found: run from a Visual Studio x64 developer prompt" }

$imports = New-Object System.Collections.Generic.List[string]
$inGameCore = $false
foreach ($line in & $dumpbin /nologo /imports $Exe) {
	if ($line -match '^\s{4}(\S+\.dll)\s*$') { $inGameCore = $Matches[1] -match '^CvGameCoreDLL(-\w+)?\.dll$'; continue }
	if ($inGameCore -and $line -match '^\s+[0-9A-Fa-f]{16}\s+[0-9A-Fa-f]+\s+(\S+)\s*$') { $imports.Add($Matches[1]) }
}
if ($imports.Count -eq 0) { throw "$Exe imports nothing from a game core" }

$exports = New-Object System.Collections.Generic.HashSet[string]
foreach ($line in & $dumpbin /nologo /exports $Dll) {
	if ($line -match '^\s+\d+\s+[0-9A-Fa-f]+\s+(?:[0-9A-Fa-f]{8}\s+)?(\S+)') { [void]$exports.Add($Matches[1]) }
}

$missing = @($imports | Where-Object { -not $exports.Contains($_) } | Sort-Object -Unique)
Write-Host "$($imports.Count) names imported, $($exports.Count) exported, $($missing.Count) missing"
$undname = Join-Path (Split-Path $dumpbin) "undname.exe"
foreach ($name in $missing) {
	Write-Host "  $name"
	if (Test-Path $undname) {
		$plain = & $undname $name | Select-String 'is :- "(.*)"'
		if ($plain) { Write-Host "      $($plain.Matches[0].Groups[1].Value)" }
	}
}
exit [int]($missing.Count -gt 0)
