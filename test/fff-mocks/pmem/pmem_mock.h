#ifndef PMEM_MOCK_H_
#define PMEM_MOCK_H_

#include <libpmem.h>

#include "fff/fff.h"
DECLARE_FAKE_VALUE_FUNC(void *, pmem_map_file, const char*, size_t, int, mode_t, size_t*, int*);

#endif
