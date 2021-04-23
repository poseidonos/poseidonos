#include <iostream>
#include <unistd.h>
#include <functional>
#include <signal.h>

#include "spdk/stdinc.h"
#include "spdk/env.h"
#include "spdk/event.h"
#include "spdk/nvme.h"

#include "nvmf_target.hpp"
#include "nvmf_volume_mock.hpp"
#include "src/device/spdk/spdk.hpp"

using namespace pos;
using namespace std;

NvmfTarget* target;

static const char* nqn = "nqn.2019-04.pos:subsystem1";
static const char* ibof_bdev0 = "Volume0";
static const char* ibof_bdev1 = "Volume1";
static const char* ip = "10.100.11.20";
static const char* port = "1158";
static const char* trtype = "TCP";
bool static api_run_mode = false;
bool static spdk_initialized = false;

void LaunchNvmfAPI();
void nvmfCreateTransportDone(void* cb_arg, int status);
void nvmfCreateSubsystemDone(void* cb_arg, int status);
void nvmfAttachNamespace1Done(void* cb_arg, int nsid);
void nvmfAttachNamespace2Done(void* cb_arg, int nsid);
void nvmfAttachListenerDone(void* cb_arg, int status);
void nvmfSubsystemInfo(void);

void CleanupNvmfAPI();
void nvmfDetachListenerDone(void* cb_arg, int status);
void nvmfDetachNamespace1Done(void* cb_arg, int status);
void nvmfDetachNamespace2Done(void* cb_arg, int status);
void nvmfDeleteNvmfSubsystemDone(void* cb_arg, int status);

static int UNVMfSubmitHandler(struct ibof_io* io);
/*
 * LaunchNvmfAPI: perfome create RDMA transport -> create Nvmf Subsystem -> create ibof_bdev -> attach namespace -> attach  listener
 */
void LaunchNvmfAPI(){
	bool ret = target->CreateTransport(trtype, NULL, nvmfCreateTransportDone, target);
	cout<<"CreateTransport ret="<<ret<<endl;
}

void nvmfAttachListenerDone(void* cb_arg, int status){
	cout<<"nvmfAttachListenerDone status="<<status<<endl;
	target->DumpSubsystemInfo();
//	CleanupNvmfAPI();
}

void nvmfAttachNamespace2Done(void* cb_arg, int nsid){
	bool ret;
	NvmfTarget* target = (NvmfTarget*)cb_arg;
	cout<<"nvmfAttachNamespace2Done nsid="<<nsid<<endl;

	ret = target->AttachListener(nqn, ip, port, trtype, nvmfAttachListenerDone, target);
	cout<<"AttachListener ret="<<ret<<endl;
}

void nvmfAttachNamespace1Done(void* cb_arg, int nsid){
	bool ret;
	NvmfTarget* target = (NvmfTarget*)cb_arg;

	cout<<"nvmfAttachNamespace1Done nsid="<<nsid<<endl;
	int new_nsid=20;
	ret = target->AttachNamespace(nqn, ibof_bdev1, new_nsid, nvmfAttachNamespace2Done, target);
	cout<<"AttachNamespace ret="<<ret<<endl;
}

void nvmfCreateSubsystemDone(void* cb_arg, int status){
	bool ret;
	NvmfTarget* target = (NvmfTarget*)cb_arg;
	cout<<"nvmfCreateSubsystemDone status="<<status<<endl;

	ret = target->CreateIBoFBdev(ibof_bdev0, 0, 128, 512, true);
	cout<<"CreateIBoFBdev ret="<<ret<<endl;

	ret = target->CreateIBoFBdev(ibof_bdev1, 1, 128, 512, true);
	cout<<"CreateIBoFBdev ret="<<ret<<endl;

	ret = target->AttachNamespace(nqn, ibof_bdev0, nvmfAttachNamespace1Done, target);
	cout<<"AttachNamespace ret="<<ret<<endl;
}

void nvmfCreateTransportDone(void* cb_arg, int status){
	bool ret;
	NvmfTarget* target = (NvmfTarget*)cb_arg;
	cout<<"nvmfCreateTransportDone status="<<status<<endl;

	ret = target->CreateNvmfSubsystem(nqn, "POS00000000000001", 0, true, nvmfCreateSubsystemDone, target);
	cout<<"CreateNvmfSubsystem ret="<<ret<<endl;
}

/*
 * CleanupNvmfAPI: performe detach listener -> detach namespace -> remove ibof_bdev -> destroy Nvmf Subsystem
 */
void CleanupNvmfAPI(){
	bool ret = target->DetachListener(nqn, ip, port, trtype, nvmfDetachListenerDone, (void*)target);
	cout<<"DetachListener ret="<<ret<<endl;
}

void nvmfDeleteNvmfSubsystemDone(void* cb_arg, int status){
	cout<<"nvmfDeleteNvmfSubsystemDone status="<<status<<endl;
}

void nvmfDetachNamespace2Done(void* cb_arg, int status){
	bool ret;
	NvmfTarget* target = (NvmfTarget*)cb_arg;
	cout<<"nvmfDetachNamespcaeDone status="<<status<<endl;

	ret = target->DeleteIBoFBdev(ibof_bdev0);
	cout<<"DeleteIBoFBdev ret="<<ret<<endl;

	ret = target->DeleteNvmfSubsystem(nqn, nvmfDeleteNvmfSubsystemDone, target);
	cout<<"DeleteNvmfSubsystem ret="<<ret<<endl;
}

void nvmfDetachNamespace1Done(void* cb_arg, int status){
	bool ret;
	NvmfTarget* target = (NvmfTarget*)cb_arg;

	cout<<"nvmfDetachNamespca1 status="<<status<<endl;
	ret = target->DetachNamespace(nqn, 20, nvmfDetachNamespace2Done, target);
	cout<<"DetachNamespace ret="<<ret<<endl;
}

void nvmfDetachListenerDone(void* cb_arg, int status){
	bool ret;
	NvmfTarget* target = (NvmfTarget*)cb_arg;

	cout<<"nvmfDetachListenerDone status="<<status<<endl;
	ret = target->DetachNamespace(nqn, 1, nvmfDetachNamespace1Done, target);
	cout<<"DetachNamespace ret="<<ret<<endl;
}


static void UNVMfCompleteHandler(void) {
	//Note: do nothing
}
/*
 * startDoneHandler : is called when Spdk initialization is done
 */
static int startDoneHandler(){
	spdk_initialized = true;

	if(api_run_mode == true){
		LaunchNvmfAPI();
	}

	NvmfVolumeMock* volume = new NvmfVolumeMock();
        unvmf_io_handler handler = { .submit = UNVMfSubmitHandler, .complete = UNVMfCompleteHandler};
	volume->SetuNVMfIOHandler(handler);
	spdk_bdev_ibof_register_io_handler(ibof_bdev0, handler);

	target = new NvmfTarget(volume);
	target->Start();

	int rc = spdk_nvme_probe(NULL, NULL, NULL, NULL, NULL);
        cout<<"spdk_nvme_probe() ret ="<<rc<<endl;
	return 0;
}

static void complete(void* arg1, void* arg2){
	struct ibof_io* io = (struct ibof_io*)arg1;
	if(io->complete_cb){
		io->complete_cb(io,0);
	}
}
/*
 * iBoFIOHandler : is called at the end of uNVMf processing
 */
static int UNVMfSubmitHandler(struct ibof_io* io) {
        //cout<<"EnQueue : DIRECTION - " <<io->direction<< endl;
	//NOTE:  iBoFIO Handler need to call complete_cb, which call bdev_ibof_io_complete() -> spdk_bdev_io_complete() -> nvmf -> rdma,,, in sequencial order.
	if(io->complete_cb){
		io->complete_cb(io, 0);
	}
	return SPDK_BDEV_IO_STATUS_SUCCESS;
}


static void exit_handler(int signo){
	spdk_initialized = false;
}
/*
 * Note : the test app shows how to make use of Nvmf Target as a separate application.
 * 	Application can register iBoFIOHandler that will be processed during nvmf - bdev - ibof volume
 *
*/
int main(int argc, char **argv) {
	if(argc  == 2 && strcmp(argv[1],"api") == 0){
		api_run_mode = true;
		cout<<"Run "<<argv[0]<<" as API mode"<<endl;
	}

	Spdk* ins = Spdk::Instance();
	ins->SetStartCompletedHandler(startDoneHandler);
	ins->Init(argc,argv);

	if (signal(SIGINT, exit_handler) == SIG_ERR) {
		cout<<"An error occurred while setting a signal handler"<<endl;
		return EXIT_FAILURE;
	}

	while(spdk_initialized){
		sleep(1);
	}

	return 0;
}

