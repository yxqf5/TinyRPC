#ifndef TINYRPC_COROUTINE_COUROUTINE_POOL_H
#define TINYRPC_COROUTINE_COUROUTINE_POOL_H



#include <vector>
#include "myrpc/coroutine/coroutine.h"
#include "myrpc/net/mutex.h"
#include "myrpc/coroutine/memory.h"


namespace tinyrpc
{

    class CoroutinePool{
     
        
    
        public:
            CoroutinePool(int pool_size, int stack_size);
            ~CoroutinePool();
            Coroutine::ptr GetCoroutine();

            void RetCoroutine(Coroutine::ptr cor);

        private:

            int m_pool_size{0};
            int m_stack_size{0};

            
            
            std::vector<std::pair<Coroutine::ptr,bool>> m_free_cors;
            std::vector<Memory::ptr> m_memory_pool;
            Mutex m_mutex;
            

        

    };
    
    
    
    
    CoroutinePool* GetCoroutinePool();
    
    
} // namespace tinyrpc


#endif