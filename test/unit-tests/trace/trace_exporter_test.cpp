#include <gtest/gtest.h>

#include "src/trace/trace_exporter.h"
#include "opentelemetry/exporters/otlp/otlp_http_exporter.h"
#include "opentelemetry/sdk/trace/simple_processor.h"
#include "opentelemetry/sdk/trace/tracer_provider.h"
#include "opentelemetry/trace/provider.h"

#include "test/unit-tests/trace/otlp_factory_mock.h"

using namespace pos;
using namespace opentelemetry;

using ::testing::_;
using ::testing::Return;

TEST(TraceExporter, Init_)
{
    // Given
    MockOtlpFactory *mof = new MockOtlpFactory;
    TraceExporter *te = new TraceExporter(mof);

    EXPECT_CALL(*mof, CreateOtlpHttpExporter).WillOnce([mof](exporter::otlp::OtlpHttpExporterOptions opts){
        return mof->OtlpFactory::CreateOtlpHttpExporter(opts);
    });
    EXPECT_CALL(*mof, CreateSimpleSpanProcessor).WillOnce([mof](exporter::otlp::OtlpHttpExporter *otlpHttpExporter){
        return mof->OtlpFactory::CreateSimpleSpanProcessor(otlpHttpExporter);
    });
    EXPECT_CALL(*mof, CreateTracerProvider).WillOnce([mof](sdk::trace::SimpleSpanProcessor *simpleSpanProcessor, sdk::resource::Resource resource){
        return mof->OtlpFactory::CreateTracerProvider(simpleSpanProcessor, resource);
    });
    EXPECT_CALL(*mof, CreateResource).WillOnce([mof](sdk::resource::ResourceAttributes resourceAttributes){
        return mof->OtlpFactory::CreateResource(resourceAttributes);
    });   
    
    // When
    te->Init("TestService", "TestVersion", "test.endpoint");

    // Then
    ASSERT_TRUE(te->IsEnabled());

    delete te;
    delete mof;
}

TEST(TraceExporter, InitWithNoOtlpHttpExporter_)
{    
    // Given
    MockOtlpFactory* mof = new MockOtlpFactory();
    TraceExporter* te = new TraceExporter(mof);

    EXPECT_CALL(*mof, CreateOtlpHttpExporter).WillOnce(Return(nullptr));
    
    // When
    te->Init("TestService", "TestVersion", "test.endpoint");

    // Then
    ASSERT_FALSE(te->IsEnabled());
    
    delete te;
    delete mof;
}

TEST(TraceExporter, InitWithNoSpanProcessor_)
{
    // Given
    MockOtlpFactory *mof = new MockOtlpFactory;
    TraceExporter *te = new TraceExporter(mof);

    EXPECT_CALL(*mof, CreateOtlpHttpExporter).WillOnce([mof](exporter::otlp::OtlpHttpExporterOptions opts){
        return mof->OtlpFactory::CreateOtlpHttpExporter(opts);
    });
    EXPECT_CALL(*mof, CreateSimpleSpanProcessor).WillOnce(Return(nullptr));

    // When
    te->Init("TestService", "TestVersion", "test.endpoint");

    // Then
    ASSERT_FALSE(te->IsEnabled());

    delete te;
    delete mof;
}

TEST(TraceExporter, InitWithNoTracerProvider_)
{
    // Given
    MockOtlpFactory *mof = new MockOtlpFactory;
    TraceExporter *te = new TraceExporter(mof);

    EXPECT_CALL(*mof, CreateOtlpHttpExporter).WillOnce([mof](exporter::otlp::OtlpHttpExporterOptions opts){
        return mof->OtlpFactory::CreateOtlpHttpExporter(opts);
    });
    EXPECT_CALL(*mof, CreateSimpleSpanProcessor).WillOnce([mof](exporter::otlp::OtlpHttpExporter *otlpHttpExporter){
        return mof->OtlpFactory::CreateSimpleSpanProcessor(otlpHttpExporter);
    });
    EXPECT_CALL(*mof, CreateTracerProvider).WillOnce(Return(nullptr));
    EXPECT_CALL(*mof, CreateResource).WillOnce([mof](sdk::resource::ResourceAttributes resourceAttributes){
        return mof->OtlpFactory::CreateResource(resourceAttributes);
    });

    // When
    te->Init("TestService", "TestVersion", "test.endpoint");

    // Then
    ASSERT_FALSE(te->IsEnabled());

    delete te;
    delete mof;
}