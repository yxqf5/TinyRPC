#ifndef TINYRPC_COROUTINE_COROUTINE_H
#define TINYRPC_COROUTINE_COROUTINE_H

#include <memory>
#include <string>
#include <functional>
#include "myrpc/coroutine/coctx.h"
// #include "myrpc/comm/run_time.h"

namespace tinyrpc
{

    int getCoroutineIndex();

    RunTime* getCurrentRunTime();

    void setCurrentRunTime(RunTime* v);

class Coroutine {

 public:
    typedef std::shared_ptr<Coroutine> ptr;

 private:
    Coroutine();

 public:
    Coroutine(int size,char* stack_ptr);
    Coroutine(int size,char* stack_ptr,void* callback);
    ~Coroutine();
    bool setCallback(std::function<void()> cb);


    int getCorID(){
        return m_cor_id;
    }

    void setIsIncoFunc(const bool v){
        m_is_in_cofunc = v;
    }

    bool getIsCoFunc(const bool v){
        return m_is_in_cofunc;
    }

    std::string getMsgNo(){
        return m_msg_no;
    }
      
    RunTime* getRunTime() {
        return &m_run_time; 
    }

    void setMsgNo(const std::string& msg_no) {
        m_msg_no = msg_no;
    }

    void setIndex(int index) {
        m_index = index;
    }

    int getIndex() {
        return m_index;
    }

    char* getStackPtr() {
        return m_stack_sp;
    }

    int getStackSize() {
        return m_stack_size;
    }

    void setCanResume(bool v) {
        m_can_resume = v;
    }


    
 public: 
    static void yield();
    static void Resume(Coroutine* co);
    static Coroutine* GetCurrentCoroutine();
    static Coroutine* GetMainCoroutine();
    static bool IsMainCoroutine();

 private:
    int m_cor_id{0};
    coctx m_coctx;
    int m_stack_size{0};
    char* m_stack_sp{nullptr};
    bool m_is_in_cofunc{false};
    //?what
    std::string m_msg_no;
    //?what
    RunTime m_run_time;

    bool m_can_resume{true};
    //?what
    int m_index{-1};
 public:
    std::function<void()> m_call_back;

};

    
} // namespace tinyrpc


#endif