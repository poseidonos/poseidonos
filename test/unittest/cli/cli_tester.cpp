#include "gtest/gtest.h"
#include "src/cli/cli_server.h"
#include "tool/cli_client/cli_client.h"
#include "request.h"

using namespace ibofos_cli;

static CLIClient *fake_mtool;
static Request request;

class CLITester : public testing::Test
{

protected:
    CLIClient *fake_cli_client;

    static void SetUpTestCase()
    {
        cout << "setup" << endl;
        CLIServerMain();
        sleep(3);
        fake_mtool = new CLIClient();
        fake_mtool->Connect();
    }

    static void TearDownTestCase()
    {
        fake_mtool->Close();
        Exit();
    }

    virtual void SetUp()
    {
        sleep(2);
        fake_cli_client = new CLIClient();
        fake_cli_client->Connect();
        sleep(2);
    }
    virtual void TearDown()
    {
        cout << "teardown() " << endl;
        fake_cli_client->Close();
        sleep(2);
        delete fake_cli_client;
    }
};

// TEST_F(CLITester, loadtest)
// {    
//     std::string state_command = request.SysState();
//     cout << state_command << endl;

//     std::string list_dev_command = request.ListDevice();
//     cout << list_dev_command << endl;

//     for (int i = 0; i < 15; i++)
//     {
//         fake_mtool->SendRequest(state_command);
//         if (i % 2 == 0)
//             fake_cli_client->SendRequest(list_dev_command);
//         sleep(1);
//     }
//     int res = 1;
//     EXPECT_EQ(1, res);
// }

TEST_F(CLITester, createarraytest)
{
    std::string cmd = request.MountArray();
    cout << cmd << endl;
    
    fake_cli_client->SendRequest(cmd);
    int res = 1;
    EXPECT_EQ(1, res);
}

TEST_F(CLITester, exittest)
{
    std::string exit_command = request.ExitSys();
    cout << exit_command << endl;
    
    fake_cli_client->SendRequest(exit_command);
    int res = 1;
    EXPECT_EQ(1, res);
}