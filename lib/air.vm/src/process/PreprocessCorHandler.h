
#ifndef AIR_PREPROCESS_COR_HANDLER_H
#define AIR_PREPROCESS_COR_HANDLER_H

#include "src/lib/Design.h"
#include "src/process/Preprocessor.h"

namespace process
{
class PreprocessCoRHandler : public lib_design::AbstractCoRHandler
{
public:
    explicit PreprocessCoRHandler(Preprocessor* new_preprocessor)
    : preprocessor(new_preprocessor)
    {
    }
    virtual ~PreprocessCoRHandler(void)
    {
    }
    virtual void
    HandleRequest(int option = 0)
    {
        preprocessor->Run(option);
    }

private:
    Preprocessor* preprocessor{nullptr};
};

} // namespace process

#endif // AIR_PREPROCESS_COR_HANDLER_H
