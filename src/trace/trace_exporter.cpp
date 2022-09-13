#include "src/trace/otlp_factory.h"
#include "src/trace/trace_exporter.h"
#include "src/logger/logger.h"
#include "src/include/pos_event_id.h"
#include "opentelemetry/trace/provider.h"

using namespace opentelemetry;

namespace pos
{

TraceExporter::TraceExporter(OtlpFactory *otlpFactory)
: enabled(false), otlpFactory(otlpFactory)
{}

TraceExporter::~TraceExporter()
{}

void
TraceExporter::Init(std::string serviceName, std::string serviceVersion, std::string endPoint)
{
    // Set trace exporter (otlp-http-exporter)
    exporter::otlp::OtlpHttpExporterOptions opts;
    opts.url = endPoint;
    auto otlpHttpExporter = otlpFactory->CreateOtlpHttpExporter(opts);

    if (nullptr == otlpHttpExporter)
    {
        POS_TRACE_INFO(EID(TRACE_EXPORTER_FAIL), "Failed to initailize trace exporter");
        return;
    }

    // Set trace processor
    auto processor = otlpFactory->CreateSimpleSpanProcessor(otlpHttpExporter);

    if (nullptr == processor)
    {
        POS_TRACE_INFO(EID(TRACE_PROCESSOR_FAIL), "Failed to initailize trace processor");
        return;
    }

    // Set trace resource
    auto resourceAttributes = sdk::resource::ResourceAttributes
    {
        {"service.name", serviceName.c_str()},
        {"service.version", serviceVersion.c_str()}
    };
    
    auto resource = otlpFactory->CreateResource(resourceAttributes);

    // Set trace provider
    auto provider = nostd::shared_ptr<trace::TracerProvider>(otlpFactory->CreateTracerProvider(processor, resource));

    if (nullptr == provider)
    {
        POS_TRACE_INFO(EID(TRACE_PROVIDER_FAIL), "Failed to initailize trace processor");
        return;        
    }

    // Set trace provider as global
    trace::Provider::SetTracerProvider(provider);

    _Enable();
}

bool
TraceExporter::IsEnabled(void)
{
    return enabled;
}

void
TraceExporter::_Enable(void)
{
    enabled = true;
}

}