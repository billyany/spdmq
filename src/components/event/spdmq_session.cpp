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

#include "spdmq_session.h"

namespace speed::mq {

spdmq_session::spdmq_session(fd_t session_id) : session_id_(session_id) {}

spdmq_session::~spdmq_session() {
    close_session();
}

fd_t spdmq_session::session_id() {
    return session_id_;
}

void spdmq_session::close_session() {
    if (session_id_ > 3) {
        close(session_id_);
    }
    session_id_ = -1;
}

} /* namespace speed::mq */
