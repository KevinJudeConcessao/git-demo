#include "Image/Image.h"
#include <ADT/Matrix.h>
#include <Image/ImageTransforms.h>

#include <stddef.h>
#include <math.h>

int (*T1)(Image Dst, Image Src) = grayscale;
int (*T2)(Image Dst, Image Src) = edge_detection;

int (*T3)(Image Dst, Image Src) = identity3x3;
int (*T4)(Image Dst, Image Src) = identity5x5;

static struct matrix_t sharpen_kernel;
static struct matrix_t edge_kernel_Gx;
static struct matrix_t edge_kernel_Gy;

static struct matrix_t identity3x3_kernel;
static struct matrix_t identity5x5_kernel;

static struct matrix_t edge_detection_kernel;

static __attribute__((constructor)) void construct_kernels() {
  static float __sharpen_kernel[3][3] = {
    { 0,    -0.2,  0  },
    { -0.2,    1, -0.2 },
    { 0,    -0.2,   0  },
  };
/*
  static float __edge_kernel_Gx[3][3] = {
    { -1, 0, 1 },
    { -2, 0, 2 },
    { -1, 0, 1 },
  };

  static float __edge_kernel_Gy[3][3] = {
    {  1,  2,  1 },
    {  0,  0,  0 },
    { -1, -2, -1 },
  };
*/

  static float __edge_kernel_Gx[3][3] = {
    { 0, 1, -1 },
    { 0, 0, 0 },
    { 0, 0, 0 },
  };

  static float __edge_kernel_Gy[3][3] = {
    { -1,  0,  0 },
    {  1,  0,  0 },
    { 0, 0, 0 },
  };

  static float __identity3x3_kernel[3][3] = {
    { 0, 0, 0 },
    { 0, 1, 0 },
    { 0, 0, 0 }
  };

  static float __identity5x5_kernel[5][5] = {
    { 0, 0, 0,  0, 0 },
    { 0, 0, 0,  0, 0 },
    { 0, 0, 1, 0, 0 },
    { 0, 0, 0,  0, 0 },
    { 0, 0, 0,  0, 0 },
  };

  static float __edge_detection_kernel[5][5] = {
    { -0.01, -0.9, -0.9,  -0.9, -0.01 },
    { -0.01, -0.9, -0.9,  -0.9, -0.01 },
    { -0.01, -0.9,  13.5, -0.9, -0.01 },
    { -0.01, -0.9, -0.9,  -0.9, -0.01 },
    { -0.01, -0.9, -0.9,  -0.9, -0.01 },
  };

  static float *__sharpen_kernel_vec[] = {
    __sharpen_kernel[0],
    __sharpen_kernel[1],
    __sharpen_kernel[2],
  };

  static float *__edge_kernel_Gx_vec[] = {
    __edge_kernel_Gx[0],
    __edge_kernel_Gx[1],
    __edge_kernel_Gx[2]
  };

  static float *__edge_kernel_Gy_vec[] = {
    __edge_kernel_Gy[0],
    __edge_kernel_Gy[1],
    __edge_kernel_Gy[2]
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

  static float *__edge_detection_kernel_vec[] = {
    __edge_detection_kernel[0],
    __edge_detection_kernel[1],
    __edge_detection_kernel[2],
    __edge_detection_kernel[3],
    __edge_detection_kernel[4]
  };

  ROWS(&sharpen_kernel)      = 3;
  COLUMNS(&sharpen_kernel)   = 3;
  ELEMENTS(&sharpen_kernel)  = __sharpen_kernel_vec;

  ROWS(&edge_kernel_Gx)     = 3;
  COLUMNS(&edge_kernel_Gx)  = 3;
  ELEMENTS(&edge_kernel_Gx) = __edge_kernel_Gx_vec;  

  ROWS(&edge_kernel_Gy)     = 3;
  COLUMNS(&edge_kernel_Gy)  = 3;
  ELEMENTS(&edge_kernel_Gy) = __edge_kernel_Gy_vec; 

  ROWS(&identity3x3_kernel)        = 3;
  COLUMNS(&identity3x3_kernel)     = 3;
  ELEMENTS(&identity3x3_kernel)    = __identity3x3_kernel_vec;

  ROWS(&identity5x5_kernel)        = 5;
  COLUMNS(&identity5x5_kernel)     = 5;
  ELEMENTS(&identity5x5_kernel)    = __identity5x5_kernel_vec;

  ROWS(&edge_detection_kernel)        = 5;
  COLUMNS(&edge_detection_kernel)     = 5;
  ELEMENTS(&edge_detection_kernel)    = __edge_detection_kernel_vec;
}

int sharpen(Image Dst, Image Src) {
  Dst->max_color = Src->max_color;
  convolution(&Dst->red_channel, &Src->red_channel, &sharpen_kernel) ;
         convolution(&Dst->green_channel, &Src->green_channel, &sharpen_kernel);
         convolution(&Dst->blue_channel, &Src->blue_channel, &sharpen_kernel);
         return 0;
}

int edge_detection_old(Image Dst, Image Src) {
  int status = 0;
  int i, j;
  struct image_t X, Y;

  image_init(&X, NULL);
  image_init(&Y, NULL);

  status =  
    convolution(&X.red_channel, &Src->red_channel, &edge_kernel_Gx)      |
    convolution(&X.green_channel, &Src->green_channel, &edge_kernel_Gx)  |
    convolution(&X.blue_channel, &Src->blue_channel, &edge_kernel_Gx);

  if (!!status)
    return status;

  status =  
    convolution(&Y.red_channel, &Src->red_channel, &edge_kernel_Gy)      |
    convolution(&Y.green_channel, &Src->green_channel, &edge_kernel_Gy)  |
    convolution(&Y.blue_channel, &Src->blue_channel, &edge_kernel_Gy);

  if (!!status)
    return status;

  image_free(Dst);
  image_alloc(Dst, DIMX(Src), DIMY(Src), MAXCOLOR(Src));

  for (i = 0; i < DIMX(Src); ++i) {
    for (j = 0; j < DIMY(Src); ++j) {
      RED(Dst, i, j) = sqrt(pow(RED(&X, i, j), 2) + pow(RED(&Y, i, j), 2));
      GREEN(Dst, i, j) = sqrt(pow(GREEN(&X, i, j), 2) + pow(GREEN(&Y, i, j), 2));
      BLUE(Dst, i, j) = sqrt(pow(BLUE(&X, i, j), 2) + pow(BLUE(&Y, i, j), 2));

    }
  }

  image_free(&X);
  image_free(&Y);

  return SUCCESS;
}

int identity3x3(Image Dst, Image Src) {
  Dst->max_color = Src->max_color;
  return convolution(&Dst->red_channel, &Src->red_channel, &identity3x3_kernel)     |
         convolution(&Dst->green_channel, &Src->green_channel, &identity3x3_kernel) |
         convolution(&Dst->blue_channel, &Src->blue_channel, &identity3x3_kernel);
}

int edge_detection(Image Dst, Image Src) {
  Dst->max_color = Src->max_color;
  return convolution(&Dst->red_channel, &Src->red_channel, &edge_detection_kernel)     |
         convolution(&Dst->green_channel, &Src->green_channel, &edge_detection_kernel) |
         convolution(&Dst->blue_channel, &Src->blue_channel, &edge_detection_kernel);
}

int identity5x5(Image Dst, Image Src) {
  Dst->max_color = Src->max_color;
  return convolution(&Dst->red_channel, &Src->red_channel, &identity5x5_kernel)     |
         convolution(&Dst->green_channel, &Src->green_channel, &identity5x5_kernel) |
         convolution(&Dst->blue_channel, &Src->blue_channel, &identity5x5_kernel);
}


int grayscale(Image Dst, Image Src) {
  int i, j;

  float red, green, blue;

  scale_image(Dst, Src, 0.2126, 0.7152, 0.0722);

  for (i = 0; i < DIMX(Dst); ++i) {
    for (j = 0; j < DIMY(Dst); ++j) {
      red   = RED(Src, i, j);
      green = GREEN(Src, i, j);
      blue  = BLUE(Src, i, j);

      RED(Dst, i, j) = GREEN(Dst, i, j) = BLUE(Dst, i, j) = (red + green + blue)/3;
    }
  } 

  return SUCCESS;
}

int scale_image(Image Dst, Image Src, float rx, float gx, float bx) {
  int i, j;

  matrix_free(&Dst->red_channel);
  matrix_free(&Dst->green_channel);
  matrix_free(&Dst->blue_channel);

  matrix_init(&Dst->red_channel, DIMX(Src), DIMY(Src));
  matrix_init(&Dst->green_channel, DIMX(Src), DIMY(Src));
  matrix_init(&Dst->blue_channel, DIMX(Src), DIMY(Src));

  for (i = 0; i < DIMX(Src); ++i) {
    for (j = 0; j < DIMY(Src); ++j) {
      ELEMENT(&Dst->red_channel, i, j)    = rx * RED(Src, i, j);
      ELEMENT(&Dst->green_channel, i, j)  = gx * GREEN(Src, i, j);
      ELEMENT(&Dst->blue_channel, i, j)   = bx * BLUE(Src, i, j);
    }
  }
  
  return SUCCESS;
}