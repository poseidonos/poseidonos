#pragma once

#include <string>

#include "src/state/interface/i_state_control.h"

namespace pos
{
class StateSubscriptionMock : public IStateControl
{
public:
    StateSubscriptionMock(void);
    virtual ~StateSubscriptionMock(void);

    virtual void Subscribe(IStateObserver* sub, string name) override {}
    virtual void Unsubscribe(IStateObserver* sub) override {}
    virtual StateContext* GetState(void) override;
    virtual void Invoke(StateContext* ctx) override;
    virtual void Remove(StateContext* ctx) override {}
    virtual bool Exists(SituationEnum situ) override { return true; }

private:
    StateContext* currentState;
};
} // namespace pos
