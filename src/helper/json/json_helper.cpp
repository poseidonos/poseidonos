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

#include "json_helper.h"

#include <unordered_map>

#include "src/event/event_manager.h"

static void
JsonFormatSubStringAddTab(JsonFormatType type, string& subString)
{
    if (JsonFormatType::JSON_FORMAT_TYPE_READABLE == type)
    {
        uint32_t pos = 0;
        string newLine = "\n";
        string tab = "    ";
        while (subString.find(newLine, pos) != string::npos)
        {
            uint32_t newLinePos = subString.find(newLine, pos);
            pos = newLinePos + 1;
            subString.insert(pos, tab);
        }
    }
};

static void
JsonFormatAddNewLine(JsonFormatType type, string& subString)
{
    string newLine = "\n";
    if (JsonFormatType::JSON_FORMAT_TYPE_READABLE == type)
    {
        subString += newLine;
    }
};

static void
JsonFormatAddNewLineAndTab(JsonFormatType type, string& subString)
{
    string newLine = "\n";
    string tab = "    ";
    if (JsonFormatType::JSON_FORMAT_TYPE_READABLE == type)
    {
        subString += newLine;
        subString += tab;
    }
};

string
JsonAttribute::ToJson()
{
    return "\"" + name + "\"" + ":" + value;
}

string
JsonArray::ToJson(JsonFormatType type)
{
    string jsonstr = "\"" + name + "\"" + ":";
    jsonstr += "[";
    JsonFormatAddNewLineAndTab(type, jsonstr);

    for (vector<JsonElement>::iterator it = item.begin();
         it != item.end();
         it++)
    {
        string subElem = it->ToJson(type);
        JsonFormatSubStringAddTab(type, subElem);

        jsonstr += subElem;

        if (std::next(it) != item.end())
        {
            jsonstr += ",";
            JsonFormatAddNewLineAndTab(type, jsonstr);
        }
    }

    JsonFormatAddNewLine(type, jsonstr);
    jsonstr += "]";
    return jsonstr;
}

string
JsonElement::ToJson(JsonFormatType type)
{
    string jsonstr = "";

    if (name.empty() == false)
    {
        jsonstr = "\"" + name + "\"" + ":";
    }
    jsonstr += "{";
    JsonFormatAddNewLineAndTab(type, jsonstr);

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
            JsonFormatAddNewLineAndTab(type, jsonstr);
        }
        addComma = true;
    }

    if (array.size() > 0 && addComma == true)
    {
        jsonstr += ",";
        JsonFormatAddNewLineAndTab(type, jsonstr);
        addComma = false;
    }

    for (vector<JsonArray>::iterator it = array.begin();
         it != array.end();
         ++it)
    {
        string subElem = it->ToJson(type);
        JsonFormatSubStringAddTab(type, subElem);
        jsonstr += subElem;

        if (std::next(it) != array.end())
        {
            jsonstr += ",";
            JsonFormatAddNewLineAndTab(type, jsonstr);
        }

        addComma = true;
    }

    if (elements.size() > 0 && addComma == true)
    {
        jsonstr += ",";
        JsonFormatAddNewLineAndTab(type, jsonstr);
        addComma = false;
    }

    // ToJson for elements
    for (vector<JsonElement>::iterator it = elements.begin();
         it != elements.end();
         it++)
    {
        string subElem = it->ToJson(type);
        JsonFormatSubStringAddTab(type, subElem);

        jsonstr += subElem;
        if (std::next(it) != elements.end())
        {
            jsonstr += ",";
            JsonFormatAddNewLineAndTab(type, jsonstr);
        }
    }

    JsonFormatAddNewLine(type, jsonstr);
    jsonstr += "}";
    return jsonstr;
}

string
JsonFormat::MakeResponse(
    string command, string rid, int eventId, string description, JsonElement info)
{
    std::string eventName = "";
    std::string cause = "";
    std::string solution = "";

    auto event_info = eventManager.GetEventInfo();
    auto it = event_info->find(eventId);
    if (it != event_info->end())
    {
        eventName = it->second.GetEventName();
        cause = it->second.GetCause();
        solution = it->second.GetSolution();
    }

    JsonElement root("");
    root.SetAttribute(JsonAttribute("command", "\"" + command + "\""));
    root.SetAttribute(JsonAttribute("rid", "\"" + rid + "\""));
    JsonElement result = JsonElement("result");
    JsonElement status = JsonElement("status");
    status.SetAttribute(JsonAttribute("code", eventId));
    status.SetAttribute(JsonAttribute("eventName", "\"" + eventName + "\""));
    status.SetAttribute(JsonAttribute("description", "\"" + description + "\""));
    status.SetAttribute(JsonAttribute("cause", "\"" + cause + "\""));
    status.SetAttribute(JsonAttribute("solution", "\"" + solution + "\""));

    result.SetElement(status);
    root.SetElement(result);
    root.SetElement(info);
    return root.ToJson(JSON_FORMAT_TYPE_DEFAULT);
}

string
JsonFormat::MakeResponse(
    string command, string rid, int eventId,
    string description, JsonElement dataElem, JsonElement info)
{
    std::string eventName = "";
    std::string cause = "";
    std::string solution = "";

    auto event_info = eventManager.GetEventInfo();
    auto it = event_info->find(eventId);
    if (it != event_info->end())
    {
        eventName = it->second.GetEventName();
        cause = it->second.GetCause();
        solution = it->second.GetSolution();
    }

    JsonElement root("");
    root.SetAttribute(JsonAttribute("command", "\"" + command + "\""));
    root.SetAttribute(JsonAttribute("rid", "\"" + rid + "\""));
    JsonElement result = JsonElement("result");
    JsonElement status = JsonElement("status");
    status.SetAttribute(JsonAttribute("code", eventId));
    status.SetAttribute(JsonAttribute("eventName", "\"" + eventName + "\""));
    status.SetAttribute(JsonAttribute("description", "\"" + description + "\""));
    status.SetAttribute(JsonAttribute("cause", "\"" + cause + "\""));
    status.SetAttribute(JsonAttribute("solution", "\"" + solution + "\""));

    result.SetElement(status);
    result.SetElement(dataElem);
    root.SetElement(result);
    root.SetElement(info);
    return root.ToJson(JSON_FORMAT_TYPE_DEFAULT);
}
