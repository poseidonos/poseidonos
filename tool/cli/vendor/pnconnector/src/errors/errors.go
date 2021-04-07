package errors

import (
	"github.com/juju/errors"
)

func New(message string) error {
	err := errors.New(message)
	return err
}
