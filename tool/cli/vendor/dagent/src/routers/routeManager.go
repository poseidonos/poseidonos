package routers

import (
	"dagent/src/routers/bmc"
	"dagent/src/routers/m9k"
	"github.com/gin-gonic/gin"
)

// Init Router Info
func InitRouter() *gin.Engine {
	router := gin.New()
	router.Use(gin.Logger())
	router.Use(gin.Recovery())

	bmc.Route(router)
	m9k.Route(router)

	return router
}
