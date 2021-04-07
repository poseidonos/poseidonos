package middleware

import (
	"pnconnector/src/log"
	"pnconnector/src/routers/m9k/model"
	"dagent/src/routers/m9k/api"
	"github.com/gin-gonic/gin"
	"github.com/gin-gonic/gin/binding"
	"io"
)

func CheckBody(ctx *gin.Context) {
	body := model.Request{}

	if "GET" != ctx.Request.Method {
		err := ctx.ShouldBindBodyWith(&body, binding.JSON)
		if err != nil && err != io.EOF {
			log.Infof("Request Body Error : %v", err)
			api.BadRequest(ctx, model.Response{}, 10120)
			return
		}
	}
}
