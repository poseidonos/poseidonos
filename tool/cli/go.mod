module cli

go 1.14

replace pnconnector => ./lib/pnconnector

require (
	code.cloudfoundry.org/bytefmt v0.0.0-20210608160410-67692ebc98de
	github.com/c2h5oh/datasize v0.0.0-20200825124411-48ed595a09d2
	github.com/google/uuid v1.3.0
	github.com/labstack/gommon v0.3.0
	github.com/lib/pq v1.0.0
	github.com/spf13/cast v1.4.1
	github.com/spf13/cobra v1.2.1
	github.com/zpatrick/go-bytesize v0.0.0-20170214182126-40b68ac70b6a
	go.opentelemetry.io/otel v1.9.0
	go.opentelemetry.io/otel/exporters/otlp/otlptrace v1.9.0
	go.opentelemetry.io/otel/exporters/otlp/otlptrace/otlptracehttp v1.9.0
	go.opentelemetry.io/otel/sdk v1.9.0
	go.opentelemetry.io/otel/trace v1.9.0
	google.golang.org/genproto v0.0.0-20211118181313-81c1377c94b1
	google.golang.org/grpc v1.46.2
	google.golang.org/protobuf v1.28.0
	pnconnector v0.0.0-00010101000000-000000000000
)
