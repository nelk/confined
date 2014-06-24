
#ifndef WORKMANAGER_H
#define WORKMANAGER_H

#include <pthread.h>

class WorkManager {
public:
  WorkManager(int work_amount, int num_batches): work_amount(work_amount), num_batches(num_batches), on_batch(0), batch_size(work_amount/num_batches) {
    pthread_mutex_init(&mutex, NULL);
  }

  ~WorkManager() {
    pthread_mutex_destroy(&mutex);
  }

  // Thread-safe method to get next batch of work.
  void getWork(bool& done, int& start, int& end);

private:
  int work_amount;
  int num_batches;
  int on_batch;
  int batch_size;
  pthread_mutex_t mutex;
};


#endif

