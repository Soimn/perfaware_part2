#include "common.h"

//__declspec(noinline) u64
u64
Compute(u64 val, u64 data_point)
{
  static u64 table[] = {
    3,
    15,
    16,
    19,
    14234,
    23534654356,
    56245645624562456,
    56456,
    456245757157,
    457457457268568,
    45756786578678957,
    345345,
    34,
    34656,
    346457686784678456,
    345345456,
    4567567867878,
    456456,
    45657884254,
    9967,
    967563457,
    23454656,
    234234,
    236457689956,
    45645658234,
    234234,
    345547789,
    4353455689,
    345345,
    4,
    56,
    78,
    456,
    456895,
    3453457834531,
    345547789,
    4353455689,
    345345,
    4,
    56,
    78,
    456,
    456895,
    3453457834531,
    967563457,
    23454656,
    234234,
  };

  for (umm i = 0; i < ARRAY_SIZE(table); ++i)
  {
    val *= table[i];
    val ^= 0x71FF7A72E845C9B2;
  }

  val ^= data_point;

  return val;
}

u64
Test_NoPrefetch(u64* data_buffer, u32* idx_buffer, umm idx_buffer_size)
{
  u64 result = 0;

  for (umm i = 0; i < idx_buffer_size-1; ++i)
  {
    result = Compute(result, data_buffer[idx_buffer[i]]);
  }

  result = Compute(result, data_buffer[idx_buffer[idx_buffer_size-1]]);

  return result;
}

u64
Test_PrefetchT0(u64* data_buffer, u32* idx_buffer, umm idx_buffer_size)
{
  u64 result = 0;

  for (umm i = 0; i < idx_buffer_size-1; ++i)
  {
    _mm_prefetch((char*)&data_buffer[idx_buffer[i+1]], _MM_HINT_T0);
    result = Compute(result, data_buffer[idx_buffer[i]]);
  }

  result = Compute(result, data_buffer[idx_buffer[idx_buffer_size-1]]);

  return result;
}

u64
Test_PrefetchT0Np1(u64* data_buffer, u32* idx_buffer, umm idx_buffer_size)
{
  u64 result = 0;

  for (umm i = 0; i < idx_buffer_size-2; ++i)
  {
    _mm_prefetch((char*)&data_buffer[idx_buffer[i+2]], _MM_HINT_T0);
    result = Compute(result, data_buffer[idx_buffer[i]]);
  }

  result = Compute(result, data_buffer[idx_buffer[idx_buffer_size-2]]);
  result = Compute(result, data_buffer[idx_buffer[idx_buffer_size-1]]);

  return result;
}

u64
Test_PrefetchT1(u64* data_buffer, u32* idx_buffer, umm idx_buffer_size)
{
  u64 result = 0;

  for (umm i = 0; i < idx_buffer_size-1; ++i)
  {
    _mm_prefetch((char*)&data_buffer[idx_buffer[i+1]], _MM_HINT_T1);
    result = Compute(result, data_buffer[idx_buffer[i]]);
  }

  result = Compute(result, data_buffer[idx_buffer[idx_buffer_size-1]]);

  return result;
}

int
main(int argc, char** argv)
{
  InitializeOSLayer();

  srand(234983477);

  umm data_buffer_size = (256*MEGABYTE)/sizeof(u64);
  umm idx_buffer_size  = data_buffer_size;

  u64* data_buffer = VirtualAlloc(0, data_buffer_size*sizeof(u64), MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
  u32* idx_buffer  = VirtualAlloc(0, idx_buffer_size*sizeof(u32),  MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);

  if (data_buffer == 0 || idx_buffer == 0) fprintf(stderr, "Failed to allocate memory\n");
  else
  {
    for (umm i = 0; i < data_buffer_size; ++i)
    {
      data_buffer[i] = ((u64)rand() << 32) | rand();
    }

    for (umm i = 0; i < idx_buffer_size; ++i) idx_buffer[i] = (u32)(i % data_buffer_size);

    for (umm i = 0; i < idx_buffer_size; ++i)
    {
      umm j = rand() % (idx_buffer_size - i);

      u32 tmp = idx_buffer[i];
      idx_buffer[i] = idx_buffer[j];
      idx_buffer[j] = tmp;
    }

    u64 (*test_funcs[])(u64* data_buffer, u32* idx_buffer, umm idx_buffer_size) = {
      &Test_NoPrefetch,
      &Test_PrefetchT0,
      &Test_PrefetchT0Np1,
      &Test_PrefetchT1,
    };

    umm bytes_to_process = idx_buffer_size*sizeof(data_buffer[0]);
    f64 idle_thresh = 1;

    Reptest tests[] = {
      [0] = {
        .name                = "No prefetch",
        .bytes_to_process    = bytes_to_process,
        .idle_time_threshold = idle_thresh,
      },
      [1] = {
        .name                = "PrefetchT0",
        .bytes_to_process    = bytes_to_process,
        .idle_time_threshold = idle_thresh,
      },
      [2] = {
        .name                = "PrefetchT0Np1",
        .bytes_to_process    = bytes_to_process,
        .idle_time_threshold = idle_thresh,
      },
      [3] = {
        .name                = "PrefetchT1",
        .bytes_to_process    = bytes_to_process,
        .idle_time_threshold = idle_thresh,
      },
    };

    for (umm i = 0; i < ARRAY_SIZE(tests); ++i)
    {
      Reptest* test = &tests[i];

      u64 result = 0;

      Reptest_BeginRound(test);

      while (Reptest_RoundIsNotDone(test))
      {
        Reptest_BeginTestSection(test);

        result = test_funcs[i](data_buffer, idx_buffer, idx_buffer_size);

        Reptest_EndTestSection(test);
        Reptest_AddBytesProcessed(test, test->bytes_to_process);
      }

      Reptest_EndRound(test);
      printf("Result: %llu\n", result);
    }
  }

  return 0;
}
