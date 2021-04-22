
#include "src/lib/Design.cpp"
#include "src/lib/Design.h"

class TestObserver : public lib_design::Observer
{
public:
    void
    Update(uint32_t type1, uint32_t type2,
        uint32_t value1, uint32_t value2,
        int pid, int cmd_type, int cmd_order)
    {
    }
    void
    Handle()
    {
    }
};

class TestSubject : public lib_design::Subject
{
public:
    int
    Notify(uint32_t index, uint32_t type1, uint32_t type2,
        uint32_t value1, uint32_t value2,
        int pid, int cmd_type, int cmd_order)
    {
        if (index >= ARR_SIZE)
            return -3; // interface error
        if (nullptr != arr_observer[index])
        {
            arr_observer[index]->Update(type1, type2, value1, value2,
                pid, cmd_type, cmd_order);
            return 0;
        }
        return -6; // lifecycle error
    }
};

class DesignTest : public ::testing::Test
{
public:
    TestObserver* observer{nullptr};
    TestSubject* subject{nullptr};

protected:
    DesignTest()
    {
        observer = new TestObserver{};
        subject = new TestSubject{};
    }
    ~DesignTest() override
    {
        if (nullptr != observer)
        {
            delete observer;
            observer = nullptr;
        }
        if (nullptr != subject)
        {
            delete subject;
            subject = nullptr;
        }
    }
    void
    SetUp() override
    {
    }
    void
    TearDown() override
    {
    }
};
