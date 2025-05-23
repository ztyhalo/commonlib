﻿/*
 * File:   timer_poll.h
 * Author: Administrator
 *
 * 文件监管库V1.2
 */
 
#ifndef __E_POLL_H__
#define __E_POLL_H__
#include <sys/types.h>
#include <fcntl.h>

#include <sys/epoll.h>
#include <stdlib.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/timerfd.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include "zprint.h"
#include <vector>
#define MAXFDS 4
#define EVENTS 100

class z_poll
{
public:
    int m_epFd;
private:
    int m_active;
    const int m_epSize;

public:
    explicit z_poll(int max_num= MAXFDS):m_epSize(max_num)
    {
        m_epFd = epoll_create(max_num);
        if(m_epFd == -1)
        {
            zprintf1("epoll creat failed!\n");
            m_active = 0;
            return;
        }
        m_active = 1;
    }

    int e_poll_add(int fd)
    {
        if(setNonBlock(fd) == false)
            return -1;
        int err = 0;
        struct epoll_event ev;
        memset(&ev, 0x00, sizeof(struct epoll_event));
        ev.data.fd = fd;
        ev.events = EPOLLIN | EPOLLET;
        err = epoll_ctl(m_epFd, EPOLL_CTL_ADD, fd, &ev);
        if(err == -1)
        {
            zprintf1("%s\n",strerror(errno));
            return err;
        }
        return 0;
    }

    int e_poll_add_lt(int fd)
    {
        if(setNonBlock(fd) == false)
            return -1;
        int                err = 0;
        struct epoll_event ev;
        memset(&ev, 0x00, sizeof(struct epoll_event));
        ev.data.fd = fd;
        ev.events  = EPOLLIN;
        err        = epoll_ctl(m_epFd, EPOLL_CTL_ADD, fd, &ev);
        if(err == -1)
        {
            zprintf1("%s\n", strerror(errno));
            return err;
        }
        return 0;
    }

    int e_poll_del(int fd)
    {
        struct epoll_event ev;
        ev.data.fd = fd;
        ev.events = EPOLLIN | EPOLLET;
        int err = epoll_ctl(m_epFd, EPOLL_CTL_DEL, fd, &ev);
        if(err == -1)
        {
            zprintf1("%s\n",strerror(errno));
            return err;
        }

        return 0;
    }

    int e_poll_del_lt(int fd)
    {
        struct epoll_event ev;
        ev.data.fd = fd;
        ev.events  = EPOLLIN;
        int err    = epoll_ctl(m_epFd, EPOLL_CTL_DEL, fd, &ev);
        if(err == -1)
        {
            zprintf1("epoll_ctl error %s\n", strerror(errno));
            return err;
        }

        return 0;
    }

    int e_poll_deactive()
    {
        m_active = 0;
        return 0;
    }
    int get_epoll_size() const
    {
        return m_epSize;
    }
    bool setNonBlock (int fd)
    {
         int flags = fcntl (fd, F_GETFL, 0);
         flags |= O_NONBLOCK;
         if (-1 == fcntl (fd, F_SETFL, flags))
         {
             zprintf1("fd%d set non block failed!\n", fd);
             return false;
         }

         return true;
    }

    int wait_fd_change(int time)
    {
        // struct epoll_event events[this->get_epoll_size()];
        // memset(&events, 0, sizeof(events));
        std::vector<struct epoll_event> events(m_epSize);

         int nfds = epoll_wait(m_epFd, events.data(), get_epoll_size(), time);
         if(nfds > 0)
         {
             return nfds;
         }
         else
             return -1;
    }
    virtual ~z_poll()
    {
        zprintf3("destory zpoll!\n");

        if(m_active)
        {

            if(m_epFd)
                close(m_epFd);
            m_active = 0;
            m_epFd = 0;
        }
    }

};

class Pth_Class
{
private:
    pthread_t m_pid;
    string    m_name;
public:
    int running;
private:
     static void * start_thread(void * arg){
            zprintf3("zty pid start!\n");
#if (__GLIBC__ > 2) || (__GLIBC__ == 2 && __GLIBC_MINOR__ >= 4)  // glibc ≥ 2.4 的代码逻辑
            zprintf3("glibc version: %d.%d!\n", __GLIBC__, __GLIBC_MINOR__);
#else
            int res = pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS,   NULL);   //设置立即取消
            if (res != 0)
            {
                perror("Thread pthread_setcancelstate failed");
                exit(EXIT_FAILURE);
            }
#endif

           static_cast<Pth_Class *>(arg)->run();
         return NULL;
     }
 public:
    explicit Pth_Class():m_pid(0),m_name(""),running(0){
        ;
     }
     virtual ~Pth_Class(){
         zprintf3("destory Pth_Class pid %d name %s!\n", (int)m_pid, m_name.c_str());
         if(m_pid > 0){
            running = 0;
            pthread_cancel(m_pid);
            pthread_join(m_pid, NULL);
            m_pid = 0;
         }
         zprintf3("destory Pth_Class delete over!\n");
     }

     int start(const string & name ="")
     {
         if(m_pid == 0)
         {
             if(pthread_create(&m_pid, NULL, start_thread,this) != 0)
             {
                 zprintf1("creat pthread failed!\n");
                 return -1;
             }
             else
             {
                 running = 1;
                 m_name = name;
                 zprintf3("zty create pid %d name %s!\n", (int)m_pid, name.c_str());
                 return 0;
             }
         }
         zprintf1("pid %d have creat\n",(int)m_pid);
         return -1;
     }

     int stop(){

         zprintf3("stop pid %d name %s!\n", (int)m_pid, m_name.c_str());
         if(m_pid > 0)
         {
             running = 0;
             pthread_cancel(m_pid);
             pthread_join(m_pid, NULL);
             m_pid = 0;
         }
         zprintf3("stop pid %d end!\n",(int)m_pid);
         return 0;
     }

     virtual void run() = 0;


};


class NCbk_Poll:public Pth_Class,public z_poll
{
public:
     explicit NCbk_Poll(int max):z_poll(max){
         ;
     }
     virtual ~NCbk_Poll(){
         zprintf3("delete NCbk_Poll!\n");
     }
};



 
#endif  /* __E_POLL_H__ */
