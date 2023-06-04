#ifdef _WIN32
#pragma warning(disable: 4710)
#pragma warning(disable: 4505)
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <math.h>

#define unused(var) (void) (var)

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

static inline double get_random_angle(double min, double delta)
{
  static const double rand_reciprocal = 1.0 / ((double) RAND_MAX);

  double t = ((double) rand()) * rand_reciprocal;
  double result = (t * delta) + min;

  if      (result < -180.0) result = 180.0 + result;
  else if (result >  180.0) result = result - 360.0;

  return(result);
}

typedef int32_t method;
enum
{
  method_none,
  method_simple,
  method_cluster,
  method_count
};

int32_t main(int32_t arg_count, char *arg_values[])
{
  char   *output_json_file_name   = NULL;
  char   *output_binary_file_name = NULL;

  int64_t  pair_count      = 65536;
  bool     print_output    = false;
  uint32_t seed            = (uint32_t) time(NULL);

  bool     bad_parameters  = false;

  FILE    *out_json_file   = NULL;
  FILE    *out_binary_file = NULL;

  method   method = method_simple;

  static const double rand_reciprocal = 1.0 / ((double) RAND_MAX);
  double   x0 = 0.0, x1 = 0.0, y0 = 0.0, y1 = 0.0;

  double   haversine     = 0.0;
  double   haversine_sum = 0.0;

  if (arg_count > 1)
  {
    for (int64_t arg_index = 1;
         arg_index < (int64_t) arg_count;
         arg_index++)
    {
      char *cur_option = arg_values[arg_index];

      if (cur_option[0] == '-')
      {
        if (strnlen(cur_option, 32) < 2)
        {
          fprintf(stderr, "Please specify an option: -h[elp], -c[ount] n, "
                  "-p[artitions] n, -s[eed] n, -b[inary] file_name -print_output\n");
        }

        switch (cur_option[1])
        {
        case 'b':
        {
          if ((arg_index + 1) >= arg_count)
          {
            fprintf(stderr, "Expected another argument to specify value for option: \"%s\"\n", cur_option);
          }

          output_binary_file_name = arg_values[++arg_index];
        } break;

        case 'c':
        {
          if ((arg_index + 1) >= arg_count)
          {
            fprintf(stderr, "Expected another argument to specify value for option: \"%s\"\n", cur_option);
          }

          char *value = arg_values[++arg_index];
          pair_count = atoi(value);
        } break;

        case 'h':
        {
          fprintf(stdout, "Haversine JSON pair generator tool: haversine.exe [output_json_file_name] "
                  "[-h[elp] -m[ethod] [cluster/simple] -c[ount] n -s[eed] n -b[inary] file-name -print_output]\n");
          return(0);
        } break;

        case 'm':
        {
          if ((arg_index + 1) >= arg_count)
          {
            fprintf(stderr, "Expected another argument to specify value for option: \"%s\"\n", cur_option);
          }

          char *value = arg_values[++arg_index];

          if (strncmp(value, "cluster", 32) == 0)
          {
            method = method_cluster;
          }
          else if (strncmp(value, "simple", 32) == 0)
          {
            method = method_simple;
          }
          else
          {
            fprintf(stdout, "Expected method to be either \"simple\" or \"cluster\"");
            return(1);
          }
        } break;

        case 'p':
        {
          print_output = true;
        } break;

        case 's':
        {
          if ((arg_index + 1) >= arg_count)
          {
            fprintf(stderr, "Expected another argument to specify value for option: \"%s\"\n", cur_option);
          }

          char *value = arg_values[++arg_index];
          seed = (uint32_t) atoi(value);
        } break;

        default:
        {
          fprintf(stderr, "Did not specify a correct option. Please use: "
                  "-c[ount], -p[artitions], -print_output, -b[inary], -h[elp]\n");
        } break;
        }
      }
      else if (output_json_file_name == NULL)
      {
        output_json_file_name = cur_option;
      }
      else
      {
        fprintf(stdout, "Haversine JSON pair generator tool: hjson.exe [output_json_file_name] "
                "[-h[elp] -m[ethod] [cluster/simple] -c[ount] n -s[eed] n -b[inary] file-name -print_output]\n");
        return(0);
      }
    }
  }

  if (pair_count <= 0)
  {
    fprintf(stderr, "The pair count needs to be greater than 0: %lld", pair_count);
    bad_parameters = false;
  }

  if (pair_count <= 0)
  {
    fprintf(stderr, "The pair count needs to be greater than 0: %lld", pair_count);
    bad_parameters = false;
  }

  if (output_json_file_name == NULL)
  {
    output_json_file_name = "haversine.json";
  }

  if (output_binary_file_name == NULL)
  {
    output_binary_file_name = "haversine.f64";
  }

  out_json_file = fopen(output_json_file_name, "w+");
  if (out_json_file == NULL)
  {
    fprintf(stderr, "Could not open file: %s\n", output_json_file_name);
    bad_parameters = false;
  }

  if (output_binary_file_name != NULL)
  {
    out_binary_file = fopen(output_binary_file_name, "w+");
    if (out_binary_file == NULL)
    {
      fprintf(stderr, "Could not open file: %s\n", output_binary_file_name);
      bad_parameters = false;
    }
  }

  if (bad_parameters)
  {
    return(1);
  }

  srand(seed);

  fprintf(out_json_file, "{\"pairs\":[");
  if (print_output)
  {
    fprintf(stdout, "{\"pairs\":[");
  }

  if (method == method_simple)
  {
    for (int64_t pair_index = 0;
         pair_index < pair_count;
         ++pair_index)
    {
      x0 = get_random_angle(-180.0, 360.0);
      y0 = get_random_angle(-180.0, 360.0);
      x1 = get_random_angle(-180.0, 360.0);
      y1 = get_random_angle(-180.0, 360.0);

      haversine = reference_haversine(x0, y0, x1, y1, 6372.8);
      haversine_sum += haversine;

      fprintf(out_json_file, "{\"x0\":%f,\"y0\":%f,\"x1\":%f,\"y1\":%f},", x0, y0, x1, y1);
      if (print_output)
      {
        fprintf(stdout, "{\"x0\":%f,\"y0\":%f,\"x1\":%f,\"y1\":%f},", x0, y0, x1, y1);
      }

      if (output_binary_file_name != NULL)
      {
        fprintf(out_binary_file, "%.8s", (char *) &haversine);
      }
    }
  }
  else if (method == method_cluster)
  {
    int64_t cluster_count = 64;

    int64_t pairs_per_cluster = (pair_count / cluster_count);
    int64_t lingering_pairs   = (pair_count % cluster_count);

    double cluster_distance = 30.0;

    for (int64_t cluster_index = 0;
         cluster_index < cluster_count;
         ++cluster_index)
    {
      double cluster_x_t = ((double) rand()) * rand_reciprocal;
      double cluster_y_t = ((double) rand()) * rand_reciprocal;

      double cluster_x = (360.0 * cluster_x_t) - 180.0;
      double cluster_y = (360.0 * cluster_y_t) - 180.0;

      double cluster_x_min = cluster_x - cluster_distance;
      double cluster_y_min = cluster_y - cluster_distance;

      for (int64_t pair_index = 0;
           pair_index < pairs_per_cluster;
           ++pair_index)
      {
        x0 = get_random_angle(cluster_x_min, cluster_distance);
        y0 = get_random_angle(cluster_y_min, cluster_distance);
        x1 = get_random_angle(cluster_x_min, cluster_distance);
        y1 = get_random_angle(cluster_y_min, cluster_distance);

        haversine = reference_haversine(x0, y0, x1, y1, 6372.8);
        haversine_sum += haversine;

        fprintf(out_json_file, "{\"x0\":%f,\"y0\":%f,\"x1\":%f,\"y1\":%f},", x0, y0, x1, y1);
        if (print_output)
        {
          fprintf(stdout, "{\"x0\":%f,\"y0\":%f,\"x1\":%f,\"y1\":%f},", x0, y0, x1, y1);
        }

        if (output_binary_file_name != NULL)
        {
          fprintf(out_binary_file, "%.8s", (char *) &haversine);
        }
      }
    }

    for (int64_t pair_index = 0;
         pair_index < lingering_pairs;
         ++pair_index)
    {
      x0 = get_random_angle(-180.0, 360.0);
      y0 = get_random_angle(-180.0, 360.0);
      x1 = get_random_angle(-180.0, 360.0);
      y1 = get_random_angle(-180.0, 360.0);

      haversine = reference_haversine(x0, y0, x1, y1, 6372.8);
      haversine_sum += haversine;

      fprintf(out_json_file, "{\"x0\":%f,\"y0\":%f,\"x1\":%f,\"y1\":%f},", x0, y0, x1, y1);
      if (print_output)
      {
        fprintf(stdout, "{\"x0\":%f,\"y0\":%f,\"x1\":%f,\"y1\":%f},", x0, y0, x1, y1);
      }

      if (output_binary_file_name != NULL)
      {
        fprintf(out_binary_file, "%.8s", (char *) &haversine);
      }
    }
  }

  fseek(out_json_file, -1, SEEK_CUR);
  fprintf(out_json_file, "]}");
  fflush(out_json_file);

  if (print_output)
  {
    fprintf(stdout, "\b{\"x0\":%f,\"y0\":%f,\"x1\":%f,\"y1\":%f}]}", x0, y0, x1, y1);
  }

  if (output_binary_file_name != NULL)
  {
    fprintf(out_binary_file, "%.8s", (char *) &haversine);
    fflush(out_binary_file);
  }

  fprintf(stdout,
          "\nSeed: %d\nPair Count: %lld\nExpected Sum: %f",
          seed, pair_count, haversine_sum / ((double) pair_count));

  return(0);
}
