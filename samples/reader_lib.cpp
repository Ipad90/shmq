#include <yaml-cpp/yaml.h>

#include "shmq/shmq.hpp"

#include <array>
#include <cassert>
#include <cstdint>
#include <format>
#include <iostream>
#include <memory>
#include <string>
#include <thread>

struct Data
{
    uint64_t sequence_id;
    uint32_t a;
    uint32_t b;
    uint32_t c;
};

int main()
{
    const std::string config = R"(
        queue_name: /dev/shm/multi_writer
        queue_type: multi_writer_multi_reader
        queue_size: 480
        message_size: 40
        is_writer: false
        clear_queue: false
    )";

    const std::uintptr_t mw_queue_ptr = SHM_initByConfigContent(config.c_str());
    std::cout << mw_queue_ptr << std::endl;

    std::array<char, sizeof(Data)> retrieved_buffer{};

    while (true) {
        const std::size_t bytes_read = SHM_read(mw_queue_ptr, reinterpret_cast<std::uintptr_t>(retrieved_buffer.data()));
        if (bytes_read == 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            continue;
        }
        const auto retrieved = std::make_unique<Data>(*reinterpret_cast<Data*>(retrieved_buffer.data()));
        assert(bytes_read == sizeof(Data));

        std::cout << std::format(
            "Bytes read: {}, Sequence ID: {} | A: {} - B: {} - C: {}\n",
            bytes_read, retrieved->sequence_id, retrieved->a, retrieved->b, retrieved->c
        );

        // std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    return 0;
}