
#ifndef RULE_CHECKER_H
#define RULE_CHECKER_H

#include <string>

class RuleChecker
{
public:
    bool CheckRule(int argc, char **argv);
    unsigned int GetRunTime() { return run_time; }
    std::string GetFilename() { return filename; }

private:
    void PrintHelp();
    bool CheckRuntime(char *arg);
    bool CheckFilename(char *arg);

    unsigned int run_time {60};         // resolution: second
    std::string filename;
};

#endif // #define RULE_CHECKER_H
