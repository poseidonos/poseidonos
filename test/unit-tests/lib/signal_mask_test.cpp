#include <gtest/gtest.h>
#include "src/lib/signal_mask.h"

namespace pos
{

TEST(SignalMask, MaskSignal_)
{
    sigset_t oldset;
    SignalMask::MaskSignal(&oldset);
    SignalMask::RestoreSignal(&oldset);
    SignalMask::MaskSignal(SIGSEGV, &oldset);
    SignalMask::RestoreSignal(&oldset);
}

}  // namespace pos
