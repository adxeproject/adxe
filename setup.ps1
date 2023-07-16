$myRoot = $PSScriptRoot
$AX_ROOT = $myRoot

$build1kPath = Join-Path $myRoot '1k/build1k.ps1'
$prefix = Join-Path $myRoot 'tools/external'
if (!(Test-Path $prefix -PathType Container)) {
    mkdir $prefix | Out-Null
}

# setup toolchains: glslcc, cmake, ninja, ndk, jdk, ...
. $build1kPath -setupOnly $true -prefix $prefix

$AX_CONSOLE_BIN = Join-Path $AX_ROOT 'tools/console/bin'

# https://learn.microsoft.com/en-us/powershell/module/microsoft.powershell.core/about/about_environment_variables
$IsWin = $IsWindows -or ("$env:OS" -eq 'Windows_NT')

if ($IsWin) {
    if ($env:AX_ROOT -ne $AX_ROOT) {
        [Environment]::SetEnvironmentVariable('AX_ROOT', $AX_ROOT, 'User')
    }

    $pathList = [System.Collections.ArrayList]$env:PATH.Split(';')
    if ($pathList.IndexOf($AX_CONSOLE_BIN) -eq -1) {
        $pathList = [System.Collections.ArrayList][Environment]::GetEnvironmentVariable('PATH', 'User').Split(';')
        $pathList.Insert(0, $AX_CONSOLE_BIN)
        $PATH = $pathList -join ';'
        [Environment]::SetEnvironmentVariable('PATH', $PATH, 'User')
    }
}
else {
    $PATH = [System.Collections.ArrayList]$env:PATH.Split(':')
    if (Test-Path $PROFILE -PathType Leaf) {
        $profileContent = Get-Content $PROFILE -raw
    }
    else {
        $profileContent = ''
    }

    $needSave = $false
    if ($profileContent.IndexOf('$env:AX_ROOT = ') -eq -1) {
        $profileContent += '$env:AX_ROOT = "{0}"{1}' -f $AX_ROOT, "`n"
        $needSave = $true
    }

    if ($profileContent.IndexOf('$env:PATH = ') -eq -1) {
        $profileContent += '$env:PATH = "{0}:$env:PATH"' -f $AX_CONSOLE_BIN
        $needSave = $true
    }

    $profileDir = Split-Path $PROFILE -Parent
    if (!(Test-Path $profileDir -PathType Container)) {
        mkdir $profileDir | Out-Null
    }

    if ($needSave) {
        Set-Content $PROFILE -Value $profileContent
    }
}

if ($IsLinux) {
    Write-Host "Are you continue install linux dependencies for axmol? (y/n) " -NoNewline
    $answer = Read-Host
    if ($answer -like 'y*') {
        b1k_print "It will take few minutes"
        sudo apt update
        # for vm, libxxf86vm-dev also required

        $DEPENDS = @()

        $DEPENDS += 'libx11-dev'
        $DEPENDS += 'automake'
        $DEPENDS += 'libtool'
        $DEPENDS += 'cmake'
        $DEPENDS += 'libxmu-dev'
        $DEPENDS += 'libglu1-mesa-dev'
        $DEPENDS += 'libgl2ps-dev'
        $DEPENDS += 'libxi-dev'
        $DEPENDS += 'libzip-dev'
        $DEPENDS += 'libpng-dev'
        $DEPENDS += 'libfontconfig1-dev'
        $DEPENDS += 'libgtk-3-dev'
        $DEPENDS += 'binutils'
        $DEPENDS += 'libbsd-dev'
        $DEPENDS += 'libasound2-dev'
        $DEPENDS += 'libxxf86vm-dev'
        $DEPENDS += 'libvlc-dev', 'libvlccore-dev', 'vlc'

        # if vlc encouter codec error, install
        # sudo apt install ubuntu-restricted-extras

        sudo apt install --allow-unauthenticated --yes $DEPENDS > /dev/null

        b1k_print "Installing latest freetype for linux ..."
        mkdir buildsrc
        Set-Location buildsrc
        git clone 'https://github.com/freetype/freetype.git'
        Set-Location freetype
        git checkout 'VER-2-13-0'
        sh autogen.sh
        ./configure '--prefix=/usr' '--enable-freetype-config' '--disable-static'
        sudo make install
        Set-Location ..
        Set-Location ..
    }
}

$b1k.pause("setup successfully")
