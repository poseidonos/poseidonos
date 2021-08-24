#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <vector>
#include <string>
#include "src/helper/query.h"

using ::testing::_;
using ::testing::Return;

namespace Enumerable
{

class Person
{
public:
    Person(std::string n, int i)
    {
        name = n;
        id  = i;
    }
    std::string GetName(void) { return name; }
    int GetID(void) { return id; }

private:
    std::string name;
    int id;
};

TEST(Query, Query_testGroupBy)
{
    // Given
    std::vector<Person*> v1;
    v1.push_back(new Person("foo", 10));
    v1.push_back(new Person("bar", 20));
    v1.push_back(new Person("foo", 30));

    // When
    auto&& personByName = Enumerable::GroupBy(v1,
        [](auto p) { return p->GetName(); });

    // Then
    ASSERT_EQ(2, personByName.size());
}

TEST(Query, Query_testDistinct)
{
    // Given
    std::vector<Person*> v1;
    Person* p1 = new Person("foo", 10);
    Person* p2 = new Person("bar", 20);
    Person* p3 = p1;
    v1.push_back(p1);
    v1.push_back(p2);
    v1.push_back(p3);

    // When
    auto&& uniquePersonList = Enumerable::Distinct(v1,
        [](auto p) { return p; });

    // Then
    ASSERT_EQ(2, uniquePersonList.size());
}

TEST(Query, Query_testWhere)
{
    // Given
    std::vector<Person*> v1;
    v1.push_back(new Person("foo", 10));
    v1.push_back(new Person("bar", 20));
    v1.push_back(new Person("foo", 30));

    // When
    auto&& personNameisfoo = Enumerable::Where(v1,
        [](auto p) { return p->GetName() == "foo"; });

    // Then
    ASSERT_EQ(2, personNameisfoo.size());
}

TEST(Query, Query_testFirst)
{
    // Given
    std::vector<Person*> v1;
    v1.push_back(new Person("foo", 10));
    v1.push_back(new Person("bar", 20));
    v1.push_back(new Person("foo", 30));

    // When
    auto&& personNameisfoo = Enumerable::First(v1,
        [](auto p) { return p->GetName() == "foo"; });

    // Then
    ASSERT_EQ(10, personNameisfoo->GetID());
}

TEST(Query, Query_testFirstNoResult)
{
    // Given
    std::vector<Person*> v1;
    v1.push_back(new Person("foo", 10));
    v1.push_back(new Person("bar", 20));
    v1.push_back(new Person("foo", 30));

    // When
    auto&& personNameistom = Enumerable::First(v1,
        [](auto p) { return p->GetName() == "tom"; });

    // Then
    ASSERT_EQ(nullptr, personNameistom);
}

TEST(Query, Query_testJoin)
{
    // Given
    std::vector<Person*> v1;
    v1.push_back(new Person("foo", 10));
    v1.push_back(new Person("bar", 20));
    v1.push_back(new Person("foo", 30));

    std::vector<int> v2;
    v2.push_back(20);
    v2.push_back(30);
    v2.push_back(50);

    // When
    auto&& personJoinById = Enumerable::Join(v1,
        [](auto p) { return p->GetID(); }, v2, [](auto i) { return i; });

    // Then
    ASSERT_EQ(2, personJoinById.size());
}

TEST(Query, Query_testSelect)
{
    // Given
    std::vector<Person*> v1;
    v1.push_back(new Person("foo", 10));
    v1.push_back(new Person("bar", 20));
    v1.push_back(new Person("foo", 30));

    // When
    auto&& peopleNameList = Enumerable::Select(v1,
        [](auto p) { return p->GetName(); });

    // Then
    ASSERT_EQ("foo", *(peopleNameList.begin()));
    ASSERT_EQ(3, peopleNameList.size());
}
}  // namespace Enumerable
