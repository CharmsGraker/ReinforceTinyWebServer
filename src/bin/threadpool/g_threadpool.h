//
// Created by nekonoyume on 2022/5/10.
//

#ifndef TINYWEB_G_THREADPOOL_H
#define TINYWEB_G_THREADPOOL_H

#include <atomic>
#include <vector>
#include <thread>
#include "../grakerThread/join_threads.h"
#include <deque>
#include <mutex>
#include <future>
#include "../concurrent/threadsafe_queue.h"
class join_threads;

class function_wrapper {
    struct impl_base {
        virtual void call() = 0;

        virtual ~impl_base() {};
    };

    template<class F>
    struct impl_type : impl_base {
        F f;

        impl_type(F &&f_) : f(std::move(f_)) {};

        void call() { f(); }
    };

    std::unique_ptr<impl_base> impl;
public:
    template<class F>
    function_wrapper(F &&f):impl(new impl_type<F>(std::move(f))) {};

    void operator()() {
        impl->call();
    }

    function_wrapper() = default;

    function_wrapper(function_wrapper &&other) : impl(std::move(other.impl)) {};

    function_wrapper &operator=(function_wrapper &&other) {
        impl = std::move(other.impl);
        return *this;
    }

    function_wrapper(const function_wrapper &) = delete;

    function_wrapper(function_wrapper &) = delete;

    function_wrapper &operator=(const function_wrapper &) = delete;
};



class work_stealing_queue {
    typedef function_wrapper data_type;
    std::deque<data_type> the_queue;
    mutable std::mutex the_mutex;
public:
    work_stealing_queue(){};
    work_stealing_queue(const work_stealing_queue& other)=delete;
    work_stealing_queue&operator=(const work_stealing_queue& other) =delete;
    void push(data_type data) {
        std::lock_guard<std::mutex> lock(the_mutex);
        the_queue.push_front(std::move(data));
    }
    bool empty() const {
        std::lock_guard<std::mutex> lock(the_mutex);
        return the_queue.empty();
    }
    bool try_pop(data_type& res) {
        std::lock_guard<std::mutex> lock(the_mutex);
        if(the_queue.empty()) {
            return false;
        }
        res = std::move(the_queue.front());
        the_queue.pop_front();
        return true;
    }
    bool try_steal(data_type& res) {
        std::lock_guard<std::mutex> lock(the_mutex);
        if(the_queue.empty()) {
            return false;
        }
        res =std::move(the_queue.back());
        the_queue.pop_back();
        return true;
    }
};

class thread_pool {
    typedef function_wrapper task_type;
private:
    std::atomic_bool done;
    std::vector<std::thread> threads;
    join_threads joiner;
    threadsafe_queue<task_type> pool_work_queue;

    std::vector<std::unique_ptr<work_stealing_queue>> queues;

    // work stealing
    static thread_local  work_stealing_queue * local_queue;
    static thread_local unsigned int my_index;
private:
    void
    worker_thread(unsigned my_index_) {

        my_index = my_index_;
        local_queue=queues[my_index].get();
        while (!done) {
            run_pending_task();
        }
    }
    bool
    pop_task_from_other_thread_queue(task_type& task) {
        for(int i=0;i<queues.size();++i) {
            unsigned const index = (my_index + i) % queues.size();
            if(queues[index]->try_steal(task)) {
                return true;
            }
        }
        return false;
    }
    bool
    pop_task_from_pool_queue(task_type& task) {
        pool_work_queue.try_pop(task);
    }
    bool
    pop_task_from_local_queue(task_type& task) {
        return local_queue && local_queue->try_pop(task);
    }
public:
    thread_pool() : done(false), joiner(threads) {
        unsigned const int thread_count = std::thread::hardware_concurrency();
        try {
            for (unsigned i = 0; i < thread_count; ++i) {
                queues.push_back(std::unique_ptr<work_stealing_queue>(new work_stealing_queue));
            }
            for (unsigned i = 0; i < thread_count; ++i) {
                threads.push_back(std::thread(&thread_pool::worker_thread, this,i));
            }
        } catch (...) {
            done = true;
            throw;
        }
    };

    ~thread_pool() {
        done = true;
    }

    template<class Function>
    std::future<typename std::result_of<Function()>::type>
    submit(Function f) {
        typedef typename std::result_of<Function()>::type result_type;
        std::packaged_task<result_type ()> task(f);
        std::future<result_type> res(task.get_future());
        if(local_queue) {
            local_queue->push(std::move(task));
        } else {
            pool_work_queue.push(std::move(task));
        }
        return res;
    }

    void
    run_pending_task() {
        task_type task;
        if(pop_task_from_local_queue(task) || pop_task_from_pool_queue(task) || pop_task_from_other_thread_queue(task)) {
            task();
        } else {
            std::this_thread::yield();
        }
    }
};

#endif //TINYWEB_G_THREADPOOL_H
