@echo off
setlocal EnableDelayedExpansion

set DEBUG=1
if !DEBUG! == 1 (
    echo - DEBUG 
    set cl_optimization=-Od -DDEBUG=1
) ELSE (
    echo - RELEASE 
    set cl_optimization=-O2 -DRELEASE=1
)

set cl_includes=-I ..\..\base\code
set cl_flags=/Zi /nologo -std:c++latest /Wall /WX /diagnostics:caret /diagnostics:color
set cl_imgui_flags=/Zi /nologo -std:c++latest
set cl_ignore_warnings=-wd4201 -wd4189 -wd4101 -wd4505 -wd4820 -wd5045 -wd4996 -wd4100 -wd4668 -wd4711 -wd4062 -wd4388 -wd4018 -wd4459 -wd4626  
rem C4201: nonstandard extension used: nameless struct/union
rem C4189: local variable is initialized but not referenced
rem C4101: unreferenced local variable
rem C4505: unreferenced local function has been removed
rem C4820: bytes padding added after data member
rem C5045: Compiler will insert Spectre mitigation for memory load if /Qspectre switch specified
rem C4996: This function or variable may be unsafe. 
rem C4100: unreferenced formal parameter
rem C4668: <term> is not defined as a preprocessor macro, replacing with '0' for '#if/#elif'
rem C4711: function selected for automatic inline expansion
rem C4710: function not inlined
rem C4062: enumerator in switch of enum is not handled
rem C4388: '<': signed/unsigned mismatch
rem C4018: '>': signed/unsigned mismatch
rem C4459: declaration of <variable> hides global declaration
rem C4626: remove and understand (something to do with defer)

IF NOT EXIST ..\build mkdir ..\build
pushd ..\build
cl /EHsc %cl_optimization% %cl_imgui_flags% %cl_includes% ..\code\meta.cpp 
if %ERRORLEVEL% == 0 (
    meta.exe
)

cl /EHsc %cl_optimization% %cl_imgui_flags% %cl_includes% ..\code\main.cpp ..\code\imgui\imgui*.cpp ..\code\tinyfiledialogs\tinyfiledialogs.cpp /Febudgeteer.exe
popd

