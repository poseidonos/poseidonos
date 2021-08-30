package ibofos

import (
	"encoding/json"
	"errors"
	"pnconnector/src/handler"
	"pnconnector/src/log"
	"pnconnector/src/routers/m9k/model"
	"time"
)

var (
	errLocked    = errors.New("Locked out buddy")
	ErrSending   = errors.New("Sending error")
	ErrReceiving = errors.New("Receiving error")
	ErrJson      = errors.New("Json error")
	ErrRes       = errors.New("Response error")
	ErrConn      = errors.New("POS Connection Error")
	ErrJsonType  = errors.New("Json Type Validation Error")
	//mutex        = &sync.Mutex{}
)

type Requester struct {
	xrId      string
	param     interface{}
	paramType interface{}
}

func (rq Requester) Send(command string) (model.Request, model.Response, error) {
	iBoFRequest := model.Request{
		Command: command,
		Rid:     rq.xrId,
	}

	err := checkJsonType(rq.param, rq.paramType)
	if err != nil {
		return iBoFRequest, model.Response{}, err
	} else {
		iBoFRequest.Param = rq.param
		res, err := sendIBoF(iBoFRequest)
		return iBoFRequest, res, err
	}
}

func checkJsonType(srcParam interface{}, paramType interface{}) error {
	var err error
	marshalled, _ := json.Marshal(srcParam)

	switch param := paramType.(type) {
	case model.ArrayParam:
		err = json.Unmarshal(marshalled, &param)
	case model.DeviceParam:
		err = json.Unmarshal(marshalled, &param)
	case model.VolumeParam:
		err = json.Unmarshal(marshalled, &param)
	case model.InternalParam:
		err = json.Unmarshal(marshalled, &param)
	case model.SystemParam:
		err = json.Unmarshal(marshalled, &param)
	case model.RebuildParam:
		err = json.Unmarshal(marshalled, &param)
	case model.LoggerParam:
		err = json.Unmarshal(marshalled, &param)
	case model.WBTParam:
		err = json.Unmarshal(marshalled, &param)
	}

	if err != nil {
		log.Debugf("checkJsonType : ", ErrJsonType.Error())
		err = ErrJsonType
	}

	return err
}

func sendIBoF(iBoFRequest model.Request) (model.Response, error) {
	conn, err := handler.ConnectToIBoFOS()
	if err != nil {
		return model.Response{}, ErrConn
	}
	defer handler.DisconnectToIBoFOS(conn)

	log.Infof("sendIBoF : %+v", iBoFRequest)

	marshaled, _ := json.Marshal(iBoFRequest)
	err = handler.WriteToIBoFSocket(conn, marshaled)

	if err != nil {
		log.Infof("sendIBoF write error : %v", err)
		return model.Response{}, ErrSending
	}

	for {
		temp, err := handler.ReadFromIBoFSocket(conn)

		if err != nil {
			log.Infof("sendIBoF read error : %v", err)
			return model.Response{}, ErrReceiving
		} else {
			log.Infof("Response From iBoF : %s", temp.String())
		}

		response := model.Response{}

		d := json.NewDecoder(&temp)
		d.UseNumber()

		if err = d.Decode(&response); err != nil {
			log.Fatal(err)
		}

		if err != nil {
			log.Infof("Response Unmarshal Error : %v", err)
			return model.Response{}, ErrJson
		} else {
			response.LastSuccessTime = time.Now().UTC().Unix()
			return response, nil
		}
	}
}
