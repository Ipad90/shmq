#ifndef SHMQ_HPP
#define SHMQ_HPP

#include <yaml-cpp/yaml.h>

#include <cstdint>

class QueueBase;

QueueBase* createQueue(YAML::Node config);

extern "C"
{
    std::uintptr_t SHM_initByConfigPath(const char* config_path);
    std::uintptr_t SHM_initByConfigContent(const char* config_content);
    uint64_t SHM_read(std::uintptr_t q_ptr, std::uintptr_t message);
    void SHM_write(std::uintptr_t q_ptr, const std::uintptr_t, const uint64_t message_length);
    void SHM_close(std::uintptr_t q_ptr);
}

#endif //   SHMQ_HPP
