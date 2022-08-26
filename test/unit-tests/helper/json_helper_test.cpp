#include "src/helper/json/json_helper.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <iostream>
#include <string>

using ::testing::_;
using ::testing::Return;

TEST(JsonFormat, MakeResponse_testJsonFormatIfElementContainsArray)
{
    // Given
    struct Person
    {
        string name;
        int id;
    };

    Person people[2] = {{"foo", 1}, {"bar", 2}};
    JsonFormat jFormat;

    JsonArray array("people");
    for (size_t i = 0; i < 2; i++)
    {
        JsonElement elem("");
        elem.SetAttribute(
            JsonAttribute("name", "\"" + people[i].name + "\""));
        elem.SetAttribute(
            JsonAttribute("id", people[i].id));
        array.AddElement(elem);
    }
    JsonElement dataElem("data");
    dataElem.SetArray(array);
    JsonElement infoElem("sign");
    infoElem.SetAttribute(JsonAttribute("ver", "\" 1.0 + \""));

    // When
    string response = jFormat.MakeResponse("PEOPLE", "0000", 0, "List of people", dataElem, infoElem);
    string expected = "{\"command\":\"PEOPLE\",\"rid\":\"0000\",\"result\":{\"status\":{\"code\":0,\"eventName\":\"SUCCESS\",\"description\":\"List of people\",\"cause\":\"NONE\",\"solution\":\"NONE\"},\"data\":{\"people\":[{\"name\":\"foo\",\"id\":1},{\"name\":\"bar\",\"id\":2}]}},\"sign\":{\"ver\":\" 1.0 + \"}}";

    // Then
    EXPECT_EQ(response, expected);
}

TEST(JsonFormat, MakeResponse_testJsonFormatIfDataElementIsNotIncluded)
{
    // Given
    JsonFormat jFormat;
    JsonElement infoElem("sign");
    infoElem.SetAttribute(JsonAttribute("ver", "\" 1.0 + \""));

    // When
    string response = jFormat.MakeResponse("PEOPLE", "1111", 0, "There is no person", infoElem);
    string expected = "{\"command\":\"PEOPLE\",\"rid\":\"1111\",\"result\":{\"status\":{\"code\":0,\"eventName\":\"SUCCESS\",\"description\":\"There is no person\",\"cause\":\"NONE\",\"solution\":\"NONE\"}},\"sign\":{\"ver\":\" 1.0 + \"}}";

    // Then
    EXPECT_EQ(response, expected);
}
