#ifndef AIR_TUI_FILE_DETECTOR_H
#define AIR_TUI_FILE_DETECTOR_H

#include <map>

namespace air
{
class FileDetector
{
public:
    ~FileDetector(void)
    {
        pid_map.clear();
    }
    int Detect(void);
    void HandleTimeout(void);

private:
    void _Monitoring(void);
    void _InitData(void);
    void _UpdatePidMap(void);
    void _UpdatePidStatus(void);
    void _SelectPid(void);

    int pid{-1};
    bool waiting{false};
    bool exit{false};
    int candidates{0};

    std::map<int, bool> pid_map;
};
} // namespace air

#endif // AIR_TUI_FILE_DETECTOR_H
