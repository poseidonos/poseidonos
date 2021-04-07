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

#include "json_helper.h"

string
JsonAttribute::ToJson()
{
    return "\"" + name + "\"" + ":" + value;
}

string
JsonArray::ToJson()
{
    string jsonstr = "\"" + name + "\"" + ":";
    jsonstr += "[";
    for (vector<JsonElement>::iterator it = item.begin();
         it != item.end();
         it++)
    {
        jsonstr += it->ToJson();
        if (std::next(it) != item.end())
            jsonstr += ",";
    }
    jsonstr += "]";
    return jsonstr;
}

string
JsonElement::ToJson()
{
    string jsonstr = "";
    if (name.empty() == false)
    {
        jsonstr = "\"" + name + "\"" + ":";
    }
    jsonstr += "{";

    bool addComma = false;
    // ToJson for attributes
    for (vector<JsonAttribute>::iterator it = attributes.begin();
         it != attributes.end();
         ++it)
    {
        jsonstr += it->ToJson();
        if (std::next(it) != attributes.end())
        {
            jsonstr += ",";
        }
        addComma = true;
    }

    if (array.size() > 0 && addComma == true)
    {
        jsonstr += ",";
        addComma = false;
    }

    for (vector<JsonArray>::iterator it = array.begin();
         it != array.end();
         ++it)
    {
        jsonstr += it->ToJson();
        if (std::next(it) != array.end())
        {
            jsonstr += ",";
        }

        addComma = true;
    }

    if (elements.size() > 0 && addComma == true)
    {
        jsonstr += ",";
        addComma = false;
    }

    // ToJson for elements
    for (vector<JsonElement>::iterator it = elements.begin();
         it != elements.end();
         it++)
    {
        jsonstr += it->ToJson();
        if (std::next(it) != elements.end())
            jsonstr += ",";
    }

    jsonstr += "}";
    return jsonstr;
}

string
JsonFormat::MakeResponse(
    string command, string rid, int code, string description, JsonElement info)
{
    JsonElement root("");
    root.SetAttribute(JsonAttribute("command", "\"" + command + "\""));
    root.SetAttribute(JsonAttribute("rid", "\"" + rid + "\""));
    JsonElement result = JsonElement("result");
    JsonElement status = JsonElement("status");
    status.SetAttribute(JsonAttribute("code", code));
    status.SetAttribute(JsonAttribute("description", "\"" + description + "\""));
    result.SetElement(status);
    root.SetElement(result);
    root.SetElement(info);
    return root.ToJson();
}

// For WBT
string
JsonFormat::MakeResponse(
    string command, string rid, int code, string description, vector<pair<string, string>> dataAttr, JsonElement info)
{
    JsonElement root("");
    root.SetAttribute(JsonAttribute("command", "\"" + command + "\""));
    root.SetAttribute(JsonAttribute("rid", "\"" + rid + "\""));
    JsonElement result = JsonElement("result");
    JsonElement status = JsonElement("status");
    JsonElement data = JsonElement("data");
    status.SetAttribute(JsonAttribute("code", code));
    status.SetAttribute(JsonAttribute("description", "\"" + description + "\""));

    for (unsigned int i = 0; i < dataAttr.size(); i++)
    {
        data.SetAttribute(JsonAttribute(dataAttr[i].first, "\"" + dataAttr[i].second + "\""));
    }

    result.SetElement(status);
    result.SetElement(data);
    root.SetElement(result);
    root.SetElement(info);

    return root.ToJson();
}

string
JsonFormat::MakeResponse(
    string command, string rid, int code,
    string description, JsonElement dataElem, JsonElement info)
{
    JsonElement root("");
    root.SetAttribute(JsonAttribute("command", "\"" + command + "\""));
    root.SetAttribute(JsonAttribute("rid", "\"" + rid + "\""));
    JsonElement result = JsonElement("result");
    JsonElement status = JsonElement("status");
    status.SetAttribute(JsonAttribute("code", code));
    status.SetAttribute(JsonAttribute("description", "\"" + description + "\""));
    result.SetElement(status);
    result.SetElement(dataElem);
    root.SetElement(result);
    root.SetElement(info);
    return root.ToJson();
}
