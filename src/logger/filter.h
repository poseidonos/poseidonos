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

#include <set>
#include <string>
#include <vector>
using namespace std;

namespace pos_logger
{
class Filter
{
public:
    void Clear();
    bool
    IsFiltered()
    {
        return !(included.empty() && excluded.empty());
    }
    int ApplyFilter(string filePath);
    bool
    ShouldLog(int id)
    {
        return !IsFiltered() || _Valid(id);
    }
    string
    IncludeRule()
    {
        return include_rule;
    }
    string
    ExcludeRule()
    {
        return exclude_rule;
    }

private:
    int _Decode(string policy);
    vector<string> _Split(string val, char separator);
    bool
    _Valid(int id)
    {
        return _Included(id) || _NotExcluded(id);
    }

    bool
    _Included(int id)
    {
        return included.find(id) != included.end();
    }

    bool
    _NotExcluded(int id)
    {
        return included.empty() &&
            excluded.find(id) == excluded.end();
    }

    void
    _Include(int id)
    {
        included.insert(id);
    }

    void
    _Exclude(int id)
    {
        excluded.insert(id);
    }

    set<int> included;
    set<int> excluded;
    char charComment = '#';
    char policyDelimiter = ':';
    char idDelimiter = ',';
    char rangeDelimiter = '-';
    string include_rule = "";
    string exclude_rule = "";
};
} // namespace pos_logger
