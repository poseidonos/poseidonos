#include "pmem_mock.h"

DEFINE_FFF_GLOBALS

DEFINE_FAKE_VALUE_FUNC(void *, pmem_map_file, const char*, size_t, int, mode_t, size_t*, int*);
//FAKE_VALUE_FUNC2(int, pmem_unmap, void *, size_t);