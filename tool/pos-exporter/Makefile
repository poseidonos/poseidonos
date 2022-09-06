PROTO_DIR = ../../proto
PROTO_OUTPUT_DIR = api

APP = pos-exporter
GO = /usr/local/go/bin/go
PROTOC = protoc

GOPATH = $(shell ${GO} env | grep GOPATH | awk -F"\"" '{print $$2}')
GOROOT = $(shell ${GO} env | grep GOROOT | awk -F"\"" '{print $$2}')

all: $(APP)

$(APP): protobuf
	$(GO) build -o $@

protobuf:
	$(GO) install google.golang.org/protobuf/cmd/protoc-gen-go@v1.27.1
	$(GO) install google.golang.org/grpc/cmd/protoc-gen-go-grpc@v1.1.0
	@`[ -d $(PROTO_OUTPUT_DIR) ] || mkdir -p $(PROTO_OUTPUT_DIR)`
	@PATH=${PATH}:$(GOROOT)/bin:$(GOPATH)/bin $(PROTOC) --go_out=$(PROTO_OUTPUT_DIR) --go_opt=paths=source_relative --go-grpc_out=$(PROTO_OUTPUT_DIR) --go-grpc_opt=paths=source_relative -I $(PROTO_DIR) metric.proto

clean:
	rm -rf $(APP)
