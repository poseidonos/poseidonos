package otelmgr

import (
	"cli/cmd/globals"
	"context"
	"fmt"
	"go.opentelemetry.io/otel"
	"go.opentelemetry.io/otel/attribute"
	"go.opentelemetry.io/otel/exporters/otlp/otlptrace"
	"go.opentelemetry.io/otel/exporters/otlp/otlptrace/otlptracehttp"
	"go.opentelemetry.io/otel/sdk/resource"
	tracesdk "go.opentelemetry.io/otel/sdk/trace"
	semconv "go.opentelemetry.io/otel/semconv/v1.12.0"
	"time"
)

type OtelManager struct {
	tp  *tracesdk.TracerProvider
	ctx context.Context
}

var instance *OtelManager

func GetOtelManagerInstance() *OtelManager {

	if instance == nil {
		instance = new(OtelManager)
	}
	return instance
}

func (mgr *OtelManager) InitTracerProvider(ctx context.Context, servName string, version string) error {

	if !globals.EnableOtel {
		return nil
	}

	client := otlptracehttp.NewClient(
		otlptracehttp.WithEndpoint(globals.OtlpServerAddress),
		otlptracehttp.WithInsecure(),
	)
	exp, err := otlptrace.New(ctx, client)
	if err != nil {
		return fmt.Errorf("creating OTLP trace exporter: %w", err)
	}

	tp := tracesdk.NewTracerProvider(
		// Always be sure to batch in production.
		tracesdk.WithBatcher(exp),
		// Record information about this application in a Resource.
		tracesdk.WithResource(resource.NewWithAttributes(
			semconv.SchemaURL,
			semconv.ServiceNameKey.String(servName),
			attribute.String("version", version),
		)),
	)

	otel.SetTracerProvider(tp)

	mgr.tp = tp
	mgr.ctx = ctx

	return nil
}

func (mgr *OtelManager) Shutdown() error {

	if !globals.EnableOtel {
		return nil
	}

	ctx, cancel := context.WithTimeout(mgr.ctx, time.Second*5)
	defer cancel()
	if err := mgr.tp.Shutdown(ctx); err != nil {
		return fmt.Errorf("fail to shutdown OTel: %w", err)
	}
	return nil
}

func (mgr *OtelManager) GetRootContext() context.Context {

	if !globals.EnableOtel {
		return nil
	}

	return mgr.ctx
}

func (mgr *OtelManager) SetTracer(appName string, funcName string) *Tracer {

	if !globals.EnableOtel {
		return nil
	}

	t := Tracer{}
	t.SetTrace(mgr.ctx, appName, funcName)
	return &t
}
