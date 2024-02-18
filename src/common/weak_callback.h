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

#include <functional>
#include <memory>

namespace speed::mq {

template<typename T, typename... ARGS>
class weak_callback {
private:
    std::weak_ptr<T> object_;
    std::function<void (T*, ARGS...)> function_;

public:
    weak_callback(const std::weak_ptr<T>& object, const std::function<void (T*, ARGS...)>& function)
        : object_(object), function_(function) {
    }

    void operator () (ARGS&&... args) const {
        std::shared_ptr<T> ptr(object_.lock());
        if (ptr) {
            function_(ptr.get(), std::forward<ARGS>(args)...);
        }
    }
};

template<typename T, typename... ARGS>
weak_callback<T, ARGS...> weak_bind(const std::shared_ptr<T>& object, void (T::*function) (ARGS...)) {
    return weak_callback<T, ARGS...> (object, function);
}

template<typename T, typename... ARGS>
weak_callback<T, ARGS...> weak_bind(const std::shared_ptr<T>& object, void (T::*function) (ARGS...) const) {
    return weak_callback<T, ARGS...> (object, function);
}

}  /* namespace speed::mq */
