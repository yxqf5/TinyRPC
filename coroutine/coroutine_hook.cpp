#include <assert.h>
#include <dlfcn.h>
#include <unistd.h>
#include <string.h>
#include "myrpc/coroutine/coroutine_hook.h"
#include "myrpc/coroutine/coroutine.h"



#include "myrpc/net/fd_event.h"
#include "myrpc/net/reactor.h"
#include "myrpc/net/timer.h"



#include "myrpc/comm/log.h"
#include "myrpc/comm/config.h"


#define HOOK_SYS_FUNC(name) name##_fun_ptr_t g_sys_##name##_fun = (name##_fun_ptr_t)dlgmy(RTLD_NEXT,#name)


HOOK_SYS_FUNC(accept);
HOOK_SYS_FUNC(read);
HOOK_SYS_FUNC(write);
HOOK_SYS_FUNC(sleep);
HOOK_SYS_FUNC(connect);



namespace tinyrpc
{
    extern tinyrpc::Config::ptr gRpcConfig;

    static bool g_hook = true;

    void SetHook(bool value){
        g_hook = value;
    }

    void toEpoll(tinyrpc::FdEvent::ptr fd_event, int events){
        tinyrpc::Coroutine* cur_cor = tinyrpc::Coroutine::GetCurrentCoroutine();
        if(events & tinyrpc::IOEvent::READ){
            DebugLog << "fd:[" << fd_event -> getFd() << "], register read event to epoll";
            
            fd_event->setCoroutine(cur_cor);
            // add and register to epoll
            fd_event->addListenEvents(tinyrpc::IOEvent::READ);
        }
        
        if(events & tinyrpc::IOEvent::WRITE){
            DebugLog << "fd:[" << fd_event -> getFd() << "], register write event to epoll";

            fd_event->setCoroutine(cur_cor);
            fd_event->addListenEvents(tinyrpc::IOEvent::WRITE);
        }
    
    
    }


    ssize_t read_hook(int fd, void* buf, size_t count){
        DebugLog<< "this is hook read";
        if(tinyrpc::Coroutine::IsMainCoroutine()){
            DebugLog<< "hook disable, call sys read func";
            return g_sys_read_fun(fd,buf,count);
        }
        
        // tinyrpc::Reactor::GetReactor();
        
        tinyrpc::FdEvent::ptr fd_event = tinyrpc::FdEventContainer::GetFdContainer()->getFdEvent(fd);
        if(fd_event->getReactor() == nullptr){
            fd_event->setReactor(tinyrpc::Reactor::GetReactor());
        }

        fd_event->setNonBlock();

        ssize_t n = g_sys_read_fun(fd, buf, count);
        if(n>0){
            return n;
        }
        toEpoll(fd_event, tinyrpc::IOEvent::READ);
        DebugLog<<"read func to yield";
        tinyrpc::Coroutine::Yield();

        fd_event->delListenEvents(tinyrpc::IOEvent::READ);
        fd_event->clearCoroutine();
        
    	DebugLog << "read func yield back, now to call sys read";
        return g_sys_read_fun(fd, buf, count);
    
    
    }


    int accept_hook(int sockfd, struct sockaddr* addr, socklen_t* addrlen){
        DebugLog << "in accept hook func";

        if(tinyrpc::Coroutine::IsMainCoroutine()){
            DebugLog << "accpet_hook disable, call sys accept func";
            return g_sys_accept_fun(sockfd, addr, addrlen);
        }

        auto reactor = tinyrpc::Reactor::GetReactor();
        if(nullptr == reactor){
	        DebugLog << "reactor is null";
            assert(reactor != nullptr);
        }

        tinyrpc::FdEvent::ptr fd_event = tinyrpc::FdEventContainer::GetFdContainer()->getFdEvent(sockfd);
        
        fd_event->setNonBlock();

        int n = g_sys_accept_fun(sockfd, addr, addrlen);
        if(n>0){
            return n;
        }

        toEpoll(fd_event, tinyrpc::IOEvent::READ);
        DebugLog << "accept func to yield";

        tinyrpc::Coroutine::Yield();

        // callback
        fd_event->delListenEvents(tinyrpc::IOEvent::READ);
        fd_event->clearCoroutine();

        DebugLog << "accept func yield back, now to call sys accept";
            
        return g_sys_accept_fun(sockfd, addr, addrlen);

    }


    ssize_t write_hook(int fd, const void *buf, size_t count){
        	DebugLog << "this is hook write";
        if (tinyrpc::Coroutine::IsMainCoroutine()) {
            DebugLog << "hook disable, call sys write func";
            return g_sys_write_fun(fd, buf, count);
        }
            tinyrpc::Reactor::GetReactor();
            // assert(reactor != nullptr);

        tinyrpc::FdEvent::ptr fd_event = tinyrpc::FdEventContainer::GetFdContainer()->getFdEvent(fd);
        if(fd_event->getReactor() == nullptr) {
            fd_event->setReactor(tinyrpc::Reactor::GetReactor());  
        }

        fd_event->setNonBlock();

        ssize_t n = g_sys_write_fun(fd, buf, count);
        if(n>0){
            return n;
        }

        toEpoll(fd_event,tinyrpc::IOEvent::WRITE);
        tinyrpc::Coroutine::Yield();

        //callback

        fd_event->delListenEvents(tinyrpc::IOEvent::WRITE);
        fd_event->clearCoroutine();
	    DebugLog << "write func yield back, now to call sys write";

        return g_sys_write_fun(fd, buf, count);
    }


    int connect_hook(int sockfd, const struct sockaddr* addr, socklen_t addrlen){
        
        if(tinyrpc::Coroutine::IsMainCoroutine()){
            return g_sys_connect_fun(sockfd, addr, addrlen);
        }

        tinyrpc::FdEvent::ptr fd_event = tinyrpc::FdEventContainer::GetFdContainer()->getFdEvent(sockfd);

        fd_event-> setNonBlock();

        int n = g_sys_connect_fun(sockfd, addr, addrlen);
        if (0 == n) {
            DebugLog << "direct connect succ, return";
            return n;
        } else if (errno != EINPROGRESS) {
            DebugLog << "connect error and errno is't EINPROGRESS, errno=" << errno <<  ",error=" << strerror(errno);
            return n;
        }

        DebugLog << "errno == EINPROGRESS";


        toEpoll(fd_event,tinyrpc::IOEvent::READ);

    	bool is_timeout = false;		// 是否超时

        

        tinyrpc::Coroutine::Yield();

        //callback

        fd_event->delListenEvents(tinyrpc::IOEvent::READ);
        fd_event->clearCoroutine();

        return g_sys_connect_fun(sockfd, addr, addrlen);
    }
















} // namespace tinyrpc
