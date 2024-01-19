#include <iostream>
#include "spdmq/spdmq.h"
using namespace speed::mq;

int main () {
    spdmq_ctx_t ctx;
    ctx.topics({"spdmq"})          // 设置订阅的 topic 信息
       .mode(COMM_MODE::SPDMQ_SUB) // 设置 sub 模式
       .heartbeat(10);             // 设置心跳为 10 ms
    auto mq_ptr = NEW_SPDMQ(ctx);
    mq_ptr->connect("ipc://speedmq"); // 连接 ipc 地址
    mq_ptr->spin(true);

    while (true) {
        spdmq_msg_t msg;
        mq_ptr->recv(msg, 0); // 阻塞接收消息
        std::cout << "data:" << (char*)msg.payload.data() << std::endl;
    }
}