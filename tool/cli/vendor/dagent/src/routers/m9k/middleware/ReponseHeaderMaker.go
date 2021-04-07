package middleware

import (
	"github.com/gin-gonic/gin"
)

func ResponseHeader(ctx *gin.Context) {
	xrid := ctx.GetHeader("X-request-Id")
	ctx.Header("X-request-Id", xrid)
}
