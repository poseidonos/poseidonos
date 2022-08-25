module cli

go 1.14

replace pnconnector => ./lib/pnconnector

require (
	code.cloudfoundry.org/bytefmt v0.0.0-20210608160410-67692ebc98de
	github.com/alecthomas/jsonschema v0.0.0-20210920000243-787cd8204a0d // indirect
	github.com/c2h5oh/datasize v0.0.0-20200825124411-48ed595a09d2
	github.com/google/uuid v1.3.0
	github.com/labstack/gommon v0.3.0
	github.com/spf13/cast v1.4.1
	github.com/spf13/cobra v1.2.1
	github.com/zpatrick/go-bytesize v0.0.0-20170214182126-40b68ac70b6a
	pnconnector v0.0.0-00010101000000-000000000000
)
