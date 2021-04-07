package dagent

import (
	"pnconnector/src/routers/m9k/model"
	"dagent/src/routers/m9k/api"
	"dagent/src/routers/m9k/header"
	"github.com/gin-gonic/gin"
	"github.com/gin-gonic/gin/binding"
)

func CallDagent(ctx *gin.Context, f func(string) (model.Response, error)) {
	req := model.Request{}
	ctx.ShouldBindBodyWith(&req, binding.JSON)
	res, err := f(header.XrId(ctx))
	api.HttpResponse(ctx, res, err)
}
