
#ifndef AIR_PROTOCOL_H
#define AIR_PROTOCOL_H

#include <cstdint>

#include "src/lib/Casting.h"

namespace protocol
{
namespace internal
{
enum class Type1 : uint32_t
{
    INPUT_TO_POLICY = 0x00000000,
    POLICY_TO_COLLECTION = 0x00000001,
    COLLECTION_TO_OUTPUT = 0x00000002,
    POLICY_TO_OUTPUT = 0x00000003,

    COUNT
};

enum class Type2 : uint32_t
{
    LINK_CHAIN = 0x00000000,

    SET_STREAMING_INTERVAL = 0x00000001,

    ENABLE_AIR = 0x00010000,

    ENABLE_NODE = 0x00010001,
    ENABLE_NODE_WITH_RANGE = 0x00010002,
    ENABLE_NODE_WITH_DEPTH = 0x00010003,
    ENABLE_NODE_WITH_CHILD = 0x00010004,
    ENABLE_NODE_WITH_GROUP = 0x00010005,
    ENABLE_NODE_ALL = 0x00010006,

    INITIALIZE_NODE = 0x00010011,
    INITIALIZE_NODE_WITH_RANGE = 0x00010012,
    INITIALIZE_NODE_WITH_DEPTH = 0x00010013,
    INITIALIZE_NODE_WITH_CHILD = 0x00010014,
    INITIALIZE_NODE_WITH_GROUP = 0x00010015,
    INITIALIZE_NODE_ALL = 0x00010016,

    SET_SAMPLING_RATE = 0x00010101,
    SET_SAMPLING_RATE_WITH_RANGE = 0x00010102,
    SET_SAMPLING_RATE_WITH_DEPTH = 0x00010103,
    SET_SAMPLING_RATE_WITH_CHILD = 0x00010104,
    SET_SAMPLING_RATE_WITH_GROUP = 0x00010105,
    SET_SAMPLING_RATE_ALL = 0x00010106,

    COUNT
};

enum class Type2_Upper : uint16_t
{
    OUTPUT = 0x0000,
    COLLECTION = 0x0001
};

enum class Type2_Lower : uint16_t
{
    LINK_CHAIN = 0x0000,
    ENABLE_MODULE = 0x0001,
    SET_STREAMING_INTERVAL = 0x0001,
    //    SET_MANAGEMENT_SERVER           = 0x0001,

    ENABLE_NODE = 0x0001,
    ENABLE_NODE_WITH_RANGE = 0x0002,
    ENABLE_NODE_WITH_DEPTH = 0x0003,
    ENABLE_NODE_WITH_CHILD = 0x0004,
    ENABLE_NODE_WITH_GROUP = 0x0005,
    ENABLE_NODE_ALL = 0x0006,

    INITIALIZE_NODE = 0x0011,
    INITIALIZE_NODE_WITH_RANGE = 0x0012,
    INITIALIZE_NODE_WITH_DEPTH = 0x0013,
    INITIALIZE_NODE_WITH_CHILD = 0x0014,
    INITIALIZE_NODE_WITH_GROUP = 0x0015,
    INITIALIZE_NODE_ALL = 0x0016,

    SET_SAMPLING_RATE = 0x0101,
    SET_SAMPLING_RATE_WITH_RANGE = 0x0102,
    SET_SAMPLING_RATE_WITH_DEPTH = 0x0103,
    SET_SAMPLING_RATE_WITH_CHILD = 0x0104,
    SET_SAMPLING_RATE_WITH_GROUP = 0x0105,
    SET_SAMPLING_RATE_ALL = 0x0106,

    GET_CONFIG = 0x0201,
};

enum class InSubject : uint32_t
{
    TO_POLICY = 0,
    TO_OUTPUT,

    COUNT
};

enum class PolicySubject : uint32_t
{
    TO_COLLECTION = 0,
    TO_OUTPUT,

    COUNT
};

enum class CollectionSubject : uint32_t
{
    TO_OUTPUT = 0,

    COUNT
};

enum class ChainHandler : uint32_t
{
    INPUT = 0,
    POLICY,
    COLLECTION,
    OUTPUT,
    PROCESS,
    STREAM,
    SWITCHGEAR,
    PREPROCESS,
    DETECT,

    COUNT
};

enum class PreprocessOption : int32_t
{
    NORMAL = 0,
    FORCED
};

enum class OnOff : uint32_t
{
    OFF = 0,
    ON,

    COUNT
};

static const uint32_t k_max_subject_size{
    (to_dtype(InSubject::COUNT) > to_dtype(PolicySubject::COUNT))
        ? to_dtype(InSubject::COUNT)
        : to_dtype(PolicySubject::COUNT)};

} // namespace internal

namespace external
{
} // namespace external

} // namespace protocol

namespace pi = protocol::internal;
namespace pe = protocol::external;

#endif // AIR_PROTOCOL_H
