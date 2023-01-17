module cli

go 1.18

replace pnconnector => ./lib/pnconnector

replace kouros => ../kouros

require (
	code.cloudfoundry.org/bytefmt v0.0.0-20211005130812-5bb3c17173e5
	github.com/c2h5oh/datasize v0.0.0-20220606134207-859f65c6625b
	github.com/google/uuid v1.3.0
	github.com/labstack/gommon v0.4.0
	github.com/lib/pq v1.10.7
	github.com/spf13/cast v1.5.0
	github.com/spf13/cobra v1.6.1
	github.com/zpatrick/go-bytesize v0.0.0-20170214182126-40b68ac70b6a
	go.opentelemetry.io/otel v1.11.2
	go.opentelemetry.io/otel/exporters/otlp/otlptrace v1.11.2
	go.opentelemetry.io/otel/exporters/otlp/otlptrace/otlptracehttp v1.11.2
	go.opentelemetry.io/otel/sdk v1.11.2
	go.opentelemetry.io/otel/trace v1.11.2
	google.golang.org/genproto v0.0.0-20230113154510-dbe35b8444a5
	google.golang.org/grpc v1.52.0
	google.golang.org/protobuf v1.28.1
	kouros v0.0.0-00010101000000-000000000000
	pnconnector v0.0.0-00010101000000-000000000000
)

require (
	github.com/cenkalti/backoff/v4 v4.2.0 // indirect
	github.com/cpuguy83/go-md2man/v2 v2.0.2 // indirect
	github.com/go-logr/logr v1.2.3 // indirect
	github.com/go-logr/stdr v1.2.2 // indirect
	github.com/golang/protobuf v1.5.2 // indirect
	github.com/grpc-ecosystem/grpc-gateway/v2 v2.7.0 // indirect
	github.com/inconshreveable/mousetrap v1.0.1 // indirect
	github.com/juju/errors v1.0.0 // indirect
	github.com/mattn/go-colorable v0.1.11 // indirect
	github.com/mattn/go-isatty v0.0.14 // indirect
	github.com/russross/blackfriday/v2 v2.1.0 // indirect
	github.com/sirupsen/logrus v1.9.0 // indirect
	github.com/spf13/pflag v1.0.5 // indirect
	github.com/valyala/bytebufferpool v1.0.0 // indirect
	github.com/valyala/fasttemplate v1.2.1 // indirect
	go.opentelemetry.io/otel/exporters/otlp/internal/retry v1.11.2 // indirect
	go.opentelemetry.io/proto/otlp v0.19.0 // indirect
	golang.org/x/net v0.4.0 // indirect
	golang.org/x/sys v0.3.0 // indirect
	golang.org/x/text v0.5.0 // indirect
	gopkg.in/yaml.v2 v2.4.0 // indirect
	gopkg.in/yaml.v3 v3.0.1 // indirect
)
