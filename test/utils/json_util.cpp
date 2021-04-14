#include <vector>

#include "src/helper/json_helper.h"
#include "src/volume/volume.h"
#include "test/utils/test_util.h"

namespace pos
{
std::string
volumeListToString(std::vector<pos::Volume*> volumes)
{
    // refer to VolumeMetaIntf::SaveVolumes() for the full details. possible candidate for refactoring
    JsonElement root("");
    JsonArray jsonArray("volumes");
    for (auto& v : volumes)
    {
        JsonElement elem("");
        elem.SetAttribute(JsonAttribute("name", "\"" + v->GetName() + "\""));
        elem.SetAttribute(JsonAttribute("id", std::to_string(v->ID)));
        elem.SetAttribute(JsonAttribute("total", std::to_string(v->TotalSize())));
        elem.SetAttribute(JsonAttribute("maxiops", std::to_string(v->MaxIOPS())));
        elem.SetAttribute(JsonAttribute("maxbw", std::to_string(v->MaxBW())));
        jsonArray.AddElement(elem);
    }
    root.SetArray(jsonArray);
    return root.ToJson();
}

} // namespace pos_test
