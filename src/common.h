#pragma once

#define NOMINMAX            1
#define WIN32_LEAN_AND_MEAN 1
#define WIN32_MEAN_AND_LEAN 1
#define VC_EXTRALEAN        1
#include <windows.h>
#include <psapi.h>
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

#define U8_MAX  0xFFU
#define U16_MAX 0xFFFFU
#define U32_MAX 0xFFFFFFFFU
#define U64_MAX 0xFFFFFFFFFFFFFFFFUL

typedef signed __int8  s8;
typedef signed __int16 s16;
typedef signed __int32 s32;
typedef signed __int64 s64;

#define S8_MIN  0x80D
#define S16_MIN 0x8000D
#define S32_MIN 0x80000000D
#define S64_MIN 0x8000000000000000DL

#define S8_MAX  0x7FD
#define S16_MAX 0x7FFFD
#define S32_MAX 0x7FFFFFFFD
#define S64_MAX 0x7FFFFFFFFFFFFFFFDL

typedef u64 umm;
typedef s64 smm;

typedef float f32;
typedef double f64;

typedef u8 bool;
#define true 1
#define false 0

#define PI       3.14159265358979323846264338327950
#define PI_DIV_2 1.57079632679489661923132169163975

#define KILOBYTE 1024
#define MEGABYTE (1024*1024)
#define GIGABYTE (1024*1024*1024)

#define MAX(A, B) ((A) > (B) ? (A) : (B))
#define MIN(A, B) ((A) < (B) ? (A) : (B))

#define ARRAY_SIZE(A) (sizeof(A)/sizeof(0[A]))

#define ASSERT(EX) ((EX) ? 1 : (AssertHandler(__FILE__, __LINE__, #EX), 0))
#define NOT_IMPLEMENTED() ASSERT(!"NOT_IMPLEMENTED")

#define CONCAT__(A, B) A##B
#define CONCAT_(A, B) CONCAT__(A, B)
#define CONCAT(A, B) CONCAT_(A, B)

static void
AssertHandler(char* file, umm line, char* expr)
{
  fprintf(stderr, "ASSERTION FAILED\n%s(%llu): %s\n", file, line, expr);
  __debugbreak();
  *(volatile int*)0 = 0;
}

static bool
Char_IsWhitespace(u8 c)
{
  return (c == ' ' || c == '\n' || c == '\r' || c == '\t');
}

static bool
Char_IsDigit(u8 c)
{
  return ((u8)(c-0x30) < (u8)10);
}

static bool
Char_IsHexAlphaDigit(u8 c)
{
  return ((u8)((c&0xDF) - 'A') <= (u8)('F' - 'A'));
}

static bool
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

static void
Zero(void* p, umm size)
{
  u8* bp = (u8*)p;
  for (umm i = 0; i < size; ++i) bp[i] = 0;
}

#define ZeroStruct(S) Zero((S), sizeof(0[S]))

static void
Memset(void* p, umm size, u8 val)
{
  u8* bp = (u8*)p;
  for (umm i = 0; i < size; ++i) bp[i] = val;
}

static struct
{
  HANDLE mem_info_handle;
  u64 rdtsc_freq;
  u64 trash_buffer_size;
  void* trash_buffer;
  u8 prime_temp;
} OSLayer;

static void
InitializeOSLayer()
{
  OSLayer.mem_info_handle = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, GetCurrentProcessId());
  if (OSLayer.mem_info_handle == NULL)
  {
    fprintf(stderr, "Failed to open process handle for memory info queries\n");
    ExitProcess(1);
  }

  { /// Estimate RDTSC freq
    u64 timing_interval_ms = 100;

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

    OSLayer.rdtsc_freq = elapsed_cpu_ticks * (perf_freq.QuadPart / elapsed_wall_ticks);
  }

  SYSTEM_LOGICAL_PROCESSOR_INFORMATION logical_processor_infos[32];
  u32 logical_processor_infos_byte_size = sizeof(logical_processor_infos);

  if (!GetLogicalProcessorInformation(logical_processor_infos, &logical_processor_infos_byte_size))
  {
    fprintf(stderr, "Failed to query logical processor information\n");
    ExitProcess(1);
  }
  else
  {
    OSLayer.trash_buffer_size = 0;
    for (umm i = 0; i < logical_processor_infos_byte_size/sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION); ++i)
    {
      OSLayer.trash_buffer_size = MAX(OSLayer.trash_buffer_size, logical_processor_infos[i].Cache.Size);
    }

    OSLayer.trash_buffer = VirtualAlloc(0, OSLayer.trash_buffer_size, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
    if (OSLayer.trash_buffer == 0)
    {
      fprintf(stderr, "Failed to allocate trash buffer\n");
      ExitProcess(1);
    }
  }
}

static u64
GetPageFaultCounter()
{
  PROCESS_MEMORY_COUNTERS counters = { .cb = sizeof(PROCESS_MEMORY_COUNTERS) };
  GetProcessMemoryInfo(OSLayer.mem_info_handle, &counters, sizeof(PROCESS_MEMORY_COUNTERS));
  return counters.PageFaultCount;
}

static void
TrashCache()
{
  for (umm i = 0; i < 2; ++i)
  {
    for (umm j = 0; j < OSLayer.trash_buffer_size; ++j)
    {
      ((u8*)OSLayer.trash_buffer)[j] |= 0xB5;
    }
  }
}

static void
PrimeCache(void* ptr, umm size)
{
  for (umm i = 0; i < 2; ++i)
  {
    for (umm j = 0; j < size; ++j)
    {
      OSLayer.prime_temp |= ((u8*)ptr)[j];
    }
  }
}

typedef struct Timed_Block_Info
{
  u64 acc_in;
  u64 acc_ex;
  u64 hits;
  char* name;
  u64 bytes_processed;
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

static Timed_Block_State
TimedBlock__Begin(u32 id, char* name, u64 bytes_processed, u32* id_out)
{
  Timed_Block_State state = {
    .prev_acc        = ProfilingState.blocks[id].acc_in,
    .start           = __rdtsc(),
    .name            = name,
    .id              = id,
    .parent          = ProfilingState.current_block,
  };

  ProfilingState.blocks[id].bytes_processed += bytes_processed;

  ProfilingState.current_block = id;
  *id_out                      = id;

  return state;
}

static void
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

#define TIMED_BLOCK(NAME, BP, ID_OUT) for (Timed_Block_State CONCAT(tbc__, __LINE__) = TimedBlock__Begin(__COUNTER__ + 1, (NAME), (BP), (ID_OUT)); CONCAT(tbc__, __LINE__).id != 0; TimedBlock__End(CONCAT(tbc__, __LINE__)), CONCAT(tbc__, __LINE__).id = 0)

#define TIME_BLOCK(NAME) TIMED_BLOCK((NAME), 0, &(u32){0})
#define TIME_THROUGHPUT(NAME, BYTES_PROCESSED) TIMED_BLOCK((NAME), (BYTES_PROCESSED), &(u32){0})
#define TIME_ANNOTATED_BLOCK(NAME, ID_OUT) TIMED_BLOCK((NAME), 0, (ID_OUT))
#define TIME_ANNOTATED_THROUGHPUT(NAME, BYTES_PROCESSED, ID_OUT) TIMED_BLOCK((NAME), (BYTES_PROCESSED), (ID_OUT))
#define ANNOTATE_BYTES_PROCESSED(ID, BYTES) ProfilingState.blocks[ID].bytes_processed = (BYTES)

#else
#define TIMED_BLOCK(...)
#define TIME_BLOCK(...)
#define TIME_THROUGHPUT(...)
#define TIME_ANNOTATED_BLOCK(...)
#define TIME_ANNOTATED_THROUGHPUT(...)
#define ANNOTATE_BYTES_PROCESSED(...)
#endif

static void
Profiling_Begin()
{
  ProfilingState.start = __rdtsc();
}

static void
Profiling_End()
{
  ASSERT(__COUNTER__ + 1 <= ARRAY_SIZE(ProfilingState.blocks));

  ProfilingState.end = __rdtsc();
}

static void
Profiling_PrintResults()
{
  u64 total_elapsed = ProfilingState.end - ProfilingState.start;

  f64 total_seconds_elapsed = (f64)total_elapsed/OSLayer.rdtsc_freq;

  printf("Total time: %.4fms (CPU freq %llu)\n", 1000.0 * total_seconds_elapsed, OSLayer.rdtsc_freq);

  for (umm i = 0; i < ARRAY_SIZE(ProfilingState.blocks); ++i)
  {
    Timed_Block_Info* block = &ProfilingState.blocks[i];

    if (block->name != 0)
    {
      printf("  %s[%llu]: %llu (%.2f%%", block->name, block->hits, block->acc_ex, 100.0 * (f64)block->acc_ex/total_elapsed);

      if (block->acc_in != block->acc_ex) printf(", w/children %.2f%%", 100.0 * (f64)block->acc_in/total_elapsed);

      printf(")");

      if (block->bytes_processed != 0)
      {
        f64 mbs_processed   = (f64)block->bytes_processed/(1024.0*1024.0);
        f64 gbs_processed   = mbs_processed/(1024.0);
        f64 seconds_elapsed = (f64)block->acc_in/OSLayer.rdtsc_freq;

        printf(" %.2f MB at %.4f GB/s", mbs_processed, gbs_processed/seconds_elapsed);
      }

      printf("\n");
    }
  }
}

typedef enum Reptest_State
{
  ReptestState_Error = 0,
  ReptestState_ReadyToStartRound,
  ReptestState_RoundInProgress,
  ReptestState_RoundFinished,
} Reptest_State;

typedef struct Reptest
{
  Reptest_State state;

  u64 elapsed;
  u64 page_faults;
  u64 bytes_processed;

  u64 test_count;
  u64 acc_time;
  u64 acc_page_faults;
  u64 min_time;
  u64 min_time_page_faults;
  u64 max_time;
  u64 max_time_page_faults;
  f64 idle_time;

  char* name;
  u64 bytes_to_process;
  f64 idle_time_threshold;
  bool should_trash_cache;

  u64 user_flags;
} Reptest;

void
Reptest_BeginTestSection(Reptest* test)
{
  test->elapsed     -= __rdtsc();
  test->page_faults -= GetPageFaultCounter();
}

void
Reptest_EndTestSection(Reptest* test)
{
  test->elapsed     += __rdtsc();
  test->page_faults += GetPageFaultCounter();
}

void
Reptest_AddBytesProcessed(Reptest* test, u64 bytes)
{
  test->bytes_processed += bytes;
}

static void
Reptest_Error(Reptest* test, char* message)
{
  fprintf(stderr, "\r%s\n", message);
  test->state = ReptestState_Error;
}

bool
Reptest_RoundIsNotDone(Reptest* test)
{
  if (test->state == ReptestState_ReadyToStartRound)
  {
    test->test_count           = 0;
    test->acc_time             = 0;
    test->acc_page_faults      = 0;
    test->min_time             = U64_MAX;
    test->min_time_page_faults = 0;
    test->max_time             = 0;
    test->max_time_page_faults = 0;

    test->elapsed         = 0;
    test->page_faults     = 0;
    test->bytes_processed = 0;

    test->state = ReptestState_RoundInProgress;
  }
  else if (test->state == ReptestState_RoundInProgress)
  {
    if (test->bytes_processed != test->bytes_to_process) Reptest_Error(test, "Invalid byte count");
    else
    {
      test->test_count      += 1;
      test->acc_time        += test->elapsed;
      test->acc_page_faults += test->page_faults;
      
      if (test->elapsed > test->max_time)
      {
        test->max_time             = test->elapsed;
        test->max_time_page_faults = test->page_faults;
      }

      test->idle_time += (f64)test->elapsed/OSLayer.rdtsc_freq;
      if (test->elapsed < test->min_time)
      {
        test->min_time             = test->elapsed;
        test->min_time_page_faults = test->page_faults;

        test->idle_time = 0;

        printf("\rMin: %llu (%.6f ns) %.6f GB/s   ", test->min_time, 1.0e9 * (f64)test->min_time/OSLayer.rdtsc_freq, (f64)test->bytes_processed/GIGABYTE * (f64)OSLayer.rdtsc_freq/test->min_time);
      }

      if (test->idle_time >= test->idle_time_threshold) test->state = ReptestState_RoundFinished;
      else
      {
        test->elapsed         = 0;
        test->page_faults     = 0;
        test->bytes_processed = 0;
        // NOTE: continue testing
      }
    }
  }

  if (test->state == ReptestState_RoundInProgress && test->should_trash_cache) TrashCache();

  return (test->state == ReptestState_RoundInProgress);
}

static void
Reptest_BeginRound(Reptest* test)
{
  printf("\n--- %s ---\n", test->name);
  test->state = ReptestState_ReadyToStartRound;
}

static void
Reptest_EndRound(Reptest* test)
{
  ASSERT(test->state == ReptestState_RoundFinished || test->state == ReptestState_Error);

  if (test->state == ReptestState_RoundFinished)
  {
    f64 min_time_ns         = 1.0e9 * (f64)test->min_time/OSLayer.rdtsc_freq;
    f64 min_time_throughput = (f64)test->bytes_processed/GIGABYTE * (f64)OSLayer.rdtsc_freq/test->min_time;
    f64 min_time_kbpfault   = (f64)test->bytes_processed/(KILOBYTE*test->min_time_page_faults);

    f64 max_time_ns         = 1.0e9 * (f64)test->max_time/OSLayer.rdtsc_freq;
    f64 max_time_throughput = (f64)test->bytes_processed/GIGABYTE * (f64)OSLayer.rdtsc_freq/test->max_time;
    f64 max_time_kbpfault   = (f64)test->bytes_processed/(KILOBYTE*test->max_time_page_faults);

    f64 avg_time = (f64)test->acc_time/test->test_count;
    f64 avg_time_ns         = 1.0e9 * (f64)avg_time/OSLayer.rdtsc_freq;
    f64 avg_time_throughput = (f64)test->bytes_processed/GIGABYTE * (f64)OSLayer.rdtsc_freq/avg_time;
    f64 avg_time_kbpfault   = (f64)(test->test_count*test->bytes_processed)/(KILOBYTE*test->acc_page_faults);

    printf("\r");
    printf("Min: %llu (%.6f ns) %.6f GB/s, PF: %llu, (%.6f KB/fault)\n", test->min_time, min_time_ns, min_time_throughput, test->min_time_page_faults, min_time_kbpfault);
    printf("Max: %llu (%.6f ns) %.6f GB/s, PF: %llu, (%.6f KB/fault)\n", test->max_time, max_time_ns, max_time_throughput, test->max_time_page_faults, max_time_kbpfault);
    printf("Avg: %llu (%.6f ns) %.6f GB/s, PF: %llu, (%.6f KB/fault)\n", test->min_time, avg_time_ns, avg_time_throughput, test->acc_page_faults,      avg_time_kbpfault);
  }
}
