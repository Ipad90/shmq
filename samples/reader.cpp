#include <yaml-cpp/yaml.h>

#include "queue.hpp"

#include <array>
#include <cassert>
#include <csignal>
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

volatile std::sig_atomic_t keep_running = true;

void signal_handler(int signal)
{
    keep_running = false;
}

int main()
{
    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);

    MultiWriterQueue mw_queue;

    const std::string config = R"(
        queue_name: /dev/shm/multi_writer
        queue_type: multi_writer_multi_reader
        queue_size: 480
        message_size: 40
        is_writer: false
        clear_queue: false
    )";

    mw_queue.init(YAML::Load(config));

    std::array<char, sizeof(Data)> retrieved_buffer{};

    uint64_t seen_messages = 0;
    while (keep_running) {
        const std::size_t bytes_read = mw_queue.read(retrieved_buffer.data());
        if (bytes_read == 0) {
            continue;
        }
        const auto retrieved = std::make_unique<Data>(*reinterpret_cast<Data*>(retrieved_buffer.data()));
        assert(bytes_read == sizeof(Data));

        seen_messages++;
        // std::cout << std::format(
        //     "Bytes read: {}, Sequence ID: {} | A: {} - B: {}\n",
        //     bytes_read, retrieved->sequence_id, retrieved->a, retrieved->b
        // );
    }

    std::cout << "seen_messages: " << seen_messages << std::endl;

    return 0;
}