#pragma once
#include "client.h"
#include "dns.h"
#include "req.h"

#include <stdbool.h>
#include <stdint.h>

/*

 * ACK response (DNS response used for ACKing job requests when theres no job)
 * ---------------------------------------------------------------------------
 * empty TXT response

 * Job response (DNS response used for transfering an available job)
 * -----------------------------------------------------------------
 * job_id, command, packet_id, is_last, data

*/

typedef enum res_type {
  RES_TYPE_ACK     = 0,
  RES_TYPE_JOB     = 1,
  RES_TYPE_INVALID = 2,
} res_type_t;

typedef struct res {
  res_type_t type;
  char       job_id[JOB_ID_SIZE + 1];
  uint8_t    command;
  uint64_t   packet_id;
  bool       is_last;
  char      *data;
  uint32_t   data_size;
} res_t;

void    res_new(res_t *res);
void    res_free(res_t *res);
int64_t res_recv(res_t *res, client_t *c);
bool    res_from_dns(res_t *res, dns_packet_t *p);
