#include <yaml-cpp/yaml.h>

#include "shmq/shmq.hpp"

#include <chrono>
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

int main(int argc, char** argv)
{
    const std::string config = R"(
        queue_name: /dev/shm/multi_writer
        queue_type: multi_writer_multi_reader
        queue_size: 480
        message_size: 40
        is_writer: true
        clear_queue: false
    )";

    const std::uintptr_t mw_queue_ptr = SHM_initByConfigContent(config.c_str());

    if (mw_queue_ptr == 0) {
        std::cerr << "Failed to initialize queue" << std::endl;
        return -1;
    }

    const std::unique_ptr<Data> data(new Data);

    data->sequence_id = 1;
    data->a = 100;
    data->b = 111;
    data->c = 1;

    for (auto i = 0; i < 100; i++) {
        std::cout << std::format("Sequence: {} A: {} B: {} C: {}\n", data->sequence_id, data->a, data->b, data->c);

        SHM_write(mw_queue_ptr, reinterpret_cast<std::uintptr_t>(reinterpret_cast<const char*>(data.get())), sizeof(Data));

        data->sequence_id += 1;
        data->a += 100;
        data->b += 1;
        data->c += 1;

        if (argc > 1) {
            std::this_thread::sleep_for(std::chrono::microseconds(std::stoi(argv[1])));
        }
    }

    SHM_close(mw_queue_ptr);
    return 0;
}
