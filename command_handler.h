#pragma once

#include <sstream>
#include <list>
#include "command_bulk.h"
#include "command_reader.h"
#include "command_processors.h"

class CommandHandler {
public:
    explicit CommandHandler(size_t commandPackSize,
                            size_t loggers_num);

    ~CommandHandler();

    void read(const char* data, size_t size);

private:
    std::list<CommandBulk>  m_data;
    CommandProcessor        m_processor;
    CommandMultipleLog      m_log;
    CommandReader           m_reader;
};

