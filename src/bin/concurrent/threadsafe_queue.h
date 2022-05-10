//
// Created by nekonoyume on 2022/5/5.
//

#ifndef TINYWEB_THREADSAFE_QUEUE_H
#define TINYWEB_THREADSAFE_QUEUE_H

#include <mutex>
#include <queue>
#include <memory>
#include <condition_variable>

template<class T>
class threadsafe_queue {
    struct node {
        std::shared_ptr<T> data;
        std::unique_ptr<node> next;
    };
public:
    threadsafe_queue() : head(new node), tail(head.get()) {}

    threadsafe_queue(const threadsafe_queue &other) = delete;

    threadsafe_queue &operator=(const threadsafe_queue &other) = delete;

    void wait_and_pop(T &value);

    bool try_pop(T &value);

    std::shared_ptr<T> wait_and_pop();

    std::shared_ptr<T> try_pop();

    void push(T&& new_value);

    bool empty() {
        std::lock_guard<std::mutex> head_lock(head_mutex);
        return (head.get() == get_tail());
    };

private:
    node *get_tail() {
        std::lock_guard<std::mutex> tail_lock(tail_mutex);
        return tail;
    }

    std::unique_ptr<node> pop_head() {
        std::unique_ptr<node> old_head = std::move(head);
        head = std::move(old_head->next);
        return old_head;
    }

    std::unique_lock<std::mutex> wait_for_data() {
        std::unique_lock<std::mutex> head_lock(head_mutex);
        data_cond.wait(head_lock, [&]() {
            return head.get() != get_tail();
        });
        return head_lock;
    }

    std::unique_ptr<node> wait_pop_head() {
        std::unique_lock<std::mutex> head_lock(wait_for_data());
        return pop_head();
    }

    std::unique_ptr<node> wait_pop_head(T &value) {
        std::unique_lock<std::mutex> head_lock(wait_for_data());
        value = *head->data;
        return pop_head();
    }

    // try pop
    std::unique_ptr<node> try_pop_head() {
        std::lock_guard<std::mutex> head_lock(head_mutex);
        if (head.get() == get_tail()) {
            return {};
        }
        return pop_head();
    }

    std::unique_ptr<node> try_pop_head(T &value) {
        std::lock_guard<std::mutex> head_lock(head_mutex);
        if (head.get() == get_tail()) {
            return {};
        }
        value = std::move(*head->data);
        return pop_head();
    }

private:
    std::mutex head_mutex;
    std::unique_ptr<node> head;
    std::mutex tail_mutex;
    node *tail;
    std::condition_variable data_cond;
};

template<typename T>
void threadsafe_queue<T>::push(T&& new_value) {
    std::shared_ptr<T> new_data(std::make_shared<T>(std::move(new_value)));
    printf("invoke push\n");
    std::unique_ptr<node> p(new node);
    {
        std::lock_guard<std::mutex> tail_lock(tail_mutex);
        tail->data = new_data;
        node *const new_tail = p.get();
        tail->next = std::move(p);
        tail = new_tail;
    }
    data_cond.notify_one();
}

template<typename T>
std::shared_ptr<T>
threadsafe_queue<T>::wait_and_pop() {
    std::unique_ptr<node> const old_head = wait_pop_head();
    return old_head->data;
}

template<typename T>
void
threadsafe_queue<T>::wait_and_pop(T &value) {
    std::unique_ptr<node> const old_head = wait_pop_head(value);
}


template<typename T>
std::shared_ptr<T>
threadsafe_queue<T>::try_pop() {
    std::unique_ptr<node> old_head = try_pop_head();
    return old_head ? old_head->data : std::shared_ptr<T>{};
}

template<typename T>
bool
threadsafe_queue<T>::try_pop(T &value) {
    std::unique_ptr<node> const old_head = try_pop_head(value);
    return old_head;
}

#endif //TINYWEB_THREADSAFE_QUEUE_H
