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

#include "filter.h"

#include <algorithm>
#include <fstream>
#include <sstream>

#include "src/helper/string/string_helper.h"
#include "src/include/pos_event_id.h"
#include "src/logger/logger.h"

namespace pos_logger
{
void
Filter::Clear()
{
    included.clear();
    excluded.clear();
    include_rule = "";
    exclude_rule = "";
}

int
Filter::ApplyFilter(string filePath)
{
    Clear();
    ifstream filterPolicy(filePath.data());

    if (filterPolicy.is_open())
    {
        while (filterPolicy.peek() != EOF)
        {
            string policy;
            getline(filterPolicy, policy);
            int ret = _Decode(policy);
            if (ret != 0)
            {
                Clear();
                return ret;
            }
        }
    }
    else
    {
        int event = EID(LOGGER_FILTER_POLICY_FILE_NOT_FOUND);
        POS_TRACE_WARN(event, "file_path:{}", filePath);
        return event;
    }

    int event = EID(LOGGER_FILTER_APPLY_SUCCESS);
    POS_TRACE_INFO(event, "file_path:{}", filePath);
    return 0;
}

vector<string>
Filter::_Split(string val, char separator)
{
    vector<string> internal;
    stringstream ss(val);
    string temp;

    while (getline(ss, temp, separator))
    {
        temp.erase(std::remove(temp.begin(), temp.end(), ' '), temp.end());
        internal.push_back(temp);
    }

    return internal;
}

int
Filter::_Decode(string policy)
{
    // include: 2,3,4,5,26-29
    // exclude: 34,56,77
    policy = trim(policy);
    vector<string> pol = _Split(policy, policyDelimiter);
    if (pol.size() != 2)
    {
        int event = EID(LOGGER_FILTER_POLICY_DECODE_FAIL);
        POS_TRACE_WARN(event, "policy:{}", policy);
        return event;
    }
    string mode = pol.at(0);
    string val = pol.at(1);

    if (mode.at(0) == charComment)
    {
        return 0;
    }

    if (mode != "include" && mode != "exclude")
    {
        int event = EID(LOGGER_FILTER_POLICY_DECODE_FAIL);
        POS_TRACE_WARN(event, "policy:{}", policy);
        return event;
    }

    vector<string> id = _Split(val, idDelimiter);
    vector<int> filter;
    try
    {
        for (string s : id)
        {
            if (s.find(rangeDelimiter) != string::npos)
            {
                vector<string> range = _Split(s, rangeDelimiter);
                if (range.size() == 2)
                {
                    int from = stoi(range[0]);
                    int to = stoi(range[1]);
                    for (int i = from; i <= to; i++)
                    {
                        filter.push_back(i);
                    }
                }
                else
                {
                            int event = EID(LOGGER_FILTER_POLICY_DECODE_FAIL);
                            POS_TRACE_WARN(event, "policy:{}", policy);
                            return event;
                }
            }
            else
            {
                filter.push_back(stoi(s));
            }
        }
    }
    catch (const std::exception& e)
    {
        filter.clear();
        int event = EID(LOGGER_FILTER_POLICY_DECODE_FAIL);
        POS_TRACE_WARN(event, "policy:{}", policy);
        return event;
    }

    if (mode == "include")
    {
        if (include_rule != "")
        {
            include_rule = include_rule + "," + val;
        }
        else
        {
            include_rule = val;
        }

        for (int i : filter)
        {
            _Include(i);
        }
    }
    else if (mode == "exclude")
    {
        if (exclude_rule != "")
        {
            exclude_rule = exclude_rule + "," + val;
        }
        else
        {
            exclude_rule = val;
        }

        for (int i : filter)
        {
            _Exclude(i);
        }
    }

    return 0;
}

} // namespace pos_logger
