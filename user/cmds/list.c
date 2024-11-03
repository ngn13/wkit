#include "../inc/cmd.h"

#include <stdbool.h>
#include <sys/stat.h>

#include <dirent.h>
#include <stdlib.h>
#include <string.h>

#include <errno.h>
#include <stdio.h>

/*

 * list command get entries from a specified directory
 * and transfers the entry list in a specific format

*/

// name, type, mode, uid, gid, size, modified time, created time
#define ent_format(r, s)                                                                                               \
  snprintf(r,                                                                                                          \
      s,                                                                                                               \
      "//%s/%d/%d/%d/%d/%lu/%lld/%lld/%d",                                                                             \
      name,                                                                                                            \
      is_dir,                                                                                                          \
      st.st_mode,                                                                                                      \
      st.st_uid,                                                                                                       \
      st.st_gid,                                                                                                       \
      st.st_size,                                                                                                      \
      (long long)(st.st_mtim.tv_sec),                                                                                  \
      (long long)(st.st_ctim.tv_sec),                                                                                  \
      0)

bool __cmd_list_ent(job_t *job, struct dirent *ent) {
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

char *cmd_list(job_t *job) {
  cmd_recv_all(job);

  struct dirent *ent = NULL;
  DIR           *dir = NULL;
  char          *res = NULL;

  // open the current dir
  if ((dir = opendir(".")) == NULL) {
    job_debgf("failed open the current directory: %s", strerror(errno));
    return strerror(errno);
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
  if (NULL != dir)
    closedir(dir);
  return res;
}
