#include <stdio.h>
#include <math.h>
#include <stdlib.h>

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

#define ARRAY_SIZE(A) (sizeof(A)/sizeof(0[A]))

bool
Char_IsWhitespace(u8 c)
{
  return (c == ' ' || c == '\n' || c == '\r' || c == '\t');
}

bool
Char_IsDigit(u8 c)
{
  return ((u8)(c-0x30) < (u8)10);
}

bool
Char_IsHexAlphaDigit(u8 c)
{
  return ((u8)((c&0xDF) - 'A') <= (u8)('F' - 'A'));
}

typedef enum Token_Kind
{
  Token_Invalid = 0,
  Token_EOF,

  Token_OpenBrace,
  Token_CloseBrace,
  Token_OpenBracket,
  Token_CloseBracket,
  Token_Colon,
  Token_Comma,
  Token_Number,

  // NOTE: Strings as token kinds because I am lazy
  Token_Pairs,

  Token__FirstCoord,
  Token_X0 = Token__FirstCoord,
  Token_X1,
  Token_Y0,
  Token_Y1,
  Token__PastLastCoord,
} Token_Kind;

typedef struct Token
{
  Token_Kind kind;
  f64 number;
} Token;

static bool
EatStrChar(u8** s, u8 c)
{
  if (**s == c)
  {
    *s += 1;
    return true;
  }
  else if ((*s)[0] == '\\' && (*s)[1] == 'u' && (*s)[2] == '0' && (*s)[3] == '0')
  {
    u8 val = 0;

    if      (Char_IsDigit((*s)[4]))         val |= (*s)[4]&0xF;
    else if (Char_IsHexAlphaDigit((*s)[4])) val |= 9 + ((*s)[4]&0x7);
    else return false;

    val <<= 4;

    if      (Char_IsDigit((*s)[5]))         val |= (*s)[5]&0xF;
    else if (Char_IsHexAlphaDigit((*s)[5])) val |= 9 + ((*s)[5]&0x7);
    else return false;

    if (val == c)
    {
      *s += 6;
      return true;
    }
  }

  return false;
}

typedef struct Lexer
{
  u8* cursor;
  Token token;
} Lexer;

static Token
NextToken(Lexer* lexer)
{
  Token token = { .kind = Token_Invalid };

  u8* cursor = lexer->cursor;

  while (Char_IsWhitespace(*cursor)) ++cursor;

  if (*cursor == 0) token.kind = Token_EOF;
  else
  {
    u8 c = *cursor;
    ++cursor;

    switch (c)
    {
      case '{': token.kind = Token_OpenBrace;    break;
      case '}': token.kind = Token_CloseBrace;   break;
      case '[': token.kind = Token_OpenBracket;  break;
      case ']': token.kind = Token_CloseBracket; break;
      case ':': token.kind = Token_Colon;        break;
      case ',': token.kind = Token_Comma;        break;

      case '"':
      {
        bool encountered_errors = false;

        if (EatStrChar(&cursor, 'x'))
        {
          if      (EatStrChar(&cursor, '0')) token.kind = Token_X0;
          else if (EatStrChar(&cursor, '1')) token.kind = Token_X1;
          else encountered_errors = true;
        }
        else if (EatStrChar(&cursor, 'y'))
        {
          if      (EatStrChar(&cursor, '0')) token.kind = Token_Y0;
          else if (EatStrChar(&cursor, '1')) token.kind = Token_Y1;
          else encountered_errors = true;
        }
        else
        {
          u8* match_cur = "pairs";
          
          while (*match_cur != 0 && EatStrChar(&cursor, *match_cur)) ++match_cur;

          if (*match_cur == 0) token.kind = Token_Pairs;
          else encountered_errors = true;
        }

        if (encountered_errors || !EatStrChar(&cursor, '"')) token.kind = Token_Invalid;
      } break;

      case '-':
      case '0': case '1': case '2': case '3': case '4':
      case '5': case '6': case '7': case '8': case '9':
      {
        bool is_negative = false;
        f64 number       = c&0xF;

        if (c == '-')
        {
          is_negative = true;
          number      = 0;
        }

        if (!is_negative || Char_IsDigit(*cursor))
        {
          bool encountered_errors = false;

          // TODO: Maybe check for loss of precision

          while (Char_IsDigit(*cursor))
          {
            number = number*10 + (*cursor&0xF);

            ++cursor;
          }

          if (!encountered_errors && *cursor == '.')
          {
            ++cursor;

            if (!Char_IsDigit(*cursor)) encountered_errors = true;
            else
            {
              f64 multiplier = 0.1;
              while (Char_IsDigit(*cursor))
              {
                number     += (*cursor&0xF)*multiplier;
                multiplier *= 0.1;

                ++cursor;
              }
            }
          }

          if (!encountered_errors && (*cursor&0xDF) == 'E')
          {
            ++cursor;

            f64 multiplier = 10;
            if      (*cursor == '+') ++cursor;
            else if (*cursor == '-')
            {
              multiplier = 0.1;
              ++cursor;
            }

            if (!Char_IsDigit(*cursor)) encountered_errors = true;
            else
            {
              // TODO: Maybe check for overflow
              umm exponent = 0;
              while (Char_IsDigit(*cursor))
              {
                exponent = exponent*10 + (*cursor&0xF);

                ++cursor;
              }

              // TODO: This can get really bad
              for (umm i = 0; i < exponent; ++i) number *= multiplier;
            }
          }

          if (is_negative) number = -number;

          if (encountered_errors) token.kind = Token_Invalid;
          else
          {
            token.kind   = Token_Number;
            token.number = number;
          }
        }
      } break;

      default: token.kind = Token_Invalid; break;
    }
  }

  if (token.kind != Token_Invalid) lexer->cursor = cursor;
  lexer->token  = token;

  return token;
}

static Token
GetToken(Lexer* lexer)
{
  return lexer->token;
}

static bool
EatToken(Lexer* lexer, Token_Kind kind)
{
  if (GetToken(lexer).kind == kind)
  {
    NextToken(lexer);
    return true;
  }
  else return false;
}

// NOTE: Eats as many as it matches, returns true if all tokens are eaten
static bool
EatTokens_(Lexer* lexer, umm token_count, Token_Kind* tokens)
{
  for (umm i = 0; i < token_count; ++i)
  {
    if (GetToken(lexer).kind == tokens[i]) NextToken(lexer);
    else return false;
  }

  return true;
}

#define EatTokens(lexer, ...) EatTokens_((lexer), sizeof((Token_Kind[]){__VA_ARGS__})/sizeof(Token_Kind), (Token_Kind[]){__VA_ARGS__})

int
main(int argc, char** argv)
{
  if (argc < 2 || argc > 3)
  {
    fprintf(stderr, "Usage: haversine_proc [json file]\n       haversine_proc [json file] [answer file]\n");
  }
  else
  {
    f64 acc_mean  = 0;
    umm num_pairs = 0;

    FILE* json_input;
    if (fopen_s(&json_input, argv[1], "rb") != 0) fprintf(stderr, "Failed to open input file\n");
    else
    {
      fseek(json_input, 0, SEEK_END);
      umm json_input_size = ftell(json_input);
      rewind(json_input);

      u8* input = malloc(json_input_size + 1);

      if      (input == 0)                                                      fprintf(stderr, "Failed to allocate memory for input\n");
      else if (fread(input, 1, json_input_size, json_input) != json_input_size) fprintf(stderr, "Failed to read input file\n");
      else
      {
        input[json_input_size] = 0;

        bool encountered_errors = false;

        Lexer lexer = { .cursor = input };
        NextToken(&lexer);

        if (!EatTokens(&lexer, Token_OpenBrace, Token_Pairs, Token_Colon, Token_OpenBracket))
        {
          //// ERROR: Missing {"pairs":[
          encountered_errors = true;
        }
        else
        {
          for (;;)
          {
            if (!EatToken(&lexer, Token_OpenBrace))
            {
              encountered_errors = true;
              break;
            }

            #define NIL_COORD 512
            f64 coords[4] = { NIL_COORD, NIL_COORD, NIL_COORD, NIL_COORD };
            f64 bounds[4] = { 180, 180, 90, 90};

            for (umm found_coords = 0;;)
            {
              Token token = GetToken(&lexer);

              if (!(token.kind >= Token__FirstCoord && token.kind < Token__PastLastCoord))
              {
                //// ERROR: name of value not matching any of the coord names
                encountered_errors = true;
                break;
              }
              else if (coords[token.kind-Token__FirstCoord] != NIL_COORD)
              {
                //// ERROR: Two elements with same name
                encountered_errors = true;
                break;
              }
              else
              {
                umm coord_idx = token.kind - Token__FirstCoord;
                
                NextToken(&lexer);

                if (!EatToken(&lexer, Token_Colon) || GetToken(&lexer).kind != Token_Number)
                {
                  //// ERROR: Missing :value
                  encountered_errors = true;
                  break;
                }
                else
                {
                  Token token = GetToken(&lexer);

                  f64 number     = token.number;
                  f64 abs_number = (number < 0 ? -number : number);

                  // TODO: Float imprecision
                  if (abs_number < -bounds[coord_idx] || abs_number > bounds[coord_idx])
                  {
                    //// ERROR: Coord is out of bounds
                    encountered_errors = true;
                    break;
                  }
                  else
                  {
                    coords[coord_idx] = number;
                    ++found_coords;
                    NextToken(&lexer);
                  }
                }
              }

              if (EatToken(&lexer, Token_Comma)) continue;
              else
              {
                if (found_coords != ARRAY_SIZE(coords))
                {
                  //// ERROR: Not all coords present
                  encountered_errors = true;
                }

                break;
              }
            }

            if (encountered_errors) break;

            if (!EatToken(&lexer, Token_CloseBrace))
            {
              encountered_errors = true;
              break;
            }

            // TODO: Compute
            f64 answer = ReferenceHaversine(coords[0], coords[2], coords[1], coords[3], 6372.8);

            acc_mean += answer;
            ++num_pairs;

            if (EatToken(&lexer, Token_Comma)) continue;
            else                               break;
          }

          if (!encountered_errors && !EatTokens(&lexer, Token_CloseBracket, Token_CloseBrace, Token_EOF))
          {
            //// ERROR: Missing ]} or extra elements in outermost object
            encountered_errors = true;
          }
        }

        if (encountered_errors)
        {
          fprintf(stderr, "Input file is ill-formed\n");
        }
        else
        {
          printf("Mean: %.16f\n", acc_mean/num_pairs);
        }
      }

      fclose(json_input);
    }
  }

  return 0;
}
