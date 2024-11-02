#include "inc/job.h"
#include "inc/req.h"
#include "inc/res.h"

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <time.h>
#include <unistd.h>

#define job_debug(j, m, ...) debug("(job: %s) " m, j->id, __VA_ARGS__)
#define __job_req_send(j)    (req_send(&j->req, j->client))
#define __job_res_recv(j)    (res_recv(&j->res, j->client))

bool job_new(job_t *job, client_t *client) {
  if (NULL == client) {
    errno = EINVAL;
    return false;
  }

  bzero(job, sizeof(job_t));
  job->client = client;

  res_new(&job->res);
  req_new(&job->req);

  return true;
}

void job_data_clear(job_t *job) {
  free(job->data);
  job->data      = NULL;
  job->data_pos  = 0;
  job->data_size = 0;
}

void job_data_set(job_t *job, char *data, uint64_t data_size) {
  if (NULL == data)
    return job_data_clear(job);

  if (data_size == 0)
    data_size = strlen(data);

  if (NULL == job->data)
    job->data = malloc((job->data_size += data_size) + 1);
  else
    job->data = realloc(job->data, (job->data_size += data_size) + 1);

  memcpy(job->data + job->data_pos, data, data_size);
  job->data[job->data_size] = 0; // just for safety
  job->data_pos += data_size;
}

bool job_recv(job_t *job, bool allow_new) {
  bool ret = false, is_new = false;
  job->req.type = REQ_TYPE_JOB;
  res_new(&job->res);

  // send a job request
  if (__job_req_send(job) < 0) {
    job_debug(job, "failed to send the job request: %s", strerror(errno));
    goto end;
  }

  // get the job response
  if (__job_res_recv(job) < 0) {
    job_debug(job, "failed to send the job response: %s", strerror(errno));
    goto end;
  }

  // did we just get an ACK?
  if (job->res.type != RES_TYPE_JOB) {
    job_debug(job, "did not receive a job response (type %d)", job->res.type);
    goto end;
  }

  // check if a new job is allowed
  if ((is_new = strcmp(job->res.job_id, job->id) != 0) && !allow_new) {
    job_debug(job, "received a new job (id %s), however allow_new is false", job->res.job_id);
    goto end;
  }

  if (is_new) {
    job_data_set(job, NULL, 0);
    job->packet_id = 0;
  }

  if (job->res.packet_id != job->packet_id) {
    job_debug(job, "invalid packet id (got: %d, expected: %d)", job->res.packet_id, job->packet_id);
    goto end;
  }

  job->complete = job->res.is_last;
  job->cmd      = job->res.command;
  job->packet_id++;

  memcpy(job->id, job->res.job_id, JOB_ID_SIZE);
  job_data_set(job, job->res.data, job->res.data_size);

  ret = true;
end:
  res_free(&job->res);
  return ret;
}

bool job_send(job_t *job, bool require_ack) {
  uint64_t data_size = 0;

  // reset the request if the last request sent belongs to different job
  if (strcmp(job->req.job_id, job->id) != 0)
    req_new(&job->req);

  // setup the job result request
  job->req.type = REQ_TYPE_RESULT;
  memcpy(job->req.job_id, job->id, JOB_ID_SIZE);

  // send data in multiple requests if required
  for (job->data_pos = 0; job->data_pos < job->data_size;) {
    data_size = REQ_DATA_SIZE_MAX;

    if (job->data_size - job->data_pos <= data_size) {
      data_size        = job->data_size - job->data_pos;
      job->req.is_last = job->complete;
    }

    job->req.data      = job->data + job->data_pos;
    job->req.data_size = data_size;

    job_debug(job, "sending job result request (pid: %lu, is_last: %d)", job->req.packet_id, job->req.is_last);

    // send the data
    if (__job_req_send(job) < 0) {
      job_debug(job, "failed to send the job result request (pid: %lu)", job->req.packet_id);
      return false;
    }

    // wait for the ack
    if (__job_res_recv(job) < 0 && require_ack) {
      job_debug(job, "did not receive ACK for job result request (pid: %lu)", job->req.packet_id);
      return false;
    }

    job->data_pos += data_size;
    job->req.packet_id++;
  }

  return true;
}
