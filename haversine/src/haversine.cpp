#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#if defined(_MSC_VER)
static inline uint64_t byteswap64(uint64_t x)
{
  uint64_t swapped = _byteswap_uint64(x);
  return(swapped);
}
#elif __APPLE__
#include <libkern/OSByteOrder.h>
static inline uint64_t byteswap64(uint64_t x)
{
  uint64_t swapped = OSSwapInt64(x);
  return(swapped);
}
#else
#include <byteswap.h>
static inline uint64_t byteswap64(uint64_t x)
{
  uint64_t swapped = __bswap_64(x);
  return(swapped);
}
#endif

#define unused(var) (void) var;
#define array_count(arr) (sizeof(arr) / sizeof(arr[0]))
#define string_length(str) (array_count(str) - 1)
#define strlitcmp(str, lit) strncmp(str, lit, string_length(lit))

typedef struct haversine_pair
{
  double x0, y0, x1, y1;
} haversine_pair;

static inline double square(double a)
{
  double result = (a * a);
  return result;
}

static inline double radians_from_degrees(double degrees)
{
  double result = 0.01745329251994329577f * degrees;
  return result;
}

// NOTE(antonio): earth_radius is generally expected to be 6372.8
static inline double reference_haversine(double x0, double y0, double x1, double y1, double earth_radius)
{
  /*
   * NOTE(antonio): This is not meant to be a "good" way to calculate the Haversine distance.
   * Instead, it attempts to follow, as closely as possible, the formula used in the real-world
   * question on which these homework exercises are loosely based.
   */

  double lat1 = y0;
  double lat2 = y1;
  double lon1 = x0;
  double lon2 = x1;

  double delta_lat = radians_from_degrees(lat2 - lat1);
  double delta_lon = radians_from_degrees(lon2 - lon1);
  lat1 = radians_from_degrees(lat1);
  lat2 = radians_from_degrees(lat2);

  double a = square(sin(delta_lat / 2.0)) + cos(lat1) * cos(lat2) * square(sin(delta_lon / 2));
  double c = 2.0 * asin(sqrt(a));

  double result = earth_radius * c;
  return result;
}

static char *read_entire_file(FILE *file, int64_t *out_file_size)
{
  char *file_buffer = NULL;

  fseek(file, 0, SEEK_END);
  *out_file_size = ftell(file);
  rewind(file);

  file_buffer = (char *) malloc(*out_file_size);

  if (file_buffer != NULL)
  {
    fread(file_buffer, *out_file_size, 1, file);
  }

  return(file_buffer);
}

static inline bool is_whitespace(char c)
{
  bool is_whitespace_char = (c == '\n') || (c == '\t') || (c == ' ') || (c == '\r');
  return(is_whitespace_char);
}

static int64_t skip_whitespace(char *buffer, int64_t buffer_size, int64_t pos)
{
  while ((pos < buffer_size) && is_whitespace(buffer[pos]))
  {
    pos++;
  }
  return(pos);
}

static int64_t get_string_in_double_quotes(char *buffer, int64_t buffer_size, int64_t pos)
{
  int64_t inner_string_length = -1;

  if (buffer[pos] == '\"')
  {
    int64_t start_string_pos = ++pos;
    int64_t end_string_pos   = start_string_pos;

    while ((end_string_pos < buffer_size) && (buffer[end_string_pos] != '\"'))
    {
      end_string_pos++;
    }

    if (end_string_pos != buffer_size)
    {
      inner_string_length = end_string_pos - start_string_pos;
      end_string_pos--;
    }
    else
    {
      inner_string_length = -2;
    }
  }

  return(inner_string_length);
}

int32_t main(int32_t arg_count, char **args)
{

  char    *json_file_name   = NULL;
  FILE    *json_file        = NULL;
  char    *json_buffer      = NULL;
  int64_t  json_buffer_size = 0;
  int64_t  json_buffer_pos  = 0;
  int64_t  json_buffer_end  = 0;

  char    *binary_file_name   = NULL;
  FILE    *binary_file        = NULL;
  double  *binary_buffer      = NULL;
  int64_t  binary_buffer_size = 0;
  int64_t  binary_buffer_pos  = 0;

  double  haversine_sum = 0.0;
  int64_t pair_count    = 0;

  unused(binary_buffer_pos);
  unused(haversine_sum);
  unused(pair_count);

  if ((2 <= arg_count) && (arg_count <= 3))
  {
    json_file_name = args[1];

    if (arg_count == 3)
    {
      binary_file_name = args[2];
    }
  }
  else
  {
    fprintf(stderr, "Need to provide at least a .json file with the requisite pairs\n");
    fprintf(stderr, "Usage: haversine.exe json_file_name [binary data]\n");
    return(1);
  }

  json_file = fopen(json_file_name, "rb");

  if (json_file == NULL)
  {
    fprintf(stderr, "The file %s doesn't exist\n", json_file_name);
    return(1);
  }

  json_buffer = read_entire_file(json_file, &json_buffer_size);

  if (binary_file_name != NULL)
  {
    binary_file = fopen(binary_file_name, "rb");

    if (binary_file == NULL)
    {
      fprintf(stderr, "The file %s doesn't exist\n", binary_file_name);
      return(1);
    }

    binary_buffer = (double *) read_entire_file(binary_file, &binary_buffer_size);
  }

  json_buffer_pos = skip_whitespace(json_buffer, json_buffer_size, json_buffer_pos);
  if (json_buffer[json_buffer_pos] == '{')
  {
    int64_t backtracker = json_buffer_size - 1;
    while ((0 <= backtracker) && (is_whitespace(json_buffer[backtracker])))
    {
      backtracker--;
    }

    json_buffer_end = backtracker;
    if (json_buffer[json_buffer_end] == '}')
    {
      json_buffer_pos++;
      json_buffer_size = json_buffer_end;
      json_buffer_pos  = skip_whitespace(json_buffer, json_buffer_size, json_buffer_pos);

      if (json_buffer[json_buffer_pos] == '\"')
      {
        int64_t string_start_pos, string_end_pos;

        json_buffer_pos++;

        string_start_pos = json_buffer_pos;

        while ((json_buffer_pos < json_buffer_size) && (json_buffer[json_buffer_pos] != '\"'))
        {
          json_buffer_pos++;
        }

        if (json_buffer_pos == json_buffer_size) 
        {
          fprintf(stderr, "Expected closing '\"' for one in index %lld\n", string_start_pos);
        }

        string_end_pos = json_buffer_pos - 1;
        
        bool size_comparison = ((string_end_pos + 1) - string_start_pos) == string_length("pairs");
        if (size_comparison && (strncmp(&json_buffer[string_start_pos], "pairs", string_length("pairs")) == 0))
        {
          json_buffer_pos++;
          json_buffer_pos = skip_whitespace(json_buffer, json_buffer_size, json_buffer_pos);

          if (json_buffer[json_buffer_pos] == ':')
          {
            json_buffer_pos++;
            json_buffer_pos = skip_whitespace(json_buffer, json_buffer_size, json_buffer_pos);

            if (json_buffer[json_buffer_pos] == '[')
            {
              backtracker = json_buffer_end - 1;
              while ((0 <= backtracker) && (is_whitespace(json_buffer[backtracker])))
              {
                backtracker--;
              }

              json_buffer_end = backtracker;
              if (json_buffer[json_buffer_end] == ']')
              {
                backtracker = json_buffer_end - 1;
                while ((0 <= backtracker) && (is_whitespace(json_buffer[backtracker])))
                {
                  backtracker--;
                }

                json_buffer_end = backtracker;
                if (json_buffer[json_buffer_end] != ',')
                {
                  haversine_pair pair;

                  int64_t string_length    = 0;

                  double  *double_to_fill   = NULL;
                  int64_t  double_start_pos = 0;
                  char    *double_end       = NULL;

                  json_buffer_pos++;
                  json_buffer_size = json_buffer_end;
                  json_buffer_pos  = skip_whitespace(json_buffer, json_buffer_size, json_buffer_pos);

                  // NOTE(antonio): haversine loop
                  while ((json_buffer_pos < json_buffer_end))
                  {
                    pair.x0 = 500.0;
                    pair.y0 = 500.0;
                    pair.x1 = 500.0;
                    pair.y1 = 500.0;

                    while ((json_buffer_pos < json_buffer_size) && (json_buffer[json_buffer_pos] != '{'))
                    {
                      json_buffer_pos++;
                    }

                    if (json_buffer_pos >= json_buffer_size)
                    {
                      break;
                    }

                    json_buffer_pos++;
                    for (int64_t value_index = 0;
                         value_index < 4;
                         ++value_index)
                    {
                      json_buffer_pos = skip_whitespace(json_buffer, json_buffer_size, json_buffer_pos);

                      string_length = get_string_in_double_quotes(json_buffer, json_buffer_size, json_buffer_pos);
                      if (string_length < 0)
                      {
                        if (string_length == -1)
                        {
                          fprintf(stderr, "Expected to find starting double quotes at %lld\n", json_buffer_pos);
                          return(1);
                        }
                        else
                        {
                          fprintf(stderr, "Did not find ending double quotes at %lld\n", json_buffer_pos);
                          return(1);
                        }
                      }

                      json_buffer_pos++;
                      if (strlitcmp(&json_buffer[json_buffer_pos], "x0") == 0)
                      {
                        double_to_fill = &pair.x0;
                      }
                      else if (strlitcmp(&json_buffer[json_buffer_pos], "y0") == 0)
                      {
                        double_to_fill = &pair.y0;
                      }
                      else if (strlitcmp(&json_buffer[json_buffer_pos], "x1") == 0)
                      {
                        double_to_fill = &pair.x1;
                      }
                      else if (strlitcmp(&json_buffer[json_buffer_pos], "y1") == 0)
                      {
                        double_to_fill = &pair.y1;
                      }
                      else
                      {
                        fprintf(stderr, "Unrecognized pair name\n");
                        return(1);
                      }

                      double_start_pos = json_buffer_pos + string_length + 1;
                      double_start_pos = skip_whitespace(json_buffer, json_buffer_size, double_start_pos);

                      if (json_buffer[double_start_pos] == ':')
                      {
                        char comma_or_end_brace = (value_index < 3) ? ',' : '}';

                        double_start_pos++;
                        double_start_pos = skip_whitespace(json_buffer, json_buffer_size, double_start_pos);
                        *double_to_fill  = strtod(&json_buffer[double_start_pos], &double_end);

                        json_buffer_pos = ((uintptr_t) double_end) - ((uintptr_t) json_buffer);
                        json_buffer_pos = skip_whitespace(json_buffer, json_buffer_size, json_buffer_pos);

                        if (json_buffer[json_buffer_pos] == comma_or_end_brace)
                        {
                          json_buffer_pos++;
                        }
                        else
                        {
                          fprintf(stderr, "json is malformed. Expected '%c' after point value\n", comma_or_end_brace);
                          return(1);
                        }
                      }
                      else
                      {
                        fprintf(stderr, "json is malformed. Expected ':' after property name\n");
                        return(1);
                      }
                    }

                    if ((pair.x0 != 500.0) && (pair.y0 != 500.0) && (pair.x1 != 500.0) && (pair.y1 != 500.0))
                    {
                      haversine_sum = reference_haversine(pair.x0, pair.y0, pair.x1, pair.y1, 6372.8);
                      pair_count++;
                    }
                    else
                    {
                      fprintf(stderr, "Missing a point in json :(\n");
                      return(1);
                    }
                  }

                  fprintf(stdout, "Average Sum: %f\n", haversine_sum / ((double) pair_count));
                }
                else
                {
                  fprintf(stderr, "Did not expect ',' at %lld\n", json_buffer_end);
                }
              }
              else
              {
                fprintf(stderr, "Expected closing ']' at %lld\n", json_buffer_end);
              }
            }
            else
            {
              fprintf(stderr, "Expected '[' at %lld\n", json_buffer_pos);
            }
          }
          else
          {
            fprintf(stderr, "Expected ':' at %lld\n", json_buffer_pos);
          }
        }
        else
        {
          fprintf(stderr, "Expected first string to be \"pairs\"\n");
        }
      }
    }
    else
    {
      fprintf(stderr, "json is malformed. Expected last token to be '}'\n");
    }
  }
  else
  {
    fprintf(stderr, "json is malformed. Expected first token to be  '{'\n");
  }

  fclose(json_file);
  if (binary_file)
  {
    fclose(binary_file);
  }

  return(0);
}
