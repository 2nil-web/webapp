
# This Makefile header needs to be executed under a posix environment with GNU make, whether it is a *nix system or Windows with msys2, cygwin ...
# Under Windows it tries first to compile with MSVC if it is available (see MSBUILD and DO_MSBUILD variables) else it falls back to gcc
# Under *nix it tries first to compile with gcc to produce a linux binary.
# To force gcc compiling under Windows then invoke the 'gcc' target.
# To force cross-compiling a Windows binary under Linux then invoke the 'w64' target (needs more thorough testing ...)
# Not tested under Mac OS but should work with the correct prerequisite (posix env, GNU make ...)

# OS is defined only under Windows
ifeq (${OS},Windows_NT)
SYS_VER=${OS}_$(shell  wmic os get Version |grep -v Version | tr -d ' ')
# Until now no need or will to cross compile under Windows for Linux ... May change one day ...
TARGET_API=windows
EXEXT=.exe
RC=windres${EXEXT}
ECHO=echo -e
PGF=$(subst \,/,$(subst C:\,/c/,$(PROGRAMFILES)))
PGF86=${PGF} (x86)
LOCAD=$(subst \,/,$(subst C:\,/c/,$(LOCALAPPDATA)))
PGU=${LOCAD}/Programs
PATH:=${PATH}:${PGF86}/Inno Setup 6
PATH:=${PATH}:${PGF}/Inkscape/bin
PATH:=${PATH}:${PGU}/Inkscape/bin
PATH:=${PATH}:${PGF86}/Pandoc
MAGICK=magick
else # If not defined then set it to "uname -s" value
ifeq (${OS},)
OS=$(shell uname -s)
endif
MAGICK=convert
endif

# Available at least under Fedora, Debian, Ubuntu, Arch and even under msys2
OS_ID=$(shell sed -n 's/^ID=//p' /etc/os-release)

CPPFLAGS += -DMY_WEBVIEW_EXT

ifeq ($(OS),Linux)
BUILD_SYS=linux_gcc
# Cross compiling for Windows under Linux
ifeq ($(MAKECMDGOALS),w64)
TARGET_API=windows
CPPFLAGS += -DWIN32_LEAN_AND_MEAN
# For IDesktopWallpaper
CPPFLAGS += -DNTDDI_VERSION=NTDDI_WIN8
CC=/usr/bin/x86_64-w64-mingw32-gcc-win32${EXEXT}
CXX=/usr/bin/x86_64-w64-mingw32-g++-win32${EXEXT}
RC=/usr/bin/x86_64-w64-mingw32-windres${EXEXT}
STRIP=/usr/bin/x86_64-w64-mingw32-strip${EXEXT}
CPPFLAGS += -I /usr/share/mingw-w64/include
CPPFLAGS += -I /usr/lib/gcc/x86_64-w64-mingw32/12-win32/include
LDFLAGS += -L /usr/x86_64-w64-mingw32/lib
else
TARGET_API=linux
endif	
ifeq ($(OS_ID),arch)
SYS_VER=Arch_Linux_$(shell sed -n 's/^VERSION_ID=//p' /etc/os-release)
else
SYS_VER=$(shell lsb_release -irs | sed 'N;s/\n/_/' | sed 's/ /_/g')
endif
ECHO=echo
endif

mkfile_path := $(abspath $(lastword $(MAKEFILE_LIST)))
mkfile_dir := $(patsubst %/,%,$(dir $(mkfile_path)))
current_dir := $(notdir $(mkfile_dir))

STRIP=strip
UPX=upx

VERSION=$(shell git describe --abbrev=0 --tags 2>/dev/null || echo 'Unknown_version')
COPYRIGHT=(C) D. LALANNE - MIT License.
DECORATION=
COMMIT=$(shell git rev-parse --short HEAD 2>/dev/null || echo 'Unknown_commit')
ISO8601 := $(shell date +%Y-%m-%dT%H:%M:%SZ)
TMSTAMP := $(shell date +%Y%m%d%H%M%S)

WVDIR=${mkfile_dir}/webview
CPPFLAGS += -I ${mkfile_dir}/webview/core/include/webview


ifeq ($(MAKECMDGOALS),gcc)
DO_MSBUILD=no
endif

ifeq ($(MAKECMDGOALS),gcc_cons)
DO_MSBUILD=no
endif

ifneq ($(DO_MSBUILD),no)
MSBUILD=/c/Program\ Files/Microsoft\ Visual\ Studio/2022/Community/MSBuild/Current/Bin/amd64/MSBuild.exe
DO_MSBUILD=$(shell test -f $(MSBUILD) && echo "yes" || echo "no")
endif

ifeq ($(DO_MSBUILD),yes)
BUILD_SYS=win_msvc
# These 2 environment variables caused troubles to the Visual Studio build prior to 17.13.2
# But after that they are necessary to avoid warning MSB8029 ... What a mess !!
# Let's wait for another change soon ...
#unexport TMP TEMP
ifeq ($(CONF),)
CONF=Release
endif
# If using MSVC then TARGET_API cannot be something else than windows
TARGET_API=windows
# We're only compiling for x64 under msvc
TARGET_DIR=build/msvc/win/x64/${CONF}
TARGET_DIR_CONS=build/msvc/win_cons/x64/${CONF}
ifeq ($(MAKECMDGOALS),msvc_cons)
TARGET_DIR=${TARGET_DIR_CONS}
endif
else
ifeq (${OS},Windows_NT)
BUILD_SYS=win_gcc
# compiling for windows api under windows gcc (msys2, cygwin ...)
TARGET_API=windows
msystem_low=$(shell echo ${MSYSTEM} | tr '[:upper:]' '[:lower:]')
# Considering different target dir whether we compile with gcc mingw64/ucrt64/clang64
TARGET_DIR=build/gcc/win/${msystem_low}
TARGET_DIR_CONS=build/gcc/win_cons/${msystem_low}
else
ifeq ($(MAKECMDGOALS),w64)
TARGET_API=windows
# Cross compiling for Windows under Linux (but not differencing whether it is mingw64/ucrt64/clang64, for now)
TARGET_DIR=build/gcc/cross_win
else
TARGET_DIR=build/gcc/${OS_ID}
endif
endif
endif


# Pour éviter 'std::aligned_storage' is deprecated' => CXXFLAGS += -std=c++20
# Pour éviter 'multiple definition of `std::type_info::operator==(std::type_info const&) const' ==> CXXFLAGS += -std=c++23 -fno-rtti
#CXXFLAGS += -fno-rtti -std=c++23
CXXFLAGS += -std=c++20
ifeq ($(patsubst %_gcc,gcc,$(BUILD_SYS)),gcc)
CXXFLAGS += -g
#CXXFLAGS += -O -s
CXXFLAGS += -Wall # -pedantic -Wextra # Utiliser ces 2 dernières options de temps en temps peut-être utile ...
CXXFLAGS += -Wno-unknown-pragmas
#LDFLAGS += -O -s
LDFLAGS += -g
#LDFLAGS += -fno-rtti
endif

ifeq ($(TARGET_API),windows)
export GCC_PATH=/mingw64/bin
export PATH:=${GCC_PATH}:${PATH}
ifeq (${MSYSTEM},CLANG64)
CC=${GCC_PATH}/clang${EXEXT}
CXX=${GCC_PATH}/clang++${EXEXT}
else
GCC=${GCC_PATH}/gcc${EXEXT}
CC=${GCC_PATH}/gcc${EXEXT}
CXX=${GCC_PATH}/g++${EXEXT}
endif
WV2SUBDIR=Microsoft.Web.WebView2.1.0.3537.50
CPPFLAGS += -I ${WVDIR}/build/external/libs/${WV2SUBDIR}/build/native/include
CPPFLAGS += -DWIN32_LEAN_AND_MEAN
# For IDesktopWallpaper
CPPFLAGS += -DNTDDI_VERSION=NTDDI_WIN8
#CPPFLAGS += -D_CONSOLE
LDFLAGS += -mwindows
LDFLAGS += -static
LDLIBS += -lwldap32 -lws2_32 -ladvapi32 -lole32 -lshell32 -lshlwapi -loleaut32 -lcomctl32 -lcomdlg32 -lactiveds -luser32 -luuid -lnetapi32 -lmpr -lpathcch -ldwmapi -lversion	
endif

LD=${CXX}

# GTK mess to deal with, make a choice ...
# The GTK 4 version works under unbuntu 24.04, fedora 40, Alma linux and Arch but I think it still has some bugs (messy filter management, for example)
# Even if it seems to be considered as a feature and not a bug (🐞🤕) but GTK 4 is also missing gtk_window_move
# And finally debian 12 has a very buggy >GTK 4 version (not a GTK 4 issue though)
#GTK_WEBKIT="gtk4 webkitgtk-6.0 webkit2gtk-web-extension-6.0 gtkmm-4.0"
# I personnally considere the GTK 3 has the only stable and almost ok for production, version (as of 2025/08).
# Also I only have a Windows 10/11 and Debian 12 machine to make my tests, for the rest I only test on VM, from time to time ...
# And finally I don't have any MacOS machine to make my test and compiling, so Cocoa is actually not taken into account.
#GTK_WEBKIT="gtk+-3.0 webkit2gtk-4.1 webkit2gtk-web-extension-4.1 gtkmm-3.0"

ifeq (${TARGET_API},linux)
ifeq ($(OS_ID),debian)
GTK_WEBKIT="gtk+-3.0 webkit2gtk-4.1"
else
GTK_WEBKIT="gtk4 webkitgtk-6.0"
endif

LDLIBS += -lX11
CXXFLAGS += $(shell pkg-config --cflags ${GTK_WEBKIT})
#LDFLAGS +=-L/usr/lib/x86_64-linux-gnu -L/usr/lib/x86_64-linux-gnu/webkit2gtk-${WK2GV} -L/usr/lib/x86_64-linux-gnu/cmake/harfbuzz -L/usr/lib/python3/dist-packages/cairo -L/usr/lib/x86_64-linux-gnu/glib-2.0 -L/usr/lib/x86_64-linux-gnu/glib-2.0
LDLIBS += $(shell pkg-config --libs ${GTK_WEBKIT})
#LDFLAGS += -static
LDFLAGS += -static-libgcc -static-libstdc++
endif

PANDOC=pandoc

ifeq ($(MAKECMDGOALS),)
GOALS=default
else
GOALS=$(MAKECMDGOALS)
endif

# SYS_VER ==> Windows_NT_10.X.YYYYY | Windows_NT_11.X.YYYYY | Ubuntu_24.04 | Fedora_40 | Arch_Linux_20240721.0.248532
# BUILD_SYS ==> win_gcc | win_msvc | linux_gcc
# TARGET_API => windows | linux
# TARGET_DIR => msvc/x64/${CONF} gcc/${SYS_VER}


