CURPATH := $(abspath $(CURDIR))

PROTO_DIR="$(CURPATH)/../../../proto"
PROTO_CPP_GENERATED_DIR="$(CURPATH)/metric"

CXX = g++
LDFLAGS += -O2 -std=c++11
LDFLAGS += -L/usr/local/lib `pkg-config --libs protobuf grpc++` -Wl,--no-as-needed -lgrpc++_reflection -Wl,--as-needed -lprotobuf -lpthread -ldl

all: gen_proto metric.pb.o metric.grpc.pb.o pos_metric_publisher.o test_metric_add_time.o
	$(CXX) metric.pb.o metric.grpc.pb.o pos_metric_publisher.o test_metric_add_time.o  -o test_metric_add_time $(LDFLAGS) 

pos_metric_publisher.o: pos_metric_publisher.cpp
	$(CXX) -c pos_metric_publisher.cpp -o pos_metric_publisher.o $(LDFLAGS)

test_metric_add_time.o: test_metric_add_time.cpp
	$(CXX)  -c test_metric_add_time.cpp -o test_metric_add_time.o $(LDFLAGS)

metric.grpc.pb.o: metric/metric.grpc.pb.cc
	$(CXX)  -c metric/metric.grpc.pb.cc -o metric.grpc.pb.o $(LDFLAGS)

metric.pb.o: metric/metric.pb.cc
	$(CXX)  -c metric/metric.pb.cc -o metric.pb.o $(LDFLAGS)

gen_proto:
	@`[ -d $(PROTO_CPP_GENERATED_DIR) ] || mkdir -p $(PROTO_CPP_GENERATED_DIR)`
	protoc --cpp_out=$(PROTO_CPP_GENERATED_DIR) --grpc_out=$(PROTO_CPP_GENERATED_DIR) --plugin=protoc-gen-grpc=/usr/local/bin/grpc_cpp_plugin --proto_path=$(PROTO_DIR) $(PROTO_DIR)/*.proto

