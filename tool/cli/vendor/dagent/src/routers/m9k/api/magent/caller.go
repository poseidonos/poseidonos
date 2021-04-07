package magent

import (
	"pnconnector/src/routers/m9k/model"
	"dagent/src/routers/m9k/api"
	"github.com/gin-gonic/gin"
	"github.com/gin-gonic/gin/binding"
)

func CallMagent(ctx *gin.Context, f func(interface{}) (model.Response, error), param interface{}) {
	req := model.Request{}
	ctx.ShouldBindBodyWith(&req, binding.JSON)
	req.Param = param
	res, err := f(req.Param)
	api.HttpResponse(ctx, res, err)
}
