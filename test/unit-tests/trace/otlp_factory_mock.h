#include <gmock/gmock.h>

#include "src/trace/otlp_factory.h"

namespace pos
{

class MockOtlpFactory : public OtlpFactory
{
public:
    using OtlpFactory::OtlpFactory;
    
    MOCK_METHOD(exporter::otlp::OtlpHttpExporter *, CreateOtlpHttpExporter, (exporter::otlp::OtlpHttpExporterOptions opt), (override));
    MOCK_METHOD(sdk::trace::SimpleSpanProcessor * , CreateSimpleSpanProcessor, (exporter::otlp::OtlpHttpExporter *otlpHttpExporter), (override));
    MOCK_METHOD(sdk::trace::TracerProvider *, CreateTracerProvider, (sdk::trace::SimpleSpanProcessor *simpleSpanProcessor, sdk::resource::Resource resource), (override));
    MOCK_METHOD(sdk::resource::Resource, CreateResource, (sdk::resource::ResourceAttributes resourceAttributes), (override));
};

}