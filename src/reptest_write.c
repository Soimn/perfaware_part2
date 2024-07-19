#include "common.h"

typedef struct Write_Params
{
  void* dest;
  umm size;
} Write_Params;

void
Test_WriteBytewise(Reptest* test, Write_Params params)
{
  while (Reptest_RoundIsNotDone(test))
  {
    Reptest_BeginTestSection(test);

    for (umm i = 0; i < params.size; ++i)
    {
      ((u8*)params.dest)[i] = (u8)i;
    }

    Reptest_EndTestSection(test);

    Reptest_AddBytesProcessed(test, params.size);
  }
}

void Test_MovSameQWordx1Loop(u64 size, void* dest);

void
Test_MovSameQWordx1(Reptest* test, Write_Params params)
{
  while (Reptest_RoundIsNotDone(test))
  {
    Reptest_BeginTestSection(test);

    Test_MovSameQWordx1Loop(params.size, params.dest);

    Reptest_EndTestSection(test);

    Reptest_AddBytesProcessed(test, params.size);
  }
}

void Test_MovSameQWordx2Loop(u64 size, void* dest);

void
Test_MovSameQWordx2(Reptest* test, Write_Params params)
{
  while (Reptest_RoundIsNotDone(test))
  {
    Reptest_BeginTestSection(test);

    Test_MovSameQWordx2Loop(params.size, params.dest);

    Reptest_EndTestSection(test);

    Reptest_AddBytesProcessed(test, params.size);
  }
}

void Test_MovSameQWordx3Loop(u64 size, void* dest);

void
Test_MovSameQWordx3(Reptest* test, Write_Params params)
{
  while (Reptest_RoundIsNotDone(test))
  {
    Reptest_BeginTestSection(test);

    Test_MovSameQWordx3Loop(params.size, params.dest);

    Reptest_EndTestSection(test);

    Reptest_AddBytesProcessed(test, params.size);
  }
}

void Test_MovBytewiseLoop(u64 size, void* dest);

void
Test_MovBytewise(Reptest* test, Write_Params params)
{
  while (Reptest_RoundIsNotDone(test))
  {
    Reptest_BeginTestSection(test);

    Test_MovBytewiseLoop(params.size, params.dest);

    Reptest_EndTestSection(test);

    Reptest_AddBytesProcessed(test, params.size);
  }
}

void Test_MovUnrolledBytewiseLoop(u64 size, void* dest);

void
Test_MovUnrolledBytewise(Reptest* test, Write_Params params)
{
  while (Reptest_RoundIsNotDone(test))
  {
    Reptest_BeginTestSection(test);

    Test_MovUnrolledBytewiseLoop(params.size, params.dest);

    Reptest_EndTestSection(test);

    Reptest_AddBytesProcessed(test, params.size);
  }
}

void Test_NopBytewiseLoop(u64 size, void* dest);

void
Test_NopBytewise(Reptest* test, Write_Params params)
{
  while (Reptest_RoundIsNotDone(test))
  {
    Reptest_BeginTestSection(test);

    Test_NopBytewiseLoop(params.size, params.dest);

    Reptest_EndTestSection(test);

    Reptest_AddBytesProcessed(test, params.size);
  }
}

void Test_Nop3BytewiseLoop(u64 size, void* dest);

void
Test_Nop3Bytewise(Reptest* test, Write_Params params)
{
  while (Reptest_RoundIsNotDone(test))
  {
    Reptest_BeginTestSection(test);

    Test_Nop3BytewiseLoop(params.size, params.dest);

    Reptest_EndTestSection(test);

    Reptest_AddBytesProcessed(test, params.size);
  }
}

void Test_Nop9BytewiseLoop(u64 size, void* dest);

void
Test_Nop9Bytewise(Reptest* test, Write_Params params)
{
  while (Reptest_RoundIsNotDone(test))
  {
    Reptest_BeginTestSection(test);

    Test_Nop9BytewiseLoop(params.size, params.dest);

    Reptest_EndTestSection(test);

    Reptest_AddBytesProcessed(test, params.size);
  }
}

void Test_CmpBytewiseLoop(u64 size, void* dest);

void
Test_CmpBytewise(Reptest* test, Write_Params params)
{
  while (Reptest_RoundIsNotDone(test))
  {
    Reptest_BeginTestSection(test);

    Test_CmpBytewiseLoop(params.size, params.dest);

    Reptest_EndTestSection(test);

    Reptest_AddBytesProcessed(test, params.size);
  }
}

void Test_DecBytewiseLoop(u64 size, void* dest);

void
Test_DecBytewise(Reptest* test, Write_Params params)
{
  while (Reptest_RoundIsNotDone(test))
  {
    Reptest_BeginTestSection(test);

    Test_DecBytewiseLoop(params.size, params.dest);

    Reptest_EndTestSection(test);

    Reptest_AddBytesProcessed(test, params.size);
  }
}

struct
{
  char* name;
  void (*func)(Reptest* state, Write_Params params);
} WriteTests[] = {
  "write bytewise", Test_WriteBytewise,
  "mov same qword x1", Test_MovSameQWordx1,
  "mov same qword x2", Test_MovSameQWordx2,
  "mov same qword x3", Test_MovSameQWordx3,
  "mov bytewise",   Test_MovBytewise,
  "mov unrolled bytewise",   Test_MovUnrolledBytewise,
  "nop bytewise",   Test_NopBytewise,
  "nop3 bytewise",  Test_Nop3Bytewise,
  "nop9 bytewise",  Test_Nop9Bytewise,
  "cmp bytewise",   Test_CmpBytewise,
  "dec bytewise",   Test_DecBytewise,
};

int
main(int argc, char** argv)
{
  InitializeOSLayer();

  if (argc != 2) fprintf(stderr, "Usage: reptest_write [number of pages to write]\n");
  else
  {
    umm size = atol(argv[1])*4096;

    void* dest = VirtualAlloc(0, size, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
    Memset(dest, size, 0xCD); // NOTE: touch buffer to avoid page faults during tests

    if (dest == 0) fprintf(stderr, "Failed to allocate memory\n");
    else
    {
      Write_Params params = { .dest = dest, .size = size };

      printf("rdtsc_freq: %llu\n", OSLayer.rdtsc_freq);

      Reptest tests[ARRAY_SIZE(WriteTests)];

      for (umm i = 0; i < ARRAY_SIZE(tests); ++i)
      {
        tests[i] = (Reptest){
          .name                = WriteTests[i].name,
          .bytes_to_process    = size,
          .idle_time_threshold = 10,
        };
      }

      for (;;)
      {
        for (umm i = 0; i < ARRAY_SIZE(tests); ++i)
        {
          Reptest* test = &tests[i];

          Reptest_BeginRound(test);
          WriteTests[i].func(test, params);
          Reptest_EndRound(test);
          printf("Max speed: %.4f cy/B\n", (f64)test->min_time/test->bytes_processed);
        }
      }
    }
  }

  return 0;
}
