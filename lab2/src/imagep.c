#include <Image/Image.h>
#include <Image/ImageTransforms.h>
#include <ADT/Matrix.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define PRINT(S) printf(S "\n")

static void usage() {
  PRINT("Usage: ./imagep [OPTIONS]...");
  PRINT("Options:");
  PRINT("   -i PATH         Path of the input image file. Default is stdin");
  PRINT("   -o PATH         Path to the transformed image file. Default is stdout");
  PRINT("   -t1             Run transform T1 on input image file.");
  PRINT("   -t2             Run transform T2 on input image file.");
  PRINT("   -tall           Run transform T1 followed by transform T2 on input");
  PRINT("                   image file.");
  PRINT("   -h              Print this help."); 
}

int main(int argc, char *argv[]) {
  char *input_path  = NULL;
  char *output_path = NULL;

  _Bool run_T1 = 0;
  _Bool run_T2 = 0;

  FILE *input;
  FILE *output;

  struct image_t input_image;
  struct image_t intermediate;
  struct image_t transformed_image;
  int status = 1;

  char **ptr = &argv[1];

  if (argc == 1) {
    usage();
    exit(2);
  }

  while (*ptr) {
    if (! strcmp(*ptr, "-i")) {
      ++ ptr; 
      input_path = *ptr;
    } else if (! strcmp(*ptr, "-o")) {
      ++ ptr;
      output_path = *ptr;
    } else if (! strcmp(*ptr, "-t1")) {
      run_T1 = 1;
    } else if (! strcmp(*ptr, "-t2")) {
      run_T2 = 1;
    } else if (! strcmp(*ptr, "-tall")) {
      run_T1 = run_T2 = 1;
    } else if (! strcmp(*ptr, "-h")) {
      usage();
      exit(2);
    } else {
      fprintf(stderr, "Invalid Argument(s)\n");
      usage();
      exit(2);
    }

    ++ ptr;
  }

  input = input_path  ? fopen(input_path, "rb")  : stdin;
  if (!input) {
    fprintf(stderr, "Cannot open file %s", input_path);
    return EXIT_FAILURE;
  }

  output = output_path ? fopen(output_path, "wb") : stdout; 
  if (!output) {
    fprintf(stderr, "Cannot open file %s", output_path);
    return EXIT_FAILURE;
  }

  image_init(&input_image, input);
  image_init(&intermediate, NULL);
  image_init(&transformed_image, NULL);

  if (run_T1 && run_T2) {
    status = T1(&intermediate, &input_image);
    if (!status)
      status = T2(&transformed_image, &intermediate);
  } else if (run_T1) {
    status = T1(&transformed_image, &input_image);
  } else if (run_T2) {
    status = T2(&transformed_image, &input_image);
  }

  if (!status)
    image_dump(&transformed_image, output);

  image_free(&input_image);
  image_free(&intermediate);
  image_free(&transformed_image);

  fclose(input);
  fclose(output);

  return status;
}