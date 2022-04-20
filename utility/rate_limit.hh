#pragma once

#include <cstdint>

namespace aru::rate_limit {

    bool consume(const uint64_t tokens);
}