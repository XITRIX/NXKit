/*
    Copyright 2021 natinusala

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

#include <Core/Utils/Animation/Core/Time.hpp>

namespace NXKit {

void Ticking::updateTickings()
{
    // Update time
    static Time previousTime = 0;

    Time currentTime = getCPUTimeUsec() / 1000;
    Time delta       = previousTime == 0 ? 0 : currentTime - previousTime;

    previousTime = currentTime;

    for (int i = 0; i < Ticking::runningTickings.size(); i++)
    {
        Ticking* ticking = Ticking::runningTickings[i];
        bool run = ticking->onUpdate(delta);

        ticking->tickCallback();

        if (!run)
            ticking->stop(true); // will remove the ticking from Ticking::runningTickings
    }
}

void Ticking::start()
{
    if (this->running)
        return;

    Ticking::runningTickings.push_back(this);

    this->running = true;

    this->onStart();
}

void Ticking::stop()
{
    this->stop(false);
}

void Ticking::stop(bool finished)
{
    auto it = std::remove(Ticking::runningTickings.begin(), Ticking::runningTickings.end(), this);
    if (Ticking::runningTickings.end() != it)
        Ticking::runningTickings.erase(it);

    if (!this->running)
        return;

    this->running = false;

    this->onStop();

    this->endCallback(finished);
}

void Ticking::setEndCallback(TickingEndCallback endCallback)
{
    this->endCallback = endCallback;
}

void Ticking::setTickCallback(TickingTickCallback tickCallback)
{
    this->tickCallback = tickCallback;
}

bool Ticking::isRunning()
{
    return this->running;
}

Ticking::~Ticking()
{
    this->stop();
}

void FiniteTicking::rewind()
{
    this->onRewind();
}

void FiniteTicking::reset()
{
    this->stop();
    this->onReset();
}

}
