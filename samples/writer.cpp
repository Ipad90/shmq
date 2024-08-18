#include <yaml-cpp/yaml.h>

#include "queue.hpp"

#include <chrono>
#include <cstdint>
#include <format>
#include <memory>
#include <string>
#include <thread>

struct Data
{
    uint64_t sequence_id;
    uint32_t a;
    uint32_t b;
    uint16_t c;
};

int main(int argc, char** argv)
{
    MultiWriterQueue mw_queue;

    const std::string config = R"(
        queue_name: /dev/shm/multi_writer
        queue_type: multi_writer_multi_reader
        queue_size: 480
        message_size: 40
        is_writer: true
        clear_queue: false
    )";

    mw_queue.init(YAML::Load(config));

    const std::unique_ptr<Data> data(new Data);

    data->sequence_id = 1;
    data->a = 100;
    data->b = 111;

    int num_messages = (argc > 1) ? std::stoi(argv[1]) : 100;
    for (auto i = 0; i < num_messages; i++) {
        // std::cout << std::format(
        //     "Sequence: {} A: {} B: {}\n",
        //     data->sequence_id, data->a, data->b
        // );
        mw_queue.write(reinterpret_cast<const char*>(data.get()), sizeof(Data));

        data->sequence_id += 1;
        data->a += 100;
        data->b += 1;

        if (argc > 2) {
            std::this_thread::sleep_for(std::chrono::microseconds(std::stoi(argv[2])));
        }
    }

    return 0;
}
