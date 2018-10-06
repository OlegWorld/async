#pragma once

#include <iostream>
#include <fstream>
#include <queue>
#include <chrono>
#include <thread>
#include <atomic>
#include <deque>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <algorithm>
#include "abstract_observer.h"
#include "thread_utility.h"

class CommandProcessor : public AbstractObserver {
public:
    CommandProcessor();
    ~CommandProcessor() override = default;

    bool ready() const noexcept;

    void run();

    void add(CommandBulk* b) override;

    void stop() override;

    void finalize();

    size_t commands() const noexcept;
    size_t blocks() const noexcept;

private:
    void process_bulk(CommandBulk* b);

private:
    std::atomic_bool m_loaded;

    std::queue<CommandBulk*> m_commands;
    std::mutex m_queue_mutex;
    std::condition_variable m_cv;
    bool m_stop_flag;
    std::thread m_thread;
    thread_guard m_guard;

    size_t m_block_counter;
    size_t m_command_counter;
};

class CommandMultipleLog : public AbstractObserver {
    class LogWriter {
    public:
        LogWriter(CommandMultipleLog& parent);
        ~LogWriter() = default;

        bool ready() const noexcept;

        void run();

        void stop();

        void finalize();

        size_t commands() const noexcept;
        size_t blocks() const noexcept;

    private:
        void create_log_file();

    private:
        CommandMultipleLog& m_parent;

        std::atomic_bool m_loaded;

        bool m_stop_flag;
        std::thread m_thread;
        thread_guard m_guard;

        size_t m_output_block_counter;
        size_t m_command_counter;
    };

public:
    explicit CommandMultipleLog(size_t num);
    ~CommandMultipleLog() override = default;

    bool ready() const noexcept;

    void add(CommandBulk* b) override;

    void stop() override;

    void finalize();

    size_t loggers() const noexcept;

    size_t commands(size_t index) const noexcept;
    size_t out_blocks(size_t index) const noexcept;
    size_t in_blocks() const noexcept;

private:
    std::queue<CommandBulk*> m_commands;
    std::mutex m_queue_mutex;
    std::condition_variable m_cv;
    std::atomic_size_t m_counter;
    std::deque<LogWriter> m_logs;

    size_t m_input_block_counter;
};
