package log

import (
	"fmt"
	"os"

	"github.com/sirupsen/logrus"
)

func init() {
	logrus.SetFormatter(&logrus.JSONFormatter{})
	logrus.SetLevel(logrus.WarnLevel)
	logrus.SetFormatter(&logrus.TextFormatter{ForceColors: true})
}

func SetOutput(RootDir string) {
	logrus.SetOutput(os.Stderr)

	logDirPath := RootDir + "/log"
	logFilePath := logDirPath + "/poseidonos-cli.log"

	os.MkdirAll(logDirPath, os.FileMode(0755))
	f, err := os.OpenFile(logFilePath, os.O_APPEND|os.O_CREATE|os.O_RDWR, 0666)
	if err != nil {
		fmt.Printf("error while opening log file: %v\n", err)
		return
	}
	logrus.SetOutput(f)
}

//set Warn as a default log level.
//log level grows like below.

func SetDebugMode() {
	logrus.SetLevel(logrus.DebugLevel)
}

func SetVerboseMode() {
	logrus.SetLevel(logrus.InfoLevel)
}

func Debug(args ...interface{}) {
	logrus.Debug(args...)
}

func Debugf(format string, args ...interface{}) {
	logrus.Debugf(format, args...)
}

func Info(args ...interface{}) {
	logrus.Info(args...)
}

func Infof(format string, args ...interface{}) {
	logrus.Infof(format, args...)
}

func Warn(args ...interface{}) {
	logrus.Warn(args...)
}

func Warnf(format string, args ...interface{}) {
	logrus.Warnf(format, args...)
}

func Error(args ...interface{}) {
	logrus.Error(args...)
}

func Errorf(format string, args ...interface{}) {
	logrus.Errorf(format, args...)
}

func Critical(args ...interface{}) {
	logrus.Fatal(args...)
}

func Fatal(args ...interface{}) {
	logrus.Fatal(args...)
}

func Fatalf(format string, args ...interface{}) {
	logrus.Fatalf(format, args...)
}
