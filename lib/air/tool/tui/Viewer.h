#ifndef AIR_TUI_VIEWER_H
#define AIR_TUI_VIEWER_H

#include <string>

#include "src/lib/json/Json.h"
#include "tool/tui/ConfigTree.h"
#include "tool/tui/EventType.h"

namespace air
{
enum class NodeType : int
{
    NULLTYPE,
    PERFORMANCE,
    LATENCY,
    QUEUE,
    UTILIZATION,
    COUNT,
};

class Viewer
{
public:
    void Render(EventData data, AConfig& tree, int pid);

private:
    void _SetFilename(int pid);
    bool _Update(EventType type);
    bool _CheckMovement(EventType type);
    bool _CheckFilesize(void);
    void _ClearWindow(void);
    void _Draw(AConfig& tree);
    void _DrawHeadline(void);
    void _DrawGroup(AConfig& tree);
    void _DrawNode(ANode& tree, std::string name, JSONdoc& doc);
    void _SumData(JSONdoc& doc, NodeType type);
    void _DrawObj(JSONdoc& doc, NodeType type);
    void _DrawDefault(JSONdoc& doc);
    void _DrawPerf(JSONdoc& doc);
    void _DrawLat(JSONdoc& doc);
    void _DrawQueue(JSONdoc& doc);
    void _DrawUtilization(JSONdoc& doc);
    void _DrawCount(JSONdoc& doc);

    std::string filename{""};
    int file_size{0};
    int prev_file_size{0};
    bool file_update{false};
    int pid{-1};

    uint64_t sum_iops;
    uint64_t sum_bw;
    uint64_t sum_util_usage;
    uint64_t sum_util_total_usage;
};

} // namespace air

#endif // AIR_TUI_VIEWER_H
