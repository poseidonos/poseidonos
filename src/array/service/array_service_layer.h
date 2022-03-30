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

#include <string>
#include <vector>

#include "src/array/service/interface/i_array_service_consumer.h"
#include "src/array/service/interface/i_array_service_producer.h"
#include "src/array/service/io_device_checker/io_device_checker.h"
#include "src/array/service/io_locker/io_locker.h"
#include "src/array/service/io_recover/io_recover.h"
#include "src/array/service/io_translator/io_translator.h"
#include "src/lib/singleton.h"

using namespace std;

namespace pos
{
class ArrayServiceLayer : public IArrayServiceProducer, public IArrayServiceConsumer
{
public:
    ArrayServiceLayer(void);
    virtual ~ArrayServiceLayer(void);
    IArrayServiceConsumer* Getter(void);
    IArrayServiceProducer* Setter(void);

    int Register(string array, unsigned int arrayIndex,
        ArrayTranslator trans, ArrayRecover recover, IDeviceChecker* checker) override;
    void Unregister(string array, unsigned int arrayIndex) override;
    void IncludeDevicesToLocker(vector<ArrayDevice*> devList) override;
    void ExcludeDevicesFromLocker(vector<ArrayDevice*> devList) override;
    IIOTranslator* GetTranslator(void) override;
    IIORecover* GetRecover(void) override;
    IIODeviceChecker* GetDeviceChecker(void) override;
    IIOLocker* GetIOLocker(PartitionType partType);

private:
    IOTranslator* ioTranslator = nullptr;
    IORecover* ioRecover = nullptr;
    IODeviceChecker* deviceChecker = nullptr;
    IOLocker* metaIOLocker = nullptr;
    IOLocker* journalIOLocker = nullptr;
};

using ArrayService = Singleton<ArrayServiceLayer>;

} // namespace pos
