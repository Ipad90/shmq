# Shared Memory Queue
## What is this
This is an array-based shared memory queue, that wraps around by overwriting the oldest element. Each slot is fixed-size, so some estimations of how big each data would be is required.

## Why is this created
I wanted to have a minimal interprocess communication platform for my programs. And I also wanted to try some IPC.

## Features
- **Support for multiple producers or single producer**: Concurrency handling.
- **Configurable**: Queue name, queue size, and message size can be configured in a YAML file.
- **Lock-free**: With atomic operations.
- **C API**: Interface provided by `extern "C"` in `shmq.cpp` allows for integration with other languages such as Python.

## How it works
### Memory Layout
| Queue Header | Data Header 1 | Data Body 1 | Data Header 2 | Data Body 2 | ... | Data Header N | Data Body N |
|--------------|---------------|-------------|---------------|-------------|-----|---------------|-------------|

Queue Header consists of these fields:
- Write Index, for producers to know what index to write on
- Queue Type
- Is writer active
- Is initialized
- Padding

Data consists of two parts:
1. Message header, which contains the size of the message (number of bytes) 
2. Message Body, which contains the contents of the message

### Read
When a reader is initialized, it retrieves the current shared write index and copies the value to become the read index. This read index
is exclusive to the reader. Other readers will have their own value of the read index.

Reader only modifies the input variable once the index has valid data 

### Write (Multiple writer setup)
1st writer writes gets the current write index, increments the write index for the next writer to use,
and clears the slot by setting the `message_size` to 0.

### Write (Single writer setup)
Writer gets the current write index, and then increments it for later use.
Functions mostly similar to the implementation used in Multiple writer setup, just with more relaxed memory ordering.

## Requirements
- yaml-cpp
- C++23 (Technically C++20 can be used, modify the CMakeLists.txt in the root folder)
    
## Installation
1. **Clone the repository**:
    ```bash
    git clone https://github.com/Ipad90/shmq.git
    cd shmq
    ```
2. **Install dependencies**
    - [yaml-cpp](https://github.com/jbeder/yaml-cpp)
3. **Build**:
    ```bash
    mkdir build
    cmake -DCMAKE_BUILD_TYPE=Debug -DBUILD_TYPE=Debug -DTHIRD_PARTY_FOLDER=$HOME/third_party_libraries -S $HOME/shmq -B $HOME/shmq/build
    cmake --build $HOME/shmq/build --target TARGETS_TO_BUILD -j 10
   ```
4. Find the built binaries:
    ```bash
    cd $HOME/shmq/build/lib
    ```
    You should see a `libshmq.so`.
    ```bash
    cd $HOME/shmq/build/bin
    ```
    You should see a few sample programs.

## Why `std::uintptr_t` as the return type instead of `QueueBase*`
One of the main reasons for returning `std::uintptr_t` instead of `QueueBase*` is because I want to use it in Python programs, and possibly Node.JS programs in the future.
Also, most foreign function interfaces (FFIs) can handle integers, but not pointer types directly.

## Usage
### Configuration
The library expects a YAML configuration, either via file or string, which specifies:
```yaml
queue_name: "/dev/shm/multi_writer"
queue_type: "multi_writer_multi_reader"
queue_size: 480
message_size: 40
is_writer: true
clear_queue: false
```
- **queue_name**: Path used for creating/opening the shared memory file (often in /dev/shm on Linux).
- **queue_type**: single_writer_multi_reader or multi_writer_multi_reader.
- **queue_size**: The total size (capacity) in bytes of the ring buffer portion.
- **message_size**: Maximum size of a single message (plus the header).
- **is_writer**: Whether the process is a writer or a reader.
- **clear_queue**: If `true`, existing data in the queue is reset.

1. **Initialization**  
If passing a YAML's contents, use `SHM_initByConfigContent`. Or if passing a YAML file, use `SHM_initByConfigPath`.  
Both functions will return a pointer in the form of `std::uintptr_t`.
2. **Writing**
   1. Call `SHM_write`, and pass in the pointer returned from the initialization function.
   2. Pass the pointer of the data to be written into `SHM_write`.
   3. Pass the length in bytes of the data being written. For example, `sizeof(SomeStruct)`.
3. **Reading**
   1. Call `SHM_read`, and pass in the pointer returned form the initialization function.
   2. Pass the pointer of the data to be read into `SHM_read`.
   3. If `SHM_read` returns a value greater than 0, the pointer of the data passed into `SHM_read` should have some valid data to be used.
4. **Cleanup**
Call `SHM_close`

## Aspects to improve
- Everything
- More cache-lining
- Using hugepages
- Benchmarking
- Support multiple sub messages in one message field
- Think of whether this design is alright
