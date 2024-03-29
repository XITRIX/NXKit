/*
    Copyright 2020 natinusala

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

#pragma once

#include <algorithm>
#include <functional>
#include <list>

namespace NXKit {

// Simple observer pattern implementation
//
// Usage:
// 1. typedef your event type
// 2. create as many events as you want using that type
// 3. call subscribe on the events with your observers
// 4. call fire when you want to fire the events
//    it wil return true if at least one subscriber exists
//    for that event
template <typename... Ts>
class NotificationEvent
{
public:
    typedef std::function<void(Ts...)> Callback;
    typedef std::list<Callback> CallbacksList;
    typedef typename CallbacksList::iterator Subscription;

    Subscription subscribe(Callback cb);
    void unsubscribe(Subscription subscription);
    bool fire(Ts... args);

private:
    CallbacksList callbacks;
};

template <typename... Ts>
typename NotificationEvent<Ts...>::Subscription NotificationEvent<Ts...>::subscribe(NotificationEvent<Ts...>::Callback cb)
{
    this->callbacks.push_back(cb);
    return --this->callbacks.end();
}

template <typename... Ts>
void NotificationEvent<Ts...>::unsubscribe(NotificationEvent<Ts...>::Subscription subscription)
{
    if (this->callbacks.size() > 0)
        this->callbacks.erase(subscription);
}

template <typename... Ts>
bool NotificationEvent<Ts...>::fire(Ts... args)
{
    for (Callback cb : this->callbacks)
        cb(args...);

    return !this->callbacks.empty();
}

}
