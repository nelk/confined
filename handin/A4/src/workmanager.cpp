#include "workmanager.hpp"
#include <cmath>
#include <iostream>

void WorkManager::getWork(bool& done, int& start, int& end) {
  pthread_mutex_lock(&mutex);
  if (on_batch >= num_batches) {
    done = true;
  } else {
    done = false;
    start = on_batch * batch_size;
    end = std::min(start + batch_size, work_amount+1);
    std::cout << "Done " << std::floor((double) on_batch / num_batches * 100.0) << "%" << std::endl;
    on_batch++;

    std::cout << "Giving work [" << start << "," << end << ")" << std::endl;
  }
  pthread_mutex_unlock(&mutex);
}

