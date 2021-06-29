/*
 *   BSD LICENSE
 *   Copyright (c) 2021 Samsung Electronics Corporation
 *   All rights reserved.
 *
 *   Redistribution and use in source and binary forms, with or without
 *   modification, are permitted provided that the following conditions
 *   are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in
 *       the documentation and/or other materials provided with the
 *       distribution.
 *     * Neither the name of Intel Corporation nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *   OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "src/allocator_service/allocator_interface_container.h"

#include "src/allocator/i_allocator_wbt.h"
#include "src/allocator/i_block_allocator.h"
#include "src/allocator/i_context_manager.h"
#include "src/allocator/i_context_replayer.h"
#include "src/allocator/i_wbstripe_allocator.h"
#include "src/include/pos_event_id.h"
#include "src/logger/logger.h"
namespace pos
{
template class AllocatorInterfaceContainer<IBlockAllocator>;
template class AllocatorInterfaceContainer<IWBStripeAllocator>;
template class AllocatorInterfaceContainer<IAllocatorWbt>;
template class AllocatorInterfaceContainer<IContextManager>;
template class AllocatorInterfaceContainer<IContextReplayer>;
template<typename T>
T*
AllocatorInterfaceContainer<T>::GetInterface(std::string name)
{
    if (name == "" && container.size() == 1)
    {
        return container.begin()->second;
    }

    auto found = container.find(name);
    if (found == container.end())
    {
        return nullptr;
    }
    else
    {
        return found->second;
    }
}

template<typename T>
void
AllocatorInterfaceContainer<T>::Register(std::string name, T* interface)
{
    if (container.find(name) == container.end())
    {
        container.emplace(name, interface);
    }
    else
    {
        POS_TRACE_ERROR(EID(MAPPER_ALREADY_EXIST), "{} for array {} is already registered", typeid(T).name(), name);
    }
}

template<typename T>
void
AllocatorInterfaceContainer<T>::Unregister(std::string name)
{
    if (container.find(name) != container.end())
    {
        container.erase(name);
    }
    else
    {
        POS_TRACE_INFO(EID(MAPPER_ALREADY_EXIST), "{} for array {} already unregistered", typeid(T).name(), name);
    }
}

} // namespace pos
