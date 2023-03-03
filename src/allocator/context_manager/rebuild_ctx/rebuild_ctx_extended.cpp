#include "src/allocator/context_manager/rebuild_ctx/rebuild_ctx_extended.h"
#include "src/logger/logger.h"
#include "proto/generated/cpp/pos_bc.pb.h"

namespace pos
{
bool
RebuildCtxExtended::ToBytes(char* destBuf)
{
    pos_bc::RebuildCtxExtendedProto proto;
    // build proto with this->* members. currently, do nothing

    size_t effectiveSize = proto.ByteSizeLong();
    if (effectiveSize <= ONSSD_SIZE)
    {
        // normal case.
        int ret = proto.SerializeToArray(destBuf, ONSSD_SIZE);
        if (ret == 0)
        {
            POS_TRACE_ERROR(EID(REBUILDCTX_EXTENDED_FAILED_TO_SERIALIZE), "");
            return false;
        }
        return true;
    }
    else
    {
        // error case. we don't serialize here to avoid memory corruption on destBuf.
        size_t expectedSize = ONSSD_SIZE;
        POS_TRACE_ERROR(EID(REBUILDCTX_EXTENDED_SERIALIZE_OVERFLOW),
            "effectiveSize: {}, expectedSize: {}", effectiveSize, expectedSize);
        return false;
    }

}

bool
RebuildCtxExtended::FromBytes(char* srcBuf)
{
    pos_bc::RebuildCtxExtendedProto proto;
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
        
        POS_TRACE_ERROR(EID(REBUILDCTX_EXTENDED_DESERIALIZE_CORRUPTION),
            "effectiveSize: {}, expectedSize: {}", effectiveSize, expectedSize);
        return false;
    }

}

size_t
RebuildCtxExtended::SerializedSize()
{
    return ONSSD_SIZE;
}
}