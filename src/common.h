#define NOMINMAX            1
#define WIN32_LEAN_AND_MEAN 1
#define WIN32_MEAN_AND_LEAN 1
#define VC_EXTRALEAN        1
#include <windows.h>
#undef NOMINMAX
#undef WIN32_LEAN_AND_MEAN
#undef WIN32_MEAN_AND_LEAN
#undef VC_EXTRALEAN
#undef far
#undef near

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>

typedef unsigned __int8  u8;
typedef unsigned __int16 u16;
typedef unsigned __int32 u32;
typedef unsigned __int64 u64;

typedef signed __int8  s8;
typedef signed __int16 s16;
typedef signed __int32 s32;
typedef signed __int64 s64;

typedef u64 umm;
typedef s64 smm;

typedef float f32;
typedef double f64;

typedef u8 bool;
#define true 1
#define false 0

#define PI       3.14159265358979323846264338327950
#define PI_DIV_2 1.57079632679489661923132169163975

#define ARRAY_SIZE(A) (sizeof(A)/sizeof(0[A]))

bool
Char_IsWhitespace(u8 c)
{
  return (c == ' ' || c == '\n' || c == '\r' || c == '\t');
}

bool
Char_IsDigit(u8 c)
{
  return ((u8)(c-0x30) < (u8)10);
}

bool
Char_IsHexAlphaDigit(u8 c)
{
  return ((u8)((c&0xDF) - 'A') <= (u8)('F' - 'A'));
}

bool
ReadEntireFile(char* filename, void** contents, u64* size)
{
  bool succeeded = false;
  
  FILE* file;
  struct __stat64 file_stat;

  if (fopen_s(&file, filename, "rb") == 0)
  {
    if (_stat64(filename, &file_stat) == 0)
    {
      u8* memory = malloc(file_stat.st_size + 1);

      if (memory != 0 && fread(memory, 1, file_stat.st_size, file) == file_stat.st_size)
      {
        memory[file_stat.st_size] = 0;

        *contents = memory;
        *size     = file_stat.st_size;
        succeeded = true;
      }
    }

    fclose(file);
  }

  return succeeded;
}

u64
EstimateRDTSCFrequency(u32 timing_interval_ms)
{
  LARGE_INTEGER perf_freq;
  QueryPerformanceFrequency(&perf_freq);
  s64 timing_interval_ticks = ((s64)timing_interval_ms * perf_freq.QuadPart)/1000;

  LARGE_INTEGER start_tick;
  QueryPerformanceCounter(&start_tick);

  u64 start_cpu_tick = __rdtsc();

  LARGE_INTEGER end_tick;
  for (;;)
  {
    QueryPerformanceCounter(&end_tick);
    if (end_tick.QuadPart - start_tick.QuadPart >= timing_interval_ticks) break;
    else                                                                  continue;
  }

  u64 end_cpu_tick = __rdtsc();

  u64 elapsed_cpu_ticks  = end_cpu_tick - start_cpu_tick;
  u64 elapsed_wall_ticks = end_tick.QuadPart - start_tick.QuadPart;

  return elapsed_cpu_ticks * (perf_freq.QuadPart / elapsed_wall_ticks);
}
