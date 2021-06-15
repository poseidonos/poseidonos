if(POS_BUILD_WITH_TELEMETRY)
    POS_ADD_INTEGRATION_TEST(telemetry.grpc.pb_test telemetry.grpc.pb_test.cpp)
endif(POS_BUILD_WITH_TELEMETRY)