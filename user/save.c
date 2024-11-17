#include "inc/save.h"
#include "inc/util.h"

#include <sys/mman.h>
#include <sys/stat.h>

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <fcntl.h>
#include <stdio.h>

FILE *save_fp = NULL;

bool __save_contains_bad_char(char *s) {
  for (; *s != 0; s++)
    if (*s == '\n')
      return true;
  return false;
}

bool save_creat() {
  int save_fd = 0;

  if (access(SHRK_SAVE_FILE, F_OK) == 0)
    return true;

  if ((save_fd = creat(SHRK_SAVE_FILE, 0600)) < 0)
    return false;

  close(save_fd);
  return true;
}

bool save_open() {
  if (NULL != save_fp)
    return fseek(save_fp, 0, SEEK_SET) == 0;
  return NULL != (save_fp = fopen(SHRK_SAVE_FILE, "a+"));
}

char *save_get(char *name) {
  if (NULL == name)
    return NULL;

  if (NULL == save_fp && !save_open())
    return NULL;

  char   *line = NULL, *value = NULL;
  size_t  _temp_size = 0, name_size = strlen(name);
  ssize_t line_size = 0;

  while ((line_size = getline(&line, &_temp_size, save_fp)) > 0) {
    line[--line_size] = 0; // replace \n

    if (line_size <= name_size)
      goto next;

    if (strncmp(line, name, name_size) == 0)
      value = strdup(line + name_size);

  next:
    free(line);
    line       = NULL;
    _temp_size = 0;
    line_size  = 0;

    if (NULL != value)
      break;
  }

  return value;
}

bool save_add(char *name, char *value) {
  if (NULL == name || NULL == value)
    return false;

  if (NULL == save_fp && !save_open()) {
    debug("failed to open the save file");
    return false;
  }

  if (__save_contains_bad_char(name)) {
    debug("name contains a bad char");
    return false;
  }

  if (__save_contains_bad_char(value)) {
    debug("value contains a bad char");
    return false;
  }

  if (fwrite(name, strlen(name), 1, save_fp) <= 0 || fwrite(value, strlen(value), 1, save_fp) <= 0 ||
      fwrite("\n", 1, 1, save_fp) != 1)
    return false;

  return true;
}

bool save_del(char *name, char *value) {
  if (NULL == name)
    return false;

  size_t   name_len = strlen(name), value_len = NULL == value ? 0 : strlen(value), size = 0;
  uint64_t start = 0, pos = 0;
  char    *content = NULL, c = 0;
  bool     ret = false;

  save_open();

  while (fread(&c, 1, 1, save_fp) == 1) {
    if (NULL == content)
      content = malloc(++size);
    else
      content = realloc(content, ++size);
    content[size - 1] = c;
  }

  save_close();
  unlink(SHRK_SAVE_FILE);

  if ((save_fp = fopen(SHRK_SAVE_FILE, "w")) == NULL) {
    printf("failed to open the save file\n");
    goto end;
  }

  for (; pos < size; pos++) {
    if ('\n' != *(content + pos))
      continue;

    if (pos - start >= name_len + value_len && strncmp(content + start, name, name_len) == 0 &&
        strncmp(content + start + name_len, value, value_len) == 0) {
      start = pos + 1;
      continue;
    }

    fwrite(content + start, sizeof(char), 1 + pos - start, save_fp);
    start = pos + 1;
  }

  ret = true;
end:
  save_close();
  return ret;
}

void save_close() {
  if (NULL != save_fp)
    fclose(save_fp);
  save_fp = NULL;
}

void save_remove() {
  save_close();
  unlink(SHRK_SAVE_FILE);
}
