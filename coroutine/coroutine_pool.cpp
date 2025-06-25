
#include <vector>
#include <sys/mman.h>
#include "myrpc/comm/config.h"
#include "myrpc/comm/log.h"
#include "myrpc/coroutine/coroutine_pool.h"
#include "myrpc/coroutine/coroutine.h"
#include "myrpc/net/mutex.h"



namespace tinyrpc
{

    extern tinyrpc::Config::ptr gRpcConfig;

    static CoroutinePool* t_coroutine_container_ptr = nullptr;

    CoroutinePool* GetCoroutinePool(){
        if(nullptr == t_coroutine_container_ptr){
            t_coroutine_container_ptr = new CoroutinePool(gRpcConfig->m_cor_pool_size,gRpcConfig->m_cor_stack_size);
        }
        return t_coroutine_container_ptr;
    }


    CoroutinePool::CoroutinePool(int pool_size,int stack_size):m_pool_size(pool_size),m_stack_size(stack_size){
        
        m_memory_pool.push_back(std::make_shared<Memory>(pool_size,stack_size));
        
        Memory::ptr temp = m_memory_pool[0];

        for(size_t i =0; i < pool_size; i++){
            Coroutine::ptr cor = std::make_shared<Coroutine>(stack_size,temp->getBlock());
            cor->setIndex(i);
            m_free_cors.push_back(std::make_pair(cor,false));
        }
    }

    CoroutinePool::~CoroutinePool(){
        
    }

    Coroutine::ptr CoroutinePool::GetCoroutine(){
        Mutex::Lock lock(m_mutex);
        for(auto iter = m_free_cors.begin(); iter!= m_free_cors.end();iter++)
        {
            if(false == iter->first->getIsInCoFunc() && false == iter->second){
                iter->second = true;
                lock.unlock();
                return iter->first;
            }
        }

        for (size_t i = 1; i < m_memory_pool.size(); i++)
        {
            char* temp = m_memory_pool[i]->getBlock();
            if(temp){

                Coroutine::ptr cor = std::make_shared<Coroutine>(m_stack_size,temp);
                return cor;
            }
        }
        
        m_memory_pool.push_back(std::make_shared<Memory>(m_stack_size,m_pool_size));
        return std::make_shared<Coroutine>(m_stack_size,m_memory_pool.back()->getBlock());
    }

    void CoroutinePool::RetCoroutine(Coroutine::ptr cor){
        int i = cor->getIndex();
        if(i >=0 && i<m_pool_size){
            m_free_cors[i].second = false;
        } else {
            for (size_t i = 0; i < m_memory_pool.size(); i++)
            {
                if(m_memory_pool[i]->hasBlock(cor->getStackPtr())){

                    m_memory_pool[i]->backBlock(cor->getStackPtr());
                }
            }
            
        }
    }


    
} // namespace tinyrpc


extern tinyrpc::Config::ptr gRpcConfig;


