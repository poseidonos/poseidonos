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

#ifndef JSON_HELPER_H_
#define JSON_HELPER_H_

#include <map>
#include <numeric>
#include <string>
#include <utility>
#include <vector>

using namespace std;

enum JsonFormatType
{
    JSON_FORMAT_TYPE_DEFAULT = 0,
    JSON_FORMAT_TYPE_READABLE
};

class JsonAttribute // name-value pair
{
public:
    JsonAttribute(string _name, string _value)
    : name(_name),
      value(_value)
    {
    }
    JsonAttribute(string _name, int _value)
    : name(_name),
      value(to_string(_value))
    {
    }
    string ToJson();

private:
    string name;
    string value; // quotes should be included in value if value type is string
};

class JsonArray;
class JsonElement // attribute collection
{
public:
    explicit JsonElement(string _name)
    : name(_name)
    {
    }
    void
    SetAttribute(JsonAttribute attr)
    {
        attributes.push_back(attr);
    }
    void
    SetArray(JsonArray& _array)
    {
        array.push_back(_array);
    }
    void
    SetElement(JsonElement& elem)
    {
        elements.push_back(elem);
    }
    string ToJson(JsonFormatType type = JSON_FORMAT_TYPE_DEFAULT);

private:
    string name;
    vector<JsonAttribute> attributes;
    vector<JsonElement> elements;
    vector<JsonArray> array;
};

class JsonArray // group of elements
{
public:
    explicit JsonArray(string _name)
    : name(_name)
    {
    }
    void
    AddElement(JsonElement& element)
    {
        item.push_back(element);
    }
    string ToJson(JsonFormatType type = JSON_FORMAT_TYPE_DEFAULT);

private:
    string name;
    vector<JsonElement> item;
};

class JsonFormat
{
public:
    string MakeResponse(
        string command, string rid, int eventId, string description, JsonElement info);
    string MakeResponse(
        string command, string rid, int eventId,
        string description, JsonElement dataElem, JsonElement info);
};

#endif // JSON_HELPER_H_
