#include "tool/tui/Viewer.h"

#include <sys/ioctl.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#include <fstream>
#include <iostream>

#include "tool/tui/LibToString.h"

#define RESET "\033[0m"
#define BLACK "\033[30m"              /* Black */
#define RED "\033[31m"                /* Red */
#define GREEN "\033[32m"              /* Green */
#define YELLOW "\033[33m"             /* Yellow */
#define BLUE "\033[34m"               /* Blue */
#define MAGENTA "\033[35m"            /* Magenta */
#define CYAN "\033[36m"               /* Cyan */
#define WHITE "\033[37m"              /* White */
#define BOLDBLACK "\033[1m\033[30m"   /* Bold Black */
#define BOLDRED "\033[1m\033[31m"     /* Bold Red */
#define BOLDGREEN "\033[1m\033[32m"   /* Bold Green */
#define BOLDYELLOW "\033[1m\033[33m"  /* Bold Yellow */
#define BOLDBLUE "\033[1m\033[34m"    /* Bold Blue */
#define BOLDMAGENTA "\033[1m\033[35m" /* Bold Magenta */
#define BOLDCYAN "\033[1m\033[36m"    /* Bold Cyan */
#define BOLDWHITE "\033[1m\033[37m"   /* Bold White */

void
air::Viewer::Render(EventData data, AConfig& tree, int pid)
{
    _SetFilename(pid);

    if (_Update(data.type))
    {
        _ClearWindow();
        _Draw(tree);
    }
}

void
air::Viewer::_SetFilename(int new_pid)
{
    pid = new_pid;
    filename = "/tmp/air_";
    time_t curr_time = time(NULL);
    tm* curr_tm = localtime(&curr_time);
    filename += std::to_string(curr_tm->tm_year + 1900);
    if (curr_tm->tm_mon + 1 < 10)
    {
        filename += "0";
    }
    filename += std::to_string(curr_tm->tm_mon + 1);
    if (curr_tm->tm_mday < 10)
    {
        filename += "0";
    }
    filename += std::to_string(curr_tm->tm_mday);
    filename += "_" + std::to_string(pid) + ".json";
}

bool
air::Viewer::_Update(EventType type)
{
    if (_CheckMovement(type))
    {
        return true;
    }

    if (_CheckFilesize())
    {
        return true;
    }

    return false;
}

bool
air::Viewer::_CheckMovement(EventType type)
{
    bool result{false};

    switch (type)
    {
        case air::EventType::MOVE_UP:
        case air::EventType::MOVE_DOWN:
        case air::EventType::MOVE_RIGHT:
        case air::EventType::MOVE_LEFT:
        {
            result = true;
            break;
        }
        default:
        {
            break;
        }
    }

    return result;
}

bool
air::Viewer::_CheckFilesize(void)
{
    bool result{false};
    struct stat file_stat;
    int rc = stat(filename.c_str(), &file_stat);
    if (0 == rc)
    {
        if (file_stat.st_size != file_size)
        {
            if (false == file_update)
            {
                prev_file_size = file_size;
            }
            file_size = file_stat.st_size;

            if (0 != prev_file_size)
            {
                file_update = true;
            }
        }
        else
        {
            if (file_update)
            {
                result = true;
                file_update = false;
            }
        }
    }

    return result;
}

void
air::Viewer::_ClearWindow(void)
{
    int result{0};
    result = system("clear");

    if (-1 == result)
    {
        std::cout << "clear window failed\n";
    }
}

void
air::Viewer::_Draw(AConfig& tree)
{
    std::ifstream file;
    std::string str_json;
    file.open(filename, std::ifstream::in | std::ifstream::ate);
    file.seekg(prev_file_size, file.beg);
    std::getline(file, str_json);
    file.close();

    air::parse("data", str_json);
    try
    {
        _DrawHeadline();
        _DrawGroup(tree);
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
    air::json_clear();
}

void
air::Viewer::_DrawHeadline(void)
{
    auto& head = air::json("data");
    std::stringstream stream;

    std::cout << "AIR TUI status: [";
    stream << head["play"];
    std::string status{stream.str()};
    if (std::string::npos != status.find("true"))
    {
        std::cout << BOLDGREEN << "play" << RESET;
    }
    else
    {
        std::cout << BOLDRED << "pause" << RESET;
    }

    std::cout << "],  interval: [";
    std::cout << BOLDGREEN << head["interval"] << RESET;
    std::cout << "],  ";

    std::cout << "timestamp: ";
    stream.str("");
    stream << head["timestamp"];
    std::string timestamp{stream.str()};

    time_t time;
    struct tm* tm;
    time = std::stoi(timestamp);
    tm = localtime(&time);
    std::cout << tm->tm_year + 1900 << "-" << tm->tm_mon + 1 << "-" << tm->tm_mday
              << ":" << tm->tm_hour << ":" << tm->tm_min << ":" << tm->tm_sec;

    std::cout << ", pid: " << pid << std::endl;

    std::cout << "arrow up/down: movement, arrow right/left: unfold/fold, q(esc): quit, 1~9: interval, i: init, o: run, x: stop\n";
}

void
air::Viewer::_DrawGroup(AConfig& tree)
{
    auto& group_json = air::json("data")["group"];
    std::stringstream stream;

    if (tree.pos_top)
    {
        std::cout << BOLDYELLOW << "(*)" << RESET;
    }
    else
    {
        std::cout << "( )";
    }
    std::cout << "   Top\n";

    for (auto i : air::range(group_json))
    {
        auto search_g = tree.group.find(i.key);
        if (tree.group.end() == search_g)
        {
            AGroup* NewGroup = new AGroup;
            stream.str("");
            stream << group_json[i.key]["group_id"];
            std::string gid{stream.str()};
            NewGroup->gid = std::stoi(gid);
            tree.group.insert({i.key, NewGroup});
        }
        search_g = tree.group.find(i.key);
        AGroup& group_st = *(search_g->second);
        if (group_st.here)
        {
            std::cout << BOLDYELLOW << "(*)" << RESET;
        }
        else
        {
            std::cout << "( )";
        }

        std::cout << "   ";
        if (group_st.fold)
        {
            std::cout << BOLDGREEN << "-" << RESET;
        }
        else
        {
            std::cout << BOLDGREEN << "+" << RESET;
        }
        std::cout << "Group:" << i.key << std::endl;

        if (group_st.fold)
        {
            continue;
        }

        auto& nodes = group_json[i.key]["node"];
        for (auto node : air::range(nodes))
        {
            auto search_n = group_st.node.find(node.key);
            if (group_st.node.end() == search_n)
            {
                ANode* NewNode = new ANode;
                stream.str("");
                stream << nodes[node.key]["node_id"];
                std::string nid{stream.str()};
                NewNode->nid = std::stoi(nid);
                group_st.node.insert({node.key, NewNode});
            }
            search_n = group_st.node.find(node.key);
            ANode& node_st = *(search_n->second);
            _DrawNode(node_st, node.key, nodes[node.key]);
        }
    }
}

void
air::Viewer::_DrawNode(ANode& node, std::string name, JSONdoc& doc)
{
    if (node.here)
    {
        std::cout << BOLDYELLOW << "(*)" << RESET;
    }
    else
    {
        std::cout << "( )";
    }

    std::stringstream stream;
    stream.str("");
    stream << doc["build"];
    std::string str_build{stream.str()};
    if (std::string::npos != str_build.find("false"))
    {
        std::cout << "[.]..Node:" << name << std::endl;
        return;
    }

    stream.str("");
    stream << doc["run"];
    std::string str_run{stream.str()};
    if (std::string::npos != str_run.find("false"))
    {
        std::cout << BOLDGREEN << "[X]" << RESET;
    }
    else
    {
        std::cout << BOLDGREEN << "[O]" << RESET;
    }

    if (node.fold)
    {
        std::cout << BOLDGREEN << "--" << RESET;
    }
    else
    {
        std::cout << BOLDGREEN << "++" << RESET;
    }

    std::cout << "Node:" << name << "(";
    std::cout << doc["type"] << ")\n";

    if (node.fold)
    {
        return;
    }

    stream.str("");
    stream << doc["type"];
    std::string str_node_type{stream.str()};
    NodeType node_type{NodeType::NULLTYPE};
    if (std::string::npos != str_node_type.find("\"performance\""))
    {
        node_type = NodeType::PERFORMANCE;
    }
    else if (std::string::npos != str_node_type.find("\"latency\""))
    {
        node_type = NodeType::LATENCY;
    }
    else if (std::string::npos != str_node_type.find("\"queue\""))
    {
        node_type = NodeType::QUEUE;
    }
    else if (std::string::npos != str_node_type.find("\"utilization\""))
    {
        node_type = NodeType::UTILIZATION;
    }
    else if (std::string::npos != str_node_type.find("\"count\""))
    {
        node_type = NodeType::COUNT;
    }

    sum_iops = 0;
    sum_bw = 0;
    sum_util_usage = 0;
    sum_util_total_usage = 0;
    for (auto obj : air::range(doc["objs"]))
    {
        _SumData(doc["objs"][obj.key], node_type);
    }

    if (NodeType::PERFORMANCE == node_type)
    {
        std::cout << "        SUM_Period(iops:" << Double2String((double)sum_iops)
                  << ", bw:" << Double2StringWithBWFormat((double)sum_bw) << ")\n";
    }

    for (auto obj : air::range(doc["objs"]))
    {
        _DrawObj(doc["objs"][obj.key], node_type);
    }
}

void
air::Viewer::_SumData(JSONdoc& doc, NodeType type)
{
    if (NodeType::PERFORMANCE == type)
    {
        std::string str;
        std::stringstream stream;
        uint64_t value;

        stream.str("");
        stream << doc["iops"];
        value = std::stoull(stream.str());
        sum_iops += value;

        stream.str("");
        stream << doc["bw"];
        value = std::stoull(stream.str());
        sum_bw += value;
    }
    else if (NodeType::UTILIZATION == type)
    {
        std::string str;
        std::stringstream stream;
        uint64_t value;

        stream.str("");
        stream << doc["usage"];
        value = std::stoull(stream.str());
        sum_util_usage += value;

        stream.str("");
        stream << doc["total_usage"];
        value = std::stoull(stream.str());
        sum_util_total_usage += value;
    }
}

void
air::Viewer::_DrawObj(JSONdoc& doc, NodeType type)
{
    _DrawDefault(doc);

    switch (type)
    {
        case NodeType::PERFORMANCE:
        {
            _DrawPerf(doc);
            break;
        }
        case NodeType::LATENCY:
        {
            _DrawLat(doc);
            break;
        }
        case NodeType::QUEUE:
        {
            _DrawQueue(doc);
            break;
        }
        case NodeType::UTILIZATION:
        {
            _DrawUtilization(doc);
            break;
        }
        case NodeType::COUNT:
        {
            _DrawCount(doc);
            break;
        }
        default:
        {
            break;
        }
    }

    std::cout << std::endl;
}

void
air::Viewer::_DrawDefault(JSONdoc& doc)
{
    std::cout << "      ";
    std::cout << doc["target_name"] << "(";
    std::cout << doc["target_id"] << "), index:";
    std::cout << doc["index"] << ", filter:";
    std::cout << doc["filter"];
}

void
air::Viewer::_DrawPerf(JSONdoc& doc)
{
    std::string str;
    std::stringstream stream;
    double number;

    str = " Period(iops:";

    stream.str("");
    stream << doc["iops"];
    number = std::stod(stream.str());
    str += Double2String(number);
    str += ", bw:";

    stream.str("");
    stream << doc["bw"];
    number = std::stod(stream.str());
    str += Double2StringWithBWFormat(number);
    str += ", ";

    for (uint32_t index = 1; index <= 10; index++)
    {
        if (doc.HasKey("cnt_" + std::to_string(index)))
        {
            if (1 != index)
            {
                str += ", ";
            }
            stream.str("");
            stream << doc["cnt_" + std::to_string(index)];
            str += stream.str();
        }
    }
    str += "), Total(iops_avg:";

    stream.str("");
    stream << doc["iops_avg"];
    number = std::stod(stream.str());
    str += Double2String(number);
    str += ", bw_avg:";

    stream.str("");
    stream << doc["bw_avg"];
    number = std::stod(stream.str());
    str += Double2StringWithBWFormat(number);
    str += ")";

    std::cout << str;
}

void
air::Viewer::_DrawLat(JSONdoc& doc)
{
    std::string str;
    std::stringstream stream;
    uint64_t number;

    str = " Period(avg:";

    stream.str("");
    stream << doc["mean"];
    number = std::stoull(stream.str());
    str += ULL2StringWithLatencyFormat(number);
    str += ", median:";

    stream.str("");
    stream << doc["median"];
    number = std::stoull(stream.str());
    str += ULL2StringWithLatencyFormat(number);
    str += ", max:";

    stream.str("");
    stream << doc["max"];
    number = std::stoull(stream.str());
    str += ULL2StringWithLatencyFormat(number);
    str += ", sample:";

    stream.str("");
    stream << doc["sample_cnt"];
    number = std::stod(stream.str());
    str += Double2String(number);
    str += "), Total(avg:";

    stream.str("");
    stream << doc["total_mean"];
    number = std::stoull(stream.str());
    str += ULL2StringWithLatencyFormat(number);
    str += ", median:";

    stream.str("");
    stream << doc["total_median"];
    number = std::stoull(stream.str());
    str += ULL2StringWithLatencyFormat(number);
    str += ", max:";

    stream.str("");
    stream << doc["total_max"];
    number = std::stoull(stream.str());
    str += ULL2StringWithLatencyFormat(number);
    str += ", sample:";

    stream.str("");
    stream << doc["total_sample_cnt"];
    number = std::stod(stream.str());
    str += Double2String(number);
    str += ")";

    std::cout << str;
}

void
air::Viewer::_DrawQueue(JSONdoc& doc)
{
    std::string str;
    std::stringstream stream;
    double number;

    str = " Period(avg:";

    stream.str("");
    stream << doc["depth_period_avg"];
    number = std::stod(stream.str());
    str += Double2String(number);
    str += ", max:";

    stream.str("");
    stream << doc["depth_period_max"];
    number = std::stod(stream.str());
    str += Double2String(number);
    str += "), Total(avg:";

    stream.str("");
    stream << doc["depth_total_avg"];
    number = std::stod(stream.str());
    str += Double2String(number);
    str += ", max:";

    stream.str("");
    stream << doc["depth_total_max"];
    number = std::stod(stream.str());
    str += Double2String(number);
    str += ")";

    std::cout << str;
}

void
air::Viewer::_DrawUtilization(JSONdoc& doc)
{
    std::string str;
    std::stringstream stream;
    uint64_t number;

    str = " Period(usage:";

    stream.str("");
    stream << doc["usage"];
    number = std::stoull(stream.str());
    str += Double2String((double)number);
    str += ", ";

    stream.str("");
    stream << std::fixed << std::setprecision(2);
    if (0 != sum_util_usage)
    {
        stream << (((double)number / sum_util_usage) * 100.0f);
    }
    str += stream.str();
    str += "%), Total(usage:";

    stream.str("");
    stream << doc["total_usage"];
    number = std::stoull(stream.str());
    str += Double2String((double)number);
    str += ", ";

    stream.str("");
    stream << std::fixed << std::setprecision(2);
    if (0 != sum_util_total_usage)
    {
        stream << (((double)number / sum_util_total_usage) * 100.0f);
    }
    str += stream.str();
    str += "%)";

    std::cout << str;
}

void
air::Viewer::_DrawCount(JSONdoc& doc)
{
    std::string str;
    std::stringstream stream;
    uint64_t number;

    str = " Period(count:";

    stream.str("");
    stream << doc["count"];
    number = std::stoll(stream.str());
    str += LL2String(number);
    str += "), Total(count:";

    stream.str("");
    stream << doc["total_count"];
    number = std::stoll(stream.str());
    str += LL2String(number);
    str += ")";

    std::cout << str;
}
