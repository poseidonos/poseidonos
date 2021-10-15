#include "src/lib/atomic_count.h"

#include <gtest/gtest.h>

namespace pos
{
TEST(AtomicCount, Instance_)
{
    //when: create an object in stack
    AtomicCount<int> count(0);

    //when: create an object in heap
    auto* bigCount = new AtomicCount<uint64_t>(10);
    delete bigCount;
}

TEST(AtomicCount, Operators)
{
    //Given: integer-type count object
    AtomicCount<int> count(0);
    //When: increase once and decrease twice, resulting in underflow (no way to check, though)
    count++;
    count--;
    count--;
 
    //Given: long-type count object
    AtomicCount<long> lcount(0);
    //When: increase once and decrease twice, resulting in underflow (no way to check, though)
    lcount++;
    lcount--;
    lcount--;

    //Given: unsigned long-type count object
    AtomicCount<unsigned long> ulcount(0);
    //When: increase once and decrease twice, resulting in underflow (no way to check, though)
    ulcount++;
    ulcount--;
    ulcount--;
}

} // namespace pos
