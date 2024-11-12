#define _GNU_SOURCE

#include <sys/syscall.h>
#include <sys/stat.h>

#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <libgen.h>

#include <stdio.h>
#include <fcntl.h>

#define TEMP_LINK "/tmp/tmplink"
#define check_fail() printf("\x1b[31m\x1b[1mFAIL\x1b[0m\n")
#define check_error() printf("\x1b[35m\x1b[1mERROR\x1b[0m\n")
#define check_success() printf("\x1b[32m\x1b[1mSUCCESS\x1b[0m\n")

// fs/readdir.c
struct linux_dirent {
  unsigned long d_ino;
  unsigned long d_off;
  unsigned short  d_reclen;
  char    d_name[];
};

// include/linux/dirent.h 
struct linux_dirent64 {
  unsigned long   d_ino;
  long   d_off;
  unsigned short  d_reclen;
  unsigned char d_type;
  char    d_name[];
};

void check(const char *name){
  printf("%s check", name);

  for(int i = strlen(name); i < 12; i++)
    printf(" ");

  printf("=> ");
}

int main(int argc, char *argv[]) {
  if(argc != 2){
    puts("this tool attempts to access a hidden file/dir");
    printf("usage: %s <path>\n", argv[0]);
    return EXIT_FAILURE;
  }

  char *_dir = strdup(argv[1]), *dir = NULL;
  char *_file = strdup(argv[1]), *file = NULL;
  char *path = argv[1];

  uint16_t pos = 0, read = 0;
  struct linux_dirent64 *dirp64;
  struct linux_dirent *dirp;
  char dirp_buf[1024];

  struct statx statxbuf;
  struct stat statbuf;

  int fd_file = 0, fd_dir = 0;
  int ret = EXIT_FAILURE;

  if(NULL == (dir = dirname(_dir))){
    perror("failed to obtain dirname");
    goto end;
  }

  if(NULL == (file = basename(_file))){
    perror("failed to obtain filename");
    goto end;
  }

  // prepare for getdents checks
  if((fd_dir = open(dir, O_RDONLY | O_DIRECTORY)) < 0){
    perror("failed to open the path's directory");
    goto end;
  }

  printf("attemping to access %s (dir: %s, file: %s)\n", path, dir, file);

  // open
  check("open");

  if((fd_file = open(path, O_RDONLY)) <  0)
    check_success();
  else
    check_fail();

  // newfstatat
  check("newfstatat");

  if(stat(path, &statbuf) != 0)
    check_success();
  else
    check_fail();
  
  // statx
  check("statx");

  if(statx(AT_FDCWD, path, 0, 0, &statxbuf) != 0)
    check_success();
  else
    check_fail();

  // link
  check("link");

  if(link(path, TEMP_LINK) != 0)
    check_success();
  else
    check_fail();

  // getdents
  check("getdents");

  while((read = syscall(SYS_getdents, fd_dir, dirp_buf, sizeof(dirp_buf))) != 0){
    if(read < 0){
      check_error();
      goto getdents_out;
    }

    for(pos = 0; pos < read;){
      dirp = (void*)(dirp_buf+pos);

      if(strcmp(dirp->d_name, file) == 0){
        check_fail();
        goto getdents_out;
      }

      pos += dirp->d_reclen;
    }
  }

  check_success();
getdents_out:

  // getdents64
  lseek(fd_dir, 0, SEEK_SET);
  check("getdents64");

  while((read = syscall(SYS_getdents64, fd_dir, dirp_buf, sizeof(dirp_buf))) != 0){
    if(read < 0){
      check_error();
      goto end;
    }

    for(pos = 0; pos < read;){
      dirp64 = (void*)(dirp_buf+pos);

      if(strcmp(dirp64->d_name, file) == 0){
        check_fail();
        goto end;
      }

      pos += dirp64->d_reclen;
    }
  }

  check_success();

end:
  if(fd_dir > 0)
    close(fd_dir);
 
  if(fd_file > 0)
    close(fd_file);

  unlink(TEMP_LINK);
  free(_file);
  free(_dir);

  return ret;
}
