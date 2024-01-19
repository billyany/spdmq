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

#include <cstdint>
#include <string>
#include <cstring>
#include <stdexcept>
#include <vector>
#include "spdmq_func.hpp"
#include "spdmq_def.h"

#define SPDMQ_UNUSED(x) (void)(x)

namespace speed::mq {

using fd_t = int32_t;

using msg_t = std::string;

typedef struct SPDMQ_URL_PARSE {
    bool parse_result;
    std::string ip;
    uint16_t port;
    std::string address;
} spdmq_url_parse_t;

typedef enum class SOCKET_MODE : uint8_t {
    TCP = 0,
    UDP = 1,
    UDS = 2,
} socket_mode_t;

typedef enum class MESSAGE_TYPE : uint8_t {
    DATA = 1,      // data message
    TOPIC = 2,     // topic message
    HEARTBEAT = 3, // heartbeat message
} message_type_t;

typedef struct comm_header {
    int32_t comm_msg_len; // session id
} comm_header_t;

typedef struct comm_msg {
    comm_msg() {}

    comm_msg(int32_t session_id) : session_id(session_id) {}

    comm_msg(int32_t session_id, const std::string& topic, const std::vector<uint8_t>& payload)
        : session_id(session_id), topic(topic), payload(payload) {}
    comm_msg(int32_t session_id, const std::vector<uint8_t>& payload)
        : session_id(session_id), payload(payload) {}

    comm_msg(int32_t session_id, std::string&& topic, std::vector<uint8_t>&& payload)
        : session_id(session_id), 
          topic(std::forward<decltype(topic)>(topic)), 
          payload(std::forward<decltype(payload)>(payload)) {}
    comm_msg(int32_t session_id, std::vector<uint8_t>&& payload)
        : session_id(session_id), 
          payload(std::forward<decltype(payload)>(payload)) {}

    int32_t session_id = {};           // session id
    message_type_t msg_type = {};      // used to distinguish different message types
    std::string topic = {};            // topic of DBUS_PUB/DBUS_SUB  mode
    std::vector<uint8_t> payload = {}; // communication payload
    int64_t send_time_stamp = {};      // send UTC time, unit microseconds

    std::size_t size() const {
        return sizeof(session_id) + sizeof(msg_type) +
               sizeof(int32_t) + topic.size() +
               sizeof(int32_t) + payload.size() +
               sizeof(send_time_stamp);
    };
} comm_msg_t;

// Serialization function for comm_msg_t
inline void serialize_comm_msg_t(const comm_msg_t& msg, std::vector<uint8_t>& buffer) {
    // Reserve the buffer capacity in advance (optional, for performance optimization)
    buffer.clear();
    buffer.reserve(msg.size());

    // Helper lambda to write data into the buffer
    auto write_to_buffer = [&buffer](const void* data, size_t size) {
        const uint8_t* bytes = reinterpret_cast<const uint8_t*>(data);
        buffer.insert(buffer.end(), bytes, bytes + size);
    };

    // Serialize data directly into the buffer
    write_to_buffer(&msg.session_id, sizeof(msg.session_id));
    write_to_buffer(&msg.msg_type, sizeof(msg.msg_type));

    int32_t topic_length = msg.topic.size();
    write_to_buffer(&topic_length, sizeof(topic_length));
    write_to_buffer(msg.topic.data(), msg.topic.size());

    int32_t payload_length = msg.payload.size();
    write_to_buffer(&payload_length, sizeof(payload_length));
    write_to_buffer(msg.payload.data(), msg.payload.size());

    write_to_buffer(&msg.send_time_stamp, sizeof(msg.send_time_stamp));
}


// Deserialization function for comm_msg_t
inline void deserialize_comm_msg_t(std::vector<uint8_t>& buffer, comm_msg_t& msg) {

    const uint8_t* ptr = buffer.data();

    // Helper lambda to read data from the buffer
    auto read_from_buffer = [&ptr](void* data, size_t size) {
        std::memcpy(data, ptr, size);
        ptr += size;
    };

    // Deserialize data directly from the buffer
    read_from_buffer(&msg.session_id, sizeof(msg.session_id));
    read_from_buffer(&msg.msg_type, sizeof(msg.msg_type));

    int32_t topic_length;
    read_from_buffer(&topic_length, sizeof(topic_length));
    msg.topic.resize(topic_length);
    read_from_buffer(&msg.topic[0], topic_length);

    int32_t payload_length;
    read_from_buffer(&payload_length, sizeof(payload_length));
    msg.payload.resize(payload_length);
    read_from_buffer(&msg.payload[0], payload_length);

    read_from_buffer(&msg.send_time_stamp, sizeof(msg.send_time_stamp));
}

inline void spdmq_msg_to_comm_msg(spdmq_msg_t& spdmq_msg, comm_msg_t& comm_msg) {
    comm_msg.session_id = spdmq_msg.session_id ;
    comm_msg.msg_type = MESSAGE_TYPE::DATA;
    comm_msg.topic = std::move(spdmq_msg.topic);
    comm_msg.payload = std::move(spdmq_msg.payload);
    comm_msg.send_time_stamp = now_usecs_timestamp();
}

inline void comm_msg_to_spdmq_msg(comm_msg_t& comm_msg, spdmq_msg_t& spdmq_msg) {
    spdmq_msg.session_id = comm_msg.session_id;
    spdmq_msg.topic = std::move(comm_msg.topic);
    spdmq_msg.payload = std::move(comm_msg.payload);
    spdmq_msg.time_cost = now_usecs_timestamp() - comm_msg.send_time_stamp;
}

} /* namespace speed::mq */