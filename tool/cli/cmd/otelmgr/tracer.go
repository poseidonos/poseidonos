package otelmgr

import (
	"cli/cmd/globals"
	"context"
	"go.opentelemetry.io/otel"
	"go.opentelemetry.io/otel/trace"
)

type Tracer struct {
	ctx  context.Context
	span trace.Span
}

func NewTracer() *Tracer {
	return new(Tracer)
}

func (mgr *Tracer) GetContext() context.Context {
	return mgr.ctx
}

func (mgr *Tracer) SetTrace(ctx context.Context, appName string, funcName string) {

	if globals.EnableOtel {
		mgr.ctx, mgr.span = otel.Tracer(appName).Start(ctx, funcName)
	}
}

func (mgr *Tracer) RecordError(err error) {

	if globals.EnableOtel {
		mgr.span.RecordError(err)
	}
}

func (mgr *Tracer) Release() {

	if globals.EnableOtel {
		mgr.span.End()
	}
}
