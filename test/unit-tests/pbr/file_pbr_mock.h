#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include "src/pbr/file_pbr.h"

namespace pbr
{
class MockFilePbr : public FilePbr
{
public:
    using FilePbr::FilePbr;
};

} // namespace pbr
