#pragma once

#include "DistanceScanProfile.h"
#include "DistanceTask.h"

class DistanceProxy {
  public:
    DistanceProxy(DistanceTask* task) : _task(task) {
    }

    void setProfile(const DistanceScanProfile& profile) {
        _task->setProfile(profile);
    }

  private:
    DistanceTask* _task;
};
