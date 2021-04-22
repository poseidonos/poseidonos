
#include "src/lib/Hash.cpp"
#include "src/profile_data/node/NodeManager.cpp"
#include "src/profile_data/node/NodeManager.h"
#include "src/profile_data/node/NodeThread.cpp"

class MockNodeManager : public node::NodeManager
{
public:
    virtual ~MockNodeManager()
    {
    }
    node::ThreadArray*
    GetThread(uint32_t tid)
    {
        std::map<uint32_t, node::ThreadArray>::iterator tid_iter;

        tid_iter = thread_map.find(tid);
        if (tid_iter != thread_map.end())
        {
            return &(tid_iter->second);
        }
        return nullptr;
    }

    int
    CreateThread(uint32_t tid)
    {
        node::ThreadArray* thread_array = new node::ThreadArray;
        thread_map.insert(std::make_pair(tid, *thread_array));

        return 1;
    }

    void
    DeleteThread(node::ThreadArray* thread_array)
    {
        return;
    }

    bool
    CanDelete(node::ThreadArray* thread_array)
    {
        std::map<uint32_t, node::ThreadArray>::iterator it;

        it = thread_map.begin();
        while (it != thread_map.end())
        {
            if (&(it->second) == thread_array &&
                it->first == 11111) // tid 11111 can not delete
            {
                return false;
            }
            it++;
        }
        return true;
    }

private:
};
