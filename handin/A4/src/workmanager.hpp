
#ifndef WORKMANAGER_H
#define WORKMANAGER_H

#include <pthread.h>
#include "raytracer.hpp"

class WorkManager {
public:
  WorkManager(int work_amount, int num_batches): work_amount(work_amount), num_batches(num_batches), on_batch(0), batch_size(work_amount/num_batches)  {
    pthread_mutex_init(&work_mutex, NULL);
    pthread_mutex_init(&stats_mutex, NULL);
  }

  ~WorkManager() {
    pthread_mutex_destroy(&work_mutex);
    pthread_mutex_destroy(&stats_mutex);
  }

  // Thread-safe method to get next batch of work.
  void getWork(bool& done, int& start, int& end);

  // Thread-safe method to report stats.
  void reportStats(const RayTraceStats& stats);

  RayTraceStats getStats() const {
    return stats;
  }

private:
  int work_amount;
  int num_batches;
  int on_batch;
  int batch_size;
  RayTraceStats stats;
  pthread_mutex_t work_mutex;
  pthread_mutex_t stats_mutex;
};


#endif

