#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/metafs/mvm/volume/file_descriptor_allocator.h"

namespace pos
{
class MockFileDescriptorAllocator : public FileDescriptorAllocator
{
public:
    using FileDescriptorAllocator::FileDescriptorAllocator;

    MOCK_METHOD(FileDescriptorType, Alloc, (std::string& fileName));
    MOCK_METHOD(FileDescriptorType, Alloc, (StringHashType fileKey));

    MOCK_METHOD(void, Free, (std::string& fileName, FileDescriptorType fd));
    MOCK_METHOD(void, Free, (StringHashType fileKey, FileDescriptorType fd));

    MOCK_METHOD(FileDescriptorType, FindFdByName, (std::string& fileName));
    MOCK_METHOD(FileDescriptorType, FindFdByHashKey, (StringHashType fileKey));

    MOCK_METHOD(bool, IsGivenFileCreated, (std::string& fileName));
    MOCK_METHOD(bool, IsGivenFileCreated, (StringHashType fileKey));

    MOCK_METHOD(void, Reset, ());
    MOCK_METHOD(void, UpdateFreeMap, (FileDescriptorType fd));
    MOCK_METHOD(void, UpdateLookupMap, (StringHashType fileKey, FileDescriptorType fd));
    MOCK_METHOD(uint32_t, GetMaxFileCount, ());

    // only for test
    MOCK_METHOD(StringHashMap*, GetLookupMap, ());
    // only for test
    MOCK_METHOD(std::set<FileDescriptorType>*, GetFreeMap, ());
};

} // namespace pos
