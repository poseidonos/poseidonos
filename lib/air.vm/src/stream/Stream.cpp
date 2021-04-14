
#include "src/stream/Stream.h"

#include <time.h>
#include <unistd.h>

#include <fstream>
#include <string>

#include "src/lib/json/Json.h"

stream::Stream::Stream(void)
{
    pid = getpid();
}

void
stream::Stream::SendPacket(void)
{
    // new json
    std::ofstream export_file;
    // out : output(default)
    // app : append
    // ate : at end, output position starts at the end of the file.
    // export_file.open("tmp.json", std::ofstream::out | std::ofstream::app);

    std::string filename{"/tmp/air_"};
    time_t curr_time = time(NULL);
    struct tm curr_tm;
    localtime_r(&curr_time, &curr_tm);
    filename += std::to_string(curr_tm.tm_year + 1900);
    if (curr_tm.tm_mon + 1 < 10)
    {
        filename += "0";
    }
    filename += std::to_string(curr_tm.tm_mon + 1);
    if (curr_tm.tm_mday < 10)
    {
        filename += "0";
    }
    filename += std::to_string(curr_tm.tm_mday);
    filename += "_";
    filename += std::to_string(pid);
    filename += ".json";

    export_file.open(filename, std::ofstream::out | std::ofstream::app);
    if (!export_file.fail())
    {
        export_file << air::json("air") << std::endl;
    }
    export_file.close();

    air::json_clear();
}
