#include "queue.hpp"

int QueueBase::init(const YAML::Node& config)
{
    static_assert(sizeof(QueueHeader) == 64, "QueueHeader should be 64 bytes long");

    this->queue_name = config["queue_name"].as<std::string>();
    this->queue_size = config["queue_size"].as<std::size_t>();
    this->message_size = config["message_size"].as<std::size_t>();
    const bool is_writer = config["is_writer"].as<bool>();
    const bool clear_queue = config["clear_queue"].as<bool>();

    if (this->queue_size == 0) { throw std::invalid_argument("Invalid queue size"); }
    if (this->message_size == 0) { throw std::invalid_argument("Invalid message size"); }

    this->message_size = this->roundUpToPowerOf2(this->message_size + sizeof(MessageHeader));
    this->queue_size = this->roundUpToPowerOf2(this->queue_size);

    const auto full_queue_size = static_cast<int64_t>(this->queue_size + sizeof(QueueHeader));
    std::cout << std::format("Created queue with full size of {}\n", full_queue_size);
    constexpr int open_flags = O_CREAT | O_RDWR;
    this->queue_fd = open(this->queue_name.c_str(), open_flags, 0666);
    if (ftruncate(this->queue_fd, full_queue_size) != 0) {
        close(this->queue_fd);
        return -1;
    }

    constexpr int prot_flags = PROT_READ | PROT_WRITE;
    constexpr int map_flags = MAP_SHARED;
    this->queue_ptr = static_cast<char*>(mmap(nullptr, full_queue_size, prot_flags, map_flags, this->queue_fd, 0));
    if (this->queue_ptr == MAP_FAILED) {
        close(this->queue_fd);
        return -1;
    }

    this->queue_header = reinterpret_cast<QueueHeader*>(this->queue_ptr);
    this->queue_header->queue_type = this->getQueueType();

    if (!this->queue_header->is_initialized || clear_queue) {
        std::memset(this->queue_header, 0, sizeof(QueueHeader));
        this->queue_header->write_index.store(0);
        this->queue_header->is_writer_active = false;
    }

    //  TODO: Need to check if existing queue type is same with config requiremenet
    if (this->queue_header->queue_type == 1) {
        if (this->queue_header->is_writer_active && is_writer) {
            std::cout << std::format("Writer already exist for single writer queue.\n");
            return -1;
        }

        if (!this->queue_header->is_writer_active && is_writer) {
            this->queue_header->is_writer_active = true;
        }
    }

    this->queue_data = this->queue_ptr + sizeof(QueueHeader);
    this->read_index = this->queue_header->write_index.load();
    this->queue_header->is_initialized = true;

    #ifdef DEBUG
        std::cout << std::format(
            "Queue type: {} | Queue name: {} | Full size: {} | Queue size: {} | Message size: {} | Is writer: {}\n",
            this->queue_header->queue_type, this->queue_name,
            full_queue_size, this->queue_size,  this->message_size, is_writer
        );
    #endif

    return 0;
}

std::size_t QueueBase::read(char* message)
{
    const uint64_t message_size = reinterpret_cast<MessageHeader*>(this->queue_data + this->read_index)->message_size;

    if (this->read_index == this->queue_header->write_index.load()) { return 0; }
    if (message_size == 0) { return 0; }

    #ifdef DEBUG
        std::cout << std::format(
            "Reader index: {} | Actual reader index: {} | Actual reader index after MessageHeader: {}\n",
            this->read_index,
            this->queue_data + this->read_index,
            this->queue_data + this->read_index + sizeof(MessageHeader)
        );
    #endif

    std::memcpy(message, this->queue_data + this->read_index + sizeof(MessageHeader), message_size);
    this->read_index = (this->read_index + this->message_size) & (this->queue_size - 1);
    return message_size;
}

std::size_t QueueBase::roundUpToPowerOf2(std::size_t value) const
{
    if (!value | (!(value & (value - 1)))) { return value; }
    uint64_t result = 1;
    while (value) {
        value = value >> 1;
        result = result << 1;
    }

    return result;
}


bool SingleWriterQueue::write(const char* message, const uint64_t message_length)
{
    const uint64_t write_size = sizeof(MessageHeader) + message_length;

    if (write_size == 0) { return false; }
    if (write_size > this->message_size) { return false; }

    uint64_t current_write_index = this->queue_header->write_index.load(std::memory_order_relaxed);
    this->queue_header->write_index.store((current_write_index + this->message_size) & (this->queue_size - 1), std::memory_order_relaxed);

    #ifdef DEBUG
        std::cout << std::format(
            "Writer index: {}\n",
            current_write_index
        );
    #endif

    std::memcpy(this->queue_data + current_write_index + sizeof(MessageHeader), message, message_length);
    reinterpret_cast<MessageHeader*>(this->queue_data + current_write_index + write_size)->message_size = 0;
    reinterpret_cast<MessageHeader*>(this->queue_data + current_write_index)->message_size = message_length;

    return true;
}

bool MultiWriterQueue::write(const char* message, const uint64_t message_length)
{
    /**
     *  Assuming 2 writers writing to the queue at the same time.
     *  Writer A takes slot A, writer B takes slot B
     *  Writer A needs to clear slot B for writer B, writer B would need to clear slot C
     */
    const uint64_t write_size = sizeof(MessageHeader) + message_length;

    if (write_size == 0) { return false; }
    if (write_size > this->message_size) { return false; }

    uint64_t current_write_index = 0;
    while (true) {
        current_write_index = this->queue_header->write_index.load(std::memory_order_acquire);

        if (this->queue_header->write_index.compare_exchange_weak(
            current_write_index, (current_write_index + this->message_size) & (this->queue_size - 1), std::memory_order_acq_rel
            )
        ) {
            break;
        }
    }

    #ifdef DEBUG
        std::cout << std::format(
            "Writer index: {} | Actual writer index: {} | Actual writer index after MessageHeader: {}\n",
            current_write_index,
            this->queue_data + current_write_index,
            this->queue_data + current_write_index + sizeof(MessageHeader)
        );
    #endif

    std::memcpy(this->queue_data + current_write_index + sizeof(MessageHeader), message, write_size);
    reinterpret_cast<MessageHeader*>(this->queue_data + current_write_index + write_size)->message_size = 0;
    reinterpret_cast<MessageHeader*>(this->queue_data + current_write_index)->message_size = message_length;

    return true;
}
