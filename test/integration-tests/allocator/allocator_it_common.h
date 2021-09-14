#pragma once

#include "src/array_models/interface/i_array_info.h"

#include <string>

namespace pos
{

const uint32_t ARRAY_ID = 0;

const uint32_t BLK_PER_CHUNK = 32;
const uint32_t CHUNK_PER_STRIPE = 4;
const uint32_t BLK_PER_STRIPE = BLK_PER_CHUNK * CHUNK_PER_STRIPE;

const uint32_t WB_STRIPE = 4;
const uint32_t STRIPE_PER_SEGMENT = 64;
const uint32_t USER_SEGMENT = 64;
const uint32_t USER_STRIPE = STRIPE_PER_SEGMENT * USER_SEGMENT;
const uint32_t USER_BLOCKS = USER_STRIPE * BLK_PER_STRIPE;


class AllocatorItCommon
{
public:
    AllocatorItCommon(void);
    virtual ~AllocatorItCommon(void);
};

}   // namespace pos
