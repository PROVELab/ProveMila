#include "pecan.hpp"

PScheduler::PScheduler(){
}

PCAN_ERR PScheduler::scheduleTask(PTask t){
    if (ctr >= MAX_TASK_COUNT){
        return NOSPACE;
    }
    this->tasks[ctr] = t;
    ctr++;
    return SUCCESS;
}