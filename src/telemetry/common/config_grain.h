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
#include <unordered_map>
#include <iostream>

#define DEFAULT_YAML_VALUE ""

namespace pos
{
class ConfigGrain
{
public:
    ConfigGrain(void)
    {
        values.clear();
    }
// Exclude destructor of abstract class from function coverage report to avoid known issues in gcc/gcov
// LCOV_EXCL_START
    virtual ~ConfigGrain(void)
    {
    }
// LCOV_EXCL_STOP

    virtual bool UpdateConfig(std::string key, std::string value)
    {
        if (values.count(key))
            values.erase(key);

        auto result = values.insert({ key, value });
        return result.second;
    }

    virtual bool UpdateConfig(std::string key, uint64_t value)
    {
        if (values.count(key))
            values.erase(key);

        auto result = values.insert({ key, std::to_string(value) });
        return result.second;
    }

    virtual bool UpdateConfig(std::string key, bool value)
    {
        std::string str = value ? "true" : "false";

        if (values.count(key))
            values.erase(key);

        auto result = values.insert({ key, str });
        return result.second;
    }

    // only for test
    std::unordered_map<std::string, std::string>& GetValues(void)
    {
        return values;
    }

    static const std::string DEFAULT_VALUE;

protected:
    std::string _GetValue(std::string key)
    {
        if (values.count(key))
            return values[key];
        return DEFAULT_YAML_VALUE;
    }

    std::unordered_map<std::string, std::string> values;
};
} // namespace pos
