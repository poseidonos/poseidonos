# Go Bytesize

[![MIT License](https://img.shields.io/badge/license-MIT-blue.svg)](https://github.com/zpatrick/go-bytesize/blob/master/LICENSE)
[![Go Report Card](https://goreportcard.com/badge/github.com/zpatrick/go-bytesize)](https://goreportcard.com/report/github.com/zpatrick/go-bytesize)
[![Go Doc](https://godoc.org/github.com/zpatrick/go-bytesize?status.svg)](https://godoc.org/github.com/zpatrick/go-bytesize)


## Overview
Go Bytesize is a utility package for working with common multiples of bytes in Go, such as: Bytes (B), Kilobytes (KB), Megabytes (MB), Gigabytes (GB), Terabytes (TB), Petabytes (PB), Exabytes (EB), Kibibytes (Kib), Mebibytes (MiB), Gibibytes (GiB), Tebibytes (TiB), Pebibytes (PiB), and Exbibytes (EiB).  

## Example
```
package main

import (
        "fmt"
        "github.com/zpatrick/go-bytesize"
)

func main() {
        b := bytesize.Bytesize(10000)
        fmt.Printf("%g bytes is: %g KB and %g MB\n", b, b.Kilobytes(), b.Megabytes())

        b = bytesize.TB * 2
        fmt.Printf("2 Terabytes is %g Gibibytes\n", b.Gibibytes())

        b = bytesize.Bytesize(1000000)
        fmt.Printf("%g bytes is %s\n", b, b.Format("mb"))
}
```

Output:
```
10000 bytes is: 10 KB and 0.01 MB
2 Terabytes is 1862.645149230957 Gibibytes
1e+06 bytes is 1MB
```

## License
This work is published under the MIT license.
Please see the `LICENSE` file for details.
