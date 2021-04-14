
#include "src/config/ConfigInterface.h"

config::Config config::ConfigInterface::config;

static_assert(0 == cfg::CheckConfigRule(), "AIR Config syntax violation");
