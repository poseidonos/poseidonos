#include "src/admin/smart_log_update_request.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "spdk/pos.h"
namespace pos
{
TEST(SmartLogUpdateRequest, SmartLogUpdateRequest_Constructor_One)
{
    struct spdk_nvme_health_information_page resultBuffer;
    struct spdk_nvme_health_information_page inputBuffer;
    uint32_t originCore = 0;
    pos_io ibofIo;
    SmartLogUpdateRequest smartLogUpdateRequest(&resultBuffer, &inputBuffer, &ibofIo, originCore);
}

TEST(SmartLogUpdateRequest, SmartLogUpdateRequest_Constructor_One_Heap)
{
    struct spdk_nvme_health_information_page resultBuffer;
    struct spdk_nvme_health_information_page inputBuffer;
    uint32_t originCore = 0;
    pos_io ibofIo;
    SmartLogUpdateRequest* smartLogUpdateRequest = new SmartLogUpdateRequest(&resultBuffer, &inputBuffer, &ibofIo, originCore);
    delete smartLogUpdateRequest;
}

TEST(SmartLogUpdateRequest, _DoSpecificJob_AlwaysTrue)
{
    struct spdk_nvme_health_information_page* resultBuffer = new struct spdk_nvme_health_information_page();
    struct spdk_nvme_health_information_page* inputBuffer = new struct spdk_nvme_health_information_page();
    uint32_t originCore = 0;
    pos_io ibofIo;
    SmartLogUpdateRequest smartLogUpdateRequestTest(resultBuffer, inputBuffer, &ibofIo, originCore);
    bool actual;
    bool expected = true;
    actual = smartLogUpdateRequestTest.Execute();
    ASSERT_EQ(expected, actual);
    delete resultBuffer;
}

TEST(SmartLogUpdateRequest, _DoSpecificJob_ResultTempZero)
{
    struct spdk_nvme_health_information_page* resultBuffer = new struct spdk_nvme_health_information_page();
    struct spdk_nvme_health_information_page* inputBuffer = new struct spdk_nvme_health_information_page();
    resultBuffer->temperature = 0;
    inputBuffer->temperature = 1;

    uint32_t originCore = 0;
    pos_io ibofIo;
    SmartLogUpdateRequest smartLogUpdateRequestTest(resultBuffer, inputBuffer, &ibofIo, originCore);
    bool actual;
    bool expected = true;
    actual = smartLogUpdateRequestTest.Execute();
    ASSERT_EQ(expected, actual);

    delete resultBuffer;
}

TEST(SmartLogUpdateRequest, _DoSpecificJob_TempFromDiskNotZero)
{
    struct spdk_nvme_health_information_page* resultPage = new struct spdk_nvme_health_information_page();
    struct spdk_nvme_health_information_page* page = new struct spdk_nvme_health_information_page();

    resultPage->temperature = 9;
    page->temperature = 10;

    page->available_spare = 9;
    resultPage->available_spare = 10;

    page->available_spare_threshold = 9;
    resultPage->available_spare_threshold = 11;

    page->percentage_used = 12;
    resultPage->percentage_used = 10;

    page->warning_temp_time = 12;
    resultPage->warning_temp_time = 10;

    page->critical_temp_time = 12;
    resultPage->critical_temp_time = 9;

    page->temp_sensor[0] = 10;
    resultPage->temp_sensor[0] = 9;
    uint32_t originCore = 0;
    pos_io ibofIo;
    SmartLogUpdateRequest smartLogUpdateRequestTest(resultPage, page, &ibofIo, originCore);
    bool actual;
    bool expected = true;
    actual = smartLogUpdateRequestTest.Execute();
    ASSERT_EQ(expected, actual);

    delete resultPage;
}
TEST(SmartLogUpdateRequest, _DoSpecificJob_TempFromDiskNotZero_ResultPageTempgreater)
{
    struct spdk_nvme_health_information_page* resultPage = new struct spdk_nvme_health_information_page();
    struct spdk_nvme_health_information_page* page = new struct spdk_nvme_health_information_page();

    resultPage->temperature = 10;
    page->temperature = 8;

    page->available_spare = 10;
    resultPage->available_spare = 9;

    page->available_spare_threshold = 11;
    resultPage->available_spare_threshold = 9;

    page->percentage_used = 10;
    resultPage->percentage_used = 12;

    page->warning_temp_time = 10;
    resultPage->warning_temp_time = 12;

    page->critical_temp_time = 9;
    resultPage->critical_temp_time = 12;

    page->temp_sensor[0] = 9;
    resultPage->temp_sensor[0] = 19;
    uint32_t originCore = 0;
    pos_io ibofIo;
    SmartLogUpdateRequest smartLogUpdateRequestTest(resultPage, page, &ibofIo, originCore);
    bool actual;
    bool expected = true;
    actual = smartLogUpdateRequestTest.Execute();
    ASSERT_EQ(expected, actual);

    delete resultPage;
}
TEST(SmartLogUpdateRequest, _CalculateVarBasedVal_)
{
}

TEST(SmartLogUpdateRequest, _ClearMemory_)
{
}

} // namespace pos
