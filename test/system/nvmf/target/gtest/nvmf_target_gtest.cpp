#include <iostream>
#include <unistd.h>
#include <functional>
#include <signal.h>
#include <stdio.h>

#include "spdk/stdinc.h"
#include "spdk/env.h"
#include "spdk/event.h"
#include "spdk/nvme.h"

#include "nvmf_target.hpp"
#include "../cpp/nvmf_volume_mock.hpp"
#include "src/device/spdk/spdk.hpp"
#include "src/include/pos_event_id.h"
#include "src/network/nvmf_target_spdk.hpp"

#include "gtest/gtest.h"

using namespace pos;
using namespace std;

NvmfTarget* target;
typedef void (*IBoFNvmfEventDoneCallback_t)(void *cb_arg, int status);

extern struct spdk_nvmf_tgt *g_spdk_nvmf_tgt;
static struct NvmfTargetCallbacks nvmfCallbacks;

const char* nqn1 = "nqn.2019-04.pos:subsystem1";
const char* nqn2 = "nqn.2019-04.pos:subsystem2";
const char* nqn3 = "nqn.2019-04.pos:subsystem3";
const char* ibof_bdev1 = "Volume0";
const char* ibof_bdev2 = "Volume1";
const char* ibof_bdev3 = "Volume2";
const char* ibof_bdev4 = "Volume3";
const char* ip = "10.100.11.20";
const char* port1 = "1158";
const char* port2 = "1158";
const char* port3 = "1159";
const char* tcp_transport = "TCP";
const char* rdma_transport = "TCP";
static bool spdk_initialized = false;

int startGtest();
void testCreateRDMATransport(void *cb_arg, int status);
void testCreateTCPTransport(void *cb_arg, int status);
void testCreateSubsystem1(void* cb_arg, int status);
void testCreateSubsystem2(void* cb_arg, int status);
void testCreateSubsystem3(void* cb_arg, int status);

void testGetSubSystemInfo1(void* cb_arg, int status);
void testGetSubSystemInfo2(void* cb_arg, int status);
void testGetSubSystemInfo3(void* cb_arg, int status);

void testAttachListener1(void* cb_arg, int status);
void testAttachListener2(void* cb_arg, int status);
void testAttachListener3(void* cb_arg, int status);

void testCreateIBoFBdev1(void* cb_arg, int status);
void testCreateIBoFBdev2(void* cb_arg, int status);
void testCreateIBoFBdev3(void* cb_arg, int status);
void testCreateIBoFBdev4(void* cb_arg, int status);

void testAttachNamespace1(void* cb_arg, int nsid);
void testAttachNamespace2(void* cb_arg, int nsid);
void testAttachNamespace3(void* cb_arg, int nsid);
void testAttachNamespace4(void* cb_arg, int nsid);
void testAttachNamespaceDone(void* cb_arg, int nsid);

void nvmfStartCleanup(void* cb_arg, int status);
void cleanupGtest();

void testDetachNamespace1(void* cb_arg, int status);
void testDetachNamespace2(void* cb_arg, int status);
void testDetachNamespace3(void* cb_arg, int status);
void testDetachNamespace4(void* cb_arg, int status);

void testDeleteIBofBdev1(void* cb_arg, int status);
void testDeleteIBofBdev2(void* cb_arg, int status);
void testDeleteIBofBdev3(void* cb_arg, int status);
void testDeleteIBofBdev4(void* cb_arg, int status);

void testDetachListener1(void* cb_arg, int status);
void testDetachListener2(void* cb_arg, int status);
void testDetachListener3(void* cb_arg, int status);

void testDeleteSubsystem1(void* cb_arg, int status);
void testDeleteSubsystem2(void* cb_arg, int status);
void testDeleteSubsystem3(void* cb_arg, int status);
void testDeleteSubsystemDone(void* cb_arg, int status);

bool CreateTransport(const char* type, struct spdk_nvmf_transport_opts *caller_option,
        IBoFNvmfEventDoneCallback_t callback, void* arg);
bool CreateNvmfSubsystem(const char* nqn, const char* serial_number, 
        uint32_t max_namespaces, bool allow_any_host, 
        IBoFNvmfEventDoneCallback_t cb, void* cb_arg);
bool DeleteNvmfSubsystem(const char* nqn, IBoFNvmfEventDoneCallback_t cb, void* cb_arg);
bool AttachListener(const char* nqn, const char* ip, const char* port, const char* trtype, IBoFNvmfEventDoneCallback_t cb, void* cb_arg);
bool DetachListener(const char* nqn, const char* ip, const char* port, const char* trtype, IBoFNvmfEventDoneCallback_t cb, void* cb_arg);
void DumpSubsystemInfo(void);
uint32_t GetSubSystemInfo(const char* nqn, struct nvmf_subsystem* subsystem);
uint32_t GetSubsystemInfo(struct nvmf_subsystem* arr_subsystem, uint32_t nr_subsystem);

/*
 * startGtest	: is called when Spdk initialization is done
 * 		: perfome create RDMA transport -> create Nvmf Subsystem -> create ibof_bdev -> attach namespace -> attach  listener
 */
int startGtest(){
	spdk_initialized = true;
	testCreateTCPTransport(NULL, 0);
	return 0;
}

void testCreateTCPTransport(void* cb_arg, int status){
	//bool ret = target->CreateTransport(tcp_transport, NULL, testCreateRDMATransport, target);
	bool ret = CreateTransport(tcp_transport, NULL, testCreateSubsystem1, target);
	EXPECT_EQ(true, ret);
}

void testCreateRDMATransport(void* cb_arg, int status){
	EXPECT_EQ(0, status);
	bool ret = CreateTransport(rdma_transport, NULL, testCreateSubsystem1, target);
	EXPECT_EQ(true, ret);
}

void testCreateSubsystem1(void* cb_arg, int status){
	EXPECT_EQ(0, status);
	bool ret = CreateNvmfSubsystem(nqn1, "POS00000000000001", 0, true, testCreateSubsystem2, target);
	EXPECT_EQ(true, ret);
}

void testCreateSubsystem2(void* cb_arg, int status){
	EXPECT_EQ(0, status);
	bool ret = CreateNvmfSubsystem(nqn2, "POS00000000000002", 0, true, testCreateSubsystem3, target);
	EXPECT_EQ(true, ret);
}

void testCreateSubsystem3(void* cb_arg, int status){
	EXPECT_EQ(0, status);
	bool ret = CreateNvmfSubsystem(nqn3, "POS00000000000003", 0, true, testAttachListener1, target);
	EXPECT_EQ(true, ret);
}

void testAttachListener1(void* cb_arg, int status){
	EXPECT_EQ(0, status);
	bool ret = AttachListener(nqn1, ip, port1, rdma_transport, testAttachListener2, target);
	EXPECT_EQ(true, ret);
}

void testAttachListener2(void* cb_arg, int status){
	EXPECT_EQ(0, status);
	bool ret = AttachListener(nqn2, ip, port2, rdma_transport, testAttachListener3, target);
	EXPECT_EQ(true, ret);
}

void testAttachListener3(void* cb_arg, int status){
	EXPECT_EQ(0, status);
	bool ret = AttachListener(nqn3, ip, port3, tcp_transport, testCreateIBoFBdev1, target);
	EXPECT_EQ(true, ret);
}

void testCreateIBoFBdev1(void* cb_arg, int status){
	bool ret = target->CreateIBoFBdev(ibof_bdev1, 0, 128, 512, true);
	EXPECT_EQ(true, ret);
	testCreateIBoFBdev2(NULL, 0);
}

void testCreateIBoFBdev2(void* cb_arg, int status){
	EXPECT_EQ(0, status);
	bool ret = target->CreateIBoFBdev(ibof_bdev2, 1, 128, 512, true);
	EXPECT_EQ(true, ret);
	testCreateIBoFBdev3(NULL, 0);
}

void testCreateIBoFBdev3(void* cb_arg, int status){
	EXPECT_EQ(0, status);
	bool ret = target->CreateIBoFBdev(ibof_bdev3, 2, 128, 512, true);
	EXPECT_EQ(true, ret);
	testCreateIBoFBdev4(NULL, 0);
}

void testCreateIBoFBdev4(void* cb_arg, int status){
	EXPECT_EQ(0, status);
	bool ret = target->CreateIBoFBdev(ibof_bdev4, 3, 128, 512, true);
	EXPECT_EQ(true, ret);
	testAttachNamespace1(NULL, 0);
}

void testAttachNamespace1(void* cb_arg, int status){
	EXPECT_EQ(0, status);
	bool ret = target->AttachNamespace(nqn1, ibof_bdev1, testAttachNamespace2, target);
	EXPECT_NE(false, ret);
}

void testAttachNamespace2(void* cb_arg, int nsid){
	EXPECT_EQ(1, nsid);
       bool ret = target->AttachNamespace(nqn1, ibof_bdev1, testAttachNamespace3, target);
	EXPECT_NE(false, ret);
}

void testAttachNamespace3(void* cb_arg, int nsid){
	EXPECT_EQ(2, nsid);
	bool ret = target->AttachNamespace(nqn1, ibof_bdev1, 10, testAttachNamespace4, target);
	EXPECT_NE(false, ret);
}

void testAttachNamespace4(void* cb_arg, int nsid){
	EXPECT_EQ(10, nsid);
	bool ret = target->AttachNamespace(nqn1, ibof_bdev1, testAttachNamespaceDone, target);
	EXPECT_NE(false, ret);
}

void testAttachNamespaceDone(void* cb_arg, int nsid){
	//NOTE: below 1 implies nsid as result of testAttachNamespace4
	EXPECT_EQ(20, nsid);
	testGetSubSystemInfo1(NULL, 0);
}
void testGetSubSystemInfo1(void* cb_arg, int status){
	EXPECT_EQ(0, status);

	int i;
	struct nvmf_subsystem arr_subsystem[4];
	uint32_t try_to_read = 4;
	uint32_t ret = GetSubsystemInfo(arr_subsystem, try_to_read);
	EXPECT_EQ(ret, 3);

	struct nvmf_subsystem subsystem;
	ret = GetSubSystemInfo(nqn1, &subsystem);
	cout<<"nqn1: "<<subsystem.nqn<<endl;
	EXPECT_EQ(strcmp(nqn1, subsystem.nqn), 0);

	cout<<"nr_namespace: "<<subsystem.nr_namespace<<endl;
	for(i=0;i<subsystem.nr_namespace;i++){
		cout<<"\tname: "<<subsystem.ns[i].name<<" nsid: "<<subsystem.ns[i].nsid<<endl;
	}
	cout<<"nr_transport: "<<subsystem.nr_transport<<endl;
	for(i=0;i<subsystem.nr_transport;i++){
		cout<<"\ttype: "<<subsystem.tr[i].type<<" adrfam: "<<subsystem.tr[i].adrfam;
		cout<<" traddr: "<<subsystem.tr[i].traddr<<" trscvid: "<<subsystem.tr[i].trsvcid<<endl;
	}

	testGetSubSystemInfo2(NULL, 0);
}

void testGetSubSystemInfo2(void* cb_arg, int status){
	EXPECT_EQ(0, status);

	int i;
	struct nvmf_subsystem arr_subsystem[4];
	uint32_t try_to_read = 4;
	uint32_t ret = GetSubsystemInfo(arr_subsystem, try_to_read);
	EXPECT_EQ(ret, 3);

	struct nvmf_subsystem subsystem;
	ret = GetSubSystemInfo(nqn2, &subsystem);
	cout<<"nqn2: "<<subsystem.nqn<<endl;
	EXPECT_EQ(strcmp(nqn2, subsystem.nqn), 0);

	cout<<"nr_namespace: "<<subsystem.nr_namespace<<endl;
	for(i=0;i<subsystem.nr_namespace;i++){
		cout<<"\tname: "<<subsystem.ns[i].name<<" nsid: "<<subsystem.ns[i].nsid<<endl;
	}

	cout<<"nr_transport: "<<subsystem.nr_transport<<endl;
	for(i=0;i<subsystem.nr_transport;i++){
		cout<<"\ttype: "<<subsystem.tr[i].type<<" adrfam: "<<subsystem.tr[i].adrfam;
		cout<<" traddr: "<<subsystem.tr[i].traddr<<" trscvid: "<<subsystem.tr[i].trsvcid<<endl;
	}

	testGetSubSystemInfo3(NULL, 0);
}

void testGetSubSystemInfo3(void* cb_arg, int status){
	EXPECT_EQ(0, status);

	int i;
	struct nvmf_subsystem arr_subsystem[4];
	uint32_t try_to_read = 4;
	uint32_t ret = GetSubsystemInfo(arr_subsystem, try_to_read);
	EXPECT_EQ(ret, 3);

	struct nvmf_subsystem subsystem;
	ret = GetSubSystemInfo(nqn3, &subsystem);
	cout<<"nqn3: "<<subsystem.nqn<<endl;
	EXPECT_EQ(strcmp(nqn3, subsystem.nqn), 0);

	cout<<"nr_namespace: "<<subsystem.nr_namespace<<endl;
	for(i=0;i<subsystem.nr_namespace;i++){
		cout<<"\tname: "<<subsystem.ns[i].name<<" nsid: "<<subsystem.ns[i].nsid<<endl;
	}

	cout<<"nr_transport: "<<subsystem.nr_transport<<endl;
	for(i=0;i<subsystem.nr_transport;i++){
		cout<<"\ttype: "<<subsystem.tr[i].type<<" adrfam: "<<subsystem.tr[i].adrfam;
		cout<<" traddr: "<<subsystem.tr[i].traddr<<" trscvid: "<<subsystem.tr[i].trsvcid<<endl;
	}

	nvmfStartCleanup(NULL, 0);
}

void nvmfStartCleanup(void* cb_arg, int status){
	EXPECT_EQ(0, status);
	//cleanupGtest();
}

/*
 * cleanupGtest: performe detach listener -> detach namespace -> remove ibof_bdev -> destroy Nvmf Subsystem
 */

void cleanupGtest(){
	testDetachListener1(NULL, 0);
}

void testDetachListener1(void* cb_arg, int status){
	EXPECT_EQ(0, status);
	bool ret = DetachListener(nqn1, ip, port1, tcp_transport, testDetachListener2, (void*)target);
	EXPECT_EQ(true, ret);
}

void testDetachListener2(void* cb_arg, int status){
	EXPECT_EQ(0, status);
	bool ret = DetachListener(nqn2, ip, port2, rdma_transport, testDetachListener3, (void*)target);
	EXPECT_EQ(true, ret);
}

void testDetachListener3(void* cb_arg, int status){
	EXPECT_EQ(0, status);
	bool ret = DetachListener(nqn3, ip, port3, rdma_transport, testDetachNamespace1, (void*)target);
	EXPECT_EQ(true, ret);
}

void testDetachNamespace1(void* cb_arg, int status){
	EXPECT_EQ(0, status);
	bool ret = target->DetachNamespace(nqn1, 2, testDetachNamespace2, target);
	EXPECT_EQ(true, ret);
}

void testDetachNamespace2(void* cb_arg, int status){
	EXPECT_EQ(0, status);
	bool ret = target->DetachNamespace(nqn1, 1, testDetachNamespace3, target);
	EXPECT_EQ(true, ret);
}

void testDetachNamespace3(void* cb_arg, int status){
	EXPECT_EQ(0, status);
	bool ret = target->DetachNamespace(nqn2, 1, testDetachNamespace4, target);
	EXPECT_EQ(true, ret);
}

void testDetachNamespace4(void* cb_arg, int status){
	EXPECT_EQ(0, status);
	bool ret = target->DetachNamespace(nqn3, 1, testDeleteIBofBdev1, target);
	EXPECT_EQ(true, ret);
	testDeleteIBofBdev1(NULL, 0);
}

void testDeleteIBofBdev1(void* cb_arg, int status){
	EXPECT_EQ(0, status);
	bool ret = target->DeleteIBoFBdev(ibof_bdev1);
	EXPECT_EQ(true, ret);
	testDeleteIBofBdev2(NULL, 0);
}

void testDeleteIBofBdev2(void* cb_arg, int status){
	EXPECT_EQ(0, status);
	bool ret = target->DeleteIBoFBdev(ibof_bdev2);
	EXPECT_EQ(true, ret);
	testDeleteIBofBdev3(NULL, 0);
}

void testDeleteIBofBdev3(void* cb_arg, int status){
	EXPECT_EQ(0, status);
	bool ret = target->DeleteIBoFBdev(ibof_bdev3);
	EXPECT_EQ(true, ret);
	testDeleteIBofBdev4(NULL, 0);
}

void testDeleteIBofBdev4(void* cb_arg, int status){
	EXPECT_EQ(0, status);
	bool ret = target->DeleteIBoFBdev(ibof_bdev4);
	EXPECT_EQ(true, ret);
	testDeleteSubsystem1(NULL, 0);
}

void testDeleteSubsystem1(void* cb_arg, int status){
	EXPECT_EQ(0, status);
	bool ret = DeleteNvmfSubsystem(nqn1, testDeleteSubsystem2, target);
	EXPECT_EQ(true, ret);
}

void testDeleteSubsystem2(void* cb_arg, int status){
	EXPECT_EQ(0, status);
	bool ret = DeleteNvmfSubsystem(nqn2, testDeleteSubsystem3, target);
	EXPECT_EQ(true, ret);
}

void testDeleteSubsystem3(void* cb_arg, int status){
	EXPECT_EQ(0, status);
	bool ret = DeleteNvmfSubsystem(nqn3, testDeleteSubsystemDone, target);
	EXPECT_EQ(true, ret);
}

void testDeleteSubsystemDone(void* cb_arg, int status){
	cout<<"All test Done"<<endl;
}


bool CreateTransport(const char* type, struct spdk_nvmf_transport_opts *caller_option,
        IBoFNvmfEventDoneCallback_t callback, void* arg)
{
    struct spdk_nvmf_transport_opts opts = { 0 };
    enum spdk_nvme_transport_type trtype;
    struct spdk_nvmf_transport* transport = NULL;

    InitNvmfCallbacks(&nvmfCallbacks);

    if(spdk_nvme_transport_id_parse_trtype(&trtype, type))
    {
        SPDK_ERRLOG("Fail to create transport : Invalid transport type: %s \n", type);
        return false;
    }

    if(spdk_nvmf_tgt_get_transport(g_spdk_nvmf_tgt, trtype))
    {
        SPDK_ERRLOG("Fail to create transport : Duplicated transport type: %s \n", type);
        return false;
    }

    if(!spdk_nvmf_transport_opts_init(trtype, &opts))
    {
        SPDK_ERRLOG("Fail to create transport : Fail to init transport option: %s \n", type);
        return false;
    }

    if(caller_option)
    {
        opts.max_queue_depth = caller_option->max_queue_depth;
        opts.max_qpairs_per_ctrlr = caller_option->max_qpairs_per_ctrlr;
        opts.in_capsule_data_size = caller_option->in_capsule_data_size;
        opts.max_io_size = caller_option->max_io_size;
        opts.io_unit_size = caller_option->io_unit_size;
        opts.max_aq_depth = caller_option->max_aq_depth;
    }

    if(trtype == SPDK_NVME_TRANSPORT_RDMA)
    {
        opts.io_unit_size = 8192;
        opts.max_qpairs_per_ctrlr = 4;
        opts.in_capsule_data_size = 0;
    }
    else if(trtype == SPDK_NVME_TRANSPORT_TCP)
    {
        opts.io_unit_size = 16384;
        opts.max_qpairs_per_ctrlr = 8;
        opts.in_capsule_data_size = 8192;
    }
    else
    {
        SPDK_ERRLOG("Fail to create transport : Invalid transport type: %s \n", type);
        return false;
    }

    /* NOTE: configurable variables
     * ('-t', '--trtype', help='Transport type (ex. RDMA)', type=str, required=True)
     ('-q', '--max-queue-depth', help='Max number of outstanding I/O per queue', type=int)
     ('-p', '--max-qpairs-per-ctrlr', help='Max number of SQ and CQ per controller', type=int)
     ('-c', '--in-capsule-data-size', help='Max number of in-capsule data size', type=int)
     ('-i', '--max-io-size', help='Max I/O size (bytes)', type=int)
     ('-u', '--io-unit-size', help='I/O unit size (bytes)', type=int)
     ('-a', '--max-aq-depth', help='Max number of admin cmds per AQ', type=int)
     */
    transport = spdk_nvmf_transport_create(trtype, &opts);
    if(transport)
    {
        struct event_context* ctx = AllocEventContext(callback, arg);
        if(!ctx)
        {
            SPDK_ERRLOG("fail to allocEventContext\n");
            return false;
        }
        spdk_nvmf_tgt_add_transport(g_spdk_nvmf_tgt, transport,
                nvmfCallbacks.createTransportDone, (void*)ctx);
        return true;
    }

    SPDK_ERRLOG("Fail to create transport %s\n", type);
    return false;
}

bool CreateNvmfSubsystem(const char* nqn, const char* serial_number,
        uint32_t max_namespaces, bool allow_any_host, IBoFNvmfEventDoneCallback_t callback, void* arg)
{
    int ret = 0;
    struct spdk_nvmf_subsystem* subsystem = NULL;
    if(!target->IsTargetExist("create nvmf subsystem"))
    {
        return false;
    }

    subsystem = spdk_nvmf_subsystem_create(g_spdk_nvmf_tgt, nqn, SPDK_NVMF_SUBTYPE_NVME, max_namespaces);
    if(!subsystem)
    {
        SPDK_ERRLOG("fail to create nvmf subsystem\n");
        return false;
    }

    if(serial_number && spdk_nvmf_subsystem_set_sn(subsystem, serial_number))
    {
        SPDK_ERRLOG("fail to specifiy subsystem serial number: %s\n", serial_number);
        return false;
    }

    spdk_nvmf_subsystem_set_allow_any_host(subsystem, allow_any_host);

    struct event_context* ctx = AllocEventContext(callback, arg);
    if(!ctx)
    {
        SPDK_ERRLOG("fail to allocEventContext\n");
        return false;
    }
    ctx->event_arg1 = strdup(nqn);
    ret = spdk_nvmf_subsystem_start(subsystem, nvmfCallbacks.subsystemStartDone, ctx);
    if(ret != 0)
    {
        SPDK_ERRLOG("fail to change subsystem status -> START NQN=%s ret=%d\n", nqn, ret);
        return false;
    }

    return true;
}

bool DeleteNvmfSubsystem(const char* nqn, 
        IBoFNvmfEventDoneCallback_t callback, void* arg){
    int ret = 0;
    struct spdk_nvmf_subsystem* subsystem = NULL;
    if(!target->IsTargetExist("delete nvmf subsystem"))
    {
        SPDK_ERRLOG("fail to delete nvmf subsystem: target is not exist\n");
        return false;
    }

    subsystem = spdk_nvmf_tgt_find_subsystem(g_spdk_nvmf_tgt, nqn);
    if(!subsystem)
    {
        SPDK_ERRLOG("fail to find subsystem(NQN=%s): it does not exist\n", nqn);
        return false;
    }

    struct event_context* ctx = AllocEventContext(callback, arg);
    if(!ctx)
    {
        SPDK_ERRLOG("fail to allocEventContext\n");
        return false;
    }
    ctx->event_arg1 = strdup(nqn);
    ret = spdk_nvmf_subsystem_stop(subsystem, nvmfCallbacks.subsystemStopDone, ctx);
    if(ret != 0)
    {
        SPDK_ERRLOG("fail to change subsystem status -> STOP NQN=%s ret=%d\n", nqn, ret);
        return false;
    }
    return true;
}

bool AttachListener(const char* nqn, const char* ip, const char* port,
        const char* trtype, IBoFNvmfEventDoneCallback_t callback, void* arg)
{
    struct spdk_nvme_transport_id* trid = NULL;
    struct spdk_nvmf_subsystem *subsystem = NULL;
    int ret = 0;
    if(!target->IsTargetExist("attach listener")){
        return false;
    }

    subsystem = spdk_nvmf_tgt_find_subsystem(g_spdk_nvmf_tgt, nqn);
    if(!subsystem){
        SPDK_ERRLOG("fail to find subsystem(NQN=%s): it does not exist\n", nqn);
        return false;
    }

    trid = (struct spdk_nvme_transport_id*)malloc(sizeof(struct spdk_nvme_transport_id));
    memset(trid,0,sizeof(struct spdk_nvme_transport_id));
    if(spdk_nvme_transport_id_parse_trtype(&trid->trtype, trtype))
    {
        SPDK_ERRLOG("invalid listen address transport type :%s\n", trtype);
        free(trid);
        return false;
    }
    //TODO: AF_INET6 and other adrfam
    trid->adrfam = SPDK_NVMF_ADRFAM_IPV4;

    snprintf(trid->traddr, sizeof(trid->traddr), "%s", ip);
    snprintf(trid->trsvcid, sizeof(trid->trsvcid), "%s", port);

    struct event_context* ctx = AllocEventContext(callback, arg);
    if(!ctx)
    {
        free(trid);
        return false;
    }
    ctx->event_arg1 = trid;
    ret = spdk_nvmf_subsystem_pause(subsystem, nvmfCallbacks.attachListenerPauseDone, (void*)ctx);
    if(ret != 0)
    {
        SPDK_ERRLOG("fail to pause subsystem(NQN=%s)\n", nqn);
        free(trid);
        return false;
    }

    return true;
}

bool DetachListener(const char* nqn, const char* ip, const char* port,
        const char* trtype, IBoFNvmfEventDoneCallback_t callback, void* arg)
{
    struct spdk_nvme_transport_id* trid = NULL;
    struct spdk_nvmf_subsystem *subsystem = NULL;
    int ret;
    if(!target->IsTargetExist("detach listener"))
    {
        return false;
    }
    
    subsystem = spdk_nvmf_tgt_find_subsystem(g_spdk_nvmf_tgt, nqn);
    if(!subsystem)
    {
        SPDK_ERRLOG("fail to find subsystem(NQN=%s): it does not exist\n", nqn);
        return false;
    }

    trid = (struct spdk_nvme_transport_id*)malloc(sizeof(struct spdk_nvme_transport_id));
    memset(trid, 0, sizeof(struct spdk_nvme_transport_id));
    if(spdk_nvme_transport_id_parse_trtype(&trid->trtype, trtype))
    {
        SPDK_ERRLOG("invalid listen address transport type :%s\n", trtype);
        free(trid);
        return false;
    }
    //TODO: AF_INET6 and other adrfam
    trid->adrfam = SPDK_NVMF_ADRFAM_IPV4;

    snprintf(trid->traddr, sizeof(trid->traddr), "%s", ip);
    snprintf(trid->trsvcid, sizeof(trid->trsvcid), "%s", port);

    struct event_context* ctx = AllocEventContext(callback, arg);
    if(!ctx)
    {
        free(trid);
        return false;
    }
    ctx->event_arg1 = trid;
    ret = spdk_nvmf_subsystem_pause(subsystem, nvmfCallbacks.detachListenerPauseDone, (void*)ctx);
    if(ret != 0)
    {
        SPDK_ERRLOG("fail to pause subsystem(NQN=%s)\n", nqn);
        free(trid);
        return false;
    }

    return true;
}

uint32_t GetSubsystemInfo(struct nvmf_subsystem* arr_subsystem, uint32_t nr_subsystem)
{
    uint32_t nr_actual_subsystem = 0;
    struct spdk_nvmf_subsystem* subsystem = NULL;

    if(!arr_subsystem || nr_subsystem == 0)
    {
        return 0;
    }

    if(!g_spdk_nvmf_tgt)
    {
        SPDK_ERRLOG("fail to retrieve nvmf subsystem: target is not exist\n");
        return 0;
    }

    subsystem = spdk_nvmf_subsystem_get_first(g_spdk_nvmf_tgt);
    subsystem = spdk_nvmf_subsystem_get_next(subsystem);
    while(subsystem)
    {
        const char* nqn = spdk_nvmf_subsystem_get_nqn(subsystem);
        if(GetSubSystemInfo(nqn, &arr_subsystem[nr_actual_subsystem]) != 0)
        {
            break;
        }
        if(++nr_actual_subsystem >= nr_subsystem)
        {
            break;
        }
        subsystem = spdk_nvmf_subsystem_get_next(subsystem);
    }
    return nr_actual_subsystem;
}

uint32_t GetSubSystemInfo(const char* nqn, struct nvmf_subsystem* subsystem)
{
    uint32_t nr_namespace = 0;
    uint32_t nr_transport = 0;
    struct spdk_nvmf_subsystem* system = NULL;
    struct spdk_nvmf_ns* ns = NULL;
    struct spdk_nvmf_listener* listener = NULL;

    if(!nqn || !subsystem)
    {
        return 1;
    }

    system = spdk_nvmf_tgt_find_subsystem(g_spdk_nvmf_tgt, nqn);
    if(!system)
    {
        SPDK_ERRLOG("fail to find subsystem(NQN=%s) is not exist\n", nqn);
        return 1;
    }

    /* read NQN */
    const char* subsystem_nqn = spdk_nvmf_subsystem_get_nqn(system);
    strncpy(subsystem->nqn, subsystem_nqn, sizeof(subsystem->nqn));
    subsystem->nqn[sizeof(subsystem->nqn) - 1] = '\0';

    /* read Namespace */
    ns = spdk_nvmf_subsystem_get_first_ns(system);
    while(ns)
    {
        subsystem->ns[nr_namespace].nsid = spdk_nvmf_ns_get_id(ns);
        strncpy(subsystem->ns[nr_namespace].name, spdk_bdev_get_name(spdk_nvmf_ns_get_bdev(ns)), sizeof(subsystem->ns[nr_namespace].name));
        nr_namespace++;
        ns = spdk_nvmf_subsystem_get_next_ns(system, ns);
    }
    subsystem->nr_namespace = nr_namespace;


    /* read Transport */
    listener = spdk_nvmf_subsystem_get_first_listener(system);
    while(listener)
    {
        const struct spdk_nvme_transport_id* trid = spdk_nvmf_listener_get_trid(listener);
        struct nvmf_transport *tr = &subsystem->tr[nr_transport];
        strncpy(tr->type, spdk_nvme_transport_id_trtype_str(trid->trtype), sizeof(tr->type));
        strncpy(tr->adrfam, spdk_nvme_transport_id_adrfam_str(trid->adrfam), sizeof(tr->adrfam));
        strncpy(tr->traddr, trid->traddr, sizeof(tr->traddr));
        tr->traddr[sizeof(tr->traddr) - 1] = '\0';
        strncpy(tr->trsvcid, trid->trsvcid, sizeof(tr->trsvcid));
        tr->trsvcid[sizeof(tr->trsvcid) - 1] = '\0';
        nr_transport++;
        listener = spdk_nvmf_subsystem_get_next_listener(system, listener);
    }
    subsystem->nr_transport = nr_transport;
    return 0;
}

void DumpSubsystemInfo(void)
 {
     uint32_t i,j;
     struct nvmf_subsystem arr_subsystem[10];
     uint32_t try_to_read = 10;
     uint32_t actual_read = GetSubsystemInfo(arr_subsystem, try_to_read);
     cout<<"************************************************************"<<endl;
     cout<<"nr running subsystem: "<<actual_read<<endl;

     for(i=0;i<actual_read;i++)
     {
         cout<<"nqn: "<<arr_subsystem[i].nqn<<endl;

         cout<<"nr_namespace: "<<arr_subsystem[i].nr_namespace<<endl;
         for(j=0;j<arr_subsystem[i].nr_namespace;j++)
         {
             cout<<"\tname: "<<arr_subsystem[i].ns[j].name<<" nsid: "<<arr_subsystem[i].ns[j].nsid<<endl;
         }

         cout<<"nr_transport: "<<arr_subsystem[i].nr_transport<<endl;
         for(j=0;j<arr_subsystem[i].nr_transport;j++)
         {
             cout<<"\ttype: "<<arr_subsystem[i].tr[j].type<<" adrfam: "<<arr_subsystem[i].tr[j].adrfam;
             cout<<" traddr: "<<arr_subsystem[i].tr[j].traddr<<" trscvid: "<<arr_subsystem[i].tr[j].trsvcid<<endl<<endl;
         }
     }
     cout<<"************************************************************"<<endl;
 }

/*
 * iBoFIOHandler : is called at the end of uNVMf processing
 */
static int UNVMfSubmitHandler(struct ibof_io* io) {
	cout<<"EnQueue : DIRECTION - " <<io->direction<< endl;
	//NOTE:  iBoFIO Handler need to call complete_cb, which call bdev_ibof_io_complete() -> spdk_bdev_io_comple te() -> nvmf -> rdma,,, in sequencial order.
	if(io->complete_cb){
		io->complete_cb(io, SPDK_BDEV_IO_STATUS_SUCCESS);
	}
	return 0;
}

static void exit_handler(int signo){
	spdk_initialized = false;
}

/*
 * Note : the test app shows how to make use of Nvmf Target as a separate application.
 * 	Application can register iBoFIOHandler that will be processed during nvmf - bdev - ibof volume
 *
*/
TEST(NVMF_TARGET_API, API) {
	EXPECT_EQ(0, 0);
}

GTEST_API_ int main(int argc, char **argv) {
	printf("Running main() from %s\n", __FILE__);
	testing::InitGoogleTest(&argc, argv);

       Spdk* ins = SpdkSingleton::Instance();
	ins->SetStartCompletedHandler(startGtest);
	ins->Init(argc,argv);

	if (signal(SIGINT, exit_handler) == SIG_ERR) {
		cout<<"An error occurred while setting a signal handler"<<endl;
		return EXIT_FAILURE;
	}

	NvmfVolumeMock* volume = new NvmfVolumeMock();
        unvmf_io_handler handler = { .submit = UNVMfSubmitHandler};
	volume->SetuNVMfIOHandler(handler);

	target = new NvmfTarget(volume);

	while(spdk_initialized){
		sleep(1);
	}

	return RUN_ALL_TESTS();
}

//}

