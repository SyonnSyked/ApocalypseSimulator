#pragma once

#include <chrono>
#include <cstdint>

// Owned by the simulation loop; one completed tick represents one simulated hour.
class SimulationClock {
    std::uint64_t tickCount = 0;
    std::chrono::steady_clock::time_point clockStart;
    std::chrono::steady_clock::time_point clockEnd;
    bool isClockRunning = false;

  public:
    void StartClock();
    void EndClock();
    void AdvanceTick();
    std::uint64_t GetTickCount() const;
    std::chrono::hours GetSimulatedTime() const;
    std::chrono::steady_clock::duration GetRealElapsedTime() const;
};
