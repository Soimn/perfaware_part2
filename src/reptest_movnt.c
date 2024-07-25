#include "common.h"

int
main(int argc, char** argv)
{
  InitializeOSLayer();

  srand(0);

  umm image_size  = 256;
  umm image_bytes = image_size*image_size;
  umm image_count = 16;

  u8* input_images  = VirtualAlloc(0, image_bytes*image_count, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
  u8* output_images = VirtualAlloc(0, image_bytes*image_count, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);

  if (input_images == 0 || output_images == 0) fprintf(stderr, "Failed to allocate memory\n");
  else
  {
    for (umm i = 0; i < image_count; ++i)
    {
      u8* image = output_images + i*image_bytes;

      for (umm y = 0; y < image_size; ++y)
      {
        for (umm x = 0; x < image_size; ++x)
        {
          image[y*image_size + x] = (u8)rand();
        }
      }
    }

    Reptest test_mov = {
      .name                = "Mov",
      .bytes_to_process    = image_bytes*image_count, // NOTE: num bytes to write
      .idle_time_threshold = 10,
    };

    Reptest test_movnt = {
      .name                = "MovNT",
      .bytes_to_process    = image_bytes*image_count, // NOTE: num bytes to write
      .idle_time_threshold = 10,
    };

    Reptest* tests[2] = { &test_mov, &test_movnt };

    for (umm test_idx = 0; test_idx < ARRAY_SIZE(tests); ++test_idx)
    {
      Reptest* test = tests[test_idx];

      Memset(output_images, image_bytes*image_count, 0);

      Reptest_BeginRound(test);

      while (Reptest_RoundIsNotDone(test))
      {
        for (umm y = 0; y < image_size; ++y)
        {
          for (umm x = 0; x < image_size; ++x)
          {
            for (umm i = 0; i < image_count; ++i)
            {
              Reptest_BeginTestSection(test);
              u8 result = 0;

              for (umm j = 0; j < image_count; ++j)
              {
                if (j == i) continue;

                result ^= (input_images + j*image_bytes)[y*image_size + x];
              }

              func(result, &(output_images + i*image_bytes)[y*image_size + x]);

              Reptest_EndTestSection(test);
              Reptest_AddBytesProcessed(test, 1);
            }
          }
        }
      }

      Reptest_EndRound(&test_mov);
    }
  }
  return 0;
}
