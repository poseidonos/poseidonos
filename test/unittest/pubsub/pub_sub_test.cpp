#include "publisher.hpp"
#include "subscriber.hpp"

using namespace ibofos;
using namespace std;


/* Test Publishers */
class PublisherStub final : public Publisher {
public:
	PublisherStub(const char* name) : Publisher(name) {}
};

/* Test Subscribers */
class SubscriberStub final : public Subscriber {
public:
	SubscriberStub(const char* name) : Subscriber(name) {}
	void onNotify(uint32_t event, void* data){
		cout<<"sub:"<<getName()<<" - notified with data: event="<<IBOF_EVENT_TYPE::getEventString(event)<<endl;
	}
};

/* Below example show Publisher:Subscriber = 2:2 */
int main(int argc, char** argv){
	char buffer[32];
	SubscriberStub* unvmf = new SubscriberStub("uNVMf");
	SubscriberStub* module = new SubscriberStub("iBofModule");
	PublisherStub* vm = new PublisherStub("VolumeManager");
	vm->addSubscriber(unvmf);
	vm->addSubscriber(module);

	PublisherStub* am = new PublisherStub("ArrayManager");
	am->addSubscriber(unvmf);
	am->addSubscriber(module);

	vm->notify(IBOF_EVENT_TYPE::IBOF_EVENT_VOLUME_CREATED);
	vm->notify(IBOF_EVENT_TYPE::IBOF_EVENT_VOLUME_CHANGED, buffer);
	vm->notify(IBOF_EVENT_TYPE::IBOF_EVENT_VOLUME_DELETED);

	am->notify(IBOF_EVENT_TYPE::IBOF_EVENT_ARRAY_CREATED);
	am->notify(IBOF_EVENT_TYPE::IBOF_EVENT_ARRAY_CHANGED, buffer);
	am->notify(IBOF_EVENT_TYPE::IBOF_EVENT_ARRAY_DELETED);

	vm->removeSubscriber(unvmf);
	vm->removeSubscriber(module);

	am->removeSubscriber(unvmf);
	am->removeSubscriber(module);

	delete unvmf;
	delete module;
	delete vm;
	delete am;

	return 0;
}
