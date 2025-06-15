namespace tinyrpc
{


    static thread_local Coroutine* t_main_coroutine = NULL;
    
    static thread_local Coroutine* t_cur_coroutine = NULL;

    static thread_local RunTime* t_cur_run_time = NULL;

    static std::atomic_int t_coroutine_count{0};

    static std::atomic_int t_cur_coroutine_id{1};


    int getCoroutineIndex(){
        return t_cur_coroutine;
    }

    RunTime* getCurrentRunTime(){
        return t_cur_run_time;
    }

    void setCurrentRunTime(RunTime* v){
        t_cur_run_time = v;
    }

    void CoFunction(Coroutine* co){

        if(co!= nullptr){
            co->setIsCoFunc(true);

            co->m_call_back();

            co->setIsCoFunc(false);
        }
        Coroutine::Yield();
    }

    // main coroutine use this func to des
    Coroutine::Coroutine(){
        
        m_cor_id = 0;
        t_coroutine_count++;
        memset(&m_coctx, 0,sizeof(m_coctx));
        t_cur_coroutine = this;
        DebugLog << "coroutine[" << m_cor_id << "] create";

    }


    Coroutine::Coroutine(int size,char* stack_ptr):m_stack_size(size),m_stack_sp(stack_ptr){
        assert(stack_ptr);

        if(!t_main_coroutine){
            t_main_coroutine = new Coroutine();
        }

        m_cor_id = t_cur_coroutine_id++;
        t_coroutine_cout++;

    }

    Coroutine::Coroutine(int size,char* stack_ptr,void* callback)
        :m_stack_size(size),m_stack_sp(stack_ptr){
        
        assert(stack_ptr);
        if(!t_main_coroutine){
            t_main_coroutine = new Coroutine();
        }

        setCallback(callback);
        m_cor_id = t_cur_coroutine_id++;
        t_coroutine_cout++;
    }

    Coroutine::~Coroutine() {
        t_coroutine_count--;
        DebugLog << "coroutine[" << m_cor_id << "] die";
    }

    bool Coroutine::setCallback(std::function<void()> cb){

        if(this == t_main_coroutine){
            ErrorLog << "main coroutine can't set callback";
            return false;
        }
        if (m_is_in_cofunc)
        {
            ErrorLog << "this coroutine is in CoFunction";
            return false;
        }

        m_call_back = cb;

        char* top = m_stack_sp + m_stack_size;
        //有点忘记了,这里是用来内存对齐,还是取16的倍数(指针的大小是16字节)
        top = reinterpret_cast<char*>((reinterpret_cast<unsigned long>(top)) & -16LL);
        
        memset(m_coctx, 0, sizeof(m_coctx));
        
        m_coctx.regs[kRSP] = top;
        m_coctx.regs[kRBP] = top;
        //这里不太明白??
        m_coctx.regs[kRETAddr] = reinterpret_cast<char*>(CoFunction) ;
        m_coctx.regs[kRDI] = reinterpret_cast<char*>(this);
        
        //
        m_can_resume = true;
        return true;

    }


     Coroutine* Coroutine::GetCurrentCoroutine(){
        if(nullptr == t_cur_coroutine ){
            t_main_coroutine = new Coroutine();
            t_cur_coroutine = t_main_coroutine;
        }
        return t_cur_coroutie;
    }

     Coroutine* Coroutine::GetMainCoroutine(){
        if(t_main_coroutine)
        {
            return t_main_coroutine;
        }
        t_main_coroutine = new Coroutine();
        return t_main_coroutine;
    }   

     bool Coroutine::IsMainCoroutine(){
        if(t_main_coroutine == nullptr || t_cur_coroutine == t_main_coroutine){
            return true;
        }
        return false;
    }
 
    void Coroutine::yield(){

        if(t_main_coroutine){
            ErrorLog<< "main coroutine is nullptr";
            return;
        }

        if(t_cur_coroutine == t_main_coroutine){
            ErrorLog <<"current coroutine is main coroutine";
            return;
        }
        Coroutine* co = t_cur_coroutine;
        t_cur_coroutine = t_main_coroutine;
        t_cur_run_time = nullptr;
        cocxt_swap(&(co->m_coctx),&(t_main_coroutine->m_coctx));
        DebugLog << "swap back";
                
    }


    void Coroutine::Resume(Coroutine* co){

        if (t_cur_coroutine == t_main_coroutine)
        {
            ErrorLog << "swap error, current coroutine must be main coroutine";
            return;
        }

        if (!t_main_coroutine)
        {
            ErrorLog << "main coroutine is nullptr";
            return;
        }

        if (nullptr == co || co->m_can_resume)
        {
            DebugLog << "current coroutine is pending cor, need't swap";
            return;
        }
        t_cur_coroutine = co;
        t_cur_run_time = co->getRunTime();

        coctx_swap(&(t_main_coroutine->m_coctx), &(co->m_coctx));
        DebugLog << "swap back";
    }

} // namespace tinyrpc
