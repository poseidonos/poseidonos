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

#include "array_service_layer.h"
#include "src/include/pos_event_id.h"

using namespace std;

namespace pos
{
ArrayServiceLayer::ArrayServiceLayer(void)
{
    ioTranslator = new IOTranslator();
    ioRecover = new IORecover();
    deviceChecker = new IODeviceChecker();
    metaIOLocker = new IOLocker("MetaLocker");
}

ArrayServiceLayer::~ArrayServiceLayer(void)
{
    delete deviceChecker;
    delete ioRecover;
    delete ioTranslator;
}

IArrayServiceConsumer*
ArrayServiceLayer::Getter(void)
{
    return this;
}

IArrayServiceProducer*
ArrayServiceLayer::Setter(void)
{
    return this;
}

int
ArrayServiceLayer::Register(string array, unsigned int arrayIndex,
    ArrayTranslator trans, ArrayRecover recover, IDeviceChecker* checker)
{
    unique_lock<mutex> lock(mtx);
    bool ret = true;

    ret = ioTranslator->Register(arrayIndex, trans);
    if (ret == false)
    {
        return EID(MOUNT_ARRAY_UNABLE_TO_REGISTER_TRANSLATOR);
    }
    ret = ioRecover->Register(arrayIndex, recover);
    if (ret == false)
    {
        return EID(MOUNT_ARRAY_UNABLE_TO_REGISTER_RECOVER);
    }
    ret = deviceChecker->Register(arrayIndex, checker);
    if (ret == false)
    {
        return EID(MOUNT_ARRAY_UNABLE_TO_REGISTER_DEVICECHECKER);
    }
    return 0;
}

void
ArrayServiceLayer::Unregister(string array, unsigned int arrayIndex)
{
    unique_lock<mutex> lock(mtx);
    deviceChecker->Unregister(arrayIndex);
    ioRecover->Unregister(arrayIndex);
    ioTranslator->Unregister(arrayIndex);
}

void
ArrayServiceLayer::IncludeDevicesToLocker(vector<ArrayDevice*> devList, bool isWT)
{
    unique_lock<mutex> lock(mtx);
    metaIOLocker->Register(devList);
    if (isWT == true)
    {
        if (journalIOLocker == nullptr)
        {
            journalIOLocker = new IOLocker("JournalLocker");
        }
        journalIOLocker->Register(devList);
    }
}

void
ArrayServiceLayer::ExcludeDevicesFromLocker(vector<ArrayDevice*> devList, bool isWT)
{
    unique_lock<mutex> lock(mtx);
    metaIOLocker->Unregister(devList);
    if (isWT == true)
    {
        journalIOLocker->Unregister(devList);
        if (journalIOLocker->Size() == 0)
        {
            delete journalIOLocker;
            journalIOLocker = nullptr;
        }
    }
}

IIOTranslator*
ArrayServiceLayer::GetTranslator(void)
{
    return ioTranslator;
}

IIORecover*
ArrayServiceLayer::GetRecover(void)
{
    return ioRecover;
}

IIODeviceChecker*
ArrayServiceLayer::GetDeviceChecker(void)
{
    return deviceChecker;
}

IIOLocker*
ArrayServiceLayer::GetIOLocker(PartitionType partType)
{
    if (partType == PartitionType::JOURNAL_SSD)
    {
        return journalIOLocker;
    }
    else if (partType == PartitionType::META_SSD)
    {
        return metaIOLocker;
    }
    return nullptr;
}

} // namespace pos
