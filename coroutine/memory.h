#ifndef TINYRPC_COROUTINE_MEMORY_H
#define TINYRPC_COROUTINE_MEMORY_H


#include <memory>
#include <atomic>
#include <vector>
#include "myrpc/net/tinypb/mutex.h"

namespace tinyrpc{


class Memory
{

 public:
    typedef std::shared_ptr<Memory> ptr;
        
    
    Memory(int block_size, int block_count);
    ~Memory();

    void free();

    int getRefCount();
    char* getStart();
    char* getEnd();
    char* getBlock();

    void backBlock(char* s);
    bool hasBlock(char* s);

 private:
    int m_block_size{0};
    int m_blick_count{0};
    
    int m_size{0};
    char* m_start{nullptr};
    char* m_end{nullptr};

    std::atomic<int> m_ref_counts{0};
    std::vector<bool> m_blocks;
    Mutex m_mutex;


};
    

}

#endif TINYRPC_COROUTINE_MEMORY_H