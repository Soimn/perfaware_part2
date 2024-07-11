#include "common.h"

int
main(int argc, char** argv)
{
  if (argc != 2) fprintf(stderr, "Usage: pagefault_testing [number of pages]\n");
  else
  {
    umm page_count = atol(argv[1]);

    umm page_size = 4096;

    /// forward
    {
      FILE* out;
      fopen_s(&out, "pagefaults_forward.csv", "wb");
      for (umm i = 0; i < page_count; ++i)
      {
        u64 initial_fault_counter = GetPageFaultCounter();

        u8* memory = VirtualAlloc(0, page_count*page_size, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
        for (umm j = 0; j < i*page_size; ++j)
        {
          memory[j] = (u8)j;
        }

        VirtualFree(memory, 0, MEM_RELEASE);

        u64 fault_count = GetPageFaultCounter() - initial_fault_counter;

        fprintf(out, "%llu, %llu, %llu, %llu,\n", page_count, i, fault_count, fault_count - i);
      }
      fclose(out);
    }

    /// backward
    {
      FILE* out;
      fopen_s(&out, "pagefaults_backward.csv", "wb");
      for (umm i = 0; i < page_count; ++i)
      {
        u64 initial_fault_counter = GetPageFaultCounter();

        u8* memory = VirtualAlloc(0, page_count*page_size, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
        for (umm j = 0; j < i*page_size; ++j)
        {
          memory[(i*page_size-1) - j] = (u8)j;
        }

        VirtualFree(memory, 0, MEM_RELEASE);

        u64 fault_count = GetPageFaultCounter() - initial_fault_counter;

        fprintf(out, "%llu, %llu, %llu, %llu,\n", page_count, i, fault_count, fault_count - i);
      }
      fclose(out);
    }

    /// ping pong
    {
      FILE* out;
      fopen_s(&out, "pagefaults_pingpong.csv", "wb");
      for (umm i = 0; i < page_count; ++i)
      {
        u64 initial_fault_counter = GetPageFaultCounter();

        u8* memory = VirtualAlloc(0, page_count*page_size, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
        for (umm j = 0; j < i*page_size; ++j)
        {
          if (j % 2 == 0) memory[j/2]                   = (u8)j;
          else            memory[(i*page_size-1) - j/2] = (u8)j;
        }

        VirtualFree(memory, 0, MEM_RELEASE);

        u64 fault_count = GetPageFaultCounter() - initial_fault_counter;

        fprintf(out, "%llu, %llu, %llu, %llu,\n", page_count, i, fault_count, fault_count - i);
      }
      fclose(out);
    }
  }

  return 0;
}
