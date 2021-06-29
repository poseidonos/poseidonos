
#include "src/lib/Casting.h"

class CastingTest : public ::testing::Test
{
public:
    enum class TypeUINT32_T : uint32_t
    {
        NUM_ZERO = 0,
        NUM_ONE = 1,
        NUM_TWO = 2,
    };

    enum class TypeUINT64_T : uint64_t
    {
        NUM_ZERO = 0,
        NUM_ONE = 1,
        NUM_TWO = 2,
    };

    enum class TypeINT : int
    {
        NUM_NEGATIVE_TWO = -2,
        NUM_NEGATIVE_ONE = -1,
        NUM_ZERO = 0,
        NUM_POSITIVE_ONE = 1,
        NUM_POSITIVE_TWO = 2,
    };

    enum class TypeUINT : unsigned int
    {
        NUM_ZERO = 0,
        NUM_ONE = 1,
        NUM_TWO = 2,
    };

    enum class TypeBOOL : bool
    {
        BOOL_FALSE = false,
        BOOL_TRUE = true,
    };

protected:
    CastingTest()
    {
    }
    ~CastingTest() override
    {
    }
    void
    SetUp() override
    {
    }
    void
    TearDown() override
    {
    }
};
