#include "shmq.hpp"
#include "queue.hpp"

#include <iostream>
#include <memory>

template<typename QUEUE_TYPE>
concept SupportedSHMQ = std::is_same_v<QUEUE_TYPE, SingleWriterQueue> || std::is_same_v<QUEUE_TYPE, MultiWriterQueue>;

QueueBase* createQueue(YAML::Node config)
{
    const auto queue_type = config["queue_type"].as<std::string>();
    if (queue_type == "multi_writer_multi_reader") {
        return std::make_unique<MultiWriterQueue>().release();
    }

    if (queue_type == "single_writer_multi_reader") {
        return std::make_unique<SingleWriterQueue>().release();
    }

    return nullptr;
}

extern "C"
{
    std::uintptr_t SHM_initByConfigPath(const char* config_path)
    {
        auto q_ptr = createQueue(YAML::LoadFile(std::string(config_path)));
        q_ptr->init(YAML::Load(std::string(config_path)));
        return reinterpret_cast<std::uintptr_t>(q_ptr);
    }

    std::uintptr_t SHM_initByConfigContent(const char* config_content)
    {
        auto q_ptr = createQueue(YAML::Load(std::string(config_content)));
        q_ptr->init(YAML::Load(std::string(config_content)));
        return reinterpret_cast<std::uintptr_t>(q_ptr);
    }

    uint64_t SHM_read(const std::uintptr_t q_ptr, std::uintptr_t message)
    {
        const auto p_shm = reinterpret_cast<QueueBase*>(q_ptr);
        return p_shm->read(reinterpret_cast<char*>(message));
    }

    void SHM_write(const std::uintptr_t q_ptr, const std::uintptr_t message, const uint64_t message_length)
    {
        const auto p_shm = reinterpret_cast<QueueBase*>(q_ptr);
        p_shm->write(reinterpret_cast<const char*>(message), message_length);
    }

    void SHM_close(const std::uintptr_t q_ptr)
    {
        std::cout << "SHM closed" << std::endl;
        const auto p_shm = reinterpret_cast<QueueBase*>(q_ptr);
        delete p_shm;
    }
}
