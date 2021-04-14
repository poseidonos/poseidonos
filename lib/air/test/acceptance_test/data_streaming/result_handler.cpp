
#include "result_handler.h"
#include "rapidjson/document.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctime>

using namespace rapidjson;

void
ResultHandler::HandleData(unsigned int read_iops, unsigned int write_iops,
        unsigned int sleep_cnt, unsigned int tick_1ms_cnt)
{
    dummy_total_time++;
    dummy_total_sleep_cnt += sleep_cnt;
    double avg_sleep_cnt = (double)dummy_total_sleep_cnt/dummy_total_time;
    double avg_lat = (double)1000000000/avg_sleep_cnt;
    dummy_lat_avg = (uint32_t)avg_lat;
    dummy_read_iops = read_iops;
    dummy_write_iops = write_iops;

    if (CheckAIRDataFormat())
    {
        printf(" [DummyIO] Read: %u:%u, Write: %u:%u, Lat: %u:%u, period: %u\r",
                dummy_read_iops, air_read_iops,
                dummy_write_iops, air_write_iops,
                dummy_lat_avg, air_lat_avg,
                tick_1ms_cnt);
        fflush(stdout);

        format_pass_count++;
    }
    else
    {
        format_fail_count++;
    }

    if (CheckAIRDataPeriod(tick_1ms_cnt))
    {
        period_pass_count++;
    }
    else
    {
        period_fail_count++;
    }
}

bool
ResultHandler::IsUpdate(void)
{
    bool result {false};
    int fd {0};
    off_t file_size {0};

    fd = open("/tmp/air_result.json", O_RDONLY);

    if (fd < 0)
    {
        return false;
    }

    file_size = lseek(fd, 0, SEEK_END);

    if (prev_file_size != file_size)
    {
        prev_file_size = file_size;
        result = true;
    }

    close(fd);
    return result;
}

void
ResultHandler::OutputResult(std::string filename)
{
    printf("\n\n");
    
    int fd {0};
    std::string data;

    fd = open(filename.c_str(), O_CREAT | O_WRONLY | O_TRUNC, 0644);

    data.clear();
    data += "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
    data += "<testsuites tests=\"2\" ";

    if (format_fail_count > 0 && period_fail_count > 0)
        data += "failures=\"2\" ";
    else if (format_fail_count == 0 && period_fail_count == 0)
        data += "failures=\"0\" ";
    else
        data += "failures=\"1\" ";

    data += "disabled=\"0\" errors=\"0\" ";

    char* timestamp = new char[40] {0,};
    time_t curr_time;
    struct tm *curr_tm;
    curr_time = time(NULL);
    curr_tm = localtime(&curr_time);
    sprintf(timestamp, "timestamp=\"%u-%02u-%02uT%02u:%02u:%02u\" " , curr_tm->tm_year + 1900,
            curr_tm->tm_mon + 1, curr_tm->tm_mday, curr_tm->tm_hour,
            curr_tm->tm_min, curr_tm->tm_sec);
    data += timestamp;
    delete[] timestamp;

    char* testtime = new char[20] {0,};
    sprintf(testtime, "time=\"%.3f\" ", (float)dummy_total_time);
    data += testtime;
    delete[] testtime;

    char* testname = new char[1000] {0,};
    sprintf(testname, "name=\"%s\">\n", filename.c_str());
    data += testname;
    delete[] testname;

    data += "  <testsuite name=\"";

    char* testsuitename = new char[1000] {0,};
    sprintf(testsuitename, "%s", filename.c_str());
    char *tsn_ptr = strtok(testsuitename, ".");
    data += tsn_ptr;

    data += "\" tests=\"2\" ";

    if (format_fail_count > 0 && period_fail_count > 0)
        data += "failures=\"2\" ";
    else if (format_fail_count == 0 && period_fail_count == 0)
        data += "failures=\"0\" ";
    else
        data += "failures=\"1\" ";

    data += "disabled=\"0\" errors=\"0\" ";

    testtime = new char[20] {0,};
    sprintf(testtime, "time=\"%.3f\">\n", (float)dummy_total_time);
    data += testtime;
    delete[] testtime;

    if (0 == format_fail_count)
    {
        data += "    <testcase name=\"DataFormat\" status=\"run\" result=\"completed\" ";

        testtime = new char[20] {0,};
        sprintf(testtime, "time=\"%.3f\" ", (float)dummy_total_time);
        data += testtime;
        delete[] testtime;

        data += "classname=\"";
        data += tsn_ptr;
        data += "\" />\n";

    }
    else
    {
        data += "    <testcase name=\"DataFormat\" status=\"run\" result=\"completed\" ";

        testtime = new char[20] {0,};
        sprintf(testtime, "time=\"%.3f\" ", (float)dummy_total_time);
        data += testtime;
        delete[] testtime;

        data += "classname=\"";
        data += tsn_ptr;
        data += "\">\n";

        data += "      <failure message=\"DataFormat Fail!";
        char* ds_result = new char[100] {0,};
        sprintf(ds_result, ": Fail Count(%u), Pass Count(%u)",
                format_fail_count, format_pass_count);
        data += ds_result;
        delete[] ds_result;
        data += "\" type=\"\"><![CDATA[]]></failure>\n";

        data += "    </testcase>\n"; 
    }

    if (0 == period_fail_count)
    {
        data += "    <testcase name=\"DataPeriod\" status=\"run\" result=\"completed\" ";

        testtime = new char[20] {0,};
        sprintf(testtime, "time=\"%.3f\" ", (float)dummy_total_time);
        data += testtime;
        delete[] testtime;

        data += "classname=\"";
        data += tsn_ptr;
        data += "\" />\n";

    }
    else
    {
        data += "    <testcase name=\"DataPeriod\" status=\"run\" result=\"completed\" ";

        testtime = new char[20] {0,};
        sprintf(testtime, "time=\"%.3f\" ", (float)dummy_total_time);
        data += testtime;
        delete[] testtime;

        data += "classname=\"";
        data += tsn_ptr;
        data += "\">\n";

        data += "      <failure message=\"DataPeriod Fail!";
        char* ds_result = new char[100] {0,};
        sprintf(ds_result, ": Fail Count(%u), Pass Count(%u)",
                period_fail_count, period_pass_count);
        data += ds_result;
        delete[] ds_result;
        data += "\" type=\"\"><![CDATA[]]></failure>\n";

        data += "    </testcase>\n"; 
    }

    data += "  </testsuite>\n";
    data += "</testsuites>";

    delete[] testsuitename;

    if (-1 == write(fd, data.c_str(), data.length()))
    {
        printf("output write failed\n");
    }

    close(fd);
}

bool
ResultHandler::CheckAIRDataPeriod(unsigned int tick_1ms_cnt)
{
    if (tick_1ms_cnt >= 900 && tick_1ms_cnt <= 1100)
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool
ResultHandler::CheckAIRDataFormat(void)
{
    int fd {0};
    char* str {nullptr};
    off_t file_size {0};
    off_t data_size {0};
    off_t remain_size {0};
    char ch;

    fd = open("/tmp/air_result.json", O_RDONLY);

    if (fd < 0)
    {
        printf("/tmp/air_result.json file not found!\n");
        return false;
    }

    file_size = lseek(fd, 0, SEEK_END);
    remain_size = file_size;

    while (remain_size)
    {
        lseek(fd, -2, SEEK_CUR);
        if (-1 == read(fd, &ch, 1))
            break;

        if (ch == '\n')
        {
            break;
        }
        data_size++;
        remain_size--;
    }

    str = (char*)malloc(sizeof(char) * (data_size + 1));
    if (nullptr == str)
    {
        printf("memory alloc failed!\n");
        close(fd);
        return false;
    }
    memset(str, 0, data_size + 1);

    if (-1 == read(fd, str, data_size))
    {
        printf("file read failed\n");
        close(fd);
        free(str);
        return false;
    }

    Document doc;

    doc.Parse(str);
    if (false == doc.IsObject())
    {
        printf("data isn't json format!\n");
        close(fd);
        free(str);
        return false;
    }

    air_read_iops = 0;
    air_write_iops = 0;
    const Value& perf = doc["perf_data"];
    for (SizeType i = 0; i < perf.Size(); i++)
    {
        const Value& tid_arr = perf[i]["tid_arr"];
        for (SizeType j = 0; j < tid_arr.Size(); j++)
        {
            const Value& aid_arr = tid_arr[j]["aid_arr"];
            for (SizeType k = 0; k < aid_arr.Size(); k++)
            {
                air_read_iops += aid_arr[k]["iops_read"].GetUint();
                air_write_iops += aid_arr[k]["iops_write"].GetUint();
            }
        }
    }

    air_lat_avg = 0;
    const Value& lat = doc["lat_data"];
    for (SizeType i = 0; i < lat.Size(); i++)
    {
        const Value& tid_arr = lat[i]["tid_arr"];
        for (SizeType j = 0; j < tid_arr.Size(); j++)
        {
            const Value& aid_arr = tid_arr[j]["aid_arr"];
            const Value& rid_arr = aid_arr[0]["timelag_arr"];
            air_lat_avg = rid_arr[0]["mean"].GetUint();
        }
    }

    close(fd);
    free(str);
    return true;
}
