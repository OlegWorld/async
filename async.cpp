#include "async.h"
#include "command_handler.h"

namespace async {

handle_t connect(std::size_t bulk) {
    const size_t loggers_number = 2;
    return new CommandHandler(bulk, loggers_number);
}

void receive(handle_t handle, const char* data, std::size_t size) {
    handle->read(data, size);
}

void disconnect(handle_t handle) {
    delete handle;
}
}
