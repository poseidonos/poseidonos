#include "src/allocator/context_manager/allocator_ctx/allocator_ctx_extended.h"
#include "src/logger/logger.h"
#include "proto/generated/cpp/pos_bc.pb.h"

namespace pos
{
bool
AllocatorCtxExtended::ToBytes(char* destBuf)
{
    pos_bc::AllocatorCtxExtendedProto proto;
    // build proto with this->* members. currently, do nothing

    size_t effectiveSize = proto.ByteSizeLong();
    if (effectiveSize <= ONSSD_SIZE)
    {
        // normal case.
        int ret = proto.SerializeToArray(destBuf, ONSSD_SIZE);
        if (ret == 0)
        {
            POS_TRACE_ERROR(EID(ALLOCATORCTX_EXTENDED_FAILED_TO_SERIALIZE), "");
            return false;
        }

        // fill zeros up to ONSSD_SIZE bytes-position
        for(unsigned int bytePos = effectiveSize; bytePos < ONSSD_SIZE; bytePos ++)
        {
            destBuf[bytePos] = 0;
        }
        return true;
    }
    else
    {
        // error case. we don't serialize here to avoid memory corruption on destBuf.
        size_t expectedSize = ONSSD_SIZE;
        POS_TRACE_ERROR(EID(ALLOCATORCTX_EXTENDED_SERIALIZE_OVERFLOW),
            "effectiveSize: {}, expectedSize: {}", effectiveSize, expectedSize);
        return false;
    }

}

bool
AllocatorCtxExtended::FromBytes(char* srcBuf)
{
    pos_bc::AllocatorCtxExtendedProto proto;
    int ret = proto.ParseFromArray(srcBuf, ONSSD_SIZE);
    if (ret == 0)
    {
        // Ignore this for now. Please refer to the comment in SegmentInfoData::FromBytes().
    } 

    size_t effectiveSize = proto.ByteSizeLong();
    if (effectiveSize <= ONSSD_SIZE)
    {
        // normal case. update internal member variables with proto
        // currently, do nothing since SegmentCtxExtended doesn't have any fields.
        return true;
    }
    else
    {
        size_t expectedSize = ONSSD_SIZE;
        
        POS_TRACE_ERROR(EID(ALLOCATORCTX_EXTENDED_DESERIALIZE_CORRUPTION),
            "effectiveSize: {}, expectedSize: {}", effectiveSize, expectedSize);
        return false;
    }

}

size_t
AllocatorCtxExtended::SerializedSize()
{
    return ONSSD_SIZE;
}
}