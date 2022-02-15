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

#include "cli_server.h"

#include <arpa/inet.h>
#include <dirent.h>
#include <errno.h>
#include <net/if.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <openssl/err.h>
#include <openssl/ssl.h>
#include <pthread.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>

#include "src/cli/request_handler.h"
#include "src/logger/logger.h"

using namespace std;
using namespace std::chrono_literals;

namespace pos_cli
{
const char* CERT_PATH = "/etc/pos/cert/";
sock_pool_t sock_pool[MAX_CLI_CNT];
fd_set rset, allset;

pthread_mutex_t mutx, workmutx;
mutex exitMutex;
condition_variable exitCond;
condition_variable cv;
bool exit_flag = false;
std::thread* cliThread = nullptr;
atomic<bool> notifyDone(false);
string threadRes(MAX_BUFF_SZ, 0);

RequestHandler* reqHandler = nullptr;

void
Wait()
{
    std::unique_lock<std::mutex> lock(exitMutex);
    while (exit_flag == false)
    {
        exitCond.wait(lock);
    }

    cliThread->join();
    if (nullptr != reqHandler)
    {
        delete reqHandler;
        reqHandler = nullptr;
    }
    int event = (int)POS_EVENT_ID::SERVER_THREAD_JOINED;
    POS_TRACE_INFO(event, cliEventDic.at(event));
}

void
Exit()
{
    std::unique_lock<std::mutex> lock(exitMutex);
    exit_flag = true;
    int event = (int)POS_EVENT_ID::SERVER_TRY_EXIT;
    POS_TRACE_INFO(event, cliEventDic.at(event));
    exitCond.notify_all();
}

ssize_t
SendMsg(sock_pool_t* client, string msg)
{
    char* buffer = (char*)calloc(msg.length(), sizeof(char));
    strncpy(buffer, msg.c_str(), msg.length());

#ifdef SSL_ON
    ssize_t ret = SSL_write((SSL*)client->ssl, buffer, msg.length());
#else
    ssize_t ret = write(client->sockfd, buffer, msg.length());
#endif

    if (ret < 0)
    {
        int event = (int)POS_EVENT_ID::MSG_SEND_FAILED;
        POS_TRACE_ERROR(event, cliEventDic.at(event), ret);
    }
    int event = (int)POS_EVENT_ID::MSG_SENT;
    POS_TRACE_INFO(event, cliEventDic.at(event), msg);

    free(buffer);
    return ret;
}

sock_pool_t*
AddClient(int sockfd)
{
    int event = (int)POS_EVENT_ID::CLIENT_CONNECTED;
    POS_TRACE_INFO(event, cliEventDic.at(event), sockfd);
    pthread_mutex_lock(&mutx);
    for (int i = 1; i < MAX_CLI_CNT; i++)
    {
        if (sock_pool[i].state == UNUSED)
        {
            sock_pool[i].sockfd = sockfd;
            sock_pool[i].state = USED;
            pthread_mutex_unlock(&mutx);
            return &sock_pool[i];
        }
    }
    pthread_mutex_unlock(&mutx);
    event = (int)POS_EVENT_ID::MAX_CLIENT_ERROR;
    POS_TRACE_WARN(event, cliEventDic.at(event));
    return nullptr;
}

void
RemoveClient(int sockfd)
{
    int event = (int)POS_EVENT_ID::CLIENT_DISCONNECTED;
    POS_TRACE_INFO(event, cliEventDic.at(event), sockfd);
    pthread_mutex_lock(&mutx);
    close(sockfd);
    for (int i = 1; i < MAX_CLI_CNT; i++)
    {
        if (sock_pool[i].sockfd == sockfd)
        {
            sock_pool[i].state = UNUSED;
            memset(sock_pool[i].recv_buff, 0x00, sizeof(sock_pool[i].recv_buff));
            break;
        }
    }
    pthread_mutex_unlock(&mutx);
}

string
TryProcessing(char* msg)
{
    mutex m;
    notifyDone = false;

    thread t([&msg]()
    {
        string res = reqHandler->ProcessCommand(msg);

        if (!notifyDone)
            threadRes = res;

        pthread_mutex_unlock(&workmutx);
        cv.notify_one();
        notifyDone = true;
    });

    t.detach();

    {
        if (!notifyDone)
        {
            unique_lock<mutex> lck(m);
            if (cv.wait_for(lck, TIME_OUT_SEC) == cv_status::timeout)
            {
                notifyDone = true;
                throw runtime_error("TimeOut");
            }
        }
    }

    return threadRes;
}

void*
ClientThread(void* arg)
{
    sock_pool_t* clnt = ((sock_pool_t*)arg);
    int str_len = 0;
    memset(clnt->recv_buff, 0x00, sizeof(clnt->recv_buff));
#ifdef SSL_ON
    if (SSL_accept((SSL*)clnt->ssl) == -1)
        ERR_print_errors_fp(stderr);
    str_len = SSL_read((SSL*)clnt->ssl, clnt->recv_buff, sizeof(clnt->recv_buff));
#else
    str_len = read(clnt->sockfd, clnt->recv_buff, MAX_BUFF_SZ);
#endif
    if (str_len > 0)
    {
        int event = (int)POS_EVENT_ID::MSG_RECEIVED;
        POS_TRACE_INFO(event, cliEventDic.at(event), clnt->recv_buff);

        if (clnt->work)
        {
            bool timedout = false;
            string response;

            try
            {
                response = TryProcessing(clnt->recv_buff);
            }
            catch (runtime_error& e)
            {
                timedout = true;
            }

            if (timedout)
            {
                int event = (int)POS_EVENT_ID::TIMED_OUT;
                POS_TRACE_INFO(event, cliEventDic.at(event));
                SendMsg(clnt, reqHandler->TimedOut(clnt->recv_buff));
            }
            else
            {
                SendMsg(clnt, response);
            }
        }
        else
        {
            clnt->recv_buff[str_len] = 0;
            SendMsg(clnt, reqHandler->PosBusy(clnt->recv_buff));
        }
        if (reqHandler->IsExit() == true)
        {
            Exit();
        }
    }
    else
    {
        int event = (int)POS_EVENT_ID::MSG_RECEIVED;
        POS_TRACE_ERROR(event, cliEventDic.at(event), strerror(errno));

        if (clnt->work)
            pthread_mutex_unlock(&workmutx);
    }
#ifdef SSL_ON
    SSL_free((SSL*)clnt->ssl);
#endif
    RemoveClient(clnt->sockfd);
    return NULL;
}

int
EnableReuseAddr(int sockfd)
{
    int optval = 1;
    int rc = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
    if (rc < 0)
    {
        int event = (int)POS_EVENT_ID::REUSE_ADDR_FAILED;
        POS_TRACE_WARN(event, cliEventDic.at(event));
        return rc;
    }
    int event = (int)POS_EVENT_ID::REUSE_ADDR_ENABLED;
    POS_TRACE_INFO(event, cliEventDic.at(event));
    return 0;
}

int
CreateSocket(int& sock)
{
    struct sockaddr_in servaddr;

    if ((sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
    {
        int event = (int)POS_EVENT_ID::SOCK_CREATE_FAILED;
        POS_TRACE_ERROR(event, cliEventDic.at(event));
        return event;
    }

    EnableReuseAddr(sock);

    memset(&servaddr, 0, sizeof(struct sockaddr_in));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(SERV_PORT);

    if (bind(sock, (struct sockaddr*)&servaddr, sizeof(struct sockaddr_in)) < 0)
    {
        int event = (int)POS_EVENT_ID::SOCK_BIND_FAILED;
        POS_TRACE_ERROR(event, cliEventDic.at(event));
        return event;
    }

    if (listen(sock, SOMAXCONN))
    {
        int event = (int)POS_EVENT_ID::SOCK_LISTEN_FAILED;
        POS_TRACE_ERROR(event, cliEventDic.at(event));
        return event;
    }

    return SUCCESS;
}

// Exclude destructor of abstract class from function coverage report to avoid known issues in gcc/gcov
// mj: it is suspected that gcc copies the codes in the function to the caller.
// LCOV_EXCL_START
void
InitClientPool()
{
    for (int i = 1; i < MAX_CLI_CNT; i++)
    {
        sock_pool[i].sockfd = -1;
        sock_pool[i].state = UNUSED;
    }
}
// LCOV_EXCL_STOP

#ifdef SSL_ON
SSL_CTX*
InitServerCTX(void)
{
    const SSL_METHOD* method;
    SSL_CTX* ctx;

    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();

    method = SSLv23_server_method();
    ctx = SSL_CTX_new(method);

    if (ctx == NULL)
    {
        ERR_print_errors_fp(stderr);
        abort();
    }

    return ctx;
}

void
LoadCertificates(SSL_CTX* ctx, string CertFile, string KeyFile)
{
    if (SSL_CTX_use_certificate_file(ctx, CertFile.c_str(), SSL_FILETYPE_PEM) <= 0)
    {
        ERR_print_errors_fp(stderr);
        abort();
    }
    if (SSL_CTX_use_PrivateKey_file(ctx, KeyFile.c_str(), SSL_FILETYPE_PEM) <= 0)
    {
        ERR_print_errors_fp(stderr);
        abort();
    }

    if (!SSL_CTX_check_private_key(ctx))
    {
        fprintf(stderr, "Private key does not match the public certificate\n");
        abort();
    }
}
#endif

void
CLIServer()
{
    int sock_fd;
#ifdef SSL_ON
    SSL_CTX* ctx;
    SSL_library_init();

    ctx = InitServerCTX();
    LoadCertificates(ctx, CERT_PATH + "cert.crt", CERT_PATH + "cert.key");
#endif

    if (CreateSocket(sock_fd) != SUCCESS)
    {
        close(sock_fd);
        return;
    }

    int efd = epoll_create(MAX_CLI_CNT);
    if (efd < 0)
    {
        int event = (int)POS_EVENT_ID::EPOLL_CREATE_FAILED;
        POS_TRACE_ERROR(event, cliEventDic.at(event));
        return;
    }

    InitClientPool();
    sock_pool[0].sockfd = sock_fd;
    sock_pool[0].state = USED;

    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = 0;
    epoll_ctl(efd, EPOLL_CTL_ADD, sock_fd, &ev);
    struct epoll_event* event = (struct epoll_event*)malloc(sizeof(struct epoll_event) * MAX_CLI_CNT);

    struct sockaddr_in cli_addr;
    socklen_t clilen = sizeof(cli_addr);

    POS_TRACE_INFO((int)POS_EVENT_ID::CLI_SERVER_READY_SUCCESS, "max_cli_count: {}", MAX_CLI_CNT);
    while (1)
    {
        int s_cnt = epoll_wait(efd, event, MAX_CLI_CNT, 1000);
        if (exit_flag == true)
            break;

        if (s_cnt <= 0)
        {
            if (exit_flag == true)
                break;
            else
                continue;
        }

        for (int i = 0; i < s_cnt; i++)
        {
            if (event[i].data.fd == 0)
            {
                int cli_fd = accept(sock_fd, (struct sockaddr*)&cli_addr, &clilen);
                if (cli_fd < 0 || reqHandler->IsExit())
                {
                    int event = (int)POS_EVENT_ID::SOCK_ACCEPT_FAILED;
                    POS_TRACE_WARN(event, cliEventDic.at(event));
                }
                else if (cli_fd >= 0)
                {
                    sock_pool_t* clnt = AddClient(cli_fd);
                    if (clnt == nullptr)
                    {
                        close(cli_fd);
                    }
                    else
                    {
#ifdef SSL_ON
                        SSL* ssl = SSL_new(ctx);
                        SSL_set_fd(ssl, cli_fd);
                        clnt->ssl = ssl;
#endif
                        if (pthread_mutex_trylock(&workmutx) == 0)
                        {
                            clnt->work = true;
                            pthread_t t_id;
                            pthread_create(&t_id, NULL, ClientThread, (void*)clnt);
                            pthread_detach(t_id);
                        }
                        else
                        {
                            clnt->work = false;
                            ClientThread(clnt);
                        }

                        pthread_mutex_unlock(&mutx);
                    }
                }
            }
        }
    }
#ifdef SSL_ON
    SSL_CTX_free(ctx);
#endif
    shutdown(sock_fd, SHUT_RDWR);
    close(sock_fd);
}

void
CLIServerMain()
{
    reqHandler = new RequestHandler;
    cliThread = new std::thread(CLIServer);
}
}; // namespace pos_cli
