#include "common.h"

int
main(int argc, char** argv)
{
  InitializeOSLayer();

  srand(0);

  umm image_size  = 256;
  umm image_bytes = image_size*image_size;
  umm in_image_count  = 8;
  umm out_image_count = 10;

  u8* input_images  = VirtualAlloc(0, image_bytes*in_image_count, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
  u8* output_images = VirtualAlloc(0, image_bytes*out_image_count, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);

  if (input_images == 0 || output_images == 0) fprintf(stderr, "Failed to allocate memory\n");
  else
  {
    for (umm i = 0; i < in_image_count; ++i)
    {
      u8* image = input_images + i*image_bytes;

      for (umm y = 0; y < image_size; ++y)
      {
        for (umm x = 0; x < image_size; ++x)
        {
          image[y*image_size + x] = (u8)rand();
        }
      }
    }

    f64 idle_thresh = 1;

    Reptest test_mov = {
      .name                = "Mov",
      .bytes_to_process    = image_bytes*out_image_count, // NOTE: num bytes to write
      .idle_time_threshold = idle_thresh,
    };

    Reptest test_movnt = {
      .name                = "MovNT",
      .bytes_to_process    = image_bytes*out_image_count, // NOTE: num bytes to write
      .idle_time_threshold = idle_thresh,
    };

    for (umm test_idx = 0; test_idx < 2; ++test_idx)
    {
      Reptest* test = (test_idx == 0 ? &test_mov : &test_movnt);

      Memset(output_images, image_bytes*out_image_count, 0);

      Reptest_BeginRound(test);

      while (Reptest_RoundIsNotDone(test))
      {
        Reptest_BeginTestSection(test);

        for (umm y = 0; y < image_size; ++y)
        {
          for (umm x = 0; x < image_size; x += 16)
          {
            __m128i full_xor = _mm_setzero_si128();

            for (umm j = 0; j < in_image_count; ++j)
            {
              full_xor = _mm_xor_si128(full_xor, _mm_load_si128((__m128i*)&(input_images + j*image_bytes)[y*image_size + x]));
            }

            for (umm i = 0; i < out_image_count; ++i)
            {
              void* addr = &(output_images + i*image_bytes)[y*image_size + x];

              void* input_to_xor_out = &(input_images + (i%in_image_count)*image_bytes)[y*image_size + x];

              __m128i result = _mm_xor_si128(full_xor, _mm_load_si128(input_to_xor_out));

              if (test_idx == 0) _mm_store_si128(addr, result);
              else               _mm_stream_si128(addr, result);
            }
          }
        }

        Reptest_EndTestSection(test);
        Reptest_AddBytesProcessed(test, 16*out_image_count*(image_size/16)*image_size);
      }

      Reptest_EndRound(test);
    }
  }

  return 0;
}
