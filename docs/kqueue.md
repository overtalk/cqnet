# KQueue 使用

> 单路：一个IO给它专门开一个线程处理。
> 多路：多个IO复用一个线程处理。（节约线程，内存）

## kqueue 是 unix 下的一个IO多路复用库。
- 引用头文件
```c++
#include <sys/event.h>
#include <sys/types.h>
```

- 创建 kqueue，获得句柄 kq，后面要通过 kq 操作 kqueue
```c++
int kq = kqueue();
```

- 向 kqueue 中添加一个需要关心的事件
```c++
struct kevent evt;    // 创建
EV_SET(&evt, sock, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);  // 赋值
kevent(kq, &evt, 1, NULL, 0, NULL);    // 添加
```

- 上面的代码解释
- kevent 是个结构体
```c++
struct kevent {
    uintptr_t   ident;      /* identifier for this event */
    int16_t     filter;     /* filter for event */
    uint16_t    flags;      /* general flags */
    uint32_t    fflags;     /* filter-specific flags */
    intptr_t    data;       /* filter-specific data */
    void        *udata;     /* opaque user data identifier */
};

/* --- 解释：--- */
ident 是事件唯一的 key，在 socket 使用中，它是 socket 的 fd 句柄
filter 是事件的类型，有 15 种，其中几种常用的是 
      EVFILT_READ   socket 可读事件
      EVFILT_WRITE  socket 可写事件
      EVFILT_SIGNAL  unix 系统的各种信号
      EVFILT_TIMER  定时器事件
      EVFILT_USER  用户自定义的事件
flags 操作方式，EV_ADD 添加，EV_DELETE 删除，EV_ENABLE 激活，EV_DISABLE 不激活
fflags 第二种操作方式，NOTE_TRIGGER 立即激活等等
data int 型的用户数据，socket 里面它是可读写的数据长度
udata 指针类型的数据，你可以携带任何想携带的附加数据。比如对象
```

- kevent() 可以往 kqueue 里添加事件，或者监听事件，kevent() 是阻塞调用的
```c++
int kevent( int kq, 
const struct kevent *changelist, 
int nchanges, 
struct kevent *eventlist,
int nevents, 
const struct timespec *timeout);

/* --- 解释：--- */
kq 就是 kqueue 的句柄
changelist 是 kevent 的数组，就是一次可以添加多个事件
nchanges 是 changelist 数组长度

eventlist 是待接收事件的数组，里面是空的，准备给 kqueue 放数据的
nevents 是 eventlist 数组长度
传了 eventlist，kevent() 将会阻塞等待事件发生才返回，返回的全部事件在 eventlist 数组里面。

timeout 是阻塞超时时间，超过这个时间就不阻塞了，直接返回
```

- 在 kqueue 中监听事件的发生
```c++
struct kevent events[Max_Event_Count];
int ret = kevent(kq, NULL, 0, events, Max_Event_Count, NULL); 

// kevent() 是阻塞调用，等到有事件才返回。
// events 是 kqueue 返回的事件数组
```

- kqueue 可以监听 socket，文件变化，系统信号，定时器事件，用户自定义事件。

- 一般用来监听 socket 的事件，是否可读，可写等等，能处理大量socket IO 请求，性能较高，原理见操作系统中的I/O，及高性能IO模型

- 用户可以自定义一些事件来控制 kqueue 开启或退出。

- 定时器事件，GCD 的 dispatch_after 就是直接用了 kqueue 的定时器事件，可以设置间隔多少秒后做什么操作。

