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

/**
 * @brief This is a class that prevents copying. 
 *        If you inherit it, you will no longer need to write 
 *        functions that prohibit copying yourself
 *
 */

namespace speed::mq {

class spdmq_uncopyable {
protected:
    spdmq_uncopyable() {}
    ~spdmq_uncopyable() {}

private:
    spdmq_uncopyable(const spdmq_uncopyable&) = delete;
    spdmq_uncopyable& operator=(const spdmq_uncopyable&) = delete;
    spdmq_uncopyable(spdmq_uncopyable&&) = delete;
    spdmq_uncopyable& operator=(spdmq_uncopyable&&) = delete;

};

}  /* namespace speed::mq */
