# Install powershell 7 on macOS, Ubuntu, ArchLinux
# usage: ./install-pwsh [pwsh_ver]
#

HOST_OS=$(uname)

myRoot=$(dirname "$0")

mkdir -p $myRoot/tmp

pwsh_ver=$1
if [ "$pwsh_ver" = "" ] ; then
    pwsh_ver='7.3.7'
fi

function check_pwsh {
    pwsh_ver=$1
    if command -v pwsh >/dev/null ; then
        pwsh_veri_a=$(pwsh --version)
        pwsh_veri_b="PowerShell $pwsh_ver"
        if [[ "$pwsh_veri_b" < "$pwsh_veri_a" || "$pwsh_veri_b" == "$pwsh_veri_a" ]] ; then
            echo axmol: $pwsh_veri_a already installed.
            exit 0
        fi
    fi
    "Installing PowerShell $pwsh_ver ..."
}

HOST_ARCH=$(uname -m)
if [ "$HOST_ARCH" = 'x86_64' ] ; then
    HOST_ARCH=x64
fi

if [ $HOST_OS = 'Darwin' ] ; then
    check_pwsh $pwsh_ver
    pwsh_pkg="powershell-$pwsh_ver-osx-$HOST_ARCH.pkg"
    pwsh_pkg_out="$myRoot/tmp/$pwsh_pkg"
    if [ ! -f  "$pwsh_pkg_out" ] ; then
        # https://github.com/PowerShell/PowerShell/releases/download/v7.3.6/powershell-7.3.6-osx-x64.pkg
        pwsh_url="https://github.com/PowerShell/PowerShell/releases/download/v$pwsh_ver/$pwsh_pkg"
        echo "Downloading $pwsh_url ..."
        curl -L "$pwsh_url" -o "$pwsh_pkg_out"
    fi
    sudo xattr -rd com.apple.quarantine "$pwsh_pkg_out"
    sudo installer -pkg "$pwsh_pkg_out" -target /
elif [ $HOST_OS = 'Linux' ] ; then
    if which dpkg > /dev/null; then  # Linux distro: deb (ubuntu)
        check_pwsh $pwsh_ver
        pwsh_pkg="powershell_$pwsh_ver-1.deb_amd64.deb"
        pwsh_pkg_out="$myRoot/tmp/$pwsh_pkg"
        if [ ! -f  "$pwsh_pkg_out" ] ; then
            curl -L "https://github.com/PowerShell/PowerShell/releases/download/v$pwsh_ver/$pwsh_pkg" -o "$pwsh_pkg_out"
        fi
        sudo dpkg -i "$pwsh_pkg_out"
        sudo apt-get install -f
    elif which pacman > /dev/null; then # Linux distro: Arch
        check_pwsh '7.3.6'
        # refer: https://ephos.github.io/posts/2018-9-17-Pwsh-ArchLinux
        git clone https://aur.archlinux.org/powershell-bin.git $myRoot/tmp/powershell-bin
        cd $myRoot/tmp/powershell-bin
        makepkg -si --needed --noconfirm
        cd -
    fi
else
    echo "Unsupported HOST OS: $HOST_OS"
    exit 1
fi

if [ $? = 0 ] ; then
    echo "Install PowerShell $pwsh_ver done"
else
    echo "Install PowerShell fail"
    if [ -f "$pwsh_pkg_out" ] ; then
        rm -f "$pwsh_pkg_out"
    fi
fi
