#include "common.h"

int
main(int argc, char** argv)
{
  InitializeOSLayer();

  // NOTE: An additional page worth of memory is allocated to avoid having to truncate the read size and compensate

  umm buffer_size = 1*GIGABYTE;
  void* buffer = VirtualAlloc(0, buffer_size + 4*KILOBYTE, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);

  if (buffer == 0) fprintf(stderr, "Failed to allocate buffer\n");
  else
  {
    Memset(buffer, buffer_size, 0xCD);

    f64 results[64];

    for (umm i = 0; i < 64; ++i)
    {
      char name [sizeof("offset by xx bytes")] = "offset by xx bytes";
      name[sizeof("offset by ")-1]  = (u8)(i/10 + '0');
      name[sizeof("offset by x")-1] = (u8)(i%10 + '0');

      Reptest test = {
        .name                = name,
        .bytes_to_process    = buffer_size,
        .idle_time_threshold = 0.5,
      };

      Reptest_BeginRound(&test);

      while (Reptest_RoundIsNotDone(&test))
      {
        Reptest_BeginTestSection(&test);

        void __vectorcall ReadTest(u64 size, void* data, u64 range);
        ReadTest(buffer_size, (u8*)buffer + i, 16*KILOBYTE);

        Reptest_EndTestSection(&test);
        Reptest_AddBytesProcessed(&test, buffer_size);
      }

      Reptest_EndRound(&test);

      results[i] = (f64)test.bytes_to_process/GIGABYTE * (f64)OSLayer.rdtsc_freq/test.min_time;
    }

    printf("\n\noffset, bandwidth [GB/s]\n");
    for (umm i = 0; i < 64; ++i)
    {
      printf("%llu, %.6f\n", i, results[i]);
    }
  }

  return 0;
}
