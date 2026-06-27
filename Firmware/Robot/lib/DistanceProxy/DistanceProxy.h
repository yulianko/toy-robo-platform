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

    void resetConfirmed() {
        _task->resetConfirmed();
    }

  private:
    DistanceTask* _task;
};
