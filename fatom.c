#include <errno.h>
#include <stdio.h>
#include <sys/file.h>
#include <sys/param.h>

int main(int argc, char* argv[]) {
  if (argc < 3) {
    printf("Usage\n");
    return 0;
  }

  char mode[2];
  mode[0] = argv[1][0];
  mode[1] = 0;

  FILE* f = fopen(argv[2], mode);
  const int fd = fileno(f);
  if (fd < 0) {
    return errno;
  }

  const int lock = mode == "r" ? LOCK_SH : LOCK_EX;
  if (flock(fd, lock)) {
    return errno;
  }

  FILE* file_read;
  FILE* file_write;
  if (mode == "r") {
    file_read = f;
    file_write = stdout;
  } else {
    file_read = stdin;
    file_write = f;
  }

  char buf[4096];
  size_t nread;
  while ((nread = fread(buf, 1, 4096, file_read)) > 0) {
    const size_t min = MIN(4096, nread);
    if (min != fwrite(buf, 1, min, file_write)) {
      return errno;
    }
  }

  if (flock(fd, LOCK_UN)) {
    return errno;
  }

  if (fclose(f)) {
    return errno;
  }

  return 0;
}
