#include "../inc/cmds.h"

#include <sys/utsname.h>

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <errno.h>
#include <stdio.h>

/*

 * info command gets system info (memory/CPU core count/relase and distro)
 * then it formats it, and sends it to the server

*/

#define info_format(r, s)      (snprintf(r, s, "/%lu/%lu/%s", mem, cores, un.release))
#define info_format_full(r, s) (snprintf(r, s, "/%lu/%lu/%s/%s", mem, cores, un.release, distro))

char *cmd_info(job_t *job) {
  cmd_recv_all(job);

  uint64_t       mem = 0, cores = 0;
  char          *distro = NULL, *info = NULL;
  int64_t        size = 0;
  struct utsname un;

  // get system memory
  mem = sysconf(_SC_PHYS_PAGES) * sysconf(_SC_PAGESIZE);
  mem /= 1024;

  // get cores
  cores = sysconf(_SC_NPROCESSORS_ONLN);

  // get kernel
  if ((uname(&un)) != 0) {
    job_debug("uname failed: %s", strerror(errno));
    return strerror(errno);
  }

  // parse /etc/os-release for distro
  if ((distro = get_distro()) == NULL) {
    if ((size = info_format(NULL, 0)) < 0)
      return "size calculation failed";

    info = malloc(++size);
    info_format(info, size);
  }

  else {
    if ((size = info_format_full(NULL, 0)) < 0)
      return "size calculation failed";

    info = malloc(++size);
    info_format_full(info, size);
  }

  // send the result
  job_data_clear(job);
  job_data_set(job, info, size - 1);

  job->complete = true;
  job_send(job, false);

  // free allocated things
  free(distro);
  free(info);

  return NULL;
}
