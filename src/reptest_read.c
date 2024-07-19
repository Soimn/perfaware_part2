#include "common.h"

typedef struct Read_Params
{
  void* dest;
  u64 size;
  char* filename;
} Read_Params;

typedef struct Read_Func
{
  char* name;
  void (*func)(Reptest* test, Read_Params params);
} Read_Func;

void
Test_freadMalloc(Reptest* test, Read_Params params)
{
  while (Reptest_RoundIsNotDone(test))
  {
    FILE* file;

    if (fopen_s(&file, params.filename, "rb") != 0) Reptest_Error(test, "Failed to open file");
    else
    {
      void* dest = malloc(params.size);

      Reptest_BeginTestSection(test);
      umm result = fread(dest, params.size, 1, file);
      Reptest_EndTestSection(test);

      free(dest);

      if (result == 1) Reptest_AddBytesProcessed(test, params.size);
      else             Reptest_Error(test, "fread failed");

      fclose(file);
    }
  }
}

void
Test_ReadFileMalloc(Reptest* test, Read_Params params)
{
  while (Reptest_RoundIsNotDone(test))
  {
    HANDLE file;

    file = CreateFileA(params.filename, GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

    if (file == INVALID_HANDLE_VALUE) Reptest_Error(test, "Failed to open file");
    else
    {
      void* dest = malloc(params.size);

      u8* cursor   = dest;
      u64 rem_size = params.size;

      while (rem_size > 0)
      {
        u32 bytes_to_read = U32_MAX;
        if (bytes_to_read > rem_size) bytes_to_read = (u32)rem_size;
        rem_size -= bytes_to_read;

        u32 bytes_read;
        Reptest_BeginTestSection(test);
        BOOL result = ReadFile(file, cursor, bytes_to_read, &bytes_read, 0);
        Reptest_EndTestSection(test);

        if (result && bytes_read == bytes_to_read) Reptest_AddBytesProcessed(test, bytes_read);
        else                                       Reptest_Error(test, "ReadFile failed");
      }

      free(dest);

      CloseHandle(file);
    }
  }
}

void
Test_fread(Reptest* test, Read_Params params)
{
  while (Reptest_RoundIsNotDone(test))
  {
    FILE* file;

    if (fopen_s(&file, params.filename, "rb") != 0) Reptest_Error(test, "Failed to open file");
    else
    {
      Reptest_BeginTestSection(test);
      umm result = fread(params.dest, params.size, 1, file);
      Reptest_EndTestSection(test);

      if (result == 1) Reptest_AddBytesProcessed(test, params.size);
      else             Reptest_Error(test, "fread failed");

      fclose(file);
    }
  }
}

void
Test_ReadFile(Reptest* test, Read_Params params)
{
  while (Reptest_RoundIsNotDone(test))
  {
    HANDLE file;

    file = CreateFileA(params.filename, GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

    if (file == INVALID_HANDLE_VALUE) Reptest_Error(test, "Failed to open file");
    else
    {
      u8* cursor   = params.dest;
      u64 rem_size = params.size;

      while (rem_size > 0)
      {
        u32 bytes_to_read = U32_MAX;
        if (bytes_to_read > rem_size) bytes_to_read = (u32)rem_size;
        rem_size -= bytes_to_read;

        u32 bytes_read;
        Reptest_BeginTestSection(test);
        BOOL result = ReadFile(file, cursor, bytes_to_read, &bytes_read, 0);
        Reptest_EndTestSection(test);

        if (result && bytes_read == bytes_to_read) Reptest_AddBytesProcessed(test, bytes_read);
        else                                       Reptest_Error(test, "ReadFile failed");
      }

      CloseHandle(file);
    }
  }
}

Read_Func TestFuncs[] = {
  { "fread",             Test_fread          },
  { "ReadFile",          Test_ReadFile       },
  { "fread + malloc",    Test_freadMalloc    },
  { "ReadFile + malloc", Test_ReadFileMalloc },
};

int
main(int argc, char** argv)
{
  InitializeOSLayer();

  if (argc != 2) fprintf(stderr, "Usage: reptest_read [file to read]\n");
  else
  {

    Read_Params test_params = {0};
    bool setup_succeeded    = false;
    {
      char* filename = argv[1];

      struct __stat64 file_info;
      if (_stat64(filename, &file_info) == 0)
      {
        void* dest = malloc(file_info.st_size);

        if (dest != 0)
        {
          test_params = (Read_Params){
            .filename = filename,
            .dest     = dest,
            .size     = file_info.st_size,
          };

          setup_succeeded = true;
        }
      }
    }

    if (!setup_succeeded) fprintf(stderr, "Failed to allocate memory for tests\n");
    else
    {

      Reptest tests[ARRAY_SIZE(TestFuncs)];
      printf("rdtsc_freq: %llu\n", OSLayer.rdtsc_freq);

      // NOTE: touch entire dest buffer
      Zero(test_params.dest, test_params.size);

      for (umm i = 0; i < ARRAY_SIZE(tests); ++i)
      {
        tests[i] = (Reptest){
          .name                = TestFuncs[i].name,
          .bytes_to_process    = test_params.size,
          .idle_time_threshold = 10,
        };
      }

      for (;;)
      {
        for (umm i = 0; i < ARRAY_SIZE(tests); ++i)
        {
          Reptest* test = &tests[i];

          Reptest_BeginRound(test);

          TestFuncs[i].func(test, test_params);

          Reptest_EndRound(test);
        }
      }
    }
  }

  return 0;
}
