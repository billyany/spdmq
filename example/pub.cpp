#include <unistd.h>
#include <iostream>
#include "spdmq/spdmq.h"
using namespace speed::mq;


int main () {
    spdmq_ctx_t ctx;
    ctx.mode(COMM_MODE::SPDMQ_PUB); // 设置 pub 模式
    auto mq_ptr = NEW_SPDMQ(ctx);
    mq_ptr->on_recv = [] (spdmq_msg_t& msg) {
        std::cout << "data:" << (char*)msg.payload.data() << std::endl;
    };
    mq_ptr->on_online = [] (spdmq_msg_t& msg) {
        std::cout << "client connect success, session id:" << msg.session_id << std::endl;
    };
    mq_ptr->on_offline = [] (spdmq_msg_t& msg) {
        std::cout << "client disconnect success, session id:" << msg.session_id << std::endl;
    };
    mq_ptr->bind("ipc://speedmq");  // 绑定 ipc 地址
    mq_ptr->spin(true); // 设置后台运行

    int64_t cnt = 0;
    while (true) {
        spdmq_msg_t msg;
        msg.topic = "spdmq"; // 发送 topic 为 spdmq 的信息
        auto data = std::to_string(cnt++);
        msg.payload = {data.begin(), data.end()};
        mq_ptr->send(msg);
        usleep(100 * 1000);
    }
}