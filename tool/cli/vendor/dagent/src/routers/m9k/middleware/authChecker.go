package middleware

import (
	"github.com/gin-gonic/gin"
	//"pnconnector/mongodb"
	//"dagent/src/routers/m9k/api"
	//"github.com/dgrijalva/jwt-go"
)

func CheckBasicAuth() gin.HandlerFunc {
	var user gin.Accounts
	//user = mongodb.ReadAllUserIdPassword()
	return gin.BasicAuth(user)
}

func CheckAPIActivate(ctx *gin.Context) {
	//username, _, _ := ctx.Request.BasicAuth()
	//user := mongodb.ReadUserCollectionById(username)
	//if user.Active == false {
	//	api.MakeUnauthorized(ctx, 10110)
	//	return
	//}
}

func JWTAuth(ctx *gin.Context) {
	// ToDo: Implement
}
