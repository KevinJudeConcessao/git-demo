#ifndef MATRIX_H_
#define MATRIX_H_

#include <stdint.h>

#define cleanup_and_return(value, ...)                                         \
  do {                                                                         \
    (void)(__VA_ARGS__, 0);                                                    \
    return value;                                                              \
  } while (0)

#define cleanup_and_return_void(...)                                           \
  do {                                                                         \
    (void)(__VA_ARGS__, 0);                                                    \
    return ;                                                                   \
  } while (0)

struct matrix_t {
  uint32_t rows;
  uint32_t columns;

  float **elements;
};

typedef struct matrix_t *Matrix;

#define ELEMENT(MATRIX, X, Y) ((MATRIX)->elements[(X)][(Y)])
#define ROWS(MATRIX)          ((MATRIX)->rows)
#define COLUMNS(MATRIX)       ((MATRIX)->columns)
#define ELEMENTS(MATRIX)      ((MATRIX)->elements)

#define SUCCESS     (0)
#define EALLOC      (1U << 0)
#define ECOMPATADD  (1U << 1)
#define ECOMPATMUL  (1U << 2)
#define ENOIMAGE    (1U << 3)
#define ECOMPATCONV (1U << 4)

int  matrix_init(Matrix M, uint32_t rows, uint32_t columns);
void matrix_free(Matrix M);

int add(Matrix P, Matrix A, Matrix B);
int subtract(Matrix P, Matrix A, Matrix B);
int multiply(Matrix P, Matrix A, Matrix B);

int convolution(Matrix P, Matrix A, Matrix kernel);

_Bool equals(Matrix A, Matrix B);
_Bool not_equals(Matrix A, Matrix B);

/* 
 * [Deprecated] Should not be here !! 
 */
int scale(Matrix A, float rx, float gx, float bx);

#endif