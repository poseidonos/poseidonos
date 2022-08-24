#include "src/trace/trace_exporter.h"
#include "src/logger/logger.h"
#include "src/include/pos_event_id.h"
#include "opentelemetry/exporters/otlp/otlp_http_exporter.h"
#include "opentelemetry/sdk/trace/simple_processor.h"
#include "opentelemetry/sdk/trace/tracer_provider.h"
#include "opentelemetry/trace/provider.h"

using namespace pos;
using namespace opentelemetry;

TraceExporter::TraceExporter()
: enabled(false) {}

TraceExporter::~TraceExporter()
{}

void
TraceExporter::Init(std::string serviceName, std::string serviceVersion, std::string endPoint)
{
    // Set trace exporter (otlp-http-exporter)
    exporter::otlp::OtlpHttpExporterOptions opts;
    opts.url = endPoint;
    auto otlpHttpExporter = std::unique_ptr<sdk::trace::SpanExporter>(new exporter::otlp::OtlpHttpExporter(opts));

    if (nullptr == otlpHttpExporter)
    {
        POS_TRACE_INFO(POS_EVENT_ID::TRACE_EXPORTER_FAIL, "Failed to initailize trace exporter");
        return;
    }

    // Set trace processor
    auto processor = std::unique_ptr<sdk::trace::SpanProcessor>(
        new sdk::trace::SimpleSpanProcessor(std::move(otlpHttpExporter)));

    if (nullptr == processor)
    {
        POS_TRACE_INFO(POS_EVENT_ID::TRACE_PROCESSOR_FAIL, "Failed to initailize trace processor");
        return;
    }

    // Set trace resource
    auto resourceAttributes = sdk::resource::ResourceAttributes
    {
        {"service.name", serviceName.c_str()},
        {"service.version", serviceVersion.c_str()}
    };
    auto resource = sdk::resource::Resource::Create(resourceAttributes);

    // Set trace provider
    auto provider = nostd::shared_ptr<trace::TracerProvider>(
        new sdk::trace::TracerProvider(std::move(processor), resource));

    if (nullptr == provider)
    {
        POS_TRACE_INFO(POS_EVENT_ID::TRACE_PROVIDER_FAIL, "Failed to initailize trace processor");
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