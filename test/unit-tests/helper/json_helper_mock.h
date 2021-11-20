#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/helper/json/json_helper.h"

class MockJsonAttribute : public JsonAttribute
{
public:
    using JsonAttribute::JsonAttribute;
};

class MockJsonElement : public JsonElement
{
public:
    using JsonElement::JsonElement;
};

class MockJsonArray : public JsonArray
{
public:
    using JsonArray::JsonArray;
};

class MockJsonFormat : public JsonFormat
{
public:
    using JsonFormat::JsonFormat;
};
