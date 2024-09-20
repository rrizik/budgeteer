@echo off

IF NOT EXIST ..\release mkdir ..\release
pushd ..\release

:: copy over save dir and create empty budget file
rmdir /S /Q .\saves
mkdir .\saves
echo. > .\saves\budget.b

:: copy over exe, font, and md files
copy /Y ..\build\budgeteer.exe .
copy /Y ..\build\config.conf .
copy /Y ..\README.md .

:: Delete the existing zip file if it exists
if exist budgeteer.zip del budgeteer.zip

:: Zip the contents of the release folder using 7-Zip
"C:\Program Files\7-Zip\7z.exe" a budgeteer.zip *

popd

