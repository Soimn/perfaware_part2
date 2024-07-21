#include "common.h"

int
main(int argc, char** argv)
{
  InitializeOSLayer();

  umm buffer_size = 1*GIGABYTE;
  void* buffer = VirtualAlloc(0, buffer_size, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);

  if (buffer == 0) fprintf(stderr, "Failed to allocate memory\n");
  else
  {
    Memset(buffer, buffer_size, 0xCD);

    umm start     = 28*KILOBYTE;
    umm increment =  4*KILOBYTE;

    f64 results[58] = {0};
    umm result_count  = 0;

    for (umm read_size = start; read_size < buffer_size && result_count < ARRAY_SIZE(results); read_size += increment)
    {
      umm size_div_512     = read_size/512;
      umm loop_count       = buffer_size/(size_div_512*512);
      umm bytes_to_process = loop_count*size_div_512*512;

      char name[sizeof("xxx K")] = "xxx K";
      name[0] = ((read_size >> 10)/100    )%10 + '0';
      name[1] = ((read_size >> 10)/10     )%10 + '0';
      name[2] = ((read_size >> 10)/1      )%10 + '0';

      Reptest test = {
        .name                = name,
        .bytes_to_process    = bytes_to_process,
        .idle_time_threshold = 1,
      };

      Reptest_BeginRound(&test);

      while (Reptest_RoundIsNotDone(&test))
      {
        Reptest_BeginTestSection(&test);

        void __vectorcall ReadRangeNTimesSingle(u64 bytes_to_process, void* data, u64 reset_point);
        ReadRangeNTimesSingle(bytes_to_process, buffer, read_size);

        Reptest_EndTestSection(&test);
        Reptest_AddBytesProcessed(&test, bytes_to_process);
      }

      Reptest_EndRound(&test);

      results[result_count++] = test.bytes_processed * ((f64)test.min_time/OSLayer.rdtsc_freq);
    }

    printf("\n\nsize,GB/s\n");
    for (umm i = 0; i < ARRAY_SIZE(results); ++i)
    {
      printf("%llu, %.6f\n", start + i*increment, results[i]);
    }
  }

  return 0;
}
