#include "command_handler.h"

CommandHandler::CommandHandler(size_t commandPackSize,
                               size_t loggers_num)
:   m_log(loggers_num),
    m_reader(m_data, commandPackSize)
{
    m_reader.subscribe(&m_processor);
    m_reader.subscribe(&m_log);
}

CommandHandler::~CommandHandler() {
    m_processor.stop();
    m_log.stop();
    m_processor.finalize();
    m_log.finalize();
}

void CommandHandler::read(const char* data, size_t size) {
    m_reader.scan_input(std::string(data, size));
}

void CommandHandler::read(const std::string& data) {
    m_reader.scan_input(data);
}
