# Build and run unit tests (math / struct / system)
# Usage: powershell -NoProfile -ExecutionPolicy Bypass -File doc\test\run.ps1
$ErrorActionPreference = 'Stop'

$root = (Resolve-Path (Join-Path $PSScriptRoot '..\..')).Path
$out = Join-Path $env:TEMP 'xebec_unit_test'
New-Item -ItemType Directory -Force -Path $out | Out-Null

$vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
if (-not (Test-Path -LiteralPath $vswhere)) { throw 'vswhere.exe not found' }
$vs = & $vswhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
if (-not $vs) { throw 'MSVC not found' }
$vcvars = Join-Path $vs 'VC\Auxiliary\Build\vcvars64.bat'
if (-not (Test-Path -LiteralPath $vcvars)) { throw "vcvars64.bat not found: $vcvars" }

# math: self-contained runner
$mathRunner = Join-Path $PSScriptRoot 'unit\math\run.ps1'
& powershell -NoProfile -ExecutionPolicy Bypass -File $mathRunner -RepoRoot $root
if ($LASTEXITCODE -ne 0) { throw 'math tests failed' }

$commonArgs = @('/nologo', '/W4', '/utf-8', '/std:c11', "/I$root")

# struct: exclude realm.c
$structSources = @("$root\doc\test\unit\struct\test_struct.c")
foreach ($f in Get-ChildItem "$root\core\struct" -Filter *.c) {
    if ($f.Name -ne 'realm.c') { $structSources += $f.FullName }
}
$structSources += "$root\core\mem.c"
$structSources += "$root\core\algo\sort.c"
$structArgs = $commonArgs + @('/Fe:test_struct.exe') + $structSources + @('/link', 'advapi32.lib', 'user32.lib')

# system: all sources
$sysSources = @("$root\doc\test\unit\system\test_system.c")
foreach ($f in Get-ChildItem "$root\core\system" -Filter *.c) { $sysSources += $f.FullName }
$sysArgs = $commonArgs + @('/Fe:test_system.exe') + $sysSources + @('/link', 'advapi32.lib', 'user32.lib')

function Build([string]$exe, [string[]]$clArgs) {
    $quoted = ($clArgs | ForEach-Object { '"' + $_ + '"' }) -join ' '
    Push-Location $out
    try {
        cmd /c "call `"$vcvars`" >nul 2>&1 && cl $quoted"
        if ($LASTEXITCODE -ne 0) { throw "$exe build failed" }
        cmd /c "`"$out\$exe`""
        if ($LASTEXITCODE -ne 0) { throw "$exe tests failed" }
    }
    finally { Pop-Location }
}

Build 'test_struct.exe' $structArgs
Build 'test_system.exe' $sysArgs
Write-Host 'all unit tests passed'