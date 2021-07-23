#pragma once

#include <string>

#include "src/cli/command.h"
#include "src/network/nvmf_target.h"

namespace pos_cli
{
class DeleteSubsystemCommand : public Command
{
public:
    DeleteSubsystemCommand(void);
    ~DeleteSubsystemCommand(void) override;
    string Execute(json& doc, string rid) override;

private:
    bool _CheckParamValidityAndGetNqn(json& doc);
    int _DeleteSubsystem(json& doc);
    string errorMessage;
    string subnqn;
    NvmfTarget* nvmfTarget;
};
}; // namespace pos_cli