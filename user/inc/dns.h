#pragma once
#include "client.h"

#include <stdbool.h>
#include <stdint.h>

/*

 * for DNS data structure definitions, see RFC 1035
 * https://www.rfc-editor.org/rfc/rfc1035

*/

typedef enum {
  DNS_RCODE_NO_ERROR        = 0,
  DNS_RCODE_FORMAT_ERROR    = 1,
  DNS_RCODE_SERVER_FAIL     = 2,
  DNS_RCODE_NAME_ERROR      = 3,
  DNS_RCODE_NOT_IMPLEMENTED = 4,
  DNS_RCODE_REFUSED         = 5,
} dns_rcode_t;

// 2.3.4. Size limits
#define DNS_LABEL_LIMIT 63
#define DNS_NAME_LIMIT  255
#define DNS_UDP_LIMIT   512
#define DNS_TXT_LIMIT   255

// 4.1.1. Header section format
typedef struct dns_header {
  uint16_t id;
  uint16_t flags;
  uint16_t qdcount;
  uint16_t ancount;
  uint16_t nscount;
  uint16_t arcount;
} dns_header_t;

// 4.1.2. Question section format
typedef struct dns_question {
  char    *qname;
  uint16_t _qname_len;
  uint16_t qtype;
  uint16_t qclass;
} dns_question_t;

// 4.1.3. Resource record format
typedef struct dns_record {
  char    *name;
  uint16_t type;
  uint16_t class_;
  uint32_t ttl;
  uint16_t rdlen;
  char    *rdata;
} dns_record_t;

// 4.1. Format
typedef struct dns_packet {
  dns_header_t   header;
  dns_question_t question[1];
  dns_record_t   answers[1];
  dns_record_t  *authorities;
  dns_record_t  *additionals;
} dns_packet_t;

#define dns_question_count(p) ((uint16_t)(sizeof(p->question) / sizeof(p->question[0])))
#define dns_answer_count(p)   ((uint16_t)(sizeof(p->answers) / sizeof(p->answers[0])))

#if SHRK_DEBUG_DUMP
#define dns_debug_dump_qname(p) (debug_dump((void *)p->question[0].qname, p->question[0]._qname_len))
#else
#define dns_debug_dump_qname(p) asm("nop;")
#endif

void dns_free(dns_packet_t *p);

void dns_label_add(dns_packet_t *p, char *label, uint64_t label_len);
void dns_label_complete(dns_packet_t *p);

int64_t dns_send(dns_packet_t *p, client_t *c);
int64_t dns_recv(dns_packet_t *p, client_t *c);
