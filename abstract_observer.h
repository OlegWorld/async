#pragma once

#include "command_bulk.h"

class AbstractObserver {
public:
    virtual ~AbstractObserver() = default;
    virtual void add(CommandBulk* name) = 0;
    virtual void stop() = 0;
};
