

function get_first {
  ls $1 | head -n 1
}

behöva uppdatera sökvägar
#!/bin/sh
WINE=${WINE:-wine}
#WINEPREFIX=${WINEPREFIX:-$HOME/.local/share/wineprefixes/psdkwin7}
#export WINEPREFIX
PROGRAMFILES="c:\Program Files"
WSDK=$(get_first "$PROGRAMFILES\Microsoft Visual Studio\2017")

echo $WSDK
#WPSDK="$PROGRAMFILES\Windows Kits\10"
#export WINEPATH="c:\windows;c:\windows\system32;$WSDK\Community\Common7\IDE;$WSDK\Community\VC\Tools\MSVC\14.16.27023\bin\Hostx86\x64"
#export INCLUDE="$WSDK\Community\VC\Tools\MSVC\14.16.27023\include;$WPSDK\Include\10.0.17763.0\um;$WPSDK\Include\10.0.17763.0\shared;$WDXSDK\Include;$WPSDK\Include\10.0.17763.0\ucrt"
#export LIB="$WSDK\Community\VC\Tools\MSVC\14.16.27023\lib\x64;$WPSDK\Lib\10.0.17763.0\um\x64;$WPSDK\Lib\10.0.17763.0\ucrt\x64"

#$WINE cl.exe $@
