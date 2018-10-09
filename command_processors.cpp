#include <sstream>
#include <experimental/filesystem>
#include "command_processors.h"

CommandProcessor::CommandProcessor()
:   m_loaded(false),
    m_stop_flag(false),
    m_thread(&CommandProcessor::run, this),
    m_guard(m_thread)
{ }

bool CommandProcessor::ready() const noexcept {
    return m_loaded;
}

void CommandProcessor::run() {
    m_loaded = true;

    while(true) {
        std::unique_lock<std::mutex> lck(m_queue_mutex);

        if (m_stop_flag) {
            break;
        }

        m_cv.wait(lck);
        while (!m_commands.empty()) {
            CommandBulk* b = m_commands.front();
            std::cout << *b;
            m_commands.pop();
        }
    }
}

void CommandProcessor::add(CommandBulk* b) {
    std::unique_lock<std::mutex> lck(m_queue_mutex);
    m_commands.push(b);
    lck.unlock();
    m_cv.notify_one();
}

void CommandProcessor::stop() {
    std::unique_lock<std::mutex> lck(m_queue_mutex);
    m_stop_flag = true;
    lck.unlock();
    m_cv.notify_one();
}

void CommandProcessor::finalize() {
    if (m_thread.joinable())
        m_thread.join();
}

////////////////////////////////////////////////////////

CommandMultipleLog::LogWriter::LogWriter(CommandMultipleLog& parent)
:   m_parent(parent),
    m_loaded(false),
    m_stop_flag(false),
    m_thread(&LogWriter::run, this),
    m_guard(m_thread)
{ }

bool CommandMultipleLog::LogWriter::ready() const noexcept {
    return m_loaded;
}

void CommandMultipleLog::LogWriter::run() {
    m_loaded = true;

    while(true) {
        std::unique_lock<std::mutex> lck(m_parent.m_queue_mutex);

        if (m_stop_flag) {
            break;
        }

        m_parent.m_cv.wait(lck);
        create_log_file();
    }
}

void CommandMultipleLog::LogWriter::stop() {
    m_stop_flag = true;
}

void CommandMultipleLog::LogWriter::finalize() {
    if (m_thread.joinable())
        m_thread.join();
}

void CommandMultipleLog::LogWriter::create_log_file() {
    using std::to_string;
    using std::chrono::duration_cast;
    using std::chrono::seconds;

    while (!m_parent.m_commands.empty()) {
        CommandBulk* b = m_parent.m_commands.front();
        auto tp = std::chrono::system_clock::now();
        std::ofstream of("bulk_" +
                         to_string(duration_cast<seconds>(tp.time_since_epoch()).count()) + '_' +
                         to_string(m_parent.m_counter.fetch_add(1)) +
                         ".log", std::ios_base::out);
        of << *b;
        of.close();

        m_parent.m_commands.pop();
    }
}

////////////////////////////////////////////////////////

CommandMultipleLog::CommandMultipleLog(size_t num)
:   m_counter(0)
{
    namespace fs = std::experimental::filesystem;

    fs::create_directory("log");
    fs::current_path("log");

    for (size_t i = 0; i < num; i++) {
        m_logs.emplace_back(*this);
    }
}

bool CommandMultipleLog::ready() const noexcept {
    return std::all_of(m_logs.begin(), m_logs.end(), [](const LogWriter& log){ return log. ready(); });
}

void CommandMultipleLog::add(CommandBulk* b) {
    std::unique_lock<std::mutex> lck(m_queue_mutex);
    m_commands.push(b);
    lck.unlock();
    m_cv.notify_one();
}

void CommandMultipleLog::stop() {
    std::unique_lock<std::mutex> lck(m_queue_mutex);
    for (auto& log : m_logs) {
        log.stop();
    }
    lck.unlock();

    m_cv.notify_all();
}

void CommandMultipleLog::finalize() {
    for (auto& log : m_logs) {
        log.finalize();
    }
}

size_t CommandMultipleLog::loggers() const noexcept {
    return m_logs.size();
}
