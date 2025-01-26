@echo off
setlocal EnableDelayedExpansion

echo - RELEASE Build: Start
set cl_optimization=-O2 -DRELEASE=1
set cl_includes=-I ..\..\base\code
set cl_flags=/Zi /nologo -std:c++latest /Wall /WX 
set cl_imgui_flags=/Zi /nologo -std:c++latest
set cl_ignore_warnings=-wd4201 -wd4189 -wd4101 -wd4505 -wd4820 -wd5045 -wd4996 -wd4100 -wd4668 -wd4711 -wd4062 -wd4388 -wd4018 -wd4459 -wd4626  

IF NOT EXIST ..\build mkdir ..\build
pushd ..\build
cl /EHsc %cl_optimization% %cl_imgui_flags% %cl_includes% ..\code\main.cpp ..\code\imgui\imgui*.cpp ..\code\tinyfiledialogs\tinyfiledialogs.cpp /Febudgeteer.exe
popd
echo - RELEASE Build: Done 

::echo - Creating release zip
:: create release dir if it doesnt exist
IF NOT EXIST ..\release mkdir ..\release

pushd ..\release

:: delete budgeteer dir and recreate it
echo - Deleting contents of /release dir 
IF EXIST budgeteer (
    rmdir /S /Q budgeteer
)
mkdir budgeteer

::if exist budgeteer.zip del budgeteer.zip
::if exist budgeteer.exe del budgeteer.exe
::if exist config.confg del config.confg
::if exist README.md del README.md
::if exist assets del /Q assets

:: copy over save dir and create empty budget file
::rmdir /S /Q .\saves
::mkdir .\saves
::echo. > .\saves\budget.b

:: copy over exe, config, and md files
echo - Copying over files
copy /Y ..\build\budgeteer.exe .\budgeteer
copy /Y ..\build\config.conf .\budgeteer
copy /Y ..\README.md .\budgeteer

:: copy over assets
xcopy /S /Q /Y "..\build\assets" .\budgeteer\assets\

:: Delete the existing zip file if it exists
::if exist budgeteer.zip del budgeteer.zip

:: Zip the contents of the release folder using 7-Zip
echo - Zipping contents into budgeteer.zip
"C:\Program Files\7-Zip\7z.exe" a budgeteer.zip budgeteer
popd

echo - Done
pause

