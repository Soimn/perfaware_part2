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

    Rand_Seed(seed);

    printf("{\"pairs\":[\n");

    if (should_cluster)
    {
      fprintf(stderr, "NOT IMPLEMENTED\n");
    }
    else
    {
      for (umm i = 0; i < num_pairs; ++i)
      {
        f64 x0 = Rand_PM1() * 180;
        f64 x1 = Rand_PM1() * 180;
        f64 y0 = Rand_PM1() * 90;
        f64 y1 = Rand_PM1() * 90;

        printf("    {\"x0\":%.6f, \"y0\":%.6f, \"x1\":%.6f, \"y1\":%.6f}%s\n", x0, y0, x1, y1, (i < num_pairs-1 ? "," : ""));
      }
    }

    printf("]}\n");
  }

  return 0;
}
