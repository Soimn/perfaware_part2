#include "common.h"

int
main(int argc, char** argv)
{
  InitializeOSLayer();

  umm log_read_range = 30;
  umm read_range = 1ULL << log_read_range;

  void* memory = VirtualAlloc(0, read_range, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
  if (memory == 0)
  {
    fprintf(stderr, "Failed to allocate memory\n");
    return 1;
  }

  Memset(memory, read_range, 0xCD);

  for (umm i = 14; i < log_read_range;)
  {
    umm amount = 1ULL << log_read_range;
    umm mask   = (1ULL << i) - 1;

    char name[sizeof("xx bits")] = "xx bits";
    name[0] = (u8)(i/10) + '0';
    name[1] = (u8)(i%10) + '0';

    Reptest test = {
      .name                = name,
      .bytes_to_process    = amount,
      .idle_time_threshold = 10,
    };

    Reptest_BeginRound(&test);

    while (Reptest_RoundIsNotDone(&test))
    {
      Reptest_BeginTestSection(&test);

      void __vectorcall ReadAmountMaskedToRange(u64 amount, void* memory, u64 mask);
      ReadAmountMaskedToRange(amount, memory, mask);

      Reptest_EndTestSection(&test);
      Reptest_AddBytesProcessed(&test, amount);
    }

    Reptest_EndRound(&test);
  }

  return 0;
}
