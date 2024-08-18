#ifndef QUEUE_HPP
#define QUEUE_HPP

#include <sys/mman.h>

#include <yaml-cpp/yaml.h>

#include <unistd.h>
#include <fcntl.h>

#include <array>
#include <atomic>
#include <cstdint>
#include <cstring>
#include <format>
#include <iostream>
#include <string>

struct QueueHeader
{
    std::atomic<uint64_t> write_index;
    uint16_t queue_type;
    bool is_writer_active;
    bool is_initialized;
    std::array<char, 52> pad;
};

struct MessageHeader
{
    uint64_t message_size;
};

class QueueBase
{
    public:
        virtual ~QueueBase() = default;
        int init(const YAML::Node& config);
        [[nodiscard]] std::size_t read(char* message);
        virtual bool write(const char* message, const uint64_t message_length) = 0;

    protected:
        [[nodiscard]] virtual uint16_t getQueueType() const = 0;

        std::size_t queue_size = 0;
        std::size_t message_size = 0;
        char* queue_ptr = nullptr;
        QueueHeader* queue_header = nullptr;
        char* queue_data = nullptr;

    private:
        std::size_t roundUpToPowerOf2(std::size_t value) const;

        std::string queue_name;
        int queue_fd = -1;
        bool is_writer = false;
        uint64_t read_index = 0;
};

class SingleWriterQueue final : public QueueBase
{
    public:
        bool write(const char* message, const uint64_t message_length) override;

    protected:
        [[nodiscard]] uint16_t getQueueType() const override { return 1; }
};

class MultiWriterQueue final : public QueueBase
{
    public:
        bool write(const char* message, const uint64_t message_length) override;

    protected:
        [[nodiscard]] uint16_t getQueueType() const override { return 2; }
};

#endif //   QUEUE_HPP
