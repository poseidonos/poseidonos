#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/allocator/address/allocator_address_info.h"

using ::testing::Return;

namespace pos
{
class AddressInfoTester : public AllocatorAddressInfo
{
public:
    AddressInfoTester(void)
    {
        const uint32_t numOfSegment = 10;
        const uint32_t numOfStripesPerSegment = 2;
        const uint32_t numOfBlksPerStripe = 128;
        const uint32_t numOfBlksPerSegment = numOfStripesPerSegment * numOfBlksPerStripe;
        const uint32_t numOfWbStripes = 1;
        const uint32_t numOfUserAreaStripes = numOfSegment * numOfStripesPerSegment * numOfBlksPerStripe;

        ON_CALL(*this, GetnumUserAreaSegments).WillByDefault(Return(numOfSegment));
        ON_CALL(*this, GetstripesPerSegment).WillByDefault(Return(numOfStripesPerSegment));
        ON_CALL(*this, GetblksPerSegment).WillByDefault(Return(numOfBlksPerSegment));
        ON_CALL(*this, GetblksPerStripe).WillByDefault(Return(numOfBlksPerStripe));
        ON_CALL(*this, GetnumWbStripes).WillByDefault(Return(numOfWbStripes));
        ON_CALL(*this, GetnumUserAreaStripes).WillByDefault(Return(numOfWbStripes));
    };

    virtual ~AddressInfoTester(void) {};

    MOCK_METHOD(uint32_t, GetnumUserAreaSegments, ());
    MOCK_METHOD(uint32_t, GetstripesPerSegment, ());
    MOCK_METHOD(uint32_t, GetblksPerSegment, ());
    MOCK_METHOD(uint32_t, GetblksPerStripe, ());
    MOCK_METHOD(uint32_t, GetnumWbStripes, ());
    MOCK_METHOD(uint32_t, GetnumUserAreaStripes, ());
};
}
