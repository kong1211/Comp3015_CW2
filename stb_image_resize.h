#ifndef STB_IMAGE_RESIZE_H
#define STB_IMAGE_RESIZE_H

#ifdef __cplusplus
extern "C" {
#endif

#define STBIR_EDGE_CLAMP   1
#define STBIR_EDGE_REFLECT 2
#define STBIR_EDGE_WRAP    3
#define STBIR_EDGE_ZERO    4

#define STBIR_FILTER_DEFAULT     0  // use same filter type that easy-to-use API chooses
#define STBIR_FILTER_BOX         1  // A trapezoid w/1-pixel wide ramps, same result as box for integer scale ratios
#define STBIR_FILTER_TRIANGLE    2  // On upsampling, produces same results as bilinear texture filtering
#define STBIR_FILTER_CUBICBSPLINE 3  // The cubic b-spline (aka Mitchell-Netrevalli with B=1,C=0), gaussian-esque
#define STBIR_FILTER_CATMULLROM  4  // An interpolating cubic spline
#define STBIR_FILTER_MITCHELL    5  // Mitchell-Netrevalli filter with B=1/3, C=1/3

// The following functions use the "default" resampling filter specified at compile time.
// For better quality, you can call the more general functions below.

int stbir_resize_uint8(     const unsigned char *input_pixels , int input_w , int input_h , int input_stride_in_bytes,
                                 unsigned char *output_pixels, int output_w, int output_h, int output_stride_in_bytes,
                                 int num_channels);
int stbir_resize_float(     const float *input_pixels , int input_w , int input_h , int input_stride_in_bytes,
                                 float *output_pixels, int output_w, int output_h, int output_stride_in_bytes,
                                 int num_channels);
int stbir_resize_uint8_srgb(const unsigned char *input_pixels , int input_w , int input_h , int input_stride_in_bytes,
                                 unsigned char *output_pixels, int output_w, int output_h, int output_stride_in_bytes,
                                 int num_channels);
int stbir_resize_uint8_srgb_edgemode(const unsigned char *input_pixels , int input_w , int input_h , int input_stride_in_bytes,
                                          unsigned char *output_pixels, int output_w, int output_h, int output_stride_in_bytes,
                                          int num_channels, int alpha_channel, int flags);

// The following functions use the specified resampling filter.

int stbir_resize_uint8_generic( const unsigned char *input_pixels , int input_w , int input_h , int input_stride_in_bytes,
                                     unsigned char *output_pixels, int output_w, int output_h, int output_stride_in_bytes,
                                     int num_channels, float s0, float t0, float s1, float t1, float *transform,
                                     int edge_wrap_mode);
int stbir_resize_float_generic( const float *input_pixels , int input_w , int input_h , int input_stride_in_bytes,
                                     float *output_pixels, int output_w, int output_h, int output_stride_in_bytes,
                                     int num_channels, float s0, float t0, float s1, float t1, float *transform,
                                     int edge_wrap_mode);

#ifdef __cplusplus
}
#endif

#endif // STB_IMAGE_RESIZE_H

#ifdef STB_IMAGE_RESIZE_IMPLEMENTATION

#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifdef _MSC_VER
#define STBIR__NOTUSED(v)  (void)(v)
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#else
#define STBIR__NOTUSED(v)  (void)sizeof(v)
#endif

#define STBIR_ASSERT(x)    STBIR__NOTUSED(x)

#define STBIR_MAX_CHANNELS 64

typedef struct
{
   int width, height, stride_in_bytes;
   int channels;
   float *pixels;
} stbir__info;

static void stbir__calculate_transform(float s0, float t0, float s1, float t1, float *transform)
{
   if (transform) {
      float scale_x = (s1 - s0) / 2;
      float scale_y = (t1 - t0) / 2;
      float translate_x = (s1 + s0) / 2;
      float translate_y = (t1 + t0) / 2;
      transform[0] = scale_x;
      transform[1] = 0;
      transform[2] = translate_x;
      transform[3] = 0;
      transform[4] = scale_y;
      transform[5] = translate_y;
      transform[6] = 0;
      transform[7] = 0;
      transform[8] = 1;
   }
}

static void stbir__calculate_coefficients(float scale, float filter_scale, float support, float *coefficients)
{
   float total = 0;
   int i;
   for (i = 0; i < (int)ceilf(support * 2 + 1); i++) {
      float x = (i - support) * filter_scale;
      float t = x * x;
      float r = ((2.0f - 1.5f * t - 0.25f * t * t) * (float)M_PI + (-3.0f + 2.0f * t + 0.25f * t * t) * (float)M_PI * (float)M_PI) / ((float)M_PI * (float)M_PI);
      coefficients[i] = r;
      total += r;
   }
   for (i = 0; i < (int)ceilf(support * 2 + 1); i++)
      coefficients[i] /= total;
}

static void stbir__resize_vertical_float(float *output, float *input, int output_h, int input_h, int input_w, int channels, float *coefficients, int filter_support)
{
   int y, x, c;
   for (y = 0; y < output_h; y++) {
      for (x = 0; x < input_w; x++) {
         for (c = 0; c < channels; c++) {
            float sum = 0;
            int i;
            for (i = 0; i < filter_support * 2 + 1; i++) {
               int input_y = y - filter_support + i;
               if (input_y < 0) input_y = 0;
               if (input_y >= input_h) input_y = input_h - 1;
               sum += input[input_y * input_w * channels + x * channels + c] * coefficients[i];
            }
            output[y * input_w * channels + x * channels + c] = sum;
         }
      }
   }
}

static void stbir__resize_horizontal_float(float *output, float *input, int output_h, int output_w, int height, int channels, float *coefficients, int filter_support)
{
   int y, x, c;
   for (y = 0; y < output_h; y++) {
      for (x = 0; x < output_w; x++) {
         for (c = 0; c < channels; c++) {
            float sum = 0;
            int i;
            for (i = 0; i < filter_support * 2 + 1; i++) {
               int input_x = x - filter_support + i;
               if (input_x < 0) input_x = 0;
               if (input_x >= height) input_x = height - 1;
               sum += input[y * height * channels + input_x * channels + c] * coefficients[i];
            }
            output[y * output_w * channels + x * channels + c] = sum;
         }
      }
   }
}

int stbir_resize_float(const float *input_pixels, int input_w, int input_h, int input_stride_in_bytes,
                       float *output_pixels, int output_w, int output_h, int output_stride_in_bytes,
                       int num_channels)
{
   float *temp = NULL;
   float *coefficients = NULL;
   int filter_support = 2;
   float filter_scale = 1.0f;
   float support = filter_support * filter_scale;
   int temp_stride = input_w * num_channels;
   int temp_size = temp_stride * output_h;
   int coefficients_size = (int)ceilf(support * 2 + 1) * sizeof(float);

   if (input_w == output_w && input_h == output_h) {
      memcpy(output_pixels, input_pixels, input_w * input_h * num_channels * sizeof(float));
      return 1;
   }

   temp = (float *)malloc(temp_size * sizeof(float));
   if (!temp) return 0;

   coefficients = (float *)malloc(coefficients_size);
   if (!coefficients) {
      free(temp);
      return 0;
   }

   stbir__calculate_coefficients(1.0f, filter_scale, support, coefficients);

   stbir__resize_vertical_float(temp, (float *)input_pixels, output_h, input_h, input_w, num_channels, coefficients, filter_support);
   stbir__resize_horizontal_float(output_pixels, temp, output_h, output_w, input_w, num_channels, coefficients, filter_support);

   free(temp);
   free(coefficients);
   return 1;
}

int stbir_resize_uint8(const unsigned char *input_pixels, int input_w, int input_h, int input_stride_in_bytes,
                       unsigned char *output_pixels, int output_w, int output_h, int output_stride_in_bytes,
                       int num_channels)
{
   float *float_input = NULL;
   float *float_output = NULL;
   int float_input_size = input_w * input_h * num_channels;
   int float_output_size = output_w * output_h * num_channels;
   int i;

   float_input = (float *)malloc(float_input_size * sizeof(float));
   if (!float_input) return 0;

   float_output = (float *)malloc(float_output_size * sizeof(float));
   if (!float_output) {
      free(float_input);
      return 0;
   }

   for (i = 0; i < float_input_size; i++)
      float_input[i] = input_pixels[i] / 255.0f;

   if (!stbir_resize_float(float_input, input_w, input_h, input_w * num_channels * sizeof(float),
                          float_output, output_w, output_h, output_w * num_channels * sizeof(float),
                          num_channels)) {
      free(float_input);
      free(float_output);
      return 0;
   }

   for (i = 0; i < float_output_size; i++)
      output_pixels[i] = (unsigned char)(float_output[i] * 255.0f + 0.5f);

   free(float_input);
   free(float_output);
   return 1;
}

int stbir_resize_uint8_srgb(const unsigned char *input_pixels, int input_w, int input_h, int input_stride_in_bytes,
                            unsigned char *output_pixels, int output_w, int output_h, int output_stride_in_bytes,
                            int num_channels)
{
   return stbir_resize_uint8(input_pixels, input_w, input_h, input_stride_in_bytes,
                            output_pixels, output_w, output_h, output_stride_in_bytes,
                            num_channels);
}

int stbir_resize_uint8_srgb_edgemode(const unsigned char *input_pixels, int input_w, int input_h, int input_stride_in_bytes,
                                     unsigned char *output_pixels, int output_w, int output_h, int output_stride_in_bytes,
                                     int num_channels, int alpha_channel, int flags)
{
   STBIR_ASSERT(flags == 0);
   STBIR_ASSERT(alpha_channel == -1);
   return stbir_resize_uint8_srgb(input_pixels, input_w, input_h, input_stride_in_bytes,
                                 output_pixels, output_w, output_h, output_stride_in_bytes,
                                 num_channels);
}

int stbir_resize_uint8_generic(const unsigned char *input_pixels, int input_w, int input_h, int input_stride_in_bytes,
                               unsigned char *output_pixels, int output_w, int output_h, int output_stride_in_bytes,
                               int num_channels, float s0, float t0, float s1, float t1, float *transform,
                               int edge_wrap_mode)
{
   STBIR_ASSERT(edge_wrap_mode == STBIR_EDGE_CLAMP);
   return stbir_resize_uint8(input_pixels, input_w, input_h, input_stride_in_bytes,
                            output_pixels, output_w, output_h, output_stride_in_bytes,
                            num_channels);
}

int stbir_resize_float_generic(const float *input_pixels, int input_w, int input_h, int input_stride_in_bytes,
                               float *output_pixels, int output_w, int output_h, int output_stride_in_bytes,
                               int num_channels, float s0, float t0, float s1, float t1, float *transform,
                               int edge_wrap_mode)
{
   STBIR_ASSERT(edge_wrap_mode == STBIR_EDGE_CLAMP);
   return stbir_resize_float(input_pixels, input_w, input_h, input_stride_in_bytes,
                            output_pixels, output_w, output_h, output_stride_in_bytes,
                            num_channels);
}

#endif // STB_IMAGE_RESIZE_IMPLEMENTATION 