#ifndef TELEMETRY_SERVER_H_
#define TELEMETRY_SERVER_H_

#include <string>

namespace pos
{

class TelemetryServer
{
public:
    void Run();

private:
    const std::string SERVER_ADDRESS = "0.0.0.0:50051";
};

} // namespace pos

#endif // TELEMETRY_SERVER_H_
