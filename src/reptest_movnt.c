#include "common.h"

int
main(int argc, char** argv)
{
  InitializeOSLayer();

  umm buffer_size = 2ULL*GIGABYTE;
  void* buffer = VirtualAlloc(0, buffer_size, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);

  if (buffer == 0) fprintf(stderr, "Failed to allocate memory\n");
  else
  {
    Reptest test_mov = {
      .name                = "Test_Mov",
      .bytes_to_process    = buffer_size,
      .idle_time_threshold = 1,
    };

    Reptest_BeginRound(&test_mov);

    while (Reptest_RoundIsNotDone(&test_mov))
    {
      Reptest_BeginTestSection(&test_mov);

      void Mov();

      Reptest_EndTestSection(&test_mov);
      Reptest_AddBytesProcessed(&test_mov, test.bytes_to_process);
    }

    Reptest_EndRound(&test_mov);
  }

  return 0;
}
