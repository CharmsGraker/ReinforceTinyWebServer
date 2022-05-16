#include "g_threadpool.h"
thread_local unsigned thread_pool::my_index = 0;
thread_local work_stealing_queue * thread_pool::local_queue = nullptr;