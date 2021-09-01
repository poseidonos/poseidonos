PROTO_DIR = ../../proto
PROTO_OUTPUT_DIR = proto

APP = collector

all: $(APP)

$(APP): protobuf
	go build -o $@

clean:
	rm -rf $(APP) proto

protobuf:
	go mod vendor 
	@`[ -d $(PROTO_OUTPUT_DIR) ] || mkdir -p $(PROTO_OUTPUT_DIR)`
	protoc --go_out=$(PROTO_OUTPUT_DIR) --go-grpc_out=$(PROTO_OUTPUT_DIR) -I $(PROTO_DIR) telemetry.proto

