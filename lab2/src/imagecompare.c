#include <Image/Image.h>
#include <Image/ImageTransforms.h>
#include <ADT/Matrix.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>


int main(int argc, char *argv[]) {
  char *first_image_path  = NULL;
  char *second_image_path = NULL;

  FILE *first_image_file;
  FILE *second_image_file;

  struct image_t first_image;
  struct image_t second_image;

  int status;

  if (argc == 1) {
    fprintf(stderr, "Please provide two filepaths !!");
    exit(2);
  }

  first_image_file = fopen(first_image_path, "rb") ;
  if (!first_image_file) {
    fprintf(stderr, "Cannot open file %s", first_image_path);
    return EXIT_FAILURE;
  }

  second_image_file = fopen(second_image_path, "rb"); 
  if (!second_image_file) {
    fprintf(stderr, "Cannot open file %s", second_image_path);
    return EXIT_FAILURE;
  }

  image_init(&first_image, first_image_file);
  image_init(&second_image, second_image_file);

  status = image_not_equals(&first_image, &second_image);

  image_free(&first_image);
  image_free(&second_image);

  fclose(first_image_file);
  fclose(second_image_file);

  return status;
}