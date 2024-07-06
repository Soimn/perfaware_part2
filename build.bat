@echo off

setlocal

cd %~dp0

if not exist build mkdir build
cd build

if "%Platform%" neq "x64" (
	echo ERROR: Platform is not "x64" - please run this from the MSVC x64 native tools command prompt.
	goto end
)

set "common_compile_options= /nologo /W3 /wd4116"
set "common_link_options= /incremental:no /opt:ref /subsystem:console"

set "debug_compile_options=%common_compile_options% /Od /Zo /Z7 /RTC1 /MTd"
set "release_compile_options=%common_compile_options% /O2"
set "link_options=%common_link_options%"

if "%1"=="debug" (
  set "compile_options=%debug_compile_options%"
) else if "%1"=="release" (
  set "compile_options=%release_compile_options%"
) else (
  goto invalid_arguments
)

cl %compile_options% ..\listings\listing_0066_haversine_generator_main.cpp /link %link_options% /pdb:haversine_gen_ref.pdb /out:haversine_gen_ref.exe
cl %compile_options% ..\listings\listing_0067_simple_haversine_main.cpp /link %link_options% /pdb:haversine_ref.pdb /out:haversine_ref.exe

cl %compile_options% ..\src\haversine_gen.c /link %link_options% /pdb:haversine_gen.pdb /out:haversine_gen.exe
cl %compile_options% ..\src\haversine_proc.c /link %link_options% /pdb:haversine_proc.pdb /out:haversine_proc.exe

goto end

:invalid_arguments
echo Invalid arguments^. Usage: build ^[debug^/release^]
goto end

:end
endlocal
