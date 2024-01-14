/*
*   Copyright 2024 billy_yan billyany@163.com
*
*   Licensed under the Apache License, Version 2.0 (the "License");
*   you may not use this file except in compliance with the License.
*   You may obtain a copy of the License at
*
*       http://www.apache.org/licenses/LICENSE-2.0
*
*   Unless required by applicable law or agreed to in writing, software
*   distributed under the License is distributed on an "AS IS" BASIS,
*   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*   See the License for the specific language governing permissions and
*   limitations under the License.
*/

#pragma once

#include <set>
#include <cstdint>
#include <string>
#include <sstream>

namespace speed::mq {

// error code
using spdmq_code_t = int32_t;
enum class SPDMQ_CODE : int32_t{
    SPDMQ_CODE_UNKNOW = -1,  // 未知错误
#define SPDMQ_CODE_UNKNOW (static_cast<spdmq_code_t>(SPDMQ_CODE::SPDMQ_CODE_UNKNOW))

    SPDMQ_CODE_OK =  0,  // OK
#define SPDMQ_CODE_OK (static_cast<spdmq_code_t>(SPDMQ_CODE::SPDMQ_CODE_OK))

    SPDMQ_CODE_ADDRESS_ERROR =  1,  // 地址格式错误
#define SPDMQ_CODE_ADDRESS_ERROR (static_cast<spdmq_code_t>(SPDMQ_CODE::SPDMQ_CODE_ADDRESS_ERROR))

//     SPDMQ_CODE_NO_DATA =  1,  // 无数据, 客户或者服务未收到数据, 或者发送的数据为空
// #define SPDMQ_CODE_NO_DATA (static_cast<spdmq_code_t>(SPDMQ_CODE::SPDMQ_CODE_NO_DATA))

//     SPDMQ_CODE_DATA_PARSE_ERROR =  2,  // 接收数据解析失败
// #define SPDMQ_CODE_DATA_PARSE_ERROR (static_cast<spdmq_code_t>(SPDMQ_CODE::SPDMQ_CODE_DATA_PARSE_ERROR))

//     SPDMQ_CODE_DATA_SEND_FAILED =  3,  // 数据发送失败
// #define SPDMQ_CODE_DATA_SEND_FAILED (static_cast<spdmq_code_t>(SPDMQ_CODE::SPDMQ_CODE_DATA_SEND_FAILED))

//     SPDMQ_CODE_ADDRESS_OCCUPATION =  4,  // 绑定地址被占用
// #define SPDMQ_CODE_ADDRESS_OCCUPATION (static_cast<spdmq_code_t>(SPDMQ_CODE::SPDMQ_CODE_ADDRESS_OCCUPATION))

//     SPDMQ_CODE_NOT_BIND_CONNECT =  6,  // 未执行bind或者connect
// #define SPDMQ_CODE_NOT_BIND_CONNECT (static_cast<spdmq_code_t>(SPDMQ_CODE::SPDMQ_CODE_NOT_BIND_CONNECT))

//     SPDMQ_CODE_RECV_TIMEOUT =  7,  // 接收数据响应超时
// #define SPDMQ_CODE_RECV_TIMEOUT (static_cast<spdmq_code_t>(SPDMQ_CODE::SPDMQ_CODE_RECV_TIMEOUT))

//     SPDMQ_CODE_CB_SET_FAILED =  8,  // 回调函数设置失败, 需要在bind和connect之前设置
// #define SPDMQ_CODE_CB_SET_FAILED (static_cast<spdmq_code_t>(SPDMQ_CODE::SPDMQ_CODE_CB_SET_FAILED))

//     SPDMQ_CODE_MODE_NOT_MATCH =  9,  // 模式不匹配
// #define SPDMQ_CODE_MODE_NOT_MATCH (static_cast<spdmq_code_t>(SPDMQ_CODE::SPDMQ_CODE_MODE_NOT_MATCH))

//     SPDMQ_CODE_SEND_DATA_EMPTY =  10, // 发送数据为空, 无法发送
// #define SPDMQ_CODE_SEND_DATA_EMPTY (static_cast<spdmq_code_t>(SPDMQ_CODE::SPDMQ_CODE_SEND_DATA_EMPTY))

//     SPDMQ_CODE_FORBID_REPEAT_BIND_OR_CONNECT =  11, // 禁止重复bind或connect
// #define SPDMQ_CODE_FORBID_REPEAT_BIND_OR_CONNECT (static_cast<spdmq_code_t>(SPDMQ_CODE::SPDMQ_CODE_FORBID_REPEAT_BIND_OR_CONNECT))

//     SPDMQ_CODE_SEND_FAILED_NOT_CONNECTED_TARGET =  12, // 发送失败, 未连接到目标
// #define SPDMQ_CODE_SEND_FAILED_NOT_CONNECTED_TARGET (static_cast<spdmq_code_t>(SPDMQ_CODE::SPDMQ_CODE_SEND_FAILED_NOT_CONNECTED_TARGET))

//     SPDMQ_CODE_CONNECT_TO_BROKEN =  13, // 连接中断
// #define SPDMQ_CODE_CONNECT_TO_BROKEN (static_cast<spdmq_code_t>(SPDMQ_CODE::SPDMQ_CODE_CONNECT_TO_BROKEN))

//     SPDMQ_CODE_NO_REQUEST =  14, // 没有请求数据
// #define SPDMQ_CODE_NO_REQUEST (static_cast<spdmq_code_t>(SPDMQ_CODE::SPDMQ_CODE_NO_REQUEST))

//     SPDMQ_CODE_RECV_CB_INTERCEPTED_DATA =  15, // 接收回调已经拦截了数据
// #define SPDMQ_CODE_RECV_CB_INTERCEPTED_DATA (static_cast<spdmq_code_t>(SPDMQ_CODE::SPDMQ_CODE_RECV_CB_INTERCEPTED_DATA))

};

class spdmq;
using spdmq_t = spdmq;

class spdmq_msg;
using spdmq_msg_t = spdmq_msg;

using time_msec_t = uint32_t;

typedef enum class COMM_MODE : uint8_t {
    SPDMQ_UNKNOW = 0, // unknow mode
    SPDMQ_PUB = 1,    // publish mode
    SPDMQ_SUB = 2,    // subscribe mode
} comm_mode_t;

typedef enum class COMM_METHOD : uint8_t {
    UNKNOW = 0,
    SOCKET = 1,
    PIPE = 2, // TODO
    MMAP = 3, // TODO
    SHMEM = 4, // TODO
} comm_method_t;

typedef enum class COMM_DOMAIN : uint8_t {
    UNKNOW = 0,
    IPV4 = 1,
    IPV6 = 2, // TODO
    IPC = 3,
} comm_domain_t;

typedef enum class COMM_PROTOCOL : uint8_t {
    UNKNOW = 0,
    TCP = 1,
    UDP = 2,  // TODO
} comm_protocol_t;

typedef enum class EVENT_MODE : uint8_t {
    EVENT_UNKNOW = 0, // unknow event
    EVENT_POLL_LT = 0, // epoll level trigger
    EVENT_POLL_ET = 1, // epoll edge trigger
} event_mode_t;

typedef class spdmq_ctx {
private:
    comm_mode_t _mode;             // communication mode
    comm_method_t _method;         // communication method
    comm_domain_t _domain;         // communication domain
    comm_protocol_t _protocol;     // communication protocol
    event_mode_t _event_mode;      // event mode
    int32_t _evt_num;              // the number of single listening events in the server model, default to 100 events
    int32_t _heartbeat;            // client mode heartbeat interval, default to 100 milliseconds
    int32_t _queue_size;           // the number of messages in the message queue, default to 1024 messages
    std::set<std::string> _topics; // topics of DBUS_PUB/DBUS_SUB mode

public:
    /* variable set */
    spdmq_ctx& mode(comm_mode_t mode);
    spdmq_ctx& method(comm_method_t method);
    spdmq_ctx& domain(comm_domain_t domain);
    spdmq_ctx& protocol(comm_protocol_t protocol);
    spdmq_ctx& event_mode(event_mode_t event_mode);
    spdmq_ctx& evt_num(int32_t evt_num);
    spdmq_ctx& heartbeat(int32_t heartbeat);
    spdmq_ctx& queue_size(int32_t queue_size);
    spdmq_ctx& topics(std::set<std::string> topics);

    /* variable get */
    comm_mode_t mode();
    comm_method_t method();
    comm_domain_t domain();
    comm_protocol_t protocol();
    event_mode_t event_mode();
    int32_t evt_num();
    int32_t heartbeat();
    int32_t queue_size();
    std::set<std::string> topics();

public:
    spdmq_ctx() {
        _mode = COMM_MODE::SPDMQ_UNKNOW;
        _method = COMM_METHOD::SOCKET;
        _domain = COMM_DOMAIN::UNKNOW;
        _protocol = COMM_PROTOCOL::TCP;
        _event_mode = EVENT_MODE::EVENT_POLL_ET;
        _evt_num = 100;
        _heartbeat = 100;
        _queue_size = 1024;
        _topics.clear();
    }

} spdmq_ctx_t;

enum class DBUS_MESSAGE_TYPE : uint8_t {
    DBUS_HEARTBEAT = 1, // heartbeat message
    DBUS_TOPIC = 2,     // topic message
};

struct spdmq_msg {
    int64_t session_id;         // session id
    DBUS_MESSAGE_TYPE msg_type; // used to distinguish different message types
    std::string topic;          // topic of DBUS_PUB/DBUS_SUB  mode
    std::string data;           // communication data
    int64_t send_time_stamp;    // send UTC time, unit microseconds
    int64_t recv_time_stamp;    // recv UTC time, unit microseconds
    int64_t time_cost;          // message sending and receiving time, unit microseconds

    spdmq_msg() {
        session_id = {};
        msg_type = {};
        topic = {};
        data = {};
        send_time_stamp = {};
        recv_time_stamp = {};
        time_cost = {};
    }

    std::string toString() {
        std::stringstream ss;
        ss << "\nsession_id: " << session_id 
           << "\nmsg_type: " << static_cast<uint32_t>(msg_type)
           << "\ntopic: " << topic 
           << "\ndata: " << data 
           << "\nsend_time_stamp: " << send_time_stamp 
           << "\nrecv_time_stamp:" << recv_time_stamp 
           << "\ntime_cost: " << time_cost
           << std::endl;
        return ss.str();
    }
};

// 序列化函数
inline std::string serializespdmq_msg_t(const spdmq_msg& dbus_data) {
    std::ostringstream oss;
    oss.write(reinterpret_cast<const char*>(&dbus_data.session_id), sizeof(dbus_data.session_id));
    oss.write(reinterpret_cast<const char*>(&dbus_data.msg_type), sizeof(dbus_data.msg_type));

    // 序列化 topic 字符串
    int32_t topic_length = dbus_data.topic.size();
    oss.write(reinterpret_cast<const char*>(&topic_length), sizeof(topic_length));
    oss.write(dbus_data.topic.data(), topic_length);

    // 序列化 data 字符串
    int32_t data_length = dbus_data.data.size();
    oss.write(reinterpret_cast<const char*>(&data_length), sizeof(data_length));
    oss.write(dbus_data.data.data(), data_length);

    oss.write(reinterpret_cast<const char*>(&dbus_data.send_time_stamp), sizeof(dbus_data.send_time_stamp));
    oss.write(reinterpret_cast<const char*>(&dbus_data.recv_time_stamp), sizeof(dbus_data.recv_time_stamp));
    oss.write(reinterpret_cast<const char*>(&dbus_data.time_cost), sizeof(dbus_data.time_cost));
    return oss.str();
}

// 反序列化函数
inline spdmq_msg deserializespdmq_msg_t(const std::string& serialized_data) {
    spdmq_msg dbus_data;
    std::istringstream iss(serialized_data);
    iss.read(reinterpret_cast<char*>(&dbus_data.session_id), sizeof(dbus_data.session_id));
    iss.read(reinterpret_cast<char*>(&dbus_data.msg_type), sizeof(dbus_data.msg_type));

    // 反序列化 topic 字符串
    int32_t topic_length;
    iss.read(reinterpret_cast<char*>(&topic_length), sizeof(topic_length));
    dbus_data.topic.resize(topic_length);
    iss.read(&dbus_data.topic[0], topic_length);

    // 反序列化 data 字符串
    int32_t data_length;
    iss.read(reinterpret_cast<char*>(&data_length), sizeof(data_length));
    dbus_data.data.resize(data_length);
    iss.read(&dbus_data.data[0], data_length);

    iss.read(reinterpret_cast<char*>(&dbus_data.send_time_stamp), sizeof(dbus_data.send_time_stamp));
    iss.read(reinterpret_cast<char*>(&dbus_data.recv_time_stamp), sizeof(dbus_data.recv_time_stamp));
    iss.read(reinterpret_cast<char*>(&dbus_data.time_cost), sizeof(dbus_data.time_cost));
    return dbus_data;
}

} /* namespace opendbus */