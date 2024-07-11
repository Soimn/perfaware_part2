#include "common.h"

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

  u64 section_start;

  u64 elapsed;
  u64 bytes_processed;

  u64 test_count;
  u64 acc_time;
  u64 min_time;
  u64 max_time;
  f64 idle_time;

  char* name;
  u64 rdtsc_freq;
  u64 bytes_to_process;
  f64 idle_time_threshold;
} Reptest;

void
Reptest_BeginTestSection(Reptest* test)
{
  test->section_start = __rdtsc();
}

void
Reptest_EndTestSection(Reptest* test)
{
  test->elapsed += __rdtsc() - test->section_start;
}

void
Reptest_AddBytesProcessed(Reptest* test, u64 bytes)
{
  test->bytes_processed += bytes;
}

void
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
    test->test_count = 0;
    test->acc_time   = 0;
    test->min_time   = U64_MAX;
    test->max_time   = 0;

    test->elapsed         = 0;
    test->bytes_processed = 0;

    test->state = ReptestState_RoundInProgress;
  }
  else if (test->state == ReptestState_RoundInProgress)
  {
    if (test->bytes_processed != test->bytes_to_process) Reptest_Error(test, "Invalid byte count");
    else
    {
      test->test_count += 1;
      test->acc_time   += test->elapsed;
      test->max_time    = (test->elapsed > test->max_time ? test->elapsed : test->max_time);

      test->idle_time += (f64)test->elapsed/test->rdtsc_freq;
      if (test->elapsed < test->min_time)
      {
        test->min_time  = test->elapsed;
        test->idle_time = 0;
        printf("\rMin: %llu (%.6f ns) %.6f GB/s", test->min_time, 1.0e9 * (f64)test->min_time/test->rdtsc_freq, (f64)test->bytes_processed/GIGABYTE * (f64)test->rdtsc_freq/test->min_time);
      }

      if (test->idle_time >= test->idle_time_threshold) test->state = ReptestState_RoundFinished;
      else
      {
        test->elapsed         = 0;
        test->bytes_processed = 0;
        // NOTE: continue testing
      }
    }
  }

  return (test->state == ReptestState_RoundInProgress);
}

void
Reptest_BeginRound(Reptest* test)
{
  printf("\n--- %s ---\n", test->name);
  test->state = ReptestState_ReadyToStartRound;
}

void
Reptest_EndRound(Reptest* test)
{
  ASSERT(test->state == ReptestState_RoundFinished || test->state == ReptestState_Error);

  if (test->state == ReptestState_RoundFinished)
  {
    f64 avg_time = (f64)test->acc_time/test->test_count;

    printf("\r");
    printf("Min: %llu (%.6f ns) %.6f GB/s\n", test->min_time, 1.0e9 * (f64)test->min_time/test->rdtsc_freq, (f64)test->bytes_processed/GIGABYTE * (f64)test->rdtsc_freq/test->min_time);
    printf("Max: %llu (%.6f ns) %.6f GB/s\n", test->min_time, 1.0e9 * (f64)test->max_time/test->rdtsc_freq, (f64)test->bytes_processed/GIGABYTE * (f64)test->rdtsc_freq/test->max_time);
    printf("Avg: %llu (%.6f ns) %.6f GB/s\n", test->min_time, 1.0e9 * (f64)avg_time      /test->rdtsc_freq, (f64)test->bytes_processed/GIGABYTE * (f64)test->rdtsc_freq/avg_time);
  }
}

typedef struct Read_Params
{
  void* dest;
  u64 size;
  char* filename;
} Read_Params;

typedef struct Read_Func
{
  char* name;
  void (*func)(Reptest* test, Read_Params params);
} Read_Func;

void
Test_fread(Reptest* test, Read_Params params)
{
  while (Reptest_RoundIsNotDone(test))
  {
    FILE* file;

    if (fopen_s(&file, params.filename, "rb") != 0) Reptest_Error(test, "Failed to open file");
    else
    {
      Reptest_BeginTestSection(test);
      umm result = fread(params.dest, params.size, 1, file);
      Reptest_EndTestSection(test);

      if (result == 1) Reptest_AddBytesProcessed(test, params.size);
      else             Reptest_Error(test, "fread failed");

      fclose(file);
    }
  }
}

void
Test_ReadFile(Reptest* test, Read_Params params)
{
  while (Reptest_RoundIsNotDone(test))
  {
    HANDLE file;

    file = CreateFileA(params.filename, GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

    if (file == INVALID_HANDLE_VALUE) Reptest_Error(test, "Failed to open file");
    else
    {
      u8* cursor   = params.dest;
      u64 rem_size = params.size;

      while (rem_size > 0)
      {
        u32 bytes_to_read = U32_MAX;
        if (bytes_to_read > rem_size) bytes_to_read = (u32)rem_size;
        rem_size -= bytes_to_read;

        u32 bytes_read;
        Reptest_BeginTestSection(test);
        BOOL result = ReadFile(file, cursor, bytes_to_read, &bytes_read, 0);
        Reptest_EndTestSection(test);

        if (result && bytes_read == bytes_to_read) Reptest_AddBytesProcessed(test, bytes_read);
        else                                       Reptest_Error(test, "ReadFile failed");
      }

      CloseHandle(file);
    }
  }
}

Read_Func TestFuncs[] = {
  { "fread",    Test_fread    },
  { "ReadFile", Test_ReadFile },
};

int
main(int argc, char** argv)
{
  if (argc != 2) fprintf(stderr, "Usage: reptest_read [file to read]\n");
  else
  {

    Read_Params test_params = {0};
    bool setup_succeeded    = false;
    {
      char* filename = argv[1];

      struct __stat64 file_info;
      if (_stat64(filename, &file_info) == 0)
      {
        void* dest = malloc(file_info.st_size);

        if (dest != 0)
        {
          test_params = (Read_Params){
            .filename = filename,
            .dest     = dest,
            .size     = file_info.st_size,
          };

          setup_succeeded = true;
        }
      }
    }

    if (!setup_succeeded) fprintf(stderr, "Failed to allocate memory for tests\n");
    else
    {

      Reptest tests[ARRAY_SIZE(TestFuncs)];
      u64 rdtsc_freq = EstimateRDTSCFrequency(100);

      for (umm i = 0; i < ARRAY_SIZE(tests); ++i)
      {
        tests[i] = (Reptest){
          .name                = TestFuncs[i].name,
          .rdtsc_freq          = rdtsc_freq,
          .bytes_to_process    = test_params.size,
          .idle_time_threshold = 10,
        };
      }

      for (;;)
      {
        for (umm i = 0; i < ARRAY_SIZE(tests); ++i)
        {
          Reptest* test = &tests[i];

          Reptest_BeginRound(test);

          TestFuncs[i].func(test, test_params);

          Reptest_EndRound(test);
        }
      }
    }
  }

  return 0;
}
