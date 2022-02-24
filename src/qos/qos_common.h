/*
 *   BSD LICENSE
 *   Copyright (c) 2021 Samsung Electronics Corporation
 *   All rights reserved.
 *
 *   Redistribution and use in source and binary forms, with or without
 *   modification, are permitted provided that the following conditions
 *   are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in
 *       the documentation and/or other materials provided with the
 *       distribution.
 *     * Neither the name of Samsung Electronics Corporation nor the names of
 *       its contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *   OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "src/device/base/ublock_device.h"
#include "src/event_scheduler/event.h"
#include "src/include/array_mgmt_policy.h"
#include "src/include/backend_event.h"
#include "src/volume/volume_list.h"

#define TIME_1_SEC_IN_USEC (1000 * 1000)
#define IBOF_QOS_TIMESLICE_IN_USEC (10000)
#define PARAMETER_COLLECTION_INTERVAL (TIME_1_SEC_IN_USEC / IBOF_QOS_TIMESLICE_IN_USEC)
#define M_MAX_REACTORS (128)
#define M_MAX_SUBSYSTEMS (1024)
#define M_KBYTES (1024ULL)
#define M_VALID_ENTRY (1)
#define M_INVALID_ENTRY (0)
#define M_NOBW_IN_MS (0)
#define M_NO_MAP (0)
#define M_DEFAULT_DEFICIET_WEIGHT (0)
#define M_RESET_TO_ZERO (0)
#define M_UINT32MAX (4294967295)
#define M_WEIGHT_CHANGE_INDEX (1)
#define M_MAX_NVRAM_STRIPES (1023)
#define M_PING_PONG_BUFFER (2)
#define M_QOS_CORRECTION_CYCLE (100)
#define M_QOS_AVERAGE_WINDOW_SIZE (100)
#define M_STRIPES_CONSUMED_HIGH_THRESHOLD (1000)
#define MAX_IO_WORKER 8 // Currently this is hardcoded, will be taken from affinity manager in next revision

#define MAX_THROTTLING_RATE (0.99)
#define POLLING_FREQ_PER_QOS_SLICE (20)
#define BASIC_BW_UNIT (1024ULL * 1024ULL * 10 / PARAMETER_COLLECTION_INTERVAL)
#define BASIC_IOPS_UNIT (1024ULL * 10 / PARAMETER_COLLECTION_INTERVAL)
#define UNIT_GLOBAL_RATE (0.01)
#define UNIT_VOLUME_RATE (0.01)
#define MIN_GUARANTEED_THROTTLING_RATE (1.1)
#define GLOBAL_INCREASE_COEFFICIENT (2)
#define MIN_GUARANTEED_INCREASE_COEFFICIENT (32)

const int MAX_REACTOR_WORKER = (M_MAX_REACTORS > MAX_IO_WORKER) ? M_MAX_REACTORS : MAX_IO_WORKER;
const int MAX_VOLUME_EVENT = (MAX_VOLUME_COUNT > pos::BackendEvent_Count) ? MAX_VOLUME_COUNT : pos::BackendEvent_Count;

const uint8_t PRIORITY_INVALID = 0;
const uint8_t PRIORITY_HIGHEST = 1;
const uint8_t PRIORITY_HIGHER = 2;
const uint8_t PRIORITY_HIGH = 3;
const uint8_t PRIORITY_MEDIUM = 4;
const uint8_t PRIORITY_LOW = 5;
const uint8_t PRIORITY_LOWER = 6;
const uint8_t PRIORITY_LOWEST = 7;
const uint8_t PRIORITY_DEFAULT = PRIORITY_HIGHEST;

const int PRIO_WT_HIGHEST = 20;
const int PRIO_WT_HIGHER = 10;
const int PRIO_WT_HIGH = 1;
const int PRIO_WT_MEDIUM = -10;
const int PRIO_WT_LOW = -100;
const int PRIO_WT_LOWER = -800;
const int PRIO_WT_LOWEST = -1000;

const int M_DEFAULT_WEIGHT = PRIO_WT_HIGHEST;
const int M_MAX_NEGATIVE_WEIGHT = PRIO_WT_LOWEST;
const int M_MAX_POSITIVE_WEIGHT = PRIO_WT_HIGHEST;

const uint16_t INVALID_SUBSYSTEM = 0;
//const uint16_t M_VALID_SUBSYSTEM = 1;

const uint32_t PENDING_CPU_THRESHOLD = 20;

const uint32_t DEFAULT_MIN_BW_MBPS = 50;
const uint32_t DEFAULT_MIN_IOPS = 1000;
const int64_t DEFAULT_MIN_BW_PCS = DEFAULT_MIN_BW_MBPS * (M_KBYTES * M_KBYTES / (PARAMETER_COLLECTION_INTERVAL));
const int64_t DEFAULT_MIN_IO_PCS = DEFAULT_MIN_IOPS / PARAMETER_COLLECTION_INTERVAL;
const int64_t DEFAULT_MAX_BW_IOPS = INT64_MAX / PARAMETER_COLLECTION_INTERVAL;
const uint32_t M_BW_10_KB = 10 * 1024;
const uint32_t BW_CORRECTION_UNIT = 5;
const uint32_t IOPS_CORRECTION_UNIT = 100;

const uint32_t LOWER_THRESHOLD_PERCT_VALUE = 90;
const uint32_t UPPER_THRESHOLD_PERCT_VALUE = 110;
const uint32_t PERCENTAGE_VALUE = 100;

const uint32_t TOTAL_NVRAM_STRIPES = 1024;
const uint32_t UPPER_GC_TH = 30;
const uint32_t MID_GC_TH = 20;
const uint32_t LOW_GC_TH = 5;

const uint32_t NO_CONTENTION_CYCLES = 100;
namespace pos
{
/* --------------------------------------------------------------------------*/
/**
 * ENUMERATION DEFINITIONS
 */
/* --------------------------------------------------------------------------*/
const uint32_t MAX_ARRAY_COUNT = ArrayMgmtPolicy::MAX_ARRAY_CNT;

const uint32_t DEFAULT_MIN_VOL = MAX_ARRAY_COUNT * MAX_VOLUME_COUNT + 1;

const uint32_t DEFAULT_MIN_ARRAY = MAX_ARRAY_COUNT + 1;

enum QosGcState
{
    QosGcState_Start = 0,
    QosGcState_Normal = QosGcState_Start,
    QosGcState_High,
    QosGcState_Medium,
    QosGcState_Critical,
    QosGcState_End,
    QosGcStateCount = QosGcState_End - QosGcState_Start,
    QosGcState_Unknown
};

// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
enum QosPriority
{
    QosPriority_Start = 0,
    QosPriority_Low = QosPriority_Start,
    QosPriority_High,
    QosPriority_End,
    QosPriorityCount = QosPriority_End - QosPriority_Start,
    QosPriority_Unknown
};

// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
enum QosCorrectionType
{
    QosCorrection_Start = 0,
    QosCorrection_VolumeThrottle = QosCorrection_Start,
    QosCorrection_EventThrottle,
    QosCorrection_EventWrr,
    QosCorrection_All,
    QosCorrection_End,
    QosCorrectionCount = QosCorrection_End - QosCorrection_Start,
    QosCorrection_Unknown,
};

// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
enum QosCorrectionDir
{
    QosCorrectionDir_Start = 0,
    QosCorrectionDir_NoChange = QosCorrectionDir_Start,
    QosCorrectionDir_Increase,
    QosCorrectionDir_Decrease,
    QosCorrectionDir_SetMaxLimit,
    QosCorrectionDir_Increase2X,
    QosCorrectionDir_Decrease2X,
    QosCorrectionDir_Increase4X,
    QosCorrectionDir_Decrease4X,
    QosCorrectionDir_PriorityHighest,
    QosCorrectionDir_PriorityHigher,
    QosCorrectionDir_PriorityHigh,
    QosCorrectionDir_PriorityMedium,
    QosCorrectionDir_PriorityLow,
    QosCorrectionDir_PriorityLower,
    QosCorrectionDir_PriorityLowest,
    QosCorrectionDir_Reset,
    QosCorrectionDir_End,
    QosCorrectionDirCount = QosCorrectionDir_End - QosCorrectionDir_Start,
    QosCorrectionDir_Unknown,
};

// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
enum QosInternalManagerType
{
    QosInternalManager_Start = 0,
    QosInternalManager_Monitor,
    QosInternalManager_Policy,
    QosInternalManager_Processing,
    QosInternalManager_Correction,
    QosInternalManager_End,
    QosInternalManagerCount = QosInternalManager_End - QosInternalManager_Start,
    QosInternalManager_Unknown,
};

/* --------------------------------------------------------------------------*/
/**
 * STRUCTURE DEFINITIONS
 */
/* --------------------------------------------------------------------------*/
// ----------------------------------------------------------------------------
// Parameters collected per millisecond
// ----------------------------------------------------------------------------
struct bw_iops_parameter
{
    bw_iops_parameter(void)
    {
        currentBW = 0;
        currentIOs = 0;
        valid = 0;
        blockSize = 0;
        for (uint8_t i = 0; i < 11; i++)
        {
            pad[i] = 0;
        }
    }
    uint64_t currentBW;  // per milli second
    uint64_t currentIOs; // per milli second
    uint32_t valid;
    uint32_t blockSize;
    uint32_t pad[11];
};

// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
struct poller_structure
{
    poller_structure(void)
    {
        nextTimeStamp = 0;
        qosTimeSlice = 0;
        id = 0;
        for (uint8_t i = 0; i < 5; i++)
        {
            pad[i] = 0;
        }
    }
    uint64_t nextTimeStamp;
    uint64_t qosTimeSlice;
    uint64_t id;
    uint64_t pad[5];
};

// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
struct qos_vol_policy
{
    qos_vol_policy(void)
    {
        minBw = 0;
        maxBw = 0;
        minIops = 0;
        maxIops = 0;
        policyChange = false;
        minPolicySet = false;
        minBwGuarantee = false;
        minIopsGuarantee = false;
        maxValueChanged = false;
    }
    uint64_t minBw;
    uint64_t maxBw;
    uint64_t minIops;
    uint64_t maxIops;
    bool policyChange;
    bool minPolicySet;
    bool minBwGuarantee;
    bool minIopsGuarantee;
    bool maxValueChanged;
};

// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
struct qos_rebuild_policy
{
    qos_rebuild_policy(void)
    {
        rebuildImpact = PRIORITY_DEFAULT;
        policyChange = false;
    }
    uint8_t rebuildImpact;
    bool policyChange;
};

// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
struct qos_correction_type
{
    qos_correction_type(void)
    {
        Reset();
    }
    void
    Reset(void)
    {
        volumeThrottle = false;
        eventThrottle = false;
        eventWrr = false;
    }
    bool volumeThrottle;
    bool eventThrottle;
    bool eventWrr;
};

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis 
 *
 */
/* --------------------------------------------------------------------------*/
class QosReturnCode
{
public:
    static const int SUCCESS = 0;
    static const int FAILURE = -1;
    static const int EVENT_POLICY_IN_EFFECT = 9000;
    static const int VOLUME_POLICY_IN_EFFECT = 9001;
    static const int MINIMUM_BW_APPLIED = 9002;
    static const int MINIMUM_BW_COMPROMISED = 9003;
    static const int INCREASE_BW_THROTTLING = 9004;
    static const int DECREASE_BW_THROTTLING = 9005;
    static const int VOLUME_NOT_PRESENT = 9006;
    static const int EVENT_NOT_PRESENT = 9007;
    static const int ONE_MINIMUM_GUARANTEE_SUPPORTED = 9008;
    static const int REACTOR_NOT_PRESENT = 9009;
    static const int MIN_IOPS_OR_MIN_BW_ONLY_ONE = 9010;
};

} // namespace pos
