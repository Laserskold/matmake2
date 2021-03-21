
setlocal

mkdir build

cd build

call "%PROGRAMFILES(X86)%\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat" x86_amd64

cl.exe /I..\include /I..\lib/json.h/include /I..\src ..\matmake.cpp ^
     /std:c++17 /EHa /D NOMINMAX /link /out:matmake2.exe /DEBUG:FULL

echo matmake created in build/

cd ../
