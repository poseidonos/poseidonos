#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/mapper_service/mapper_service.h"

namespace pos
{
class MockMapperService : public MapperService
{
public:
    using MapperService::MapperService;
    MOCK_METHOD(IVSAMap*, GetIVSAMap, (std::string arrayName), (override));
    MOCK_METHOD(IVSAMap*, GetIVSAMap, (int arrayId), (override));
};

} // namespace pos
