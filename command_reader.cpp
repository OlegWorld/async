#include "command_reader.h"

CommandReader::CommandReader(std::list<CommandBulk>& data,
                             size_t commandPackSize)
:   m_data(data),
    m_current_state(std::make_unique<NormalState>(commandPackSize, m_stream)),
    m_other_state(std::make_unique<BracedState>(m_stream)),
    m_line_counter(0), m_command_counter(0), m_block_counter(0)
{ }

void CommandReader::scan_input(const std::string& data) {
    m_stream.clear();
    m_stream.str(data);
    m_current_state->read_commands(this);
}

void CommandReader::subscribe(AbstractObserver* obs) {
    m_observers.push_back(obs);
}

void CommandReader::push_bulk(CommandBulk* b) {
    if (b->empty()) {
        b->clear();
        return;
    }

    m_command_counter += b->size();
    ++m_block_counter;

    for (auto& obs : m_observers)
        obs->add(b);

    recycle_data();
}

//void CommandReader::stop(CommandBulk* b) {
//    push_bulk(b);
//    for (auto& obs : m_observers)
//        obs->stop();
//}

void CommandReader::switch_state() {
    std::swap(m_current_state, m_other_state);
    m_current_state->read_commands(this);
}

CommandBulk* CommandReader::new_bulk() {
    m_data.emplace_back();
    return &m_data.back();
}

void CommandReader::recycle_data() {
    auto it = m_data.begin();
    while (it != m_data.end()) {
        if (it->recyclable()) {
            it = m_data.erase(it);
        }
        ++it;
    }
}

NormalState::NormalState(size_t commandPackSize, std::istream& input)
:   m_command_pack_size(commandPackSize),
    m_input(input),
    m_current(nullptr),
    m_current_size(0)
{ }

void NormalState::open_brace(CommandReader* reader) {
    push_current_bulk(reader);
    reader->switch_state();
}

void NormalState::close_brace(CommandReader*) { }

void NormalState::read_commands(CommandReader* reader) {
    command_name_t name;
    while (std::getline(m_input, name)) {
        if (!m_current) {
            m_current = reader->new_bulk();
        }

        if (name == "{") {
            open_brace(reader);
            break;
        }

        if (name == "}") {
            continue;
        }

        m_current->push_command(std::move(name));

        if (++m_current_size == m_command_pack_size) {
            push_current_bulk(reader);
        }
    }
}

void NormalState::push_current_bulk(CommandReader* reader) {
    reader->push_bulk(m_current);
    m_current = nullptr;
    m_current_size = 0;
}

BracedState::BracedState(std::istream& input)
:   m_brace_counter(0),
    m_input(input),
    m_current(nullptr)
{ }

void BracedState::open_brace(CommandReader*) { }

void BracedState::close_brace(CommandReader* reader) {
    reader->push_bulk(m_current);
    m_current = nullptr;
    reader->switch_state();
}

void BracedState::read_commands(CommandReader* reader) {
    m_brace_counter++;

    if (!m_current) {
        m_current = reader->new_bulk();
    }

    command_name_t name;
    while(std::getline(m_input, name)) {
//        if (name.empty()) {
//            m_current->clear();
//            reader->stop(m_current);
//            return;
//        }

        if (name == "{") {
            m_brace_counter++;
            continue;
        }

        if (name == "}") {
            if (!--m_brace_counter) {
                close_brace(reader);
                break;
            }

            continue;
        }

        m_current->push_command(std::move(name));
    }
}

