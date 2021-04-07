package header

import "github.com/gin-gonic/gin"

func XrId(c *gin.Context) string {
	return c.GetHeader("X-request-Id")
}
