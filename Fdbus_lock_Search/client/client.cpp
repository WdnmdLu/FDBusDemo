﻿#include <string.h>
#include <stdlib.h>
#include <iostream>
#include <fdbus/fdbus.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <string>
#include "json.hpp"
#define XCLT_TEST_SINGLE_DIRECTION 0
#define XCLT_TEST_BI_DIRECTION     1
using namespace ipc::fdbus;
using json = nlohmann::json;

static int TotalMax = 0;//此次接收的文件的一个大小是多少
static int Count = 0;//接收到的数据的大小是多少

pthread_mutex_t mut;
sem_t Full;
sem_t Empty;
pthread_cond_t Cond;
// 客户端发送消息以json的格式进行消息的发送
class FDBusClient : public CBaseClient
{
public:
    FDBusClient(const char* name, CBaseWorker* worker = 0)
        : CBaseClient(name, worker)//worker：如果指定则所有回调函数在该worker上运行
                                   //否则在fdbus工作线程FDB_CONTEXT上运行
    {
    }
protected:
    void onOnline(const CFdbOnlineInfo& info) override
    {
        printf("........ [%s] onOnline mSid=%d.............\n", name().c_str(), info.mSid);

        std::vector<int32_t> subIDs;
        subIDs.push_back(1);

        CFdbMsgSubscribeList subscribe_list;
        for (auto id : subIDs)
        {
            addNotifyItem(subscribe_list, id);
        }
        subscribe(subscribe_list);
        
    }

    void onOffline(const CFdbOnlineInfo& info) override
    {
        printf("\nOffline...\n");
        exit(1);
    }
    // 消耗Empty，生产Full
    void onReply(CBaseJob::Ptr& msg_ref) override
    {
        sem_wait(&Empty);

        auto msg = castToMessage<CBaseMessage*>(msg_ref);
        json js = json::parse(msg->getPayloadBuffer());

        int id = js["Id"].get<int>();
        //printf("id: %d\n",id);
        switch (id)
        {
        case 0://第一次接收到数据
            {
                TotalMax = js["FileSize"].get<int>();//文件总大小
                Count += js["Count"].get<int>();// 目前已经接收到的文件字节数
                //printf("TotalMax: %d   Count: %d\n",TotalMax,Count);
                std::cout<<js["Msg"].get<std::string>()<<std::endl;
            

                if(TotalMax == Count){
                    TotalMax=0;
                    Count = 0;
                    printf("文件传输完成\n");
                    sem_post(&Full);
                    return;
                }
                json Send;
                Send["Fd"]=js["Fd"];//从哪个文件读书节
                Send["Id"]=1;


                char buffer[64]={0};
                strcpy(buffer,Send.dump().c_str());
                this->invoke(XCLT_TEST_SINGLE_DIRECTION, buffer, sizeof(buffer));
                sem_post(&Empty);
            }
            break;
        case 1:
            {
                Count += js["Count"].get<int>();
                //printf("TotalMax: %d   Count: %d\n",TotalMax,Count);
                std::cout<<js["Msg"].get<std::string>()<<std::endl;
                if(TotalMax == Count){
                    printf("FileSize: %d\n",TotalMax);
                    TotalMax=0;
                    Count = 0;
                    printf("文件传输完成\n");
                    sem_post(&Full);
                    return;
                }
                json Send;
                Send["Fd"]=js["Fd"];//从哪个文件读书节
                Send["Id"]=1;
                char buffer[64]={0};
                strcpy(buffer,Send.dump().c_str());
                this->invoke(XCLT_TEST_SINGLE_DIRECTION, buffer, sizeof(buffer));
                sem_post(&Empty);
            }
        case -1:
            {
                //查询失败了,退出查询
                printf("查询失败了\n");
                pthread_mutex_unlock(&mut);
                pthread_cond_signal(&Cond);
                sem_post(&Full);
                break;
            }
        //第一次并没有将这些文件全部发送完成，需要下一次继续进行发送
        }

    }
    void onStatus(CBaseJob::Ptr& msg_ref, int32_t error_code, const char* description) override
    {
        std::string sError = "[error] ";
        sError += "code:" + std::to_string(error_code) + ";";
        sError += "detail:";
        sError += description;
    }

    void onBroadcast(CBaseJob::Ptr& msg_ref) override
    {
        printf("[%s]\n", __FUNCTION__);
        auto msg = castToMessage<CBaseMessage*>(msg_ref);
    }
};

int main()
{
    sem_init(&Full, 0, 1);//设置了一个初始值为1的信号量
    sem_init(&Empty, 0, 0);//设置了一个初始值为1的信号量
    FDB_CONTEXT->enableLogger(false);

    FDB_CONTEXT->start(); //启动fdbus收发线程；其本质就是一个worker线程

    // 专门创建这个线程进行数据的接收
    auto spWorker = std::make_shared<CBaseWorker>();
    spWorker->start(); 
    std::string url(FDB_URL_SVC);
    url += FDB_XTEST_NAME;
    auto client = new FDBusClient("TestClient", spWorker.get());    //指定链接TestClient(名字寻址)，.get()指定原始地址
    client->connect(url.c_str());

    // 这个主线程就专门进行数据的发送
    // 消耗Full，生产Empty
    while (1)
    {
        //消耗了一个信号量
        sem_wait(&Full);

        char buffer[64] = {0};


        
        std::cout<<"请输入搜索的内容: ";
        fflush(NULL);
        fgets(buffer, sizeof(buffer), stdin);

        // 移除换行符
        size_t length = strlen(buffer);
        if (length > 0 && buffer[length - 1] == '\n') {
            buffer[length - 1] = '\0'; // 将换行符替换为字符串结束符
        }
        json js;
        js["Query"] = buffer;
        js["Id"] = 0;//说明这是一个请求查询
        std::string Send = js.dump();

        char str[64]={0};
        strcpy(str,Send.c_str());
        printf("str: %s\n",str);

        client->invoke(XCLT_TEST_SINGLE_DIRECTION, str, sizeof(str));
        sem_post(&Empty);
    }
    return 0;
}
    