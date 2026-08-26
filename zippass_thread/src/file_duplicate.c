// Copyright [2023] <Bryan Ulate>

# include "file_duplicate.h"

int create_temporary_dir(char* path) {
  struct stat st = {0};
  if (stat(path, &st) == -1) {
    mkdir(path, 0700);
    return EXIT_SUCCESS;
  }
  return EXIT_FAILURE;
}

int create_temporary_file(char* path_original_file, char* tmp_file_path) {
  int error = EXIT_SUCCESS;

  FILE* original;
  FILE* temporal;

  original = fopen(path_original_file, "rb");
  temporal = fopen(tmp_file_path, "wb");

  // Tomado de: https://stackoverflow.com/a/5263102/19025248
  size_t n, m;
  unsigned char buff[8192];
  do {
    n = fread(buff, 1, sizeof buff, original);
    if (n) m = fwrite(buff, 1, n, temporal);
    else   m = 0;
  } while ((n > 0) && (n == m));
  if (m) perror("copy");

  fclose(temporal);
  fclose(original);

  return error;
}

int remove_temporary_dir(char* path) {
  struct stat st = {0};
  if (stat(path, &st) != -1) {
    return rmdir(path);
  }
  return EXIT_FAILURE;
}

int remove_temporary_file(char* tmp_file_path) {
  return remove(tmp_file_path);
}
