#pragma once
#include <cstddef>
#include <cstdint>

namespace cam {
    int init(void);
    void send_jpg_buffer(const char* topic);
}
