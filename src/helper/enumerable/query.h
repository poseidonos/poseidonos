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

#include <vector>
#include <map>
#include <iterator>

namespace Enumerable
{
template<typename S/*Source*/, typename F/*KeySelector*/>
auto GroupBy(S&& s, F f)
{
    using type_t = std::decay_t<decltype(*std::begin(s))>;
    using discriminator_t = decltype(f(*std::begin(s)));
    std::map<discriminator_t, std::vector<type_t>> group;
    for (auto&& i : s)
    {
        group[f(i)].emplace_back(i);
    }
    return group;
}

template<typename S/*Source*/, typename F/*Comparer*/>
auto Distinct(S&& s, F f)
{
    using type_t = std::decay_t<decltype(*std::begin(s))>;
    std::vector<type_t> result;
    for (auto&& i : s)
    {
        if (find(result.begin() , result.end(), f(i)) == result.end())
        {
            result.emplace_back(i);
        }
    }
    return result;
}

template<typename S/*Source*/, typename F/*Func<bool>*/>
auto Where(S&& s, F f)
{
    using type_t = std::decay_t<decltype(*std::begin(s))>;
    std::vector<type_t> result;
    for (auto&& i : s)
    {
        if (f(i) == true)
        {
            result.emplace_back(i);
        }
    }
    return result;
}

template<typename S/*Source*/, typename F/*Predicate*/>
auto First(S&& s, F f)
{
    using type_t = std::decay_t<decltype(*std::begin(s))>;
    type_t ret = nullptr;
    for (auto&& i : s)
    {
        if (f(i) == true)
        {
            ret = i;
            break;
        }
    }
    return ret;
}

template<typename OC/*OuterContainer*/, typename OK/*OuterKeySelector*/, typename IC/*InnerContainer*/, typename IK/*InnerKeySelector*/>
auto Join(OC&& oc, OK ok, IC&& ic, IK ik)
{
    using type_t = std::decay_t<decltype(*std::begin(oc))>;
    std::vector<type_t> r;
    for (auto&& i : ic)
    {
        for (auto&& o : oc)
        {
            if (ik(i) == ok(o))
            {
                r.emplace_back(o);
            }
        }
    }
    return r;
}

template<typename S/*Source*/, typename F/*Selector*/>
auto Select(S&& s, F f)
{
    using type_t = decltype(f(*std::begin(s)));
    std::vector<type_t> result;
    for (auto&& i : s)
    {
        result.emplace_back(f(i));
    }
    return result;
}

template<typename S/*Source*/, typename FS/*Selector*/, typename FW/*Where<bool>*/>
auto SelectWhere(S&& s, FS fs, FW fw)
{
    using type_t = decltype(fs(*std::begin(s)));
    std::vector<type_t> result;
    for (auto&& i : s)
    {
        if (fw(i) == true)
        {
            result.emplace_back(fs(i));
        }
    }
    return result;
}

template<typename S/*Source*/, typename F/*Sort by*/>
auto Minimum(S&& s, F f)
{
    using type_t = std::decay_t<decltype(*std::begin(s))>;
    type_t ret = *std::begin(s);
    for (auto&& i : s)
    {
        if (f(ret) > f(i))
        {
            ret = i;
        }
    }
    return ret;
}

template<typename S/*Source*/, typename F/*Sort by*/>
auto Maximum(S&& s, F f)
{
    using type_t = std::decay_t<decltype(*std::begin(s))>;
    type_t ret = *std::begin(s);
    for (auto&& i : s)
    {
        if (f(ret) < f(i))
        {
            ret = i;
        }
    }
    return ret;
}


} // namespace Enumerable
