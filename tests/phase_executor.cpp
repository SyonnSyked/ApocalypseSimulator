#include "PhaseExecutor.h"

#include <atomic>
#include <cstddef>
#include <stdexcept>
#include <thread>
#include <vector>

int main()
{
    bool invalid = false;
    try { PhaseExecutor bad(0); } catch (const std::invalid_argument &) { invalid = true; }
    if (!invalid) return 1;
    PhaseExecutor executor(4);
    if (executor.workerCount() != 4) return 2;
    for (int repeat = 0; repeat < 40; ++repeat) {
        std::vector<int> values(1003, 0);
        executor.run(values.size(), 17, [&](std::size_t begin, std::size_t end, std::size_t batch) {
            if (begin != batch * 17 || end > values.size()) throw std::runtime_error("invalid batch");
            for (auto i = begin; i < end; ++i) ++values[i];
        });
        for (auto value : values) if (value != 1) return 3;
    }
    executor.run(0, 1, [](std::size_t, std::size_t, std::size_t) { throw std::runtime_error("empty work"); });
    invalid = false;
    try { executor.run(1, 0, [](std::size_t, std::size_t, std::size_t) {}); }
    catch (const std::invalid_argument &) { invalid = true; }
    if (!invalid) return 4;
    std::atomic<int> completed{0};
    bool failed = false;
    try {
        executor.run(100, 1, [&](std::size_t, std::size_t, std::size_t batch) {
            ++completed;
            if (batch == 3) throw std::runtime_error("batch failed");
        });
    } catch (const std::runtime_error &) { failed = true; }
    if (!failed || completed != 100) return 5;
    failed = false;
    try {
        executor.run(1, 1, [&](std::size_t, std::size_t, std::size_t) {
            executor.run(1, 1, [](std::size_t, std::size_t, std::size_t) {});
        });
    } catch (const std::logic_error &) { failed = true; }
    if (!failed) return 6;
    PhaseExecutor single(1);
    std::vector<std::size_t> singleBatches(103), parallelBatches(103);
    auto fill = [](std::vector<std::size_t> &values) {
        return [&values](std::size_t begin, std::size_t end, std::size_t batch) {
            for (auto i = begin; i < end; ++i) values[i] = batch;
        };
    };
    single.run(singleBatches.size(), 8, fill(singleBatches));
    executor.run(parallelBatches.size(), 8, fill(parallelBatches));
    if (singleBatches != parallelBatches) return 8;
    completed = 0;
    auto submit = [&] { executor.run(50, 1, [&](std::size_t, std::size_t, std::size_t) { ++completed; }); };
    std::thread first(submit), second(submit);
    first.join(); second.join();
    if (completed != 100) return 7;
    return 0;
}
