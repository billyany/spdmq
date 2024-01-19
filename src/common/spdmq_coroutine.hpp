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

namespace speed::mq {

#include <iostream>
#include <functional>
#include <vector>

// 协程状态
typedef enum class COROUTINE_STATE {
    READY,
    RUNNING,
    SUSPENDED,
    FINISHED
} coroutine_state_t;

// 协程类
class coroutine {
public:
    std::function<void()> func; // 协程的函数
    coroutine_state_t state; // 协程的状态

    coroutine(std::function<void()> f) : func(f), state(COROUTINE_STATE::READY) {}

    // 运行或恢复协程
    void resume() {
        if (state == COROUTINE_STATE::FINISHED) {
            return;
        }
        state = COROUTINE_STATE::RUNNING;
        func();
        if (state != COROUTINE_STATE::SUSPENDED) {
            state = COROUTINE_STATE::FINISHED;
        }
    }

    // 挂起协程
    void suspend() {
        if (state == COROUTINE_STATE::RUNNING) {
            state = COROUTINE_STATE::SUSPENDED;
        }
    }

    bool is_finished() const {
        return state == COROUTINE_STATE::FINISHED;
    }
};

// 协程调度器
class scheduler {
public:
    std::vector<coroutine> coroutines;

    void add_coroutine(coroutine&& co) {
        coroutines.push_back(std::move(co));
    }

    void run() {
        while (!coroutines.empty()) {
            for (auto it = coroutines.begin(); it != coroutines.end(); ) {
                it->resume();
                if (it->is_finished()) {
                    it = coroutines.erase(it);
                } else {
                    ++it;
                }
            }
        }
    }
};

} /*  */
