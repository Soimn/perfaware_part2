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
set "release_compile_options=%common_compile_options% /O2 /Zo /Z7"
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

cl %compile_options% ..\src\reptest_read.c /link %link_options% /pdb:reptest_read.pdb /out:reptest_read.exe
cl %compile_options% ..\src\pagefault_testing.c /link %link_options% /pdb:pagefault_testing.pdb /out:pagefault_testing.exe

del reptest_write_asm.*
del reptest_write.*

nasm -o reptest_write_asm.obj -f win64 ..\src\reptest_write.asm

cl %compile_options% /O1 ..\src\reptest_write.c /link %link_options% /pdb:reptest_write.pdb /out:reptest_write.exe reptest_write_asm.obj

del reptest_cache_asm.*
del reptest_cache.*

nasm -o reptest_cache_asm.obj -f win64 ..\src\reptest_cache.asm

cl %compile_options% ..\src\reptest_cache.c /link %link_options% /pdb:reptest_cache.pdb /out:reptest_cache.exe reptest_cache_asm.obj

goto end

:invalid_arguments
echo Invalid arguments^. Usage: build ^[debug^/release^]
goto end

:end
endlocal
