#include <gtest/gtest.h>
#include "src/journal_rocks_intf/journal_rocks_intf.h"
#include <experimental/filesystem>

namespace pos {

TEST(JournalRocksIntf, OpenAndClose) {
    // Given array name and When JournalRocks opened
    JournalRocksIntf journalRocks("testJournalRocks");
    int openStatus = journalRocks.Open();
    std::string targetDirName = "/etc/pos/testJournalRocks_RocksJournal";

    // Then : Directory is exist, open status is success (0) and isOpened variable is true
    EXPECT_EQ(std::experimental::filesystem::exists(targetDirName), true);
    EXPECT_EQ(openStatus, 0);
    EXPECT_EQ(journalRocks.IsOpened(), true);

    // When : JournalRocks closed
    int closedStatus = journalRocks.Close();

    // Then : isOpened variable is false and closed status is success (0)
    EXPECT_EQ(journalRocks.IsOpened(), false);
    EXPECT_EQ(closedStatus, 0);

    // Teardown : remove rocksdb log files by removing temporary directory.
    int ret = std::experimental::filesystem::remove_all(targetDirName);
    EXPECT_TRUE(ret >= 1);
}

}  // namespace pos
