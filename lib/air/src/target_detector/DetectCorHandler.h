
#ifndef AIR_DETECT_COR_HANDLER_H
#define AIR_DETECT_COR_HANDLER_H

#include "src/lib/Design.h"
#include "src/target_detector/Detector.h"

namespace detect
{
class DetectCoRHandler : public lib_design::AbstractCoRHandler
{
public:
    explicit DetectCoRHandler(Detector* new_detector)
    : detector(new_detector)
    {
    }
    virtual ~DetectCoRHandler(void)
    {
    }
    virtual void
    HandleRequest(int option = 0)
    {
        detector->Process();
    }

private:
    Detector* detector{nullptr};
};

} // namespace detect

#endif // AIR_DETECT_COR_HANDLER_H
