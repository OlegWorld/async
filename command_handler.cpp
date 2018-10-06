#include "command_handler.h"
#include <string>

CommandHandler::CommandHandler(size_t commandPackSize,
                               size_t loggers_num)
:   m_log(loggers_num),
    m_reader(m_data, commandPackSize)
{
    m_reader.subscribe(&m_processor);
    m_reader.subscribe(&m_log);

    while(!m_processor.ready());
    while(!m_log.ready());

    m_reader.scan_input();
}

CommandHandler::~CommandHandler() {
    m_processor.finalize();
    m_log.finalize();
}

void CommandHandler::read(const char* data, size_t size) {
    m_reader.scan_input(std::string(data, size));
}
