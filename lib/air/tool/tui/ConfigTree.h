#ifndef AIR_TUI_CONFIG_TREE_H
#define AIR_TUI_CONFIG_TREE_H

#include <map>
#include <string>

namespace air
{
struct ANode
{
    int nid{-1};
    bool here{false};
    bool fold{false};
};

struct AGroup
{
    void
    Clean(void)
    {
        for (auto i : node)
        {
            ANode* n = i.second;
            delete n;
        }
    }
    int gid{-1};
    bool here{false};
    bool fold{false};
    std::map<std::string, ANode*> node;
};

struct AConfig
{
    ~AConfig()
    {
        for (auto i : group)
        {
            AGroup* g = i.second;
            g->Clean();
            delete g;
        }
    }
    bool pos_top{true};
    bool pos_group{false};
    bool pos_node{false};
    int pos_id{-1};
    std::map<std::string, AGroup*> group;
};

} // namespace air

#endif // AIR_TUI_CONFIG_TREE_H
