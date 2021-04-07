
#ifndef AIR_STREAM_H
#define AIR_STREAM_H

#include <sys/types.h>

namespace stream
{
class Stream
{
public:
    Stream(void);
    virtual ~Stream(void)
    {
    }

    void SendPacket(void);

private:
    pid_t pid{0};
};

} // namespace stream

#endif // AIR_STREAM_H
