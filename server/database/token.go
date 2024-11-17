package database

import "github.com/ngn13/shrk/server/util"

func (d *Type) TokenAdd() string {
	token := util.RandomStr(32)
	d.Tokens = append(d.Tokens, token)
	return token
}

func (d *Type) TokenDel(token string) {
	i := d.TokenExists(token)
	d.Tokens = append(d.Tokens[:i], d.Tokens[i+1:]...)
}

func (d *Type) TokenExists(token string) int {
	for i := range d.Tokens {
		if d.Tokens[i] == token {
			return i
		}
	}
	return -1
}
