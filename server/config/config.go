package config

import (
	"fmt"
	"net/url"
	"os"
	"reflect"
	"strconv"
)

const VERSION = "1.0"

type Type struct {
	// not modifiable
	Version    string
	SourcePath string
	HTTP_URL_p *url.URL
	C2_URL_p   *url.URL

	// modifiable (with env)
	HTTP_Addr string `env:"HTTP_ADDR"`
	C2_Addr   string `env:"C2_ADDR"`
	HTTP_URL  string `env:"HTTP_URL"`
	C2_URL    string `env:"C2_URL"`
	Static    string `env:"STATIC"`
	Source    string `env:"SOURCE"`
	Script    string `env:"SCRIPT"`
	Views     string `env:"VIEWS"`
	Path      string `env:"PATH"`
	Database  string `env:"DB"`
	Password  string `env:"PASSWORD"`
	Debug     bool   `env:"DEBUG"`
}

func (t *Type) Load() error {
	var (
		val reflect.Value
		typ reflect.Type

		field_val reflect.Value
		field_typ reflect.StructField

		env_name string
		env_val  string

		err error
		ok  bool
	)

	val = reflect.ValueOf(t).Elem()
	typ = val.Type()

	for i := 0; i < val.NumField(); i++ {
		field_val = val.Field(i)
		field_typ = typ.Field(i)

		if env_name, ok = field_typ.Tag.Lookup("env"); !ok || !field_val.CanSet() {
			continue
		}

		env_name = fmt.Sprintf("SHRK_%s", env_name)

		if env_val = os.Getenv(env_name); env_val == "" {
			continue
		}

		switch field_val.Kind() {
		case reflect.String:
			field_val.SetString(env_val)

		case reflect.Int:
			var env_val_int int
			if env_val_int, err = strconv.Atoi(env_val); err != nil {
				return fmt.Errorf("%s should be an integer", env_name)
			}
			field_val.SetInt(int64(env_val_int))

		case reflect.Bool:
			var env_val_bool bool
			if env_val == "true" || env_val == "1" {
				env_val_bool = true
			} else if env_val == "false" || env_val == "0" {
				env_val_bool = false
			} else {
				return fmt.Errorf("%s should be a boolean", env_name)
			}
			field_val.SetBool(env_val_bool)
		}
	}

	return nil
}

func New() (*Type, error) {
	var (
		conf Type
		err  error
	)

	// setup non modifiable values
	conf.Version = VERSION
	conf.SourcePath = ""

	// setup modifiable values to default
	conf.HTTP_Addr = "127.0.0.1:7070"
	conf.C2_Addr = "127.0.0.1:1053"
	conf.HTTP_URL = "http://127.0.0.1:7070"
	conf.C2_URL = "dns://127.0.0.1:1053"
	conf.Script = "../scripts/install.sh"
	conf.Source = fmt.Sprintf("../release/shrk-client-%s.tar.gz", conf.Version)
	conf.Static = "./static"
	conf.Views = "./views"
	conf.Database = "./data"
	conf.Path = ""
	conf.Password = ""
	conf.Debug = false

	if err = conf.Load(); err != nil {
		return nil, err
	}

	if conf.HTTP_URL_p, err = url.Parse(conf.HTTP_URL); err != nil {
		return nil, fmt.Errorf("failed to parse the HTTP URL: %s", err.Error())
	}

	if conf.C2_URL_p, err = url.Parse(conf.C2_URL); err != nil {
		return nil, fmt.Errorf("failed to parse the C2 URL: %s", err.Error())
	}

	if conf.HTTP_URL_p.Scheme != "http" && conf.HTTP_URL_p.Scheme != "https" {
		return nil, fmt.Errorf("HTTP URL only supports HTTP or HTTPS schemes")
	}

	if conf.C2_URL_p.Scheme != "dns" {
		return nil, fmt.Errorf("C2 URL only supports DNS schemes")
	}

	if _, err = os.Stat(conf.Script); err != nil {
		return nil, fmt.Errorf("installation script (%s) is not accessible: %s", conf.Script, err.Error())
	}

	if _, err = os.Stat(conf.Source); err != nil {
		return nil, fmt.Errorf("source archive (%s) is not accessible: %s", conf.Source, err.Error())
	}

	return &conf, nil
}
