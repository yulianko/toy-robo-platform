#pragma once

#include "RobotContext.h"
#include "RobotEvent.h"

class IMode {
  public:
    virtual const char* name() const = 0;

    virtual void onEnter(RobotContext& ctx) = 0;
    virtual void onExit() = 0;
    virtual void onEvent(const RobotEvent& event) = 0;
};
