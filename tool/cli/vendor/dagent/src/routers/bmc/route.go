package bmc

import (
	"github.com/gin-gonic/gin"
)

func Route(router *gin.Engine) {
	redfish := router.Group("/redfish")

	// Redfish reroute
	redfish.Any("/*anything", RedirectRedfish)
}
