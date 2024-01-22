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

#include <any>
#include <set>
#include <map>
#include <utility>
#include <vector>
#include <cstdint>
#include <string>
#include <sstream>
#include <functional>

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

    SPDMQ_CODE_NO_DATA =  1,  // 无数据, 客户或者服务未收到数据, 或者发送的数据为空
#define SPDMQ_CODE_NO_DATA (static_cast<spdmq_code_t>(SPDMQ_CODE::SPDMQ_CODE_NO_DATA))

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

    SPDMQ_CODE_MODE_NOT_MATCH =  9,  // 模式不匹配
#define SPDMQ_CODE_MODE_NOT_MATCH (static_cast<spdmq_code_t>(SPDMQ_CODE::SPDMQ_CODE_MODE_NOT_MATCH))

//     SPDMQ_CODE_SEND_DATA_EMPTY =  10, // 发送数据为空, 无法发送
// #define SPDMQ_CODE_SEND_DATA_EMPTY (static_cast<spdmq_code_t>(SPDMQ_CODE::SPDMQ_CODE_SEND_DATA_EMPTY))

//     SPDMQ_CODE_FORBID_REPEAT_BIND_OR_CONNECT =  11, // 禁止重复bind或connect
// #define SPDMQ_CODE_FORBID_REPEAT_BIND_OR_CONNECT (static_cast<spdmq_code_t>(SPDMQ_CODE::SPDMQ_CODE_FORBID_REPEAT_BIND_OR_CONNECT))

//     SPDMQ_CODE_SEND_FAILED_NOT_CONNECTED_TARGET =  12, // 发送失败, 未连接到目标
// #define SPDMQ_CODE_SEND_FAILED_NOT_CONNECTED_TARGET (static_cast<spdmq_code_t>(SPDMQ_CODE::SPDMQ_CODE_SEND_FAILED_NOT_CONNECTED_TARGET))

    SPDMQ_CODE_CONNECT_TO_BROKEN =  13, // 连接中断
#define SPDMQ_CODE_CONNECT_TO_BROKEN (static_cast<spdmq_code_t>(SPDMQ_CODE::SPDMQ_CODE_CONNECT_TO_BROKEN))

//     SPDMQ_CODE_NO_REQUEST =  14, // 没有请求数据
// #define SPDMQ_CODE_NO_REQUEST (static_cast<spdmq_code_t>(SPDMQ_CODE::SPDMQ_CODE_NO_REQUEST))

    SPDMQ_CODE_RECV_CB_INTERCEPTED_DATA =  15, // The received callback has intercepted the data
#define SPDMQ_CODE_RECV_CB_INTERCEPTED_DATA (static_cast<spdmq_code_t>(SPDMQ_CODE::SPDMQ_CODE_RECV_CB_INTERCEPTED_DATA))

};

class spdmq;
using spdmq_t = spdmq;

using time_msec_t = int32_t;
using spdmq_config_t = std::any;

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

typedef enum class COMM_PROTOCOL_TYPE : uint8_t {
    UNKNOW = 0,
    TCP = 1,
    UDP = 2,  // TODO
} comm_protocol_type_t;

typedef enum class EVENT_MODE : uint8_t {
    EVENT_UNKNOW = 0, // unknow event
    EVENT_POLL_LT = 0, // epoll level trigger
    EVENT_POLL_ET = 1, // epoll edge trigger
} event_mode_t;

typedef class spdmq_ctx {
private:
    comm_mode_t _mode;                        // communication mode
    comm_method_t _method;                    // communication method
    comm_domain_t _domain;                    // communication domain
    comm_protocol_type_t _protocol_type;      // communication protocol type
    event_mode_t _event_mode;                 // event mode
    uint32_t _evt_num;                        // the number of single listening events in the server model, default to 100 events
    uint32_t _heartbeat;                      // heartbeat interval, default to 100 ms, minimum 10 ms, the timeout server will clearly connect
    uint32_t _reconnect_interval;             // reconnect interval
    uint32_t _queue_size;                     // the number of messages in the message queue, default to 1024 messages
    std::set<std::string> _topics;            // topics of PUB/SUB mode
    std::map<std::string, std::any>  _config; // configure map

public:
    /* variable set */
    spdmq_ctx& mode(comm_mode_t mode);
    spdmq_ctx& method(comm_method_t method);
    spdmq_ctx& domain(comm_domain_t domain);
    spdmq_ctx& protocol_type(comm_protocol_type_t protocol_type);
    spdmq_ctx& event_mode(event_mode_t event_mode);
    spdmq_ctx& evt_num(uint32_t evt_num);
    spdmq_ctx& heartbeat(uint32_t heartbeat);
    spdmq_ctx& reconnect_interval(uint32_t reconnect_interval);
    spdmq_ctx& queue_size(uint32_t queue_size);
    spdmq_ctx& topics(std::set<std::string> topics);
    template<typename T>
    spdmq_ctx& config(const std::string& param, const T& val) {
        _config[param] = val;
        return *this;
    }

    /* variable get */
    comm_mode_t mode();
    comm_method_t method();
    comm_domain_t domain();
    comm_protocol_type_t protocol_type();
    event_mode_t event_mode();
    uint32_t evt_num();
    uint32_t heartbeat();
    uint32_t reconnect_interval();
    uint32_t queue_size();
    std::set<std::string> topics();
    template<typename T>
    T config(const std::string& param) {
        return std::any_cast<T>(_config[param]);
    }

    bool has_config(const std::string& param) {
        return _config.find(param) != _config.end();
    }

public:
    spdmq_ctx() {
        _mode = COMM_MODE::SPDMQ_UNKNOW;
        _method = COMM_METHOD::SOCKET;
        _domain = COMM_DOMAIN::UNKNOW;
        _protocol_type = COMM_PROTOCOL_TYPE::TCP;
        _event_mode = EVENT_MODE::EVENT_POLL_ET;
        _evt_num = 100;
        _heartbeat = 100;
        _reconnect_interval = 500;
        _queue_size = 1024;
        _topics.clear();
    }

} spdmq_ctx_t;

enum class DBUS_MESSAGE_TYPE : uint8_t {
    DBUS_HEARTBEAT = 1, // heartbeat message
    DBUS_TOPIC = 2,     // topic message
};

typedef struct spdmq_msg {
    spdmq_msg() {}

    spdmq_msg(int32_t session_id) : session_id(session_id) {}

    spdmq_msg(int32_t session_id, const std::string& topic, const std::vector<uint8_t>& payload)
        : session_id(session_id), topic(topic), payload(payload) {}
    spdmq_msg(int32_t session_id, const std::vector<uint8_t>& payload)
        : session_id(session_id), payload(payload) {}

    spdmq_msg(int32_t session_id, std::string&& topic, std::vector<uint8_t>&& payload)
        : session_id(session_id), 
          topic(std::forward<decltype(topic)>(topic)), 
          payload(std::forward<decltype(payload)>(payload)) {}
    spdmq_msg(int32_t session_id, std::vector<uint8_t>&& payload)
        : session_id(session_id), 
          payload(std::forward<decltype(payload)>(payload)) {}

    int32_t session_id = {};           // session id
    std::string topic = {};            // topic of DBUS_PUB/DBUS_SUB  mode
    std::vector<uint8_t> payload = {}; // communication payload
    int64_t time_cost = {};            // message sending and receiving time, unit microseconds

    std::string to_string() {
        std::stringstream ss;
        ss << "\nsession_id: " << session_id 
           << "\ntopic: " << topic 
           << "\npayload_size: " << payload.size()
           << "\npayload: " << payload.data()
           << "\ntime_cost: " << time_cost
           << std::endl;
        return ss.str();
    }
} spdmq_msg_t;

using spdmq_callback_t = std::function<void(spdmq_msg_t&)>;

} /* namespace opendbus */