#include "inc/dns.h"
#include "inc/client.h"
#include "inc/util.h"

#include <asm-generic/errno-base.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>

#include <errno.h>
#include <stdio.h>

char *dns_fake_labels[][3] = {
    {"rr1---sn-nja7sne6", "googlevideo", "com"},
    {"rr2---sn-njaeyn7s", "googlevideo", "com"},
    {"rr3---sn-nja7snll", "googlevideo", "com"},
};

#define __dns_fake_label_count() (sizeof(dns_fake_labels) / sizeof(dns_fake_labels[0]))
#define __dns_fake_label_pick()  (dns_fake_labels[randint(0, __dns_fake_label_count() - 1)])

void dns_free(dns_packet_t *p) {
  debug("freeing dns packet (dns header id %d)", p->header.id);

  uint16_t i = 0;

  for (i = 0; i < p->header.qdcount; i++)
    free(p->question[i].qname);

  for (i = 0; i < p->header.ancount; i++)
    free(p->answers[i].rdata);

  bzero(p, sizeof(dns_packet_t));
}

void dns_label_add(dns_packet_t *p, char *label, uint64_t label_len) {
  if (label_len == 0)
    return;

  dns_question_t *q = &p->question[0];

  if (NULL == q->qname)
    q->qname = malloc(label_len);
  else
    q->qname = realloc(q->qname, q->_qname_len + label_len + 1);

  q->qname[q->_qname_len++] = label_len;
  memcpy(q->qname + q->_qname_len, label, label_len);
  q->_qname_len += label_len;

  return;
}

void dns_label_complete(dns_packet_t *p) {
  char **fake_names = __dns_fake_label_pick();

  for (uint8_t i = 0; i < 3; i++)
    dns_label_add(p, fake_names[i], strlen(fake_names[i]));

  // add 0 to mark the end
  p->question[0]._qname_len += 1;

  if (NULL == p->question[0].qname)
    p->question[0].qname = malloc(p->question[0]._qname_len);
  else
    p->question[0].qname = realloc(p->question[0].qname, p->question[0]._qname_len);

  p->question[0].qname[p->question[0]._qname_len - 1] = 0;
}

int64_t dns_send(dns_packet_t *p, client_t *c) {
  if (NULL == p || NULL == c)
    goto fail;

  if (p->header.qdcount > dns_question_count(p)) {
    debug("cannot send dns packet (header id %d), contains too many questions", p->header.id);
    goto fail;
  }

  char            _buf[DNS_UDP_LIMIT], *buf = _buf;
  dns_question_t *q = NULL;
  dns_header_t    hcp;

  bzero(buf, sizeof(_buf));

  hcp.id      = htons(p->header.id);
  hcp.flags   = htons(p->header.flags);
  hcp.qdcount = htons(p->header.qdcount);
  hcp.ancount = htons(p->header.ancount);
  hcp.nscount = htons(p->header.nscount);
  hcp.arcount = htons(p->header.arcount);

  buf += copy(buf, &hcp, sizeof(hcp));

  for (uint8_t i = 0; i < p->header.qdcount; i++) {
    q = &p->question[i];

    q->qtype  = htons(q->qtype);
    q->qclass = htons(q->qclass);

    buf += copy(buf, q->qname, q->_qname_len);
    buf += copy(buf, &q->qtype, sizeof(q->qtype));
    buf += copy(buf, &q->qclass, sizeof(q->qclass));
  }

  debug("sending dns packet (header id %d)", p->header.id);
  debug_dump((void *)_buf, buf - _buf);

  return client_send(c, _buf, buf - _buf);

fail:
  errno = EINVAL;
  return -1;
}

int16_t __dns_label_skip_all(uint8_t *buf, int16_t bufsize) {
  uint16_t i = 0;

  for (i = 0; i < bufsize; i++) {
    if (buf[i] == 0)
      break;

    if (i + buf[i] < bufsize)
      i += buf[i];
    else
      return -1;
  }

  return ++i;
}

int64_t dns_recv(dns_packet_t *p, client_t *c) {
  if (NULL == p || NULL == c) {
    errno = EINVAL;
    return -1;
  }

  int16_t       indx = 0, res = 0, bufsize = 0, i = 0;
  char          buf[DNS_UDP_LIMIT];
  dns_record_t *r = NULL;

  bzero(buf, sizeof(buf));
  bzero(p, sizeof(dns_packet_t));

  if ((bufsize = client_recv(c, buf, DNS_UDP_LIMIT)) < 0)
    return -1;

  if (bufsize < sizeof(p->header)) {
    debug("received buffer is too small for dns header (%l)", bufsize);
    goto fail;
  }

  indx += copy(&p->header, buf + indx, sizeof(p->header));

  p->header.id      = ntohs(p->header.id);
  p->header.flags   = ntohs(p->header.flags);
  p->header.qdcount = ntohs(p->header.qdcount);
  p->header.ancount = ntohs(p->header.ancount);
  p->header.nscount = ntohs(p->header.nscount);
  p->header.arcount = ntohs(p->header.arcount);

  if (p->header.qdcount > dns_question_count(p)) {
    debug("received invalid dns packet (header id %d), bad question count: %d", p->header.qdcount);
    goto fail;
  }

  if (p->header.ancount > dns_answer_count(p)) {
    debug("received invalid dns packet (header id %d), bad answer count: %d", p->header.ancount);
    goto fail;
  }

  // why read the question(s)? we don't need it
  for (i = 0; i < p->header.qdcount; i++) {
    // get to the end of qname
    if ((res = __dns_label_skip_all((void *)buf + indx, bufsize - indx)) < 0) {
      debug("failed to reach end of question %d qname", i);
      goto fail;
    }

    indx += res;

    // get to the end of qtype and qclass
    indx += sizeof(p->question[i].qtype);
    indx += sizeof(p->question[i].qclass);

    if (indx > bufsize) {
      debug("received data is too small for question %d", i);
      goto fail;
    }

    continue;
  }

  // now read the answer(s)
  for (i = 0; i < p->header.ancount; i++) {
    // make it easier to access
    r = &p->answers[i];

    // get to the end of name (dont need it)
    if ((res = __dns_label_skip_all((void *)buf + indx, bufsize - indx)) < 0) {
      debug("failed to reach end of answer %d name", i);
      goto fail;
    }

    indx += res;

    // also skip type, class and ttl
    indx += sizeof(r->type);
    indx += sizeof(r->class_);
    indx += sizeof(r->ttl);

    if (indx + sizeof(r->rdlen) > bufsize) {
      debug("received data is too small for answer %d", i);
      goto fail;
    }

    // read rdlen
    indx += copy(&r->rdlen, buf + indx, sizeof(r->rdlen));
    r->rdlen = ntohs(r->rdlen);

    // check the rdlen
    if (indx + r->rdlen > bufsize) {
      debug("rdlen is too large for answer %d", i);
      goto fail;
    }

    // now read rdata
    r->rdata = malloc(r->rdlen + 1);
    memcpy(r->rdata, buf + indx, r->rdlen);

    // just for safety
    r->rdata[r->rdlen] = 0;
  }

  debug("received dns packet (header id %d)", p->header.id);
  debug_dump((void *)buf, bufsize);

  return bufsize;

fail:
  errno = ENOMSG;
  return -1;
}
