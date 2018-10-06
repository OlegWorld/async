#pragma once

#include <string>
#include <deque>
#include <iostream>
#include <fstream>

using command_name_t = std::string;

class CommandBulk {
    friend std::ostream& operator<<(std::ostream& os, const CommandBulk& b);
    friend std::ofstream& operator<<(std::ofstream& os, const CommandBulk& b);

public:
    CommandBulk();
    ~CommandBulk() = default;

    void push_command(command_name_t&& name);
    bool empty() const noexcept;
    void clear();
    bool recyclable() const noexcept;
    size_t size() const noexcept;

    CommandBulk(const CommandBulk&) = delete;
    CommandBulk& operator=(const CommandBulk&) = delete;

private:
    std::deque<command_name_t> m_command_queue;

    mutable bool m_printed;
    mutable bool m_logged;
};

std::ostream& operator<<(std::ostream& os, const CommandBulk& b);

std::ofstream& operator<<(std::ofstream& os, const CommandBulk& b);