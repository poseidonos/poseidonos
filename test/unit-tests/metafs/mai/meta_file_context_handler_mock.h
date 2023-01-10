#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/metafs/mai/meta_file_context_handler.h"

namespace pos
{
class MockMetaFileContextHandler : public MetaFileContextHandler
{
public:
    using MetaFileContextHandler::MetaFileContextHandler;
    MOCK_METHOD(void, Initialize, (const uint64_t signature));
    MOCK_METHOD(MetaFileContext*, GetFileContext, (const FileDescriptorType fd, const MetaVolumeType type));
    MOCK_METHOD(void, RemoveFileContext, (const FileDescriptorType fd, const MetaVolumeType type));
    MOCK_METHOD(void, AddFileContext, (std::string& fileName, const FileDescriptorType fd, const MetaVolumeType type));
};

} // namespace pos
