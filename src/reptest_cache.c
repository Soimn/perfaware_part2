#include "common.h"

#define LINEAR_ACCESS_PATTERN 0

int
main(int argc, char** argv)
{
  InitializeOSLayer();

  // NOTE: Half cache size for each level, except "Main memory", which is 4x L3 size
  // NOTE: Working set sizes must be a power of 2, with the largest being the last
  Reptest tests[9] = {
    {
      .name                = "L1 cold",
      .bytes_to_process    = 32*KILOBYTE,
      .idle_time_threshold = 0.01,
      .should_trash_cache  = true,
      .user_flags          = LINEAR_ACCESS_PATTERN,
    },
    {
      .name                = "L1 hot",
      .bytes_to_process    = 32*KILOBYTE,
      .idle_time_threshold = 0.01,
      .user_flags          = LINEAR_ACCESS_PATTERN,
    },

    {
      .name                = "L2 cold",
      .bytes_to_process    = 128*KILOBYTE,
      .idle_time_threshold = 0.01,
      .should_trash_cache  = true,
      .user_flags          = LINEAR_ACCESS_PATTERN,
    },
    {
      .name                = "L2 hot",
      .bytes_to_process    = 128*KILOBYTE,
      .idle_time_threshold = 0.01,
      .user_flags          = LINEAR_ACCESS_PATTERN,
    },

    {
      .name                = "L3 cold",
      .bytes_to_process    = 6*MEGABYTE,
      .idle_time_threshold = 0.1,
      .should_trash_cache  = true,
      .user_flags          = LINEAR_ACCESS_PATTERN,
    },
    {
      .name                = "L3 hot",
      .bytes_to_process    = 6*MEGABYTE,
      .idle_time_threshold = 0.1,
      .user_flags          = LINEAR_ACCESS_PATTERN,
    },

    {
      .name                = "Main memory",
      .bytes_to_process    = 48*MEGABYTE,
      .idle_time_threshold = 0.1,
      .user_flags          = LINEAR_ACCESS_PATTERN,
    },
  };

  umm memory_size = 0;
  for (umm i = 0; i < ARRAY_SIZE(tests); ++i) memory_size = MAX(memory_size, tests[i].bytes_to_process);

  void* memory = VirtualAlloc(0, memory_size, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);

  if (memory == 0) fprintf(stderr, "Failed to allocate memory for tests\n");
  else
  {
    Memset(memory, memory_size, 0xCD);

    for (;;)
    {
      for (umm i = 0; i < ARRAY_SIZE(tests); ++i)
      {
        Reptest* test = &tests[i];

        Reptest_BeginRound(test);

        while (Reptest_RoundIsNotDone(test))
        {
          if (test->user_flags == LINEAR_ACCESS_PATTERN)
          {
            Reptest_BeginTestSection(test);

            void __vectorcall Test_ReadFullSpeed(u64 size, void* memory);
            Test_ReadFullSpeed(test->bytes_to_process, memory);

            Reptest_EndTestSection(test);
          }
          else
          {
          }

          Reptest_AddBytesProcessed(test, test->bytes_to_process);
        }

        Reptest_EndRound(test);
        printf("%.4f cy/line\n", (f64)test->min_time/(test->bytes_to_process/64));
      }
    }
  }

  return 0;
}
