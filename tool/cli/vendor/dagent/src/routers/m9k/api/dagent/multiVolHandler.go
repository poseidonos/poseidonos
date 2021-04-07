package dagent

import (
	"pnconnector/src/log"
	iBoFOS "pnconnector/src/routers/m9k/api/ibofos"
	"pnconnector/src/routers/m9k/model"
	"pnconnector/src/util"
	"bytes"
	"dagent/src/routers/m9k/header"
	"encoding/json"
	"github.com/gin-gonic/gin"
	"github.com/gin-gonic/gin/binding"
	"github.com/google/uuid"
	"io/ioutil"
	"net/http"
	"strconv"
	"time"
)

const (
	CREATE_VOLUME        = "CREATEVOLUME"
	MOUNT_VOLUME         = "MOUNTVOLUME"
	SINGLE_VOLUME_COUNT  = 1
	AUTHORIZATION_HEADER = "Authorization"
	CALLBACK_IP          = "127.0.0.1"
	CALLBACK_PORT        = "5000"
	CONTENT_TYPE         = "application/json"
	CALLBACK_URL         = "http://" + CALLBACK_IP + ":" + CALLBACK_PORT + "/api/v1.0/multi_vol_response/"
	MAX_RETRY_COUNT      = 5
	POST                 = "POST"
	POS_API_ERROR        = 11040
	COUNT_EXCEEDED_ERROR = 11050
	DELAY                = 1 * time.Second
)

var (
	CreateVolumeMutex  bool = false
	MountVolumeMutex   bool = false
	CreateVolResponses []model.Response
	MountVolResponses  []model.Response
	CreateVolPassCount int = 0
	MountVolPassCount  int = 0
)

func callbackMethod(Buffer []model.Response, Auth string, PassCount int, totalCount int) {
	responseData := &model.CallbackMultiVol{
		TotalCount:    totalCount,
		Pass:          PassCount,
		Fail:          totalCount - PassCount,
		MultiVolArray: Buffer,
	}
	for retryCount := 1; retryCount < MAX_RETRY_COUNT; retryCount = retryCount + 1 {
		bytesRepresentation, err := json.Marshal(responseData)
		if err != nil {
			log.Error(err)
			continue
		}
		//Create Request Object
		req, _ := http.NewRequest(
			POST,
			CALLBACK_URL,
			bytes.NewBuffer(bytesRepresentation),
		)
		//Add Request Header
		req.Header.Add(AUTHORIZATION_HEADER, Auth)
		res, err := http.DefaultClient.Do(req)
		if err != nil {
			log.Error(err)
			continue
		}

		// read response body
		data, _ := ioutil.ReadAll(res.Body)

		// close response body
		res.Body.Close()

		// print response status and body
		log.Infof("status: %d\n", res.StatusCode)
		log.Infof("body: %s\n", data)
		break
	}
}

/*
  createVolumeWrite function tries to create the volumes and mount them. For each Volume the following steps are done
  1. Call the create Volume API
  2. If Create VOlume Succeeds, go to step 4
  3. If Create Volume fails, Try MAX_RETRY_COUNT times. If it still fails, move to next volume and Start from step 1
  4. If MountAll parameter is true, Call the Mount Volume API
  5. If Mount Volume fails, try MAX_RETRY_COUNT times. If it still fails, move to next voluem and Start from Step 1
*/
func createVolumeWrite(CreateVolCh chan model.Response, ctx *gin.Context, volParam *model.VolumeParam) {
	volId := volParam.NameSuffix
	volName := volParam.Name
	for volItr := 0; volItr < int(volParam.TotalCount); volItr, volId = volItr+1, volId+1 {
		posErr := false
		volParam.Name = volName + strconv.Itoa(int(volId))

		// Retry Create Volume API if the API fails
		for createItr := 1; createItr <= MAX_RETRY_COUNT; createItr = createItr + 1 {

			// Call the API after a delay
			time.Sleep(DELAY)
			_, res, err := iBoFOS.CreateVolume(header.XrId(ctx), *volParam)

			// If Create Volume API fails, retry
			if err != nil || res.Result.Status.Code != 0 {
				if createItr == MAX_RETRY_COUNT {
					CreateVolCh <- res
					posErr = true
				}
				continue
			}
			CreateVolCh <- res
			if posErr {
				break
			}

			// if MountAll parameter is true, mount the volume
			if volParam.MountAll {
				// Retry Mount Volume API if it fails
				for mountItr := 1; mountItr <= MAX_RETRY_COUNT; mountItr = mountItr + 1 {
					time.Sleep(DELAY)
					_, res, err = iBoFOS.MountVolume(header.XrId(ctx), *volParam)
					if err != nil || res.Result.Status.Code != 0 {
						if mountItr == MAX_RETRY_COUNT {
							posErr = true
							CreateVolCh <- res
						}
						continue
					}
					CreateVolCh <- res
					break
				}
			}
			break

		}
		if !posErr {
			CreateVolPassCount++
		} else if volParam.StopOnError {
			break
		}
	}
	close(CreateVolCh)

}

func createVolumeRead(CreateVolCh chan model.Response, Auth string, totalCount int) {
	for {
		res, ok := <-CreateVolCh
		if ok == false {
			log.Info("Channel Close ", ok)
			break
		}
		log.Info("channel open")
		CreateVolResponses = append(CreateVolResponses, res)
	}
	CreateVolumeMutex = false
	callbackMethod(CreateVolResponses, Auth, CreateVolPassCount, totalCount)
	CreateVolResponses = nil
	CreateVolPassCount = 0
}

func mountVolumeWrite(MountVolCh chan model.Response, ctx *gin.Context, f func(string, interface{}) (model.Request, model.Response, error), volParam *model.VolumeParam) {
	volId := volParam.NameSuffix
	volName := volParam.Name
	for volItr := 0; volItr < int(volParam.TotalCount); volItr, volId = volItr+1, volId+1 {
		volParam.Name = volName + strconv.Itoa(int(volId))
		_, res, err := f(header.XrId(ctx), *volParam)
		MountVolCh <- res
		if err != nil || res.Result.Status.Code != 0 {
			if volParam.StopOnError == true {
				break
			}
		} else {
			MountVolPassCount++
		}
	}
	close(MountVolCh)

}

func mountVolumeRead(MountVolCh chan model.Response, Auth string, totalCount int) {
	for {
		res, ok := <-MountVolCh
		if ok == false {
			log.Info("Channel Close ", ok)
			break
		}
		MountVolResponses = append(MountVolResponses, res)
	}
	MountVolumeMutex = false
	callbackMethod(MountVolResponses, Auth, MountVolPassCount, totalCount)
	MountVolResponses = nil
	MountVolPassCount = 0
}

func IsMultiVolume(ctx *gin.Context) (model.VolumeParam, bool) {
	req := model.Request{}
	ctx.ShouldBindBodyWith(&req, binding.JSON)
	marshalled, _ := json.Marshal(req.Param)
	volParam := model.VolumeParam{}
	err := json.Unmarshal(marshalled, &volParam)
	if err != nil {
		log.Info("Unmarshalling Error in Implement Async multi volume function: %v", err)
	}
	if volParam.TotalCount > SINGLE_VOLUME_COUNT {
		return volParam, true
	} else {
		return volParam, false
	}
}

func maxCountExceeded(count int) (int, bool) {
	req := model.Request{}
	listXrid, _ := uuid.NewUUID()
	countXrid, _ := uuid.NewUUID()
	_, volList, err := iBoFOS.ListVolume(listXrid.String(), req.Param)
	_, volMaxCount, err := iBoFOS.GetMaxVolumeCount(countXrid.String(), req.Param)
	if err != nil {
		return POS_API_ERROR, true
	}
	volCount := 0
	maxCount := 0
	if volList.Info.(map[string]interface{})["state"].(string) != "NORMAL" {
		return 12090, true
	}
	if volList.Result.Data != nil {
		volumes := volList.Result.Data.(map[string]interface{})["volumes"]
		volCount = len(volumes.([]interface{}))
	}
	maxCount, err = strconv.Atoi(volMaxCount.Result.Data.(map[string]interface{})["count"].(string))
	if err != nil {
		return POS_API_ERROR, true
	}
	if count <= (maxCount - volCount) {
		return 0, false
	}
	return COUNT_EXCEEDED_ERROR, true
}

func ImplementAsyncMultiVolume(ctx *gin.Context, f func(string, interface{}) (model.Request, model.Response, error), volParam *model.VolumeParam, command string) {
	res := model.Response{}
	res.Result.Status, _ = util.GetStatusInfo(10202)

	if status, ok := maxCountExceeded(int(volParam.TotalCount)); ok {
		res.Result.Status, _ = util.GetStatusInfo(status)
		ctx.AbortWithStatusJSON(http.StatusServiceUnavailable, &res)
		return
	}

	if (command == CREATE_VOLUME && CreateVolumeMutex) || (command == MOUNT_VOLUME && MountVolumeMutex) {
		res.Result.Status, _ = util.GetStatusInfo(11030)
		ctx.AbortWithStatusJSON(http.StatusServiceUnavailable, &res)
		return
	}

	switch command {
	case CREATE_VOLUME:
		if CreateVolumeMutex == false {
			CreateVolCh := make(chan model.Response)
			CreateVolumeMutex = true
			go createVolumeWrite(CreateVolCh, ctx, volParam)
			go createVolumeRead(CreateVolCh, ctx.Request.Header.Get(AUTHORIZATION_HEADER), int(volParam.TotalCount))
		}
	case MOUNT_VOLUME: //Optional Functionality for MTool
		if MountVolumeMutex == false {
			MountVolCh := make(chan model.Response)
			MountVolumeMutex = true
			go mountVolumeWrite(MountVolCh, ctx, f, volParam)
			go mountVolumeRead(MountVolCh, ctx.Request.Header.Get(AUTHORIZATION_HEADER), int(volParam.TotalCount))
		}
	}
	//Pending Request 202
	ctx.AbortWithStatusJSON(http.StatusAccepted, &res)
}
