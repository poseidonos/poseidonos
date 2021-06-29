
#include "fake_cli_send.h"
#include "fake_output_observer.h"
#include "src/input/In.cpp"
#include "src/input/In.h"
#include "src/lib/Design.cpp"
#include "src/lib/Design.h"

class InTest : public ::testing::Test
{
public:
    input::InCommand* in_command{nullptr};
    FakeCliSend* fake_cli_send{nullptr};
    input::Subject* input_subject{nullptr};
    FakeOutputObserver* fake_output_observer{nullptr};

protected:
    InTest()
    {
        fake_cli_send = new FakeCliSend{};
        fake_output_observer = new FakeOutputObserver{};
        input_subject = new input::Subject{};
        input_subject->Attach(fake_output_observer, 0);
        in_command = new input::InCommand{input_subject};
    }
    ~InTest()
    {
        if (nullptr != fake_output_observer)
        {
            delete fake_output_observer;
        }
        if (nullptr != input_subject)
        {
            delete input_subject;
        }
        if (nullptr != fake_cli_send)
        {
            delete fake_cli_send;
        }
        if (nullptr != in_command)
        {
            delete in_command;
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
