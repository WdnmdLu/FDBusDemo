# 本项目是一个基于fdbus通信实现的搜索引擎
客户端会向服务器发起搜索请求，服务器返回搜索的结果给到客户端
分别使用了锁机制和信号量机制确保了只能客户端完全接受了服务器发送的数据，客户端才能发起新的搜索请求

可执行文件最终都生成在Demo目录下
![image](https://github.com/WdnmdLu/FDBusDemo/assets/141596071/3d393d47-aab5-47ff-868a-612cc792b312)
![image](https://github.com/WdnmdLu/FDBusDemo/assets/141596071/ad270cd7-8a03-47ae-8a1a-6481f6b560c1)


进入到build目录下
![image](https://github.com/WdnmdLu/FDBusDemo/assets/141596071/f292282f-6dd3-4564-a825-1155c65869a3)
直接执行make命令就会在Demo目录下生成可执行文件
