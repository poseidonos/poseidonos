#!/bin/bash

ROOT_DIR=$(readlink -f $(dirname $0))/
cd $ROOT_DIR/../

if [ -d "../../tool" ]; then
	export GOROOT="$ROOT_DIR../../../lib/go"
	export GOPATH="$ROOT_DIR../../"
	export PATH=$GOPATH/bin:$GOROOT/bin:$PATH
fi

wget -q --tries=1 --timeout=3 --spider http://google.com

if [[ $? -eq 0 ]]; then
	echo "The Internet is available. Use the Internet repository for the libraries."
	export GOPROXY=
	export GOSUMDB=
else
	echo "The Internet is not available. Use an alternative repository for the libraries."
	export GOPROXY="http://10.227.253.89:8081/artifactory/api/go/go"
	export GOSUMDB=off
fi

go mod vendor

export GIT_COMMIT_CLI=$(git rev-list -1 HEAD)
export BUILD_TIME_CLI=$(date +%s)

lib/pnconnector/script/build_resource.sh
go build -mod vendor -tags debug,ssloff -ldflags "-X cli/cmd.GitCommit=$GIT_COMMIT_CLI -X cli/cmd.BuildTime=$BUILD_TIME_CLI"

mv ./cli bin/poseidonos-cli
