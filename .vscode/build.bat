@echo off
if "%1" == "x64" (
	set "PYTHONINC=C:\Program Files\Python36\include"
) else (
	set "PYTHONINC=C:\Program Files (x86)\Python36-32\include"
)
for /F "delims=" %%l in (.vscode/params) do set "%%l"
call "C:\Program Files (x86)\Microsoft Visual Studio\2017\BuildTools\VC\Auxiliary\Build\vcvarsall.bat" %*
mkdir "build"
cl /nologo /w44145 /w44715 /w44005 /Zi /DEBUG /FC /Fe"build\%FILE%.pyd" /Fo"build\\" /Fd"build\\" /LD /I"%PYTHONINC%" "%CSRCDIR%\%FILE%.c" user32.lib shell32.lib comctl32.lib gdi32.lib python3.lib
mt -nologo -manifest "%CSRCDIR%\%FILE%.pyd.manifest" -outputresource:"build\%FILE%.pyd;2"
move /Y "build\%FILE%.pyd" "%PACKAGE%\" > nul
move /Y "build\%FILE%.pdb" "%PACKAGE%\" > nul
move /Y "build\vc140.pdb" "%PACKAGE%\" > nul
rmdir /Q /S "build"
@echo on
