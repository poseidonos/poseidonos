
#ifndef AIR_STREAM_COR_HANDLER_H
#define AIR_STREAM_COR_HANDLER_H

#include "src/lib/Design.h"
#include "src/stream/Stream.h"

namespace stream
{
class StreamCoRHandler : public lib_design::AbstractCoRHandler
{
public:
    explicit StreamCoRHandler(Stream* new_stream)
    : stream(new_stream)
    {
    }
    virtual ~StreamCoRHandler(void)
    {
    }
    virtual void
    HandleRequest(int option = 0)
    {
        stream->SendPacket();
    }

private:
    Stream* stream{nullptr};
};

} // namespace stream

#endif // AIR_STREAM_COR_HANDLER_H
