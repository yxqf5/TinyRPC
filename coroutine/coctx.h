#ifndef TINYRPC_COROUTINE_COCTX_H
#define TINYRPC_COROUTINE_COCTX_H


namespace tinyrpc{

    enum{
        kRBP = 6,       //协程栈栈底
        kRDI = 7,       //协程栈,函数调用时第一部分
        kRSI = 8,       //函数调用时的二部分
        kRETAddr =9,    //
        kRSP =13,       //协程栈顶
    };
        

    struct coctx{
        void* regs[14];
    };

    extern "C"{

        extern void coctx_swap(coctx*, coctx*) asm ("coctx_swap");
    };



};


#endif TINYRPC_COROUTINE_COCTX_H