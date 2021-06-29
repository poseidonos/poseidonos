#include <gtest/gtest.h>
#include <gtest/gtest_prod.h>
#include "../../../src/scheduler/io_queue.h"

using namespace std;

namespace pos{ 
class IOQueueTest : public testing ::Test{
protected:
    IOQueue *iq;
    IOQueueTest (){}
    virtual ~IOQueueTest(){}

    virtual void SetUp(){
        std::cout << "Before IOQueueTest" << std::endl;
        iq = new IOQueue();
    }
    
    virtual void TearDown(){
        std::cout << "After IOQueueTest" << std::endl;
        delete iq;
    }
    
}; 
//To use private member/function 
//-> add needed function to public or use FRIEND_TEST()
TEST_F(IOQueueTest, EnqueuePositiveTest){
    Ubio *first_ubio, *second_ubio;
    iq->EnqueueUbio(first_ubio);
    EXPECT_EQ(1, iq->GetQueueSize());
    iq->EnqueueUbio(second_ubio);
    EXPECT_EQ(2, iq->GetQueueSize());
}

TEST_F(IOQueueTest, DequeueNegativeTest){
   Ubio* ubio;
   EXPECT_NE(ubio,iq->DequeueUbio());
}

TEST_F(IOQueueTest, DequeuePositiveTest){
    Ubio *first_ubio;
    iq->EnqueueUbio(first_ubio);
    EXPECT_EQ(first_ubio, iq->DequeueUbio());
}
TEST_F(IOQueueTest, QueueScenerioTest){
    Ubio *first_ubio;
    iq->EnqueueUbio(first_ubio);
    EXPECT_EQ(first_ubio, iq->DequeueUbio());
    Ubio *second_ubio;
    iq->EnqueueUbio(second_ubio);
    EXPECT_NE(first_ubio, second_ubio);
}
}//namespace
  
