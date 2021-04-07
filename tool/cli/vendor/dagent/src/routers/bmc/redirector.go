package bmc

import (
	"pnconnector/src/setting"
	"crypto/tls"
	"github.com/gin-gonic/gin"
	"net/http"
	"net/http/httputil"
)

func RedirectRedfish(ctx *gin.Context) {
	http.DefaultTransport.(*http.Transport).TLSClientConfig = &tls.Config{InsecureSkipVerify: true}

	director := func(req *http.Request) {
		req.URL.Scheme = "https"
		req.URL.Host = setting.Config.Server.BMC.IP
	}

	reverseProxy := &httputil.ReverseProxy{Director: director}
	reverseProxy.ServeHTTP(ctx.Writer, ctx.Request)
}
