TOP := $(abspath $(CURDIR)/../../..)
SRCS += $(wildcard *.cpp)
include $(TOP)/Makefile.rules

proto:
	/home/ibof/grpc-bin/bin/protoc --cpp_out=.  --grpc_out=. --plugin=protoc-gen-grpc=/home/ibof/grpc-bin/bin/grpc_cpp_plugin telemetry.proto
	mv telemetry.pb.cc telemetry.pb.cpp
	mv telemetry.grpc.pb.cc telemetry.grpc.pb.cpp

proto_clean:
	rm -f *.o telemetry.pb.h telemetry.pb.cpp telemetry.grpc.pb.h telemetry.grpc.pb.cpp
