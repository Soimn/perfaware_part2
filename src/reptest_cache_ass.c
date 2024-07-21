#include "common.h"

int
main(int argc, char** argv)
{
  InitializeOSLayer();

  umm buffer_size = 1*GIGABYTE + 512*MEGABYTE;
  void* buffer = VirtualAlloc(0, buffer_size, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);

  if (buffer == 0) fprintf(stderr, "Failed to allocate memory\n");
  else
  {
    Memset(buffer, buffer_size, 0xCD);

    f64 results[32];

    for (umm i = 1; i < ARRAY_SIZE(results); ++i)
    {
      char name[sizeof("xx stride")] = "xx stride";
      name[0] = (u8)(i/10 + '0');
      name[1] = (u8)(i%10 + '0');

      umm repeat_count     = buffer_size/256;
      umm bytes_to_process = repeat_count*256;

      Reptest test = {
        .name                = name,
        .bytes_to_process    = bytes_to_process,
        .idle_time_threshold = 2,
      };

      Reptest_BeginRound(&test);

      while (Reptest_RoundIsNotDone(&test))
      {
        Reptest_BeginTestSection(&test);

        void __vectorcall AssTest64K(u64 repeat_count, void* data, void* end);
        AssTest64K(repeat_count, buffer, (u8*)buffer + (i*64*KILOBYTE));

        Reptest_EndTestSection(&test);
        Reptest_AddBytesProcessed(&test, bytes_to_process);

        results[i] = ((f64)test.bytes_to_process/GIGABYTE) * ((f64)OSLayer.rdtsc_freq/test.min_time);
      }

      Reptest_EndRound(&test);
    }

    printf("\nstride [lines], bandwidth [GB/s]");
    for (umm i = 1; i < ARRAY_SIZE(results); ++i)
    {
      printf("%llu, %.6f\n", i, results[i]);
    }
  }

  return 0;
}
