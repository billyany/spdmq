# spdmq

[![Linux](https://badgen.net/badge/Linux/success/green?icon=github)](https://github.com/billyany/spdmq/actions/workflows/.c-cpp.yml?query=branch%3Adev)
<br>
[![release](https://badgen.net/github/release/billyany/spdmq?icon=github)](https://github.com/billyany/spdmq/releases)
[![stars](https://badgen.net/github/stars/billyany/spdmq?icon=github)](https://github.com/billyany/spdmq/stargazers)
[![forks](https://badgen.net/github/forks/billyany/spdmq?icon=github)](https://github.com/billyany/spdmq/forks)
[![issues](https://badgen.net/github/issues/billyany/spdmq?icon=github)](https://github.com/billyany/spdmq/issues)
[![PRs](https://badgen.net/github/prs/billyany/spdmq?icon=github)](https://github.com/billyany/spdmq/pulls)
[![contributors](https://badgen.net/github/contributors/billyany/spdmq?icon=github)](https://github.com/billyany/spdmq/contributors)
[![license](https://badgen.net/github/license/billyany/spdmq?icon=github)](LICENSE)


`spdmq`是一个类似于`zmq`的网络库，提供了更易用的接口，基于C++17标准开发（目前暂时只支持Linux平台，后续会支持跨平台）。


## ⚡️ 快速入门

### 体验
运行编译脚本`./scripts/build.sh`:

#### PUB 服务端
```c++
#include <unistd.h>
#include "spdmq/spdmq.h"
using speed::mq;


int main () {
    spdmq_ctx_t ctx;
    ctx.mode(COMM_MODE::SPDMQ_PUB); // 设置 pub 模式
    auto mq_ptr = NEW_SPDMQ(ctx);
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
```

#### SUB 客户端
```c++
#include <iostream>
#include "spdmq/spdmq.h"
using speed::mq;

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
```