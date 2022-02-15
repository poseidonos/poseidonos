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
 *     * Neither the name of Samsung Electronics Corporation nor the names of
 *       its contributors may be used to endorse or promote products derived
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

#include "src/include/pos_event_id.h"

namespace pos_cli
{
static const int FAIL = -1;
static const int SUCCESS = (int)POS_EVENT_ID::SUCCESS;
static const int BADREQUEST = 400000;
static const int INTERNAL_ERROR = 500000;

class CliEventInfoEntry
{
    public:
        CliEventInfoEntry(const char* eventName, const char* message,
            const char* cause)
        {
            this->eventName = eventName;
            this->message = message;
            this->cause = cause;
        }
        std::string GetEventName() { return eventName; }
        std::string GetMessage() { return message; }
        std::string GetCause() { return cause; }

    private:
        std::string eventName = "";
        std::string message = "";
        std::string cause = ""; // mj: Fill in this field for erroneous events only.
};

static const std::map<int, CliEventInfoEntry*> CliEventInfo =
    {
        // map<eventId, CliEventInfoEntry(eventName, message, cause)>
        {(int)POS_EVENT_ID::CLI_SERVER_INITIALIZED,
            new CliEventInfoEntry("CLI_SERVER_INITIALIZED",
                "The CLI server has been initialized successfully.", "")},
        {(int)POS_EVENT_ID::CLI_CLIENT_ACCEPTED,
            new CliEventInfoEntry("CLI_CLIENT_ACCEPTED",
                "A new client has been accepted (connected).", "")},
        {(int)POS_EVENT_ID::CLI_CLIENT_DISCONNECTED,
            new CliEventInfoEntry("CLI_CLIENT_DISCONNECTED",
                "A client has been disconnected.", "")},
        {(int)POS_EVENT_ID::CLI_MSG_RECEIVED,
            new CliEventInfoEntry("CLI_MSG_RECEIVED",
                "CLI server has recieved a message from a CLI client.", "")},
        {(int)POS_EVENT_ID::CLI_MSG_SENT,
            new CliEventInfoEntry("CLI_MSG_SENT",
                "CLI server has sent a message to a client.", "")},
        {(int)POS_EVENT_ID::CLI_MSG_SENDING_FAILURE,
            new CliEventInfoEntry("CLI_MSG_SENDING_FAILURE,",
                "CLI server has failed to send a message to a client.", "")},
        {(int)POS_EVENT_ID::CLI_SERVER_FINISH,
            new CliEventInfoEntry("CLI_SERVER_FINISH",
                "CLI server has finished successfully.", "")},
        {(int)POS_EVENT_ID::CLI_SERVER_THREAD_JOINED,
            new CliEventInfoEntry("CLI_SERVER_THREAD_JOINED",
                "A CLI server thread has joined. The main thread is blocked until the CLI processing finishes", "")},
        {(int)POS_EVENT_ID::CLI_REUSE_ADDR_ENABLED,
            new CliEventInfoEntry("CLI_REUSE_ADDR_ENABLED",
                "CLI server has enabled to reuse address.", "")},
        {(int)POS_EVENT_ID::CLI_REUSE_ADDR_FAILURE,
            new CliEventInfoEntry("CLI_REUSE_ADDR_FAILURE",
                "CLI server has failed to reuse address.", "")},
        {(int)POS_EVENT_ID::CLI_SOCK_CREATE_FAILURE,
            new CliEventInfoEntry("CLI_SOCK_CREATE_FAILURE",
                "CLI server has failed to create a socket.", "")},
        {(int)POS_EVENT_ID::CLI_SOCK_BIND_FAILURE,
            new CliEventInfoEntry("CLI_SOCK_BIND_FAILURE",
                "CLI server has failed to bind a socket.", "")},
        {(int)POS_EVENT_ID::CLI_SOCK_LISTEN_FAILURE,
            new CliEventInfoEntry("CLI_SOCK_LISTEN_FAILURE",
                "CLI server has failed to listen to a socket.", "")},
        {(int)POS_EVENT_ID::CLI_EPOLL_CREATE_FAILURE,
            new CliEventInfoEntry("CLI_EPOLL_CREATE_FAILURE",
                "CLI server has failed to create a epoll.", "")},
        {(int)POS_EVENT_ID::CLI_SOCK_ACCEPT_FAILURE,
            new CliEventInfoEntry("CLI_SOCK_ACCEPT_FAILURE",
                "CLI server has failed to accept the client socket.", "")},
        {(int)POS_EVENT_ID::CLI_ADD_CLIENT_FAILURE_MAX_CLIENT,
            new CliEventInfoEntry("CLI_ADD_CLIENT_FAILURE_MAX_CLIENT",
                "CLI server has failed to add a client.",
                    "The number of clients exceeds the maximum.")},
        {(int)POS_EVENT_ID::CLI_MSG_RECEIVE_FAILURE,
            new CliEventInfoEntry("CLI_MSG_RECEIVE_FAILURE",
                "CLI server has failed to receive a message from a client.", "")},
        {(int)POS_EVENT_ID::CLI_SERVER_TIMED_OUT,
            new CliEventInfoEntry("CLI_SERVER_TIMED_OUT",
                "CLI server has timed out.", "Command processing takes too long.")},
        {(int)POS_EVENT_ID::CLI_SERVER_BUSY,
            new CliEventInfoEntry("CLI_SERVER_BUSY",
                "CLI server could not receive the command.",
                "POS is processing a previously receieved command.")},
    };
}; // namespace pos_cli
