#include <ADT/Matrix.h>

#include <stdlib.h>

int matrix_init(Matrix M, uint32_t rows, uint32_t columns) {
  uint32_t i = 0;
  uint32_t j = 0;

  M->rows = rows;
  M->columns = columns;
  M->elements = rows ? calloc(rows, sizeof(float *)) : NULL;

  if (!M->elements)
    cleanup_and_return(EALLOC, free(M));

  for (i = 0; i < rows; ++i) {
    M->elements[i] = calloc(columns, sizeof(float));
    if (!M->elements) {
      for (j = 0; j < i; ++j)
        free(M->elements[j]);
    }

    cleanup_and_return(EALLOC, free(M->elements), free(M));
  }

  return SUCCESS;
}

void matrix_free(Matrix M) {
  int i;

  for (i = 0; i < ROWS(M); ++i)
    free(M->elements[i]);

  free(M->elements);
}

int add(Matrix P, Matrix A, Matrix B) {
  int i, j;

  if ((ROWS(A) != ROWS(B)) || (COLUMNS(A) != COLUMNS(B)))
    return ECOMPATADD;

  matrix_free(P);
  matrix_init(P, ROWS(A), COLUMNS(A));

  for (i = 0; i < ROWS(P); ++i) {
    for (j = 0; j < COLUMNS(P); ++j)
      ELEMENT(P, i, j) = ELEMENT(A, i, j) + ELEMENT(B, i, j);
  }

  return SUCCESS;
}

int sub(Matrix P, Matrix A, Matrix B) {
  int i, j;

  if ((ROWS(A) != ROWS(B)) || (COLUMNS(A) != COLUMNS(B)))
    return ECOMPATADD;

  matrix_free(P);
  matrix_init(P, ROWS(A), COLUMNS(A));

  for (i = 0; i < ROWS(P); ++i) {
    for (j = 0; j < COLUMNS(P); ++j)
      ELEMENT(P, i, j) = ELEMENT(A, i, j) - ELEMENT(B, i, j);
  }

  return SUCCESS;
}

int multiply(Matrix P, Matrix A, Matrix B) {
  int i, j, k;

  if (COLUMNS(A) != ROWS(B))
    return ECOMPATMUL;

  for (i = 0; i < ROWS(A); ++i) {
    for (j = 0; j < COLUMNS(B); ++j) {
      ELEMENT(P, i, j) = 0;
      for (k = 0; k < COLUMNS(A); ++k)
        ELEMENT(P, i, j) += ELEMENT(A, i, k) * ELEMENT(B, k, j);
    }
  }

  return SUCCESS;
}

/*
 * [Deprecated] Should not be here !!
 */
int scale(Matrix A, float rx, float gx, float bx) {
  int i, j;

  typedef float pixel_t[3];
  pixel_t *pixel;
  pixel_t *end;
  enum { R, G, B };

  if (ROWS(A) % 3 != 0)
    return ENOIMAGE;

  for (i = 0; i < ROWS(A); ++i) {
    pixel = (pixel_t *)(ELEMENTS(A)[i]);
    end = (pixel_t *)(ELEMENTS(A)[i] + COLUMNS(A));

    while (pixel < end) {
      (*pixel)[R] = (*pixel)[R] * rx;
      (*pixel)[G] = (*pixel)[G] * gx;
      (*pixel)[B] = (*pixel)[B] * bx;

      ++pixel;
    }
  }

  return SUCCESS;
}

int convolution(Matrix P, Matrix A, Matrix kernel) {
  struct matrix_t window;
  int i, j;
  int ki, kj; // kernel index variable
  int mi, mj; // location of the central variable in the kernel
  float sum;

  if (ROWS(kernel) != COLUMNS(kernel) || ROWS(kernel) % 2 != 0 ||
      ROWS(kernel) > ROWS(A) || COLUMNS(kernel) > COLUMNS(A))
    return ECOMPATCONV;

  matrix_init(&window, ROWS(kernel), COLUMNS(kernel));
  matrix_free(P);
  matrix_init(P, ROWS(A), COLUMNS(A));

  mi = mj = (ROWS(P) >> 1) + 1;

#define GET(MATRIX, I, J)                                                      \
  ((I) < 0 || (I) > (ROWS(MATRIX) - 1) || (J) < 0 ||                           \
   (J) > (COLUMNS(MATRIX) - 1))                                                \
      ? 0                                                                      \
      : (ELEMENT(MATRIX, I, J))

  for (i = 0; i < ROWS(P); ++i) {
    for (j = 0; j < COLUMNS(P); ++j) {

      for (ki = 0; ki < ROWS(kernel); ++ki) {
        for (kj = 0; kj < COLUMNS(kernel); ++kj) {
          ELEMENT(P, i, j) += ELEMENT(kernel, ki, kj) * GET(A, i + ki - mi, j + kj - mj);
        }
      }        
    }
  }

  return SUCCESS;
}

_Bool equals(Matrix A, Matrix B) {
  int i, j;

  if (!(ROWS(A) == ROWS(B) && COLUMNS(A) == COLUMNS(B)))
    return 0;
  
  for (i = 0; i < ROWS(A); ++i) {
    for (j = 0; j < COLUMNS(A); ++j) {
      if (ELEMENT(A, i, j) != ELEMENT(B, i, j))
        return 0;
    }
  }
  return 1;
}

_Bool not_equals(Matrix A, Matrix B) {
  return ! equals(A, B);
}