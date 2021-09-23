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
 *     * Neither the name of Intel Corporation nor the names of its
 *       contributors may be used to endorse or promote products derived
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

#include <map>
#include <chrono>
#include <iomanip>
#include <list>
#include <sstream>
#include <string>

namespace pos
{
class Metric
{
public:
    Metric(void) {}
    ~Metric(void) {}
    virtual std::string GetId(void) { return id; }
    virtual time_t GetTime(void) { return time; }
    virtual std::string GetTimeString(void) { return strTime; }
    virtual void SetCommonMetric(std::string id_, time_t t_, std::string st_)
    {
        id = id_;
        time = t_;
        strTime = st_;
    }

protected:
    std::string id;
    time_t time;
    std::string strTime;
};

class MetricUint32 : public Metric
{
public:
    MetricUint32(void) {}
    ~MetricUint32(void) {}
    uint32_t GetValue(void) { return value; }
    void SetMetric(std::string id_, time_t t_, uint32_t v_, std::string st_)
    {
        SetCommonMetric(id_, t_, st_);
        value = v_;
    }

private:
    uint32_t value;
};

class MetricString : public Metric
{
public:
    MetricString(void) {}
    ~MetricString(void) {}
    std::string GetValue(void) { return value; }
    void SetMetric(std::string id_, time_t t_, std::string v_, std::string st_)
    {
        SetCommonMetric(id_, t_, st_);
        value = v_;
    }

private:
    std::string value;
};

} // namespace pos
