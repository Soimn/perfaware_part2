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

typedef struct Timed_Block_Info
{
  u64 acc_in;
  u64 acc_ex;
  u64 hits;
  char* name;
} Timed_Block_Info;

static struct
{
  u64 start;
  u64 end;

  u32 current_block;

  Timed_Block_Info blocks[1024];
} ProfilingState = {0};

#ifndef TIMED_BLOCK_PROFILING
#define TIMED_BLOCK_PROFILING 0
#endif

#if TIMED_BLOCK_PROFILING

typedef struct Timed_Block_State
{
  u64 prev_acc;
  u64 start;
  char* name;
  u32 id;
  u32 parent;
} Timed_Block_State;

Timed_Block_State
TimedBlock__Begin(u32 id, char* name)
{
  Timed_Block_State state = {
    .prev_acc = ProfilingState.blocks[id].acc_in,
    .start    = __rdtsc(),
    .name     = name,
    .id       = id,
    .parent   = ProfilingState.current_block,
  };

  ProfilingState.current_block = id;

  return state;
}

void
TimedBlock__End(Timed_Block_State state)
{
  u64 elapsed = __rdtsc() - state.start;

  ProfilingState.blocks[state.id].acc_ex  += elapsed;
  ProfilingState.blocks[state.id].acc_in   = state.prev_acc + elapsed;
  ProfilingState.blocks[state.id].hits    += 1;
  ProfilingState.blocks[state.id].name     = state.name;

  ProfilingState.blocks[state.parent].acc_ex -= elapsed;

  ProfilingState.current_block = state.parent;
}

#define TIMED_BLOCK(NAME) for (Timed_Block_State CONCAT(tbc__, __LINE__) = TimedBlock__Begin(__COUNTER__ + 1, (NAME)); CONCAT(tbc__, __LINE__).id != 0; TimedBlock__End(CONCAT(tbc__, __LINE__)), CONCAT(tbc__, __LINE__).id = 0)

#else
#define TIMED_BLOCK(NAME)
#endif

void
Profiling_Begin()
{
  ProfilingState.start = __rdtsc();
}

void
Profiling_End()
{
  ASSERT(__COUNTER__ + 1 <= ARRAY_SIZE(ProfilingState.blocks));

  ProfilingState.end = __rdtsc();
}

void
Profiling_PrintResults()
{
  u64 total_elapsed = ProfilingState.end - ProfilingState.start;

  u64 rdtsc_freq = EstimateRDTSCFrequency(100);

  printf("Total time: %.4fms (CPU freq %llu)\n", 1000.0 * (f64)total_elapsed/rdtsc_freq, rdtsc_freq);

  for (umm i = 0; i < ARRAY_SIZE(ProfilingState.blocks); ++i)
  {
    Timed_Block_Info* block = &ProfilingState.blocks[i];

    if (block->name != 0)
    {
      printf("  %s[%llu]: %llu (%.2f%%", block->name, block->hits, block->acc_ex, 100.0 * (f64)block->acc_ex/total_elapsed);

      if (block->acc_in != block->acc_ex) printf(", w/children %.2f%%", 100.0 * (f64)block->acc_in/total_elapsed);

      printf(")\n");
    }
  }
}
