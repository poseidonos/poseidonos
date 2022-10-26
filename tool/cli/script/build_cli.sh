#!/bin/bash

ROOT_DIR=$(readlink -f $(dirname $0))/..
cd $ROOT_DIR

GO=/usr/local/go/bin/go

export POS_CLI_VERSION=1.0.1
export GIT_COMMIT_CLI=$(git rev-list -1 HEAD)
export BUILD_TIME_CLI=$(date +%s)

GOPATH=$(${GO} env | grep GOPATH | awk -F"\"" '{print $2}')
GOROOT=$(${GO} env | grep GOROOT | awk -F"\"" '{print $2}')
export PATH=${PATH}:${GOROOT}/bin:${GOPATH}/bin

mkdir -p api
protoc --go_out=api --go_opt=paths=source_relative \
    --go-grpc_out=api --go-grpc_opt=paths=source_relative \
    -I $ROOT_DIR/../../proto cli.proto

# Build CLI binary
lib/pnconnector/script/build_resource.sh

# Patching protojson
echo "Installing and patching protojson..."
rm -rf ${GOPATH}/pkg/mod/google.golang.org/protobuf@v1.28.0
${GO} mod download
patch -f --silent ${GOPATH}/pkg/mod/google.golang.org/protobuf@v1.28.0/encoding/protojson/encode.go ./patches/protojson_encode.patch
if [ $? -ne 0 ];
then
    echo "patching protojson failed!"
    exit 1
fi

# Build CLI
${GO} build -tags debug,ssloff -ldflags "-X cli/cmd.PosCliVersion=$POS_CLI_VERSION -X cli/cmd.GitCommit=$GIT_COMMIT_CLI -X cli/cmd.BuildTime=$BUILD_TIME_CLI -X cli/cmd.RootDir=$ROOT_DIR"
mv ./cli bin/poseidonos-cli

# Build CLI markdown and manpage documents
cd docs
rm markdown manpage -rf
mkdir -p markdown manpage
${GO} build -o ../bin/gen_md gen_md.go
${GO} build -o ../bin/gen_man gen_man.go
chmod +x ../bin/gen_md ../bin/gen_man
../bin/gen_md
../bin/gen_man

# Return to the cli directory
cd ..

