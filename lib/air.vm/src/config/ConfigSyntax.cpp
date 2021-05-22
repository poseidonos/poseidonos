
#include "src/config/ConfigSyntax.h"

// ConfigSyntax.cpp file do nothing but include ConfigSyntax.h only.
// ConfigSyntax.h would never be included except here.
// So when build this file, compiler can check the AIR config file's syntax is valid or not.
// If invalid, static_assert(...) will be called at compile-time.
