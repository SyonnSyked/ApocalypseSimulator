#pragma once

#include <cstddef>
#include <functional>
#include <memory>

// A synchronous phase executor. Each batch owns the half-open index range
// [begin, end). The calling thread waits until every batch has completed.
class PhaseExecutor {
  public:
    using Batch = std::function<void(std::size_t begin, std::size_t end, std::size_t batchIndex)>;

    explicit PhaseExecutor(std::size_t workerCount);
    ~PhaseExecutor();
    PhaseExecutor(const PhaseExecutor &) = delete;
    PhaseExecutor &operator=(const PhaseExecutor &) = delete;
    PhaseExecutor(PhaseExecutor &&) = delete;
    PhaseExecutor &operator=(PhaseExecutor &&) = delete;

    std::size_t workerCount() const noexcept;
    void run(std::size_t itemCount, std::size_t batchSize, const Batch &batch);

  private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};
