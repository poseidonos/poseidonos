#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/io_scheduler/dispatcher_policy.h"

namespace pos
{
class MockDispatcherPolicyI : public DispatcherPolicyI
{
public:
    using DispatcherPolicyI::DispatcherPolicyI;
    MOCK_METHOD(void, Submit, (IOWorker * ioWorker, UbioSmartPtr ubio), (override));
    MOCK_METHOD(void, Process, (), (override));
};

class MockDispatcherPolicyDirect : public DispatcherPolicyDirect
{
public:
    using DispatcherPolicyDirect::DispatcherPolicyDirect;
    MOCK_METHOD(void, Submit, (IOWorker * ioWorker, UbioSmartPtr ubio), (override));
    MOCK_METHOD(void, Process, (), (override));
};

class MockDispatcherPolicyQos : public DispatcherPolicyQos
{
public:
    using DispatcherPolicyQos::DispatcherPolicyQos;
    MOCK_METHOD(void, Submit, (IOWorker * ioWorker, UbioSmartPtr ubio), (override));
    MOCK_METHOD(void, Process, (), (override));
};

} // namespace pos
