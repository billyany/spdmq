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
#include "spdmq_internal_def.h"

namespace speed::mq {

class spdmq_session {
private:
    fd_t session_id_;

public:
    spdmq_session(fd_t session_id);
    ~spdmq_session();
    fd_t session_id();
    void close_session();
};

} /* namespace speed::mq */
