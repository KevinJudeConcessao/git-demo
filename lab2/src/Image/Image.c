#include <ADT/Matrix.h>
#include <Image/Image.h>

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

int image_alloc(Image I, uint32_t dim_x, uint32_t dim_y, uint16_t max_color) {
  int status = SUCCESS;

  status = matrix_init(&I->red_channel, dim_x, dim_y);
  if (!!status)
    return status;

  status = matrix_init(&I->green_channel, dim_x, dim_y);
  if (!!status)
    cleanup_and_return(status, matrix_free(&I->red_channel));

  status = matrix_init(&I->blue_channel, dim_x, dim_y);
  if (!!status)
    cleanup_and_return(status, matrix_free(&I->red_channel), matrix_free(&I->green_channel));

  I->max_color = max_color;

  return status;
}

int image_init(Image I, FILE *ImageFile) {
  char buffer[1024];
  uint32_t i, j;
  unsigned int rows;
  unsigned int columns;
  unsigned int max_value;

  uint8_t  pixel8[3];
  uint16_t pixel16[3];

  *I = (struct image_t) { 0 };
  if (!ImageFile)
    return SUCCESS;

  fscanf(ImageFile, "%s", buffer);
  if (strncmp((char *)buffer, "P6", 2))
    return ENOIMG;

  getc(ImageFile);

  do {
    if (!fgets(buffer, 1024, ImageFile))
      return ENOIMG;
  } while (!strncmp(buffer, "#", 1));

  if (sscanf(buffer, "%u %u", &rows, &columns) != 2)
    return ENOIMG;

  if (fscanf(ImageFile, "%u", &max_value) != 1)
    return ENOIMG;

  I->max_color = max_value > UINT8_MAX ? UINT16_MAX : UINT8_MAX;
  matrix_init(&I->red_channel, rows, columns);
  matrix_init(&I->green_channel, rows, columns);
  matrix_init(&I->blue_channel, rows, columns);

  fseek(ImageFile, 1, SEEK_CUR);

  if (I->max_color == UINT8_MAX) {
    for (i = 0; i < rows; ++i) {
      for (j = 0; j < columns; ++j) {
        fread(pixel8, sizeof(uint8_t), 3, ImageFile);

        RED(I, i, j)    = pixel8[0];
        GREEN(I, i, j)  = pixel8[1];
        BLUE(I, i, j)   = pixel8[2];
      }
    }
  } else if (I->max_color == UINT16_MAX) {
    for (i = 0; i < rows; ++i) {
      for (j = 0; j < columns; ++j) {
        fread(pixel16, sizeof(uint8_t), 3, ImageFile);

        RED(I, i, j)    = pixel16[0];
        GREEN(I, i, j)  = pixel16[1];
        BLUE(I, i, j)   = pixel16[2];
      }
    }
  }

  return SUCCESS;
}

int image_dump(Image I, FILE *TargetFile) {
  int i, j;
  uint8_t  pixel8[3];
  uint16_t pixel16[3];

  fprintf(TargetFile, "P6\n%d %d\n%d\n", DIMX(I), DIMY(I), MAXCOLOR(I));
  
  if (MAXCOLOR(I) == UINT8_MAX) {
    for (i = 0; i < DIMX(I); ++i) {
      for (j = 0; j < DIMY(I); ++j) {
        RED(I, i, j) = RED(I, i, j) > 255 ? 255 : RED(I, i, j);
        RED(I, i, j) = RED(I, i, j) < 0 ? 0     : RED(I, i, j);

        GREEN(I, i, j) = GREEN(I, i, j) > 255 ? 255 : GREEN(I, i, j);
        GREEN(I, i, j) = GREEN(I, i, j) < 0 ? 0     : GREEN(I, i, j);

        BLUE(I, i, j) = BLUE(I, i, j) > 255 ? 255 : BLUE(I, i, j);
        BLUE(I, i, j) = BLUE(I, i, j) < 0 ? 0     : BLUE(I, i, j);

        pixel8[0] = (uint8_t) (RED(I, i, j));
        pixel8[1] = (uint8_t) (GREEN(I, i, j));
        pixel8[2] = (uint8_t) (BLUE(I, i, j));

        fwrite(pixel8, sizeof(uint8_t), 3, TargetFile);        
      }
    }
  } else if (MAXCOLOR(I) == UINT16_MAX) {
    for (i = 0; i < DIMX(I); ++i) {
      for (j = 0; j < DIMY(I); ++j) {
        pixel16[0] = (uint16_t) (RED(I, i, j));
        pixel16[1] = (uint16_t) (GREEN(I, i, j));
        pixel16[2] = (uint16_t) (BLUE(I, i, j));

        fwrite(pixel16, sizeof(uint16_t), 3, TargetFile);
      }
    }
  }

  return SUCCESS;
}

void image_free(Image I) {
  matrix_free(&I->red_channel);
  matrix_free(&I->green_channel);
  matrix_free(&I->blue_channel);
}

_Bool image_equals(Image I, Image J) {
  return equals(&I->red_channel, &I->red_channel)     &&
         equals(&I->green_channel, &J->green_channel) &&
         equals(&I->blue_channel, &J->blue_channel);
}

_Bool image_not_equals(Image I, Image J) {
  return ! image_equals(I, J);
}