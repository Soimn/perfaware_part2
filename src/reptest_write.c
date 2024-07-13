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

void Test_MovBytewise(Reptest* test, Write_Params params);
void Test_NopBytewise(Reptest* test, Write_Params params);
void Test_CmpBytewise(Reptest* test, Write_Params params);
void Test_DecBytewise(Reptest* test, Write_Params params);

struct
{
  char* name;
  void (*func)(Reptest* state, Write_Params params);
} WriteTests[] = {
  "write bytewise", Test_WriteBytewise,
  "mov bytewise",   Test_MovBytewise,
  "nop bytewise",   Test_NopBytewise,
  "cmp bytewise",   Test_CmpBytewise,
  "dec bytewise",   Test_DecBytewise,
};

int
main(int argc, char** argv)
{
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

      u64 rdtsc_freq = EstimateRDTSCFrequency(100);
      printf("rdtsc_freq: %llu\n", rdtsc_freq);

      Reptest tests[ARRAY_SIZE(WriteTests)];

      for (umm i = 0; i < ARRAY_SIZE(tests); ++i)
      {
        tests[i] = (Reptest){
          .name                = WriteTests[i].name,
          .rdtsc_freq          = rdtsc_freq,
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
        }
      }
    }
  }

  return 0;
}
