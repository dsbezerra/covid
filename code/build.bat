@echo off

set IncludePath="..\covid\libs\include"
set LibPath="..\covid\libs"

set CommonCompilerFlags=-MTd -nologo -Gm- -GR- -EHa- -Od -Oi -WX -W4 -wd4201 -wd4100 -wd4189 -wd4505 -FC -Z7
set CommonCompilerFlags=-DDEVELOPMENT=1 %CommonCompilerFlags%
set CommonLinkerFlags=-incremental:no -opt:ref user32.lib gdi32.lib myhtml_static.lib Winhttp.lib opengl32.lib

set ReleaseCompilerFlags=-MTd -O2 -Oi

if not exist ..\..\build mkdir ..\..\build
pushd ..\..\build

REM Debug 64-bit build
del *.pdb > NUL 2> NUL
cl /I %IncludePath% %CommonCompilerFlags% -D_CRT_SECURE_NO_WARNINGS ..\covid\code\win32_covid.cpp -Fmwin32_covid.map /link /LIBPATH:%LibPath% %CommonLinkerFlags% /NODEFAULTLIB:libcmtd.lib

REM Release 64-bit build:
REM cl /I %IncludePath% %ReleaseCompilerFlags% -D_CRT_SECURE_NO_WARNINGS ..\covid\code\win32_covid.cpp /link /LIBPATH:%LibPath% %CommonLinkerFlags% /NODEFAULTLIB:libcmtd.lib

popd
