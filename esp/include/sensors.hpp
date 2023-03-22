#pragma once

#include <cstddef>
#include <cstdint>

namespace sensors {
    int init(void);
    uint16_t get_dist_mm(void);
}
