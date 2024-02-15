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

#include <map>
#include <vector>
#include <cstdint>
#include "spdmq_uncopyable.h"



namespace speed::mq {

class event_loop;
class socket_channel;

///
/// Base class for IO Multiplexing
///
/// This class doesn't own the Channel objects.
typedef std::vector<socket_channel*> channel_list_t;
typedef std::map<int32_t, socket_channel*> channel_map_t;

class base_poller : spdmq_uncopyable {
protected:
    channel_map_t channels_;

private:
    event_loop& owner_loop_;

public:
    base_poller(event_loop& loop);
    virtual ~base_poller();

    /// Polls the I/O events.
    /// Must be called in the loop thread.
    virtual void poll(int32_t timeout_ms, channel_list_t* activec_hannels) = 0;

    /// Changes the interested I/O events.
    /// Must be called in the loop thread.
    virtual void update_channel(socket_channel* channel) = 0;

    /// Remove the socket_channel, when it destructs.
    /// Must be called in the loop thread.
    virtual void remove_channel(socket_channel* channel) = 0;

    virtual bool has_channel(socket_channel* channel) const;

    void assert_in_loop_thread() const;

    static base_poller* new_poller(event_loop& loop);
};

}  /* namespace speed::mq */
