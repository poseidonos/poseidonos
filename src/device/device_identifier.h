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

#ifndef DEVICE_IDENTIFIER_H_
#define DEVICE_IDENTIFIER_H_

#include <functional>
#include <string>

#include "src/device/base/ublock_device.h"

using namespace std;

namespace pos
{
class DeviceIdentifierPredicate
{
public:
    const std::string KEY;
    DeviceIdentifierPredicate(std::string key)
    : KEY(key)
    {
    }

    virtual bool operator()(UblockSharedPtr dev) const = 0;
};

class DevNamePred : public DeviceIdentifierPredicate
{
public:
    DevNamePred(std::string key)
    : DeviceIdentifierPredicate(key)
    {
    }

    bool
    operator()(UblockSharedPtr dev) const override
    {
        return (dev->GetName() == KEY);
    }
};

class DevSNPred : public DeviceIdentifierPredicate
{
public:
    DevSNPred(std::string key)
    : DeviceIdentifierPredicate(key)
    {
    }

    bool
    operator()(UblockSharedPtr dev) const override
    {
        return (dev->GetSN() == KEY);
    }
};

class DeviceIdentifier
{
public:
    typedef function<bool(UblockSharedPtr&)> Predicate;
    DeviceIdentifier(void)
    {
    }

    DeviceIdentifier(const string val)
    : val(val)
    {
    }

    string val = "";

    virtual Predicate GetPredicate(void) = 0;
};

class DevName : public DeviceIdentifier
{
public:
    DevName(void)
    {
    }

    DevName(const string val)
    : DeviceIdentifier(val)
    {
    }

    Predicate
    GetPredicate(void)
    {
        return DevNamePred(val);
    }
};

class DevUid : public DeviceIdentifier
{
public:
    DevUid(void)
    {
    }

    DevUid(const DevUid& uid)
    {
        this->val = uid.val;
    }

    DevUid&
    operator=(const DevUid uid)
    {
        this->val = uid.val;
        return *this;
    }

    DevUid(const string val)
    : DeviceIdentifier(val)
    {
    }

    Predicate
    GetPredicate(void)
    {
        return DevSNPred(val);
    }
};

} // namespace pos
#endif // DEVICE_IDENTIFIER_H_
