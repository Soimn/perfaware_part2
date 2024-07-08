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

#define ASSERT(EX) ((EX) ? 1 : (AssertHandler(__FILE__, __LINE__, #EX), 0))
#define NOT_IMPLEMENTED() ASSERT(!"NOT_IMPLEMENTED")

#define CONCAT__(A, B) A##B
#define CONCAT_(A, B) CONCAT__(A, B)
#define CONCAT(A, B) CONCAT_(A, B)

void
AssertHandler(char* file, umm line, char* expr)
{
  fprintf(stderr, "ASSERTION FAILED\n%s(%llu): %s\n", file, line, expr);
  __debugbreak();
  *(volatile int*)0 = 0;
}

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

void
Zero(void* p, umm size)
{
  u8* bp = (u8*)p;
  for (umm i = 0; i < size; ++i) bp[i] = 0;
}

#define ZeroStruct(S) Zero((S), sizeof(0[S]))

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

typedef struct Timed_Section_Info
{
  char* name;
  u64 acc;
  u64 start;
  u64 hits;
} Timed_Section_Info;

typedef struct Profiling_State
{
  Timed_Section_Info timed_section_cache[1024];
  u64 start;
  u64 end;
} Profiling_State;

Profiling_State ProfilingState;

u64
TimedSection_Begin(u64 id, char* name)
{
  ASSERT(id < ARRAY_SIZE(ProfilingState.timed_section_cache));
  ASSERT(name != 0);

  ProfilingState.timed_section_cache[id].name  = name;
  ProfilingState.timed_section_cache[id].start = __rdtsc();

  return id;
}

void
TimedSection_End(u64 id)
{
  ProfilingState.timed_section_cache[id].acc  += __rdtsc() - ProfilingState.timed_section_cache[id].start;
  ProfilingState.timed_section_cache[id].hits += 1;
}

#define TIMED_BLOCK(NAME) for (s64 CONCAT(tb_c, __LINE__) = TimedSection_Begin(__COUNTER__, (NAME)); CONCAT(tb_c, __LINE__) != -1; TimedSection_End((u64)CONCAT(tb_c, __LINE__)), CONCAT(tb_c, __LINE__) = -1)

void
Profiling_Begin()
{
  ZeroStruct(&ProfilingState);
  ProfilingState.start = __rdtsc();
}

void
Profiling_End()
{
  ProfilingState.end = __rdtsc();
}

void
Profiling_PrintResults()
{
  u64 total_ticks = ProfilingState.end - ProfilingState.start;

  u64 rdtsc_freq = EstimateRDTSCFrequency(100);

  printf("Total time: %.04fms (CPU freq %llu)\n", 1000.0 * (f64)total_ticks/rdtsc_freq, rdtsc_freq);

  for (umm i = 0; i < ARRAY_SIZE(ProfilingState.timed_section_cache); ++i)
  {
    Timed_Section_Info* ti = &ProfilingState.timed_section_cache[i];

    if (ti->name != 0)
    {
      printf("  %s[%llu]: %llu (%.2f%%)\n", ti->name, ti->hits, ti->acc, 100.0 * (f64)ti->acc/total_ticks);
    }
  }
}
