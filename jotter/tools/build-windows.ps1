param([string]$QtPath = "")

$ErrorActionPreference = "Stop"
Set-StrictMode -Version Latest
$project = Split-Path $PSScriptRoot -Parent

function Run-Checked {
    param([string]$Exe, [string[]]$Arguments)
    & $Exe @Arguments
    if ($LASTEXITCODE -ne 0) {
        throw "'$Exe' failed with exit code $LASTEXITCODE."
    }
}

function Get-QtKit {
    param([string]$Candidate)
    if (-not $Candidate -or -not (Test-Path -LiteralPath $Candidate)) { return }
    $root = (Resolve-Path -LiteralPath $Candidate).Path
    # Accept the kit root, bin directory, qmake executable, or Qt6_DIR.
    if (Test-Path -LiteralPath $root -PathType Leaf) { $root = Split-Path $root -Parent }
    if ((Split-Path $root -Leaf) -eq "bin") { $root = Split-Path $root -Parent }
    if ((Split-Path $root -Leaf) -eq "Qt6") {
        $root = Split-Path (Split-Path (Split-Path $root -Parent) -Parent) -Parent
    }
    $config = Join-Path $root "lib\cmake\Qt6\Qt6Config.cmake"
    $qmake = Join-Path $root "bin\qmake.exe"
    $qconfig = Join-Path $root "mkspecs\qconfig.pri"
    if (-not (Test-Path $config) -or -not (Test-Path $qmake) -or -not (Test-Path $qconfig)) { return }
    # Do not accidentally select a MinGW or ARM Qt kit for the x64 MSVC build.
    $spec = & $qmake -query QMAKE_XSPEC 2>$null
    if ($LASTEXITCODE -ne 0 -or "$spec".Trim() -ne "win32-msvc") { return }
    $architecture = Get-Content -LiteralPath $qconfig -Raw
    if ($architecture -notmatch '(?m)^\s*QT_ARCH\s*=\s*x86_64\s*$') { return }
    $versionText = & $qmake -query QT_VERSION 2>$null
    if ($LASTEXITCODE -ne 0) { return }
    $version = $null
    if (-not [version]::TryParse("$versionText".Trim(), [ref]$version)) { return }
    if ($version -lt [version]"6.8" -or $version.Major -ne 6) { return }
    [pscustomobject]@{ Root = $root; Version = $version }
}

try {
    Write-Host "`nJotter - Windows x64 Release build`n"
    $vswhere = Join-Path ${env:ProgramFiles(x86)} "Microsoft Visual Studio\Installer\vswhere.exe"
    if (-not (Test-Path $vswhere)) {
        throw "Install Visual Studio 2026 or later with Desktop development with C++ and C++ CMake tools for Windows."
    }
    $vs = & $vswhere -latest -products '*' -version '[18.0,)' `
        -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
    if (-not $vs) {
        throw "Visual Studio 2026+ with the x64 C++ toolchain was not found. Add the Desktop development with C++ workload."
    }
    $vs = "$vs".Trim()
    $devcmd = Join-Path $vs "Common7\Tools\VsDevCmd.bat"
    if (-not (Test-Path $devcmd)) { throw "Visual Studio developer setup was not found: $devcmd" }
    Write-Host "Visual Studio: $vs"
    # Import the developer environment without changing the user's global PATH.
    $environment = & $env:ComSpec /d /s /c "`"`"$devcmd`" -no_logo -arch=x64 -host_arch=x64 >nul && set`""
    if ($LASTEXITCODE -ne 0) { throw "Could not initialize the Visual Studio x64 compiler." }
    foreach ($line in $environment) {
        if ($line -match '^([^=]+)=(.*)$') {
            [Environment]::SetEnvironmentVariable($Matches[1], $Matches[2], "Process")
        }
    }

    $cmake = Get-Command cmake.exe -ErrorAction SilentlyContinue
    $ninja = Get-Command ninja.exe -ErrorAction SilentlyContinue
    $cmakePath = if ($cmake) { $cmake.Source } else {
        Join-Path $vs "Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"
    }
    $ninjaPath = if ($ninja) { $ninja.Source } else {
        Join-Path $vs "Common7\IDE\CommonExtensions\Microsoft\CMake\Ninja\ninja.exe"
    }
    if (-not (Test-Path $cmakePath) -or -not (Test-Path $ninjaPath)) {
        throw "CMake and Ninja are required. Add C++ CMake tools for Windows in Visual Studio Installer."
    }

    $kit = $null
    if ($QtPath) {
        $kit = Get-QtKit $QtPath
        if (-not $kit) { throw "The supplied Qt path is not a Qt 6.8+ x64 MSVC kit: $QtPath" }
    } else {
        # Explicit environment settings take priority over automatic discovery.
        foreach ($candidate in @($env:QT_ROOT, $env:QTDIR, $env:Qt6_DIR)) {
            $kit = Get-QtKit $candidate
            if ($kit) { break }
        }
        if (-not $kit) {
            $candidates = @()
            foreach ($name in @("qmake.exe", "qmake6.exe")) {
                $commands = @(Get-Command $name -All -ErrorAction SilentlyContinue)
                foreach ($command in $commands) { $candidates += $command.Source }
            }
            # Search standard Qt installer locations on every filesystem drive.
            $roots = @("$env:USERPROFILE\Qt", "$env:LOCALAPPDATA\Qt", "$env:ProgramFiles\Qt")
            foreach ($drive in Get-PSDrive -PSProvider FileSystem) {
                $roots += Join-Path $drive.Root "Qt"
            }
            foreach ($root in ($roots | Select-Object -Unique)) {
                if (-not (Test-Path -LiteralPath $root -PathType Container)) { continue }
                $candidates += $root
                foreach ($versionDir in Get-ChildItem -LiteralPath $root -Directory -ErrorAction SilentlyContinue) {
                    $candidates += $versionDir.FullName
                    foreach ($kitDir in Get-ChildItem -LiteralPath $versionDir.FullName -Directory -ErrorAction SilentlyContinue) {
                        $candidates += $kitDir.FullName
                    }
                }
            }
            $kits = @(foreach ($candidate in ($candidates | Select-Object -Unique)) { Get-QtKit $candidate })
            $kit = $kits | Sort-Object Version -Descending | Select-Object -First 1
        }
    }
    if (-not $kit) {
        throw 'Qt 6.8+ x64 MSVC was not found. Install its MSVC 64-bit kit with the Qt installer, or run build.bat "D:\your-Qt\6.x.x\msvc2022_64". MinGW and ARM kits cannot be used.'
    }
    Write-Host "Qt: $($kit.Root) (version $($kit.Version))"

    $vcpkgRoot = $null
    $vcpkgCommand = Get-Command vcpkg.exe -ErrorAction SilentlyContinue
    $vcpkgCandidates = @($env:VCPKG_ROOT, "$vs\VC\vcpkg", "$env:USERPROFILE\vcpkg", "C:\dev\vcpkg", "C:\vcpkg")
    if ($vcpkgCommand) { $vcpkgCandidates += Split-Path $vcpkgCommand.Source -Parent }
    foreach ($candidate in $vcpkgCandidates) {
        if (-not $candidate) { continue }
        if ((Test-Path "$candidate\vcpkg.exe") -and (Test-Path "$candidate\scripts\buildsystems\vcpkg.cmake")) {
            $vcpkgRoot = $candidate
            break
        }
    }
    if (-not $vcpkgRoot) {
        throw "vcpkg is required for OpenSSL and libzip. Install/bootstrap vcpkg and set VCPKG_ROOT to its folder. See README.md."
    }
    Write-Host "vcpkg: $vcpkgRoot"
    Write-Host "Missing OpenSSL/libzip packages will be downloaded and built; Qt will NOT be reinstalled."
    # Separate manifest and build folder avoid rebuilding Qt through the root manifest
    # and avoid reusing the Visual Studio preset's cache with another Qt installation.
    $build = Join-Path $project "out\build\detected-qt"
    $env:PATH = "$($kit.Root)\bin;$(Split-Path $ninjaPath -Parent);$env:PATH"
    Run-Checked $cmakePath @(
        "-S", $project, "-B", $build, "-G", "Ninja",
        "-DCMAKE_BUILD_TYPE=Release",
        "-DCMAKE_MAKE_PROGRAM=$ninjaPath",
        "-DCMAKE_C_COMPILER=cl.exe", "-DCMAKE_CXX_COMPILER=cl.exe",
        "-DCMAKE_TOOLCHAIN_FILE=$vcpkgRoot\scripts\buildsystems\vcpkg.cmake",
        "-DVCPKG_TARGET_TRIPLET=x64-windows",
        "-DVCPKG_MANIFEST_DIR=$PSScriptRoot\windows-deps",
        "-DVCPKG_APPLOCAL_DEPS=ON",
        "-DQt6_DIR=$($kit.Root)\lib\cmake\Qt6",
        "-DCMAKE_PREFIX_PATH=$($kit.Root)"
    )
    Run-Checked $cmakePath @("--build", $build, "--parallel")

    # Supply Qt runtime DLLs/plugins so the output can be launched from Explorer.
    $deploy = Join-Path $kit.Root "bin\windeployqt.exe"
    if (-not (Test-Path $deploy)) { throw "Build succeeded, but windeployqt.exe is missing from the Qt kit. Install Qt's deployment tools." }
    Run-Checked $deploy @("--release", "--compiler-runtime", (Join-Path $build "jotter.exe"))
    Write-Host "`nBuild complete. Launch:`n$build\jotter.exe" -ForegroundColor Green
    Write-Host "Keep the accompanying DLLs and plugins with the executable."
    exit 0
} catch {
    Write-Host "`nERROR: $($_.Exception.Message)" -ForegroundColor Red
    exit 1
}
