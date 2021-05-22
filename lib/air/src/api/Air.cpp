
#include "src/api/Air.h"

air::InstanceManager* AIR<true, true>::instance_manager = nullptr;
node::NodeManager* AIR<true, true>::node_manager = nullptr;
collection::CollectionManager* AIR<true, true>::collection_manager = nullptr;
thread_local node::NodeDataArray* AIR<true, true>::node_data_array = nullptr;
