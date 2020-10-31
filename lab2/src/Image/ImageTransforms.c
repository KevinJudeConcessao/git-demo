#include <ADT/Matrix.h>
#include <Image/ImageTransforms.h>

int (*T1)(Image Dst, Image Src) = emboss_image;
int (*T2)(Image Dst, Image Src) = edge_detection;

int (*T3)(Image Dst, Image Src) = identity3x3;
int (*T4)(Image Dst, Image Src) = identity5x5;

static struct matrix_t emboss_kernel;
static struct matrix_t edge_kernel;

static struct matrix_t identity3x3_kernel;
static struct matrix_t identity5x5_kernel;

static __attribute__((constructor)) void construct_kernels() {
    static float __emboss_kernel[5][5] = {
    { 0,  0,  0,  0,  0 },
    { 0,  0, -1,  0,  0 },
    { 0, -1,  5, -1,  0 },
    { 0,  0, -1,  0,  0 },
    { 0,  0,  0,  0,  0 }, 
  };

  static float __edge_kernel[3][3] = {
    { 0,  1, 0 },
    { 1, -4, 1 },
    { 0,  1, 0 },
  };

  static float __identity3x3_kernel[3][3] = {
    { 0, 0, 0 },
    { 0, 1, 0 },
    { 0, 0, 0 }
  };

  static float __identity5x5_kernel[5][5] = {
    { 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0 },
    { 0, 0, 1, 0, 0 },
    { 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0 },
  };

  static float *__emboss_kernel_vec[] = {
    __emboss_kernel[0],
    __emboss_kernel[1],
    __emboss_kernel[2],
    __emboss_kernel[3],
    __emboss_kernel[4]
  };

  static float *__edge_kernel_vec[] = {
    __edge_kernel[0],
    __edge_kernel[1],
    __edge_kernel[2]
  };

  static float *__identity3x3_kernel_vec[] = {
    __identity3x3_kernel[0],
    __identity3x3_kernel[1],
    __identity3x3_kernel[2]
  };

    static float *__identity5x5_kernel_vec[] = {
    __identity5x5_kernel[0],
    __identity5x5_kernel[1],
    __identity5x5_kernel[2],
    __identity5x5_kernel[3],
    __identity5x5_kernel[4]
  };

  ROWS(&emboss_kernel)      = 5;
  COLUMNS(&emboss_kernel)   = 5;
  ELEMENTS(&emboss_kernel)  = __emboss_kernel_vec;

  ROWS(&edge_kernel)        = 3;
  COLUMNS(&edge_kernel)     = 3;
  ELEMENTS(&edge_kernel)    = __edge_kernel_vec;  

  ROWS(&identity3x3_kernel)        = 3;
  COLUMNS(&identity3x3_kernel)     = 3;
  ELEMENTS(&identity3x3_kernel)    = __identity3x3_kernel_vec;

  ROWS(&identity5x5_kernel)        = 5;
  COLUMNS(&identity5x5_kernel)     = 5;
  ELEMENTS(&identity5x5_kernel)    = __identity5x5_kernel_vec;
}

int emboss_image(Image Dst, Image Src) {
  Dst->max_color = Src->max_color;
  return convolution(&Dst->red_channel, &Src->red_channel, &emboss_kernel)      |
         convolution(&Dst->green_channel, &Src->green_channel, &emboss_kernel)  |
         convolution(&Dst->blue_channel, &Src->blue_channel, &emboss_kernel);
}

int edge_detection(Image Dst, Image Src) {
  Dst->max_color = Src->max_color;
  return convolution(&Dst->red_channel, &Src->red_channel, &edge_kernel)      |
         convolution(&Dst->green_channel, &Src->green_channel, &edge_kernel)  |
         convolution(&Dst->blue_channel, &Src->blue_channel, &edge_kernel);
}

int identity3x3(Image Dst, Image Src) {
  Dst->max_color = Src->max_color;
  return convolution(&Dst->red_channel, &Src->red_channel, &identity3x3_kernel)     |
         convolution(&Dst->green_channel, &Src->green_channel, &identity3x3_kernel) |
         convolution(&Dst->blue_channel, &Src->blue_channel, &identity3x3_kernel);
}

int identity5x5(Image Dst, Image Src) {
  Dst->max_color = Src->max_color;
  return convolution(&Dst->red_channel, &Src->red_channel, &identity5x5_kernel)     |
         convolution(&Dst->green_channel, &Src->green_channel, &identity5x5_kernel) |
         convolution(&Dst->blue_channel, &Src->blue_channel, &identity5x5_kernel);
}