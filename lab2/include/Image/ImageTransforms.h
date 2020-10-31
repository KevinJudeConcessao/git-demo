#ifndef IMAGE_TRANSFORMS_H_
#define IMAGE_TRANSFORMS_H_

#include <Image/Image.h>

int emboss_image(Image Dst, Image Src);
int edge_detection(Image Dst, Image Src);

int identity3x3(Image Dst, Image Src);
int identity5x5(Image Dst, Image Src);

/*
 * [DEPRECATED]
 */

extern int (*T1)(Image Dst, Image Src);
extern int (*T2)(Image Dst, Image Src);

extern int (*T3)(Image Dst, Image Src);
extern int (*T4)(Image Dst, Image Src);

#endif // IMAGE_TRANSFORMS_H_