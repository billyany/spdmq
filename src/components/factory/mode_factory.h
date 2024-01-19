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

#include "spdmq_mode.h"
namespace speed::mq {

class mode_factory {
public:
    static mode_factory* instance();
    std::shared_ptr<spdmq_mode> create_mode(spdmq_ctx_t& ctx);

protected:
    mode_factory() {}
    ~mode_factory() {}
};

} // namespace hh_v2x_ipc
