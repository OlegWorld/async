#include "command_bulk.h"

CommandBulk::CommandBulk()
:   m_printed(false),
    m_logged(false)
{ }

void CommandBulk::push_command(command_name_t&& name) {
    m_command_queue.push_back(std::move(name));
}

bool CommandBulk::empty() const noexcept {
    return m_command_queue.empty();
}

void CommandBulk::clear() {
    m_command_queue.clear();
    m_printed = true;
    m_logged = true;
}

bool CommandBulk::recyclable() const noexcept {
    return m_printed && m_logged;
}

size_t CommandBulk::size() const noexcept {
    return m_command_queue.size();
}

std::ostream& operator<<(std::ostream& os, const CommandBulk& b) {
    if (!b.m_command_queue.empty()) {
        os << "bulk: ";
        for (const auto& command : b.m_command_queue) {
            os << command << (&b.m_command_queue.back() == &command ? "" : ", ");
        }

        os << std::endl;
    }

    b.m_printed = true;
    return os;
}

std::ofstream& operator<<(std::ofstream& os, const CommandBulk& b) {
    if (!b.m_command_queue.empty()) {
        for (const auto& command : b.m_command_queue) {
            os << command << std::endl;
        }
    }

    b.m_logged = true;
    return os;
}


