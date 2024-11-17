package c2

import (
	"bytes"
	"encoding/binary"
	"fmt"
)

/*

 * stolen code from https://github.com/ngn13/ezcat
 * which is another one of my projects so go check it out

 * also refs are from RFC 1035

 */

// 2.3.4. Size limits
var (
	DNS_LABEL_LIMIT int = 63
	DNS_NAME_LIMIT  int = 255
	DNS_UDP_LIMIT   int = 512
	DNS_TXT_LIMIT   int = 255
)

// 4.1.1. Header section format
type DNS_Header struct {
	ID      uint16
	Flags   uint16
	QDCount uint16
	ANCount uint16
	NSCount uint16
	ARCount uint16
}

// 4.1.2. Question section format
type DNS_Question struct {
	Qname  []string
	Qtype  uint16
	Qclass uint16
}

func (q *DNS_Question) Read(buf *bytes.Buffer) error {
	/*

	 * first we read the qname
	 * if you are confused, please see QNAME under 4.1.2. Question section format

	 */
	var (
		res_len   int
		label_len int

		c byte

		err error
	)

	label_len = 0

	for c, err = buf.ReadByte(); err == nil && int(c) > 0; c, err = buf.ReadByte() {
		label_len = int(c)

		if label_len > DNS_LABEL_LIMIT {
			return fmt.Errorf("bad label size (%d)", label_len)
		}

		if label_len > buf.Len() {
			return fmt.Errorf("label size is too large for the buffer (%d)", label_len)
		}

		if res_len+label_len > DNS_NAME_LIMIT {
			return fmt.Errorf("bad name size (%d)", res_len+label_len)
		}

		label := string(buf.Next(label_len))
		q.Qname = append(q.Qname, label)

		res_len += label_len

		if len(q.Qname) != 1 {
			res_len++ // dot
		}
	}

	if err != nil {
		return err
	}

	if buf.Len() < 4 {
		return fmt.Errorf("qtype and qclass size is too large for the buffer")
	}

	// then we read the rest of the fields
	q.Qtype = binary.BigEndian.Uint16(buf.Next(2))
	q.Qclass = binary.BigEndian.Uint16(buf.Next(2))

	return nil
}

func (q *DNS_Question) Write(buf *bytes.Buffer) error {
	// write da qname
	for _, l := range q.Qname {
		buf.WriteByte(byte(len(l)))
		buf.Write([]byte(l))
	}
	buf.WriteByte(byte(0))

	// write qtype and qclass
	binary.Write(buf, binary.BigEndian, q.Qtype)
	binary.Write(buf, binary.BigEndian, q.Qclass)

	return nil
}

// 4.1.3. Resource record format
type DNS_Record struct {
	Name     []string
	Type     uint16
	Class    uint16
	TTL      uint32
	RDLength uint16
	RData    []byte
}

func (r *DNS_Record) Write(buf *bytes.Buffer) error {
	// write da name
	for _, l := range r.Name {
		buf.WriteByte(byte(len(l)))
		buf.Write([]byte(l))
	}
	buf.WriteByte(byte(0))

	// write type, class, ttl and rdlen
	binary.Write(buf, binary.BigEndian, r.Type)
	binary.Write(buf, binary.BigEndian, r.Class)
	binary.Write(buf, binary.BigEndian, r.TTL)
	binary.Write(buf, binary.BigEndian, r.RDLength)

	// lastly write da data
	buf.Write(r.RData)

	return nil
}

// 4.1. Format
type DNS_Packet struct {
	Header      DNS_Header
	Question    DNS_Question
	Answers     DNS_Record
	Authorities []DNS_Record
	Additionals []DNS_Record
}

func (p *DNS_Packet) Read(buf *bytes.Buffer) error {
	var err error

	if binary.Read(buf, binary.BigEndian, &p.Header); err != nil {
		return err
	}

	if p.Header.QDCount != 1 {
		return fmt.Errorf("bad question count")
	}

	if err = p.Question.Read(buf); err != nil {
		return err
	}

	return nil
}

func (p *DNS_Packet) Write(buf *bytes.Buffer) error {
	var err error

	if binary.Write(buf, binary.BigEndian, &p.Header); err != nil {
		return err
	}

	if p.Header.QDCount > 1 {
		return fmt.Errorf("bad question count")
	}

	if p.Header.ANCount > 1 {
		return fmt.Errorf("bad answer count")
	}

	if p.Header.QDCount == 1 {
		if err = p.Question.Write(buf); err != nil {
			return err
		}
	}

	if p.Header.ANCount == 1 {
		if err = p.Answers.Write(buf); err != nil {
			return err
		}
	}

	return nil
}
