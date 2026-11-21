#include "PhaseExecutor.h"

#include <algorithm>
#include <atomic>
#include <condition_variable>
#include <exception>
#include <mutex>
#include <stdexcept>
#include <thread>
#include <vector>

namespace {
thread_local const void *activeExecutor = nullptr;
}

struct PhaseExecutor::Impl {
    std::mutex dispatchMutex;
    std::mutex mutex;
    std::condition_variable start, done;
    std::vector<std::thread> workers;
    bool stopping = false;
    std::size_t generation = 0, finished = 0;
    std::size_t itemCount = 0, batchSize = 0, batchCount = 0;
    std::atomic<std::size_t> next{0};
    Batch work;
    std::vector<std::exception_ptr> errors;

    explicit Impl(std::size_t count)
    {
        if (count == 0) throw std::invalid_argument("worker count must be positive");
        try {
            workers.reserve(count);
            for (std::size_t i = 0; i < count; ++i)
                workers.emplace_back([this] { worker(); });
        } catch (...) {
            stop();
            throw;
        }
    }

    ~Impl() { stop(); }

    void stop() noexcept
    {
        {
            std::lock_guard<std::mutex> lock(mutex);
            stopping = true;
        }
        start.notify_all();
        for (auto &thread : workers) if (thread.joinable()) thread.join();
    }

    void worker()
    {
        std::size_t seen = 0;
        std::unique_lock<std::mutex> lock(mutex);
        for (;;) {
            start.wait(lock, [&] { return stopping || generation != seen; });
            if (stopping) return;
            seen = generation;
            lock.unlock();
            activeExecutor = this;
            for (;;) {
                const auto index = next.fetch_add(1, std::memory_order_relaxed);
                if (index >= batchCount) break;
                const auto begin = index * batchSize;
                const auto end = begin + std::min(batchSize, itemCount - begin);
                try { work(begin, end, index); }
                catch (...) { errors[index] = std::current_exception(); }
            }
            activeExecutor = nullptr;
            lock.lock();
            if (++finished == workers.size()) done.notify_one();
        }
    }
};

PhaseExecutor::PhaseExecutor(std::size_t workerCount) : impl_(std::make_unique<Impl>(workerCount)) {}
PhaseExecutor::~PhaseExecutor() = default;
std::size_t PhaseExecutor::workerCount() const noexcept { return impl_->workers.size(); }

void PhaseExecutor::run(std::size_t itemCount, std::size_t batchSize, const Batch &batch)
{
    if (activeExecutor != nullptr) throw std::logic_error("nested phase dispatch is not supported");
    if (batchSize == 0 || !batch) throw std::invalid_argument("phase requires a positive batch size and a callback");
    std::lock_guard<std::mutex> dispatch(impl_->dispatchMutex);
    if (itemCount == 0) return;
    const auto count = 1 + (itemCount - 1) / batchSize;
    std::unique_lock<std::mutex> lock(impl_->mutex);
    impl_->errors.assign(count, {});
    impl_->work = batch;
    impl_->itemCount = itemCount;
    impl_->batchSize = batchSize;
    impl_->batchCount = count;
    impl_->next.store(0, std::memory_order_relaxed);
    impl_->finished = 0;
    ++impl_->generation;
    impl_->start.notify_all();
    impl_->done.wait(lock, [&] { return impl_->finished == impl_->workers.size(); });
    impl_->work = {};
    for (const auto &error : impl_->errors) if (error) std::rethrow_exception(error);
}
