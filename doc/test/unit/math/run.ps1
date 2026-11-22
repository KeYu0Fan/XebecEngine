# Xebec math test runner (Windows / MSVC)
# Usage: powershell -ExecutionPolicy Bypass -File doc\test\unit\math\run.ps1 [-RepoRoot D:\Xebec]

param(
    [string]$RepoRoot = ''
)

$ErrorActionPreference = 'Stop'

if ([string]::IsNullOrWhiteSpace($RepoRoot)) {
    $RepoRoot = (Resolve-Path (Join-Path $PSScriptRoot '..\..\..\..')).Path
}

$tests = @(
    'test_math_core.c',
    'test_math_geometry.c',
    'test_math_edge.c',
    'test_math_layout.c',
    'test_math_api.c',
    'test_math_primitives.c'
)

$build = Join-Path $env:TEMP 'xebec_math_test'
New-Item -ItemType Directory -Force -Path $build | Out-Null

# locate MSVC environment
$vcvars = $null
$cl = Get-Command 'cl.exe' -ErrorAction SilentlyContinue
if (-not $cl) {
    $vswhereCandidates = @(
        (Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'),
        (Join-Path $env:ProgramFiles 'Microsoft Visual Studio\Installer\vswhere.exe')
    )
    foreach ($vswhere in $vswhereCandidates) {
        if (Test-Path -LiteralPath $vswhere) {
            $vsPath = & $vswhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath 2>$null
            if ($vsPath) {
                $candidate = Join-Path $vsPath 'VC\Auxiliary\Build\vcvars64.bat'
                if (Test-Path -LiteralPath $candidate) { $vcvars = $candidate; break }
            }
        }
    }
    if (-not $vcvars -and $env:VCINSTALLDIR) {
        $candidate = Join-Path $env:VCINSTALLDIR 'Auxiliary\Build\vcvars64.bat'
        if (Test-Path -LiteralPath $candidate) { $vcvars = $candidate }
    }
    if (-not $vcvars) {
        Write-Error 'MSVC not found. Install Visual Studio C++ tools or run from a Developer Command Prompt.'
    }
}

$results = @()
foreach ($test in $tests) {
    $src = Join-Path $PSScriptRoot $test
    if (-not (Test-Path -LiteralPath $src)) {
        Write-Error "missing test source: $src"
    }
    $name = [IO.Path]::GetFileNameWithoutExtension($test)
    $exe = Join-Path $build ($name + '.exe')
    $obj = Join-Path $build ($name + '.obj')

    $compile = 'cl /nologo /W4 /utf-8 /I "' + $RepoRoot + '" /Fe:"' + $exe + '" /Fo:"' + $obj + '" "' + $src + '"'
    if ($vcvars) {
        $compile = 'call "' + $vcvars + '" >nul 2>&1 && ' + $compile
    }
    $output = cmd /c $compile 2>&1
    if ($LASTEXITCODE -ne 0) {
        Write-Host ("[BUILD FAIL] {0}" -f $test) -ForegroundColor Red
        $output | ForEach-Object { Write-Host $_ }
        $results += [pscustomobject]@{ Test = $test; Result = 'BUILD FAIL'; Detail = ($output -join ' | ') }
        continue
    }

    $runOutput = & $exe 2>&1
    $code = $LASTEXITCODE
    $tail = ($runOutput | Select-Object -Last 1)
    if ($code -eq 0) {
        Write-Host ("[PASS] {0}  {1}" -f $test, $tail) -ForegroundColor Green
        $results += [pscustomobject]@{ Test = $test; Result = 'PASS'; Detail = $tail }
    }
    else {
        Write-Host ("[FAIL] {0}" -f $test) -ForegroundColor Red
        $runOutput | ForEach-Object { Write-Host $_ }
        $results += [pscustomobject]@{ Test = $test; Result = 'FAIL'; Detail = ($runOutput -join ' | ') }
    }
}

$failed = @($results | Where-Object { $_.Result -ne 'PASS' }).Count
Write-Host ''
$results | Format-Table -AutoSize
if ($failed -gt 0) {
    Write-Host ("{0} of {1} test programs failed" -f $failed, $results.Count) -ForegroundColor Red
    exit 1
}
Write-Host ("all {0} test programs passed" -f $results.Count) -ForegroundColor Green
exit 0