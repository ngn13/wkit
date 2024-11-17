#include "../inc/cmds.h"
#include "../inc/kernel.h"

#include <stdbool.h>
#include <sys/stat.h>

#include <dirent.h>
#include <stdlib.h>
#include <string.h>

#include <errno.h>
#include <stdio.h>

/*

 * ps command gets a list of processes and their details
 * and transfers the list in a specific format

*/

// pid, ppid, uid, gid, vm size, command line
#define proc_format(r, s) snprintf(r, s, "\\\\%d\\%d\\%u\\%u\\%lu\\%s", pid, ppid, uid, gid, vmsize, cmdline)

bool __cmd_send_proc(job_t *job, pid_t pid) {
  char    *cmdline = NULL, *line = NULL, pid_path[PATH_MAX + 1], c = 0;
  uint32_t uid = -1, gid = -1;
  uint64_t vmsize = 0;
  FILE    *pid_fp = NULL;
  size_t   size   = 0;
  pid_t    ppid   = 0;
  bool     ret    = false;

  job_data_clear(job);
  job->complete = false;

  if (pid == 0)
    return false;

  // open /proc/[pid]/status
  snprintf(pid_path, PATH_MAX + 1, "/proc/%d/status", pid);

  if ((pid_fp = fopen(pid_path, "r")) == NULL) {
    job_debug_err("failed to open the status file for %d", pid);
    goto end;
  }

  ret = true;

  while (getline(&line, &size, pid_fp) > 0) {
    if (strncmp(line, "PPid:", 5) == 0) {
      sscanf(line, "PPid:%*s%d", &ppid);
      if (!(ret = ppid >= 0))
        debug("failed to parse the PPID for %d", pid);
    }

    else if (strncmp(line, "Uid:", 4) == 0) {
      sscanf(line, "Uid:%*s%d", &uid);
      if (!(ret = uid >= 0))
        debug("failed to parse the UID for %d", pid);
    }

    else if (strncmp(line, "Gid:", 4) == 0) {
      sscanf(line, "Gid:%*s%d", &gid);
      if (!(ret = gid >= 0))
        debug("failed to parse the GID for %d", pid);
    }

    else if (strncmp(line, "VmSize:", 7) == 0) {
      sscanf(line, "VmSize: %lu", &vmsize);
      if (!(ret = vmsize > 0))
        debug("failed to parse the VmSize for %d", pid);
    }

    if (!ret)
      goto end;

    free(line);
    line = NULL;
    size = 0;
  }

  ret = false;
  fclose(pid_fp);

  // open /proc/[pid]/cmdline for UID and GID
  snprintf(pid_path, PATH_MAX + 1, "/proc/%d/cmdline", pid);

  if ((pid_fp = fopen(pid_path, "r")) == NULL) {
    job_debug_err("failed to open the cmdline file for %d", pid);
    goto end;
  }

  for (size = 0; fread(&c, sizeof(c), 1, pid_fp) == 1;) {
    if ('\\' == c)
      continue;

    if (NULL == cmdline)
      cmdline = malloc(++size);
    else
      cmdline = realloc(cmdline, ++size);

    if (0 == c)
      c = ' ';

    cmdline[size - 1] = c;
  }

  if (size == 0) {
    ret = true;
    goto end;
  }

  if (NULL == cmdline)
    cmdline = malloc(++size);
  else
    cmdline = realloc(cmdline, ++size);

  cmdline[size - 1] = 0;

  if ((job->data_size = 1 + proc_format(job->data, 0)) < 0) { // 1 for the null terminator
    job_debug("failed to calculate data size for %d", pid);
    goto end;
  }

  if ((job->data_size = proc_format(job->data = malloc(job->data_size), job->data_size)) < 0) {
    job_debug("failed to format data for %d", pid);
    goto end;
  }

  job_debug("sending the process data for %d", pid);

  if (!job_send(job, false)) {
    job_debug("failed to send the process data for %d", pid);
    goto end;
  }

  ret = true;
end:
  if (NULL != pid_fp)
    fclose(pid_fp);

  free(cmdline);
  free(line);

  return ret;
}

char *cmd_ps(job_t *job) {
  cmd_recv_all(job);

  struct dirent *ent = NULL;
  DIR           *dir = NULL;
  char          *res = NULL;
  pid_t          pid = 0;

  // open the /proc dir
  if ((dir = opendir("/proc")) == NULL) {
    job_debug_err("failed open the /proc directory");
    return strerror(errno);
  }

  // check for processes
  while ((ent = readdir(dir)) != NULL) {
    // only dirs
    if (ent->d_type != DT_DIR)
      continue;

    // convert name to PID
    if ((pid = atoi(ent->d_name)) <= 0)
      continue;

    // send the current entry
    if (__cmd_send_proc(job, pid))
      continue;

    job_debug("failed to send process details: %d", pid);
    res = "failed to transfer all the process details";
    goto end;
  }

  res = "\\";
end:
  if (NULL != dir)
    closedir(dir);
  return res;
}
