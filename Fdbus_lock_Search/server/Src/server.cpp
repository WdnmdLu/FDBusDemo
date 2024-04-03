#include <string.h>
#include <stdlib.h>
#include <iostream>
#include <fdbus/fdbus.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "json.hpp"
#include "InverTerm.hpp"
#define XCLT_TEST_SINGLE_DIRECTION 0
#define XCLT_TEST_BI_DIRECTION     1
using json = nlohmann::json;

using namespace ipc::fdbus;

class CXServer : public CBaseServer
{
public:
    CXServer(const char* name, CBaseWorker* worker = 0)
        : CBaseServer(name, worker)//worker：如果指定则所有回调函数在该worker上运行；
                                   //否则在fdbus工作线程FDB_CONTEXT上运行
    {
        // 建立倒排索引
        Temp.set_search_path("../server/File");
    }
    
protected:
    void broadcastElapseTime(CMethodLoopTimer<CMediaServer> *timer)
    void onOnline(const CFdbOnlineInfo& info) override
    {
        printf("客户端上线\n");

    }
    void onOffline(const CFdbOnlineInfo& info) override
    {
        printf("客户端下线\n");

    }
    // 数据接收回调函数
    void onInvoke(CBaseJob::Ptr& msg_ref) override
    {
        printf("收到了客户端的数据\n");
        auto msg = castToMessage<CBaseMessage*>(msg_ref);
        auto sid = msg->getSession()->sid();
        // 获取消息中的 payload 数据
        char* message = (char*)(msg->getPayloadBuffer());

        string Res(message);//接收到的消息是Json格式的消息
        std::cout<<Res<<std::endl;
        json js = json::parse(Res);//将结果进行反序列化

        int id = js["Id"].get<int>();
        printf("js[\"Id\"] %d\n",id);
        char buffer[1024]={0};
        char SendBuffer[2048]={0};
        switch (id)
        {
            // 第一次发送了查询的请求
            case 0:
            {
               
                // 获取打开的文件描述符
                Res = js["Query"].get<string>();
                string Path = Temp.Query(Res);
                //搜索失败了
                if(Path == "NULL"){
                    printf("查询失败\n");
                    json faile;
                    faile["Id"] = -1;
                    char buff[8]={0};
                    strcpy(buff,faile.dump().c_str());
                    msg->reply(msg_ref, buff, sizeof(buff));
                    break;
                }
                printf("查询成功 %s\n",Path.c_str());
                struct stat file_stat;
                stat(Path.c_str(), &file_stat);

                int fd = open(Path.c_str(), O_RDONLY);
                int len = read(fd,buffer,1024);
                int FileSize = file_stat.st_size;
                printf("File Size: %d bytes   Read Bytes: %d\n", FileSize,len);
                
                json Send;
                Send["Count"] = len;
                Send["FileSize"] = FileSize;
                
                //一次就把数据给全部都读完了
                if(len == FileSize){
                    printf("文件传输完成了");
                    close(fd);
                }

                Send["Fd"] = fd;
                Send["Id"] = 0;//这是第一次传输文件数据
                Send["Msg"] = buffer;
                strcpy(SendBuffer,Send.dump().c_str());

                msg->reply(msg_ref, SendBuffer, sizeof(SendBuffer));

                break;
            }
            //不是第一次发送的查询请求，此时发送的数据还没有发送完成
            case 1:
            {
                printf("接着将数据发送给客户端\n");
                int fd = js["Fd"];
                int len = read(fd,buffer,1024);
                printf("len: %d\n",len);
                //说明这次把数据给全部读完了
                if(len < 1024){
                    printf("文件传输完成了\n");
                    close(fd);
                }

                json Send;
                Send["Count"]=len;//本次读到的字节数是len个字节
                Send["Fd"] = fd;
                Send["Id"] = 1;
                Send["Msg"] = buffer;


                strcpy(SendBuffer,Send.dump().c_str());
                msg->reply(msg_ref, SendBuffer, sizeof(SendBuffer));
                //printf("成功发送了数据\n");
                break;
            }
        }
    }

private:
    InvertIndex Temp;
    uint64_t mTotalBytesReceived;
    uint64_t mTotalRequest;
    uint64_t mIntervalBytesReceived;
    uint64_t mIntervalRequest;
    void incrementReceived(uint32_t size)
    {
        mTotalBytesReceived += size;
        mIntervalBytesReceived += size;
        mTotalRequest++;
        mIntervalRequest++;
    }
};


int main(int argc, char** argv)
{

    FDB_CONTEXT->enableLogger(false);
    /* start fdbus context thread */
    FDB_CONTEXT->start();//启动fdbus收发线程；其本质就是一个worker线程

    auto server = new CXServer(FDB_XTEST_NAME);


    server->TCPEnabled();
    server->bind();
    
    /* convert main thread into worker */
    CBaseWorker background_worker;
    background_worker.start(FDB_WORKER_EXE_IN_PLACE);//FDB_WORKER_EXE_IN_PLACE：置位表示不启动线程，在调用的地方运行worker的主循环
                                                    //否则启动新线程执行worker的主循环
}