#include "async.h"

namespace async {

handle_t connect(std::size_t bulk) {
    return new CommandHandler(bulk, 2);
}

void receive(handle_t handle, const char* data, std::size_t size) {
    handle->read(data, size);
}

void disconnect(handle_t handle) {
    delete handle;
}
}
