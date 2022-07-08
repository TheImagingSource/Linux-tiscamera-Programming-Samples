
/////////////////////////////////////////////////////////////////
// This file is from  https://github.com/moya-lang/Event
//
#ifndef AllocatorH
#define AllocatorH

#include <mutex>
#include <condition_variable>
#include <chrono>

namespace Moya {

class Event
{
    public:

        Event(bool initial, bool manual) :
            state(initial), manual(manual)
        {
        }

        void change(bool manual)
        {
            std::unique_lock<std::mutex> lock(mutex);

            this->manual = manual;
        }

        void set()
        {
            std::unique_lock<std::mutex> lock(mutex);

            if (state)
                return;

            state = true;
            if (manual)
                condition.notify_all();
            else
                condition.notify_one();
        }

        void reset()
        {
            std::unique_lock<std::mutex> lock(mutex);

            state = false;
        }

        void wait()
        {
            std::unique_lock<std::mutex> lock(mutex);

            condition.wait(lock, [this] { return state; });

            if (!manual)
                state = false;
        }

        template<class Rep, class Period>
        void wait(const std::chrono::duration<Rep, Period> &timeout)
        {
            std::unique_lock<std::mutex> lock(mutex);

            if (!condition.wait_for(lock, timeout, [this] { return state; }))
                return;

            if (!manual)
                state = false;
        }

    private:

        std::mutex mutex;
        std::condition_variable condition;
        bool state, manual;
};

}

#endif