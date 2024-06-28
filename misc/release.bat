echo off

rem find all files F
rem zip F
rem move F to release dir
rem git add
rem git commit
rem git push 

set clangcl_flags=-I ..\..\base\code -Od -Z7 -Wconversion -Werror -Wall -Wextra 
rem review these at some poins
set clangcl_ignore_warnings=-Wno-language-extension-token -Wno-writable-strings -Wno-switch -Wno-c99-designator -Wno-microsoft-goto -Wno-deprecated-declarations -Wno-tautological-constant-out-of-range-compare -Wno-missing-declarations -Wno-double-promotion -Wno-double-promotion -Wno-c++17-extensions -Wno-old-style-cast -Wno-gnu-anonymous-struct -Wno-missing-braces -Wno-float-equal -Wno-c++98-compat-pedantic -Wno-nested-anon-types -Wno-c11-extensions -Wno-zero-as-null-pointer-constant -Wno-unused-variable -Wno-comma -Wno-shadow -Wno-missing-field-initializers -Wno-cast-align -Wno-cast-qual -Wno-covered-switch-default -Wno-unused-macros -Wno-global-constructors -Wno-unused-parameter -Wno-format-nonliteral -Wno-unused-function -Wno-header-hygiene -Wno-c99-extensions -Wno-nonportable-system-include-path -Wno-sign-compare
rem I don't know if I want this
set compiler_defines=/D DEBUG=%DEBUG% /D RELEASE=%RELEASE%
set linker_flags=-incremental:no -opt:ref

set DEBUG=0
if %DEBUG% == 1 (
    echo - DEBUG 
    set optimization=-Od -DDEBUG=1
) ELSE (
    echo - RELEASE 
    set optimization=-O2 -DRELEASE=1
)

set cl_includes=-I ..\..\base\code
set cl_flags=-Zi -nologo -std:c++latest -Wall -WX 
set cl_ignore_warnings=-wd4201 -wd4189 -wd4101 -wd4505 -wd4820 -wd5045 -wd4996 -wd4100 -wd4668 -wd4711 -wd4062 -wd4388 -wd4018 -wd4459 -wd4626  
rem C4201: nonstandard extension used: nameless struct/union
rem C4189: local variable is initialized but not referenced
rem C4101: unreferenced local variable
rem rem C4505: unreferenced local function has been removed
rem C4820: bytes padding added after data member
rem C5045: Compiler will insert Spectre mitigation for memory load if /Qspectre switch specified
rem C4996: This function or variable may be unsafe. 
rem C4100: unreferenced formal parameter
rem C4668: <term> is not defined as a preprocessor macro, replacing with '0' for '#if/#elif'
rem C4711: function selected for automatic inline expansion
rem rem C4710: function not inlined
rem C4062: enumerator in switch of enum is not handled
rem C4388: '<': signed/unsigned mismatch
rem C4018: '>': signed/unsigned mismatch
rem C4459: declaration of <variable> hides global declaration
rem
rem C4626: remove and understand (something to do with defer)

IF NOT EXIST ..\build mkdir ..\build
pushd ..\build
rem IF NOT EXIST data move ..\data . 
rem clang-cl %clangcl_flags% %clangcl_ignore_warnings% ..\code\main.cpp %compiler_defines% -link %linker_flags% %linker_libs% -MAP
cl %cl_flags% %optimization% %cl_ignore_warnings% %cl_includes% ..\code\main.cpp 
popd
