@echo off

IF NOT EXIST ..\release mkdir ..\release
pushd ..\release
rmdir /S /Q .\saves
mkdir .\saves
echo. > .\saves\budget.b

copy /Y ..\build\budgeteer.exe .
copy /Y ..\build\config.conf .

xcopy /Y ..\build\shaders .\shaders /E /I /H
popd

