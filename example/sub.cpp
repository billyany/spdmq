#include <iostream>
#include "spdmq/spdmq.h"
using namespace speed::mq;

int main () {
    spdmq_ctx_t ctx;
    ctx.mode(COMM_MODE::SPDMQ_SUB) // 设置 sub 模式
       .heartbeat(1000) // 设置心跳为 50 ms
       .topics({"spdmq"}); // 设置订阅的 topic 信息
       
    auto mq_ptr = NEW_SPDMQ(ctx);
    mq_ptr->on_recv = [] (spdmq_msg_t& msg) {
        std::cout << "data:" << (char*)msg.payload.data() << std::endl;
    };
    mq_ptr->on_online = [] (spdmq_msg_t& msg) { // 设置客户端在线回调函数
        std::cout << "connect success, session id:" << msg.session_id << std::endl;
    };
    mq_ptr->on_offline = [] (spdmq_msg_t& msg) { // 设置客户端离线回调函数
        std::cout << "disconnect success, session id:" << msg.session_id << std::endl;
    };
    // mq_ptr->connect("ipc://speedmq"); // 连接 ipc 地址
    mq_ptr->connect("tcp://0.0.0.0:12345"); // 绑定 tcp 端口地址
    mq_ptr->spin();
}
