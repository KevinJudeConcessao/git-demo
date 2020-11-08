#ifndef IMAGE_H_
#define IMAGE_H_

#include <ADT/Matrix.h>
#include <stdint.h>
#include <stdio.h>

#define ENOIMG    (1U << 0)
#define EIMGINVAL (1U << 1)

struct image_t {
  struct matrix_t red_channel;
  struct matrix_t green_channel;
  struct matrix_t blue_channel;

  uint16_t max_color;
};

typedef struct image_t *Image;

#define RED(IMAGE, X, Y)    ELEMENT(&((IMAGE)->red_channel), (X), (Y))
#define GREEN(IMAGE, X, Y)  ELEMENT(&((IMAGE)->green_channel), (X), (Y))
#define BLUE(IMAGE, X, Y)   ELEMENT(&((IMAGE)->blue_channel), (X), (Y))

#define DIMX(IMAGE)         ((IMAGE)->red_channel).rows
#define DIMY(IMAGE)         ((IMAGE)->red_channel).columns
#define MAXCOLOR(IMAGE)     ((IMAGE)->max_color > UINT8_MAX ? UINT16_MAX : UINT8_MAX)

int  image_alloc(Image I, uint32_t dim_x, uint32_t dim_y, uint16_t max_color);
int  image_init(Image I, FILE *ImageFile);
int  image_dump(Image I, FILE *TargetFile);
void image_free(Image I);

_Bool image_equals(Image I, Image J);
_Bool image_not_equals(Image I, Image J);

#endif // IMAGE_H_