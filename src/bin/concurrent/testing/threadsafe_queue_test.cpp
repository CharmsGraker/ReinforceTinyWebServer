#include "../threadsafe_queue.h"
#include <thread>
#include <iostream>
#include <cassert>
threadsafe_queue<int> queue;

void thread_func(int low, int high) {
    for (int i = low; i < high; ++i) {
        queue.push(std::move(i));
    }
}

void Test_threadsafe_queue() {

    int n = 5;
    std::vector<std::thread> threads(n);

    for (int i = 0; i < n; ++i) {
        threads[i] = std::thread(thread_func, i * 10, (i + 1) * 10);
    }
    for (auto &&t: threads) {
        t.join();
    }
    int size =0;
    while (!queue.empty()) {
        int t;
        ++size;
        auto ptr = queue.wait_and_pop();
        std::cout << *ptr.get() << std::endl;
    }
    assert(size == 50);
};


int main() {
    Test_threadsafe_queue();
}