#include "workmanager.hpp"
#include <cmath>
#include <iostream>

#define WORK_MANAGER_LOG

void WorkManager::getWork(bool& done, int& start, int& end) {
  pthread_mutex_lock(&work_mutex);
  if (on_batch >= num_batches) {
    done = true;
  } else {
    done = false;
    start = on_batch * batch_size;
    end = std::min(start + batch_size, work_amount+1);
#ifdef WORK_MANAGER_LOG
    std::cout << "Done " << std::floor((double) on_batch / num_batches * 100.0) << "%  "
      << "Giving work [" << start << "," << end << ")" << std::endl;
#endif
    on_batch++;
  }
  pthread_mutex_unlock(&work_mutex);
}


void WorkManager::reportStats(const RayTraceStats& stats) {
  pthread_mutex_lock(&stats_mutex);
  this->stats.merge(stats);
  pthread_mutex_unlock(&stats_mutex);
}

