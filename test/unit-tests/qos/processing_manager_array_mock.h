#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/qos/processing_manager_array.h"

namespace pos
{
class MockQosProcessingManagerArray : public QosProcessingManagerArray
{
public:
    using QosProcessingManagerArray::QosProcessingManagerArray;
};

} // namespace pos
