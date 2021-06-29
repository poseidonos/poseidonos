#include "state_subscription_mock.h"
#include "src/state/state_context.h"

namespace pos
{
StateSubscriptionMock::StateSubscriptionMock(void)
: currentState(nullptr)
{
}

StateSubscriptionMock::~StateSubscriptionMock(void)
{
}

StateContext*
StateSubscriptionMock::GetState(void)
{
    return currentState;
}

void
StateSubscriptionMock::Invoke(StateContext* ctx)
{
    currentState = ctx;
}
} // namespace pos
