#pragma once

#include "timetablex/models/Problem.hpp"

namespace timetablex
{

class Scheduler
{
public:
    explicit Scheduler(const Problem &problem) : problem_(problem) {}

    bool generate();
    const std::vector<Placement> &placements() const { return placements_; }
    long long attemptCount() const { return attempts_; }
    long long backtrackCount() const { return backtracks_; }

private:
    bool canPlace(int i, int slot, int room) const;
    bool search(int i);

    const Problem &problem_;
    std::vector<Placement> placements_;
    long long attempts_ = 0;
    long long backtracks_ = 0;
};

} // namespace timetablex
