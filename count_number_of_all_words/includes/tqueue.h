//
// Created by myralllka on 3/29/20.
//

#ifndef COUNT_NUMBER_OF_ALL_WORDS_TQUEUE_H
#define COUNT_NUMBER_OF_ALL_WORDS_TQUEUE_H

#include <deque>
#include <iostream>
#include <mutex>
#include <thread>
#include <condition_variable>

class t_queue {
private:
    std::deque<int> queue;
    mutable std::mutex mut;
    std::condition_variable cond_variable;
public:
    t_queue() = default;

    ~t_queue() = default;

    void push(int d) {
        std::lock_guard<std::mutex> lg(mut);
        queue.push_back(d);
        cond_variable.notify_one();
    }

    int pop() {
        std::unique_lock<std::mutex> lg(mut);
        if (queue.size() == 0) {
            cond_variable.wait(lg);
        }
        cond_variable.wait(lg, [this]() { return queue.size() != 0; });
        int d = queue.front();
        queue.pop_front();
        return d;
    }

    size_t get_size() const {
        std::lock_guard<std::mutex> lg(mut);
        return queue.size();
    }
};

#endif //COUNT_NUMBER_OF_ALL_WORDS_TQUEUE_H
