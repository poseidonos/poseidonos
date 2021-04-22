#include "mock_global_meta.h"
#include "mock_node_meta.h"
#include "src/lib/Design.cpp"
#include "src/policy/PolicyCorHandler.h"
#include "src/policy/PolicyObserver.cpp"
#include "src/policy/PolicyObserver.h"
#include "src/policy/RuleManager.cpp"
#include "src/policy/RuleManager.h"
#include "src/policy/Ruler.cpp"
#include "src/policy/Ruler.h"

class PolicyTest : public ::testing::Test
{
public:
    MockGlobalMeta* mock_global_meta{nullptr};
    MockNodeMeta* mock_node_meta{nullptr};
    policy::Ruler* ruler{nullptr};
    policy::Subject* policy_subject{nullptr};
    policy::RuleManager* rule_manager{nullptr};
    policy::Observer* policy_observer{nullptr};
    policy::PolicyCoRHandler* policy_cor_handler{nullptr};

protected:
    PolicyTest()
    {
        mock_global_meta = new MockGlobalMeta{};
        mock_node_meta = new MockNodeMeta{};
        ruler = new policy::Ruler{mock_node_meta, mock_global_meta};

        policy_subject = new policy::Subject{};
        rule_manager = new policy::RuleManager{ruler, policy_subject};
        policy_observer = new policy::Observer{rule_manager};
        policy_cor_handler = new policy::PolicyCoRHandler{policy_observer};
    }
    ~PolicyTest()
    {
        if (nullptr != mock_global_meta)
        {
            delete mock_global_meta;
            mock_global_meta = nullptr;
        }
        if (nullptr != mock_node_meta)
        {
            delete mock_node_meta;
            mock_node_meta = nullptr;
        }
        if (nullptr != ruler)
        {
            delete ruler;
            ruler = nullptr;
        }
        if (nullptr != policy_subject)
        {
            delete policy_subject;
            policy_subject = nullptr;
        }
        if (nullptr != rule_manager)
        {
            delete rule_manager;
            rule_manager = nullptr;
        }
        if (nullptr != policy_subject)
        {
            delete policy_subject;
            policy_subject = nullptr;
        }
        if (nullptr != rule_manager)
        {
            delete rule_manager;
            rule_manager = nullptr;
        }
        if (nullptr != policy_observer)
        {
            delete policy_observer;
            policy_observer = nullptr;
        }
        if (nullptr != policy_cor_handler)
        {
            delete policy_cor_handler;
            policy_cor_handler = nullptr;
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
