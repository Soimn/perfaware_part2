#include <stdio.h>
#include <math.h>

#include "splitmix64.h"
#include "xoroshiro256plus.h"

typedef unsigned __int8 u8;
typedef unsigned __int64 u64;

typedef u64 umm;

typedef double f64;

typedef u8 bool;
#define true 1
#define false 0

#define PI       3.14159265358979323846264338327950
#define PI_DIV_2 1.57079632679489661923132169163975

#include "../listings/listing_0065_haversine_formula.cpp"

// NOTE: Random bullshit go!
void
Rand_Seed(u64 seed)
{
  s[0] = splitmix64_next(seed);
  s[1] = splitmix64_next(s[0]);
  s[2] = splitmix64_next(s[1]);
  s[3] = splitmix64_next(s[2]);
}

f64
Rand_PM1()
{
  u64 bits = xoroshiro256plus_next();

  f64 rnd01 = (union{ u64 bits; f64 f; }){ .bits = 0x3FF0000000000000 | bits&0x000FFFFFFFFFFFFF }.f;

  return (rnd01 - 1.5)*2;
}

int
main(int argc, char** argv)
{
  bool should_cluster = false;
  umm seed            = 0;
  umm num_pairs       = 0;

  { /// Parse args
    bool succeeded = false;

    if (argc == 4)
    {
      char* uniform = "uniform";

      bool matches_uniform = true;
      for (char* scan = argv[1]; *scan != 0 || *uniform != 0; ++scan, ++uniform)
      {
        if (*scan != *uniform)
        {
          matches_uniform = false;
          break;
        }
      }

      char* cluster = "cluster";

      bool matches_cluster = true;
      for (char* scan = argv[1]; *scan != 0 || *uniform != 0; ++scan, ++cluster)
      {
        if (*scan != *cluster)
        {
          matches_cluster = false;
          break;
        }
      }

      if (matches_uniform || matches_cluster)
      {
        should_cluster = matches_cluster;

        bool seed_valid = true;
        for (char* scan = argv[2]; *scan != 0; ++scan)
        {
          if ((u8)(*scan-0x30) >= (u8)10 ||
              (seed*10 + (*scan&0xF))/10 != seed)
          {
            seed_valid = false;
            break;
          }
          else
          {
            seed = seed*10 + (*scan&0xF);
          }
        }

        bool num_pairs_valid = true;
        for (char* scan = argv[3]; *scan != 0; ++scan)
        {
          if ((u8)(*scan-0x30) >= (u8)10 ||
              (num_pairs*10 + (*scan&0xF))/10 != num_pairs)
          {
            num_pairs_valid = false;
            break;
          }
          else
          {
            num_pairs = num_pairs*10 + (*scan&0xF);
          }
        }

        succeeded = (seed_valid && num_pairs_valid);
      }
    }


    if (!succeeded)
    {
      printf("Usage: haversine_gen [uniform/cluster] [random seed] [# of coord pairs]\n");
      return -1;
    }
  }

  { /// Gen points
    bool encountered_errors = false;

    Rand_Seed(seed);

    char json_filename[sizeof("data__flex.json") + 19];
    char answer_filename[sizeof("data__haveranswer.f64") + 19];

    snprintf(json_filename, sizeof(json_filename), "data_%llu_flex.json", num_pairs);
    snprintf(answer_filename, sizeof(answer_filename), "data_%llu_haveranswer.f64", num_pairs);

    FILE* json_output;
    FILE* answer_output;
    if (fopen_s(&json_output, json_filename, "wb") != 0 || fopen_s(&answer_output, answer_filename, "wb") != 0)
    {
      fprintf(stderr, "Failed to open output files\n");
      encountered_errors = true;
    }
    else
    {
      fprintf(json_output, "{\"pairs\":[\n");

      f64 acc_answer = 0;

      if (should_cluster)
      {
        fprintf(stderr, "NOT IMPLEMENTED\n");
        encountered_errors = true;
      }
      else
      {
        for (umm i = 0; i < num_pairs; ++i)
        {
          f64 x0 = Rand_PM1() * 180;
          f64 x1 = Rand_PM1() * 180;
          f64 y0 = Rand_PM1() * 90;
          f64 y1 = Rand_PM1() * 90;

          f64 answer = ReferenceHaversine(x0, y0, x1, y1, 6372.8);
          acc_answer += answer;

          fprintf(json_output, "    {\"x0\":%.16f, \"y0\":%.16f, \"x1\":%.16f, \"y1\":%.16f}%s\n", x0, y0, x1, y1, (i < num_pairs-1 ? "," : ""));
          fwrite(&answer, sizeof(f64), 1, answer_output);
        }
      }

      fprintf(json_output, "]}\n");

      f64 mean_answer = acc_answer/num_pairs;
      fwrite(&mean_answer, sizeof(f64), 1, answer_output);

      fclose(json_output);
      fclose(answer_output);
    }
  }

  return 0;
}
