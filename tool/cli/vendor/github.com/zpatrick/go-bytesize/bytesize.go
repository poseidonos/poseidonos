package bytesize

import (
	"fmt"
	"strings"
)

// A Bytesize represents a single byte as a float64
type Bytesize float64

// bytes sizes generally used for computer storage and memory
const (
	B  Bytesize = 1
	KB Bytesize = 1000 * B
	MB Bytesize = 1000 * KB
	GB Bytesize = 1000 * MB
	TB Bytesize = 1000 * GB
	PB Bytesize = 1000 * TB
	EB Bytesize = 1000 * PB

	KiB Bytesize = 1024 * B
	MiB Bytesize = 1024 * KiB
	GiB Bytesize = 1024 * MiB
	TiB Bytesize = 1024 * GiB
	PiB Bytesize = 1024 * TiB
	EiB Bytesize = 1024 * PiB
)

// Bytes returns the number of Bytes (B) in b
func (b Bytesize) Bytes() float64 {
	return float64(b)
}

// Kilobytes returns the number of Kilobytes (KB) in b
func (b Bytesize) Kilobytes() float64 {
	return float64(b / KB)
}

// Megabytes returns the number of Megabytes (MB) in b
func (b Bytesize) Megabytes() float64 {
	return float64(b / MB)
}

// Gigabytes returns the number of Gigabytes (GB) in b
func (b Bytesize) Gigabytes() float64 {
	return float64(b / GB)
}

// Terabytes returns the number of Terabytes (TB) in b
func (b Bytesize) Terabytes() float64 {
	return float64(b / TB)
}

// Petabytes returns the number of Petabytes (PB) in b
func (b Bytesize) Petabytes() float64 {
	return float64(b / PB)
}

// Exabytes returns the number of Exabytes (EB) in b
func (b Bytesize) Exabytes() float64 {
	return float64(b / EB)
}

// Kibibytes returns the number of Kibibytes (KiB) in b
func (b Bytesize) Kibibytes() float64 {
	return float64(b / KiB)
}

// Mebibytes returns the number of Mebibbytes (MiB) in b
func (b Bytesize) Mebibytes() float64 {
	return float64(b / MiB)
}

// Gibibytes returns the number of Gibibytes (GiB) in b
func (b Bytesize) Gibibytes() float64 {
	return float64(b / GiB)
}

// Tebibytes returns the number of Tebibytes (TiB) in b
func (b Bytesize) Tebibytes() float64 {
	return float64(b / TiB)
}

// Pebibytes returns the number of Pebibytes (PiB) in b
func (b Bytesize) Pebibytes() float64 {
	return float64(b / PiB)
}

// Exbibytes returns the number of Exbibytes (EiB) in b
func (b Bytesize) Exbibytes() float64 {
	return float64(b / EiB)
}

// Format returns a textual representation of the Bytesize value formatted
// according to layout, which defines the format by specifying an abbreviation.
// Abbreviations are not case-sensitive.
//
// Valid abbreviations are as follows:
//	B (Bytesizes)
//	KB (Kilobytes)
//	MB (Megabytes)
//	GB (Gigabytes)
//	TB (Terabytes)
//	PB (Petabytes)
//	EB (Exabytes)
//	KiB (Kibibytes)
//	MiB (Mebibytes)
//	GiB (Gibibytes)
//	TiB (Tebibtyes)
//	PiB (Pebibytes)
//	EiB (Exbibyte)
func (b Bytesize) Format(layout string) string {
	switch strings.ToLower(layout) {
	case "b":
		return fmt.Sprintf("%gB", b.Bytes())
	case "kb":
		return fmt.Sprintf("%gKB", b.Kilobytes())
	case "mb":
		return fmt.Sprintf("%gMB", b.Megabytes())
	case "gb":
		return fmt.Sprintf("%gGB", b.Gigabytes())
	case "tb":
		return fmt.Sprintf("%gTB", b.Terabytes())
	case "pb":
		return fmt.Sprintf("%gPB", b.Petabytes())
	case "eb":
		return fmt.Sprintf("%gEB", b.Exabytes())
	case "kib":
		return fmt.Sprintf("%gKiB", b.Kibibytes())
	case "mib":
		return fmt.Sprintf("%gMiB", b.Mebibytes())
	case "gib":
		return fmt.Sprintf("%gGiB", b.Gibibytes())
	case "tib":
		return fmt.Sprintf("%gTiB", b.Tebibytes())
	case "pib":
		return fmt.Sprintf("%gPiB", b.Pebibytes())
	case "eib":
		return fmt.Sprintf("%gEiB", b.Exbibytes())
	default:
		return fmt.Sprintf("%%!%s(byte=%g)", layout, b)
	}
}
