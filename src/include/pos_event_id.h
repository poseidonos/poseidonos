#pragma once

#include "src/event/event_manager.h"

#define UNKNOWN_EVENT_ID -1000
#define STR(S) #S

#define EID(X) (eventManager.GetEventIdFromMap(STR(X)))
#define ERRID(X) (-eventManager.GetEventIdFromMap(STR(X)))