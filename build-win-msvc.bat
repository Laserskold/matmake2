
mkdir build
mkdir "build\msvc"

cd build/msvc

call "%PROGRAMFILES(X86)%\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat" x86_amd64

cl.exe /I..\..\include /I..\..\lib/json.h/include /I..\..\src ..\..\matmake.cpp ^
     /std:c++17 /EHa /D NOMINMAX /link /out:matmake.exe

echo matmake created in build/msvc
