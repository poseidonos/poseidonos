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

#ifndef IBOFOS_SINGLETON_H_
#define IBOFOS_SINGLETON_H_

#include <atomic>
#include <mutex>
#include <utility>

namespace pos
{
template<typename T>
class Singleton
{
public:
    template<typename... Args>
    static T*
    Instance(Args... args)
    {
        T* tmp = instance.load();
        if (tmp == nullptr)
        {
            std::lock_guard<std::mutex> lock(instanceMutex);
            tmp = instance.load();
            if (tmp == nullptr)
            {
                tmp = new T(std::forward<Args>(args)...);
                instance.store(tmp);
                atexit(ResetInstance);
            }
        }
        return tmp;
    }

    static void
    ResetInstance()
    {
        T* tmp = instance.load();
        if (tmp != nullptr)
        {
            delete tmp;
            tmp = nullptr;
            instance.store(tmp);
        }
    }

protected:
    Singleton()
    {
    }
    virtual ~Singleton()
    {
    }

private:
    static std::atomic<T*> instance;
    static std::mutex instanceMutex;
};

template<typename T>
std::atomic<T*> Singleton<T>::instance;

template<typename T>
std::mutex Singleton<T>::instanceMutex;

} // namespace pos

#endif // IBOFOS_SINGLETON_H_
