#pragma once

#include <thread>

class thread_guard {
public:
    explicit thread_guard(std::thread& t)
    :   m_t(t)
    { }

    ~thread_guard() {
        if(m_t.joinable()) {
            m_t.join();
        }
    }

    thread_guard(const thread_guard&) = delete;
    thread_guard& operator=(const thread_guard&) = delete;

private:
    std::thread& m_t;
};
