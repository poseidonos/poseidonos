
#include "result_handler.h"
#include "rapidjson/document.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctime>

using namespace rapidjson;

bool
ResultHandler::HandleData(unsigned int read_iops, unsigned int write_iops,
        unsigned int sleep_cnt, unsigned int new_error_margin,
        unsigned int new_confidence_interval)
{
    bool result {false};

    dummy_total_time++;
    dummy_total_sleep_cnt += sleep_cnt;
    double avg_sleep_cnt = (double)dummy_total_sleep_cnt/dummy_total_time;
    double avg_lat = (double)1000000000/avg_sleep_cnt;
    dummy_lat_avg = (uint32_t)avg_lat;
    dummy_read_iops = read_iops;
    dummy_write_iops = write_iops;
    error_margin = new_error_margin;
    confidence_interval = new_confidence_interval;

    if (GetAIRData())
    {
        ComparePerfData();
        CompareLatData();
        result = CheckResult();
    }

    return result;
}

void
ResultHandler::OutputResult(std::string filename, bool result)
{
    int fd {0};
    std::string data;

    fd = open(filename.c_str(), O_CREAT | O_WRONLY | O_TRUNC, 0644);

    data.clear();
    data += "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
    data += "<testsuites tests=\"2\" ";

    char* failures = new char[15] {0,};
    sprintf(failures, "failures=\"%u\" ", fail_count);
    data += failures;
    delete[] failures;

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

    failures = new char[15] {0,};
    sprintf(failures, "failures=\"%u\" ", fail_count);
    data += failures;
    delete[] failures;

    data += "disabled=\"0\" errors=\"0\" ";

    testtime = new char[20] {0,};
    sprintf(testtime, "time=\"%.3f\">\n", (float)dummy_total_time);
    data += testtime;
    delete[] testtime;

    if (iops_pass)
    {
        data += "    <testcase name=\"Performance\" status=\"run\" result=\"completed\" ";

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
        data += "    <testcase name=\"Performance\" status=\"run\" result=\"completed\" ";

        testtime = new char[20] {0,};
        sprintf(testtime, "time=\"%.3f\" ", (float)dummy_total_time);
        data += testtime;
        delete[] testtime;

        data += "classname=\"";
        data += tsn_ptr;
        data += "\">\n";

        data += "      <failure message=\"Performance CI unsatisfied";
        char* perf_result = new char[100] {0,};
        float iops_crd = (float)iops_pass_cnt*100/(iops_pass_cnt + iops_fail_cnt);
        sprintf(perf_result, ": %.2f%%[%u/%u]", iops_crd, iops_pass_cnt,
                (iops_pass_cnt + iops_fail_cnt));
        data += perf_result;
        delete[] perf_result;
        data += "\" type=\"\"><![CDATA[]]></failure>\n";

        data += "    </testcase>\n"; 
    }

    if (lat_pass)
    {
        data += "    <testcase name=\"Latency\" status=\"run\" result=\"completed\" ";

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
        data += "    <testcase name=\"Latency\" status=\"run\" result=\"completed\" ";

        testtime = new char[20] {0,};
        sprintf(testtime, "time=\"%.3f\" ", (float)dummy_total_time);
        data += testtime;
        delete[] testtime;

        data += "classname=\"";
        data += tsn_ptr;
        data += "\">\n";

        data += "      <failure message=\"Latency CI unsatisfied";
        char* lat_result = new char[100] {0,};
        float lat_crd = (float)lat_pass_cnt*100/(lat_pass_cnt + lat_fail_cnt);
        sprintf(lat_result, ": %.2f%%[%u/%u]", lat_crd, lat_pass_cnt,
                (lat_pass_cnt + lat_fail_cnt));
        data += lat_result;
        delete[] lat_result;
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
ResultHandler::GetAIRData()
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
        const Value& aid_arr = lat[i]["aid_arr"];
        for (SizeType j = 0; j < aid_arr.Size(); j++)
        {
            const Value& sid_arr = aid_arr[j]["sid_arr"];
            air_lat_avg = sid_arr[0]["mean"].GetUint();
        }
    }

    close(fd);
    free(str);
    return true;
}

void
ResultHandler::ComparePerfData()
{
    double upper_margin = ((double)(100 + error_margin))/100;
    double lower_margin = ((double)(100 - error_margin))/100;

    if (dummy_read_iops)
    {
        uint32_t upper_bound_read_iops =
            (uint32_t)((double)dummy_read_iops*upper_margin);
        uint32_t lower_bound_read_iops =
            (uint32_t)((double)dummy_read_iops*lower_margin);

        if (upper_bound_read_iops >= air_read_iops &&
                lower_bound_read_iops <= air_read_iops)
        {
            iops_pass_cnt++;
        }
        else
        {
            iops_fail_cnt++;
        }
    }

    if (dummy_write_iops)
    {
        uint32_t upper_bound_write_iops =
            (uint32_t)((double)dummy_write_iops*upper_margin);
        uint32_t lower_bound_write_iops =
            (uint32_t)((double)dummy_write_iops*lower_margin);

        if (upper_bound_write_iops >= air_write_iops &&
                lower_bound_write_iops <= air_write_iops)
        {
            iops_pass_cnt++;
        }
        else
        {
            iops_fail_cnt++;
        }
    }
}

void
ResultHandler::CompareLatData()
{
    double upper_margin = ((double)(100 + error_margin))/100;
    double lower_margin = ((double)(100 - error_margin))/100;

    if (dummy_lat_avg)
    {
        uint32_t upper_bound_lat_avg =
            (uint32_t)((double)dummy_lat_avg*upper_margin);
        uint32_t lower_bound_lat_avg =
            (uint32_t)((double)dummy_lat_avg*lower_margin);

        if (upper_bound_lat_avg >= air_lat_avg &&
                lower_bound_lat_avg <= air_lat_avg)
        {
            lat_pass_cnt++;
        }
        else
        {
            lat_fail_cnt++;
        }
    }
}

bool
ResultHandler::CheckResult()
{
    bool result {false};

    printf(" [DummyIO] Read: %u:%u, Write: %u:%u, Lat: %u:%u\n",
            dummy_read_iops, air_read_iops,
            dummy_write_iops, air_write_iops,
            dummy_lat_avg, air_lat_avg);
    
    float iops_crd = (float)iops_pass_cnt*100/(iops_pass_cnt + iops_fail_cnt);
    float lat_crd = (float)lat_pass_cnt*100/(lat_pass_cnt + lat_fail_cnt);
    printf(" IOPS: %.2f%%[%u/%u], avg lat: %.2f%%[%u/%u]\n\n",
            iops_crd, iops_pass_cnt, iops_pass_cnt + iops_fail_cnt,
            lat_crd, lat_pass_cnt, lat_pass_cnt + lat_fail_cnt);

    fflush(stdout);

    float con_int = (float)confidence_interval;

    fail_count = 0;
    iops_pass = true;
    lat_pass = true;

    if (con_int > iops_crd)
    {
        fail_count++;
        iops_pass = false;
    }

    if (con_int > lat_crd)
    {
        fail_count++;
        lat_pass = false;
    }

    if (con_int <= iops_crd && con_int <= lat_crd)
    {
        result = true;
    }

    return result;
}
