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

#include <unistd.h>
#include "base_poller.h"

struct epoll_event;

namespace speed::mq {

class epoll_poller : public base_poller {
private:
    int epollfd_;
    static const int kInitEventListSize = 16;
    typedef std::vector<struct epoll_event> event_list_t;
    event_list_t events_;

public:
    epoll_poller(event_loop& loop);
    ~epoll_poller() override;

    void poll(int32_t timeout_ms, channel_list_t* activec_channels) override;
    void update_channel(socket_channel* channel) override;
    void remove_channel(socket_channel* channel) override;

private:
    void fill_active_channels(int num_events, channel_list_t* activec_channels) const;
    void update(int operation, socket_channel* channel);
};

} /* opendbus*/