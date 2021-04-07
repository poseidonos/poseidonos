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

#pragma once

#include <map>
#include <string>

#include "src/include/ibof_event_id.h"

namespace ibofos_cli
{
static const int FAIL = -1;
static const int SUCCESS = (int)IBOF_EVENT_ID::SUCCESS;
static const int BADREQUEST = 400000;
static const int INTERNAL_ERROR = 500000;

static const std::map<int, std::string> cliEventDic =
    {
        {(int)IBOF_EVENT_ID::SERVER_READY, "CLI server initialized successfully."},
        {(int)IBOF_EVENT_ID::CLIENT_CONNECTED, "new client {} is connected"},
        {(int)IBOF_EVENT_ID::CLIENT_DISCONNECTED, "client {} is disconnected"},
        {(int)IBOF_EVENT_ID::MSG_RECEIVED, "msg received:{}"},
        {(int)IBOF_EVENT_ID::MSG_SENT, "msg sent:{}"},
        {(int)IBOF_EVENT_ID::SERVER_TRY_EXIT, "server try to exit"},
        {(int)IBOF_EVENT_ID::SERVER_THREAD_JOINED, "server thread joined"},
        {(int)IBOF_EVENT_ID::REUSE_ADDR_ENABLED, "socket opt - reuseaddr enabled"},
        {(int)IBOF_EVENT_ID::SOCK_CREATE_FAILED, "socket creation failed"},
        {(int)IBOF_EVENT_ID::SOCK_BIND_FAILED, "socket biding failed"},
        {(int)IBOF_EVENT_ID::SOCK_LISTEN_FAILED, "socket listen failed"},
        {(int)IBOF_EVENT_ID::REUSE_ADDR_FAILED, "failed to enable reuse addr"},
        {(int)IBOF_EVENT_ID::EPOLL_CREATE_FAILED, "failed to create epoll"},
        {(int)IBOF_EVENT_ID::SOCK_ACCEPT_FAILED, "failed to accept socket"},
        {(int)IBOF_EVENT_ID::MAX_CLIENT_ERROR, "max client exceed"},
        {(int)IBOF_EVENT_ID::MSG_SEND_FAILED, "failed to send msg:{}"},
        {(int)IBOF_EVENT_ID::MSG_RECEIVE_EXCEPTION, "exception on msg receiving:{}"},
        {(int)IBOF_EVENT_ID::TIMED_OUT, "take too much time to do a job so send a timed out msg"},
        {(int)IBOF_EVENT_ID::POS_BUSY, "pos is doing other job"},
};
}; // namespace ibofos_cli
