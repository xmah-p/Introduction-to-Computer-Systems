- [Misc](#misc)
- [并发](#并发)
- [接口](#接口)
- [网络](#网络)
- [ECF](#ecf)
- [虚存](#虚存)
- [链接](#链接)
- [信息的表示和处理](#信息的表示和处理)
  - [数据长度](#数据长度)
  - [整数字面量](#整数字面量)
  - [`double` 不一定比 `float` 精确](#double-不一定比-float-精确)
  - [运算符优先级](#运算符优先级)
  - [负数编码](#负数编码)
  - [取模](#取模)
  - [大端小端](#大端小端)
  - [运算](#运算)
  - [未定义行为](#未定义行为)
  - [其他](#其他)
  - [浮点](#浮点)
- [程序的机器级表示](#程序的机器级表示)
  - [其他](#其他-1)

# Misc

![](images/7-15-linux运行时内存映像.png)

ELF: text, rodata, data, bss

在处理信号之前相同信号接收到多次，只按照一次计算. 另一种情况是处理程序中接收到多于一个相同信号.

主线程调用 `exit` 结束整个进程, 但调用 `pthread_exit` 只结束主线程, 不结束其他线程. 因此主线程结束不一定引发所有对等线程结束

符号表中, 块作用域的静态变量会被名字修饰 (无论是否有重名), 全局作用域的静态变量不会.

处理器芯片开发流程:

- 指令集 ISA
- 微架构设计: 指令集手册 + 微架构设计 -> 设计文档 + 工程开发 -> RTL 代码 + EDA 工具 -> 芯片版图
- 企业流片、封装、测试

微架构设计与实现是芯片设计的核心能力, 一套微架构设计可以支持多种指令集. Intel 处理器性能增强主要靠微架构演进

IP 32 位, 端口号 16 位.

Freeaddrinfo(listp)

`dup` 复制的新文件描述符与旧文件描述符指向同一个文件表条目, 所以共享文件状态和文件位置.

高速缓存每路的行数是它的组数, 不是每组的行数.

客户端通过代理给服务器发送请求, 客户端的请求源是客户端, 目的是服务器. 代理的请求源是代理, 目的也是服务器.

# 并发

第一类读者-写者问题：**读者优先**，除非已经将适用对象的权限赋予了一个写者。读者不会因为有一个写者在等待而等待。

以下是对第一类读者-写者问题的解答。

`mutex` 保护对 `readcnt` 的访问，`readcnt` 统计当前在临界区中的读者数量。

通过维护 `readcnt`，读者自动形成了一个读序列。

第一个进入临界区的读者对 `w` 加锁，阻止写者进入临界区。最后一个离开临界区的读者对 `w` 解锁，允许写者进入临界区。同时，写者也通过 `w` 阻塞其他写者。`w` 同时实现了读序列和写者的互斥以及写者之间的互斥。

```c
/* Solution to the first readers-writers problem */

/* Global variables */
int readcnt;        /* Initially = 0 */
sem_t mutex, w;     /* Both initially = 1 */

void reader(void) {
    while (1) {
        P(&mutex);
        readcnt++;
        if (readcnt == 1)    /* First in */
            P(&w);
        V(&mutex);

        /* Critical section */
        /* Reading happens */

        P(&mutex);
        readcnt--;
        if (readcnt == 0)
            V(&w);
        V(&mutex);
    }
}

void writer(void) {
    while (1) {
        P(&w);

        /* Critical section */
        /* Writing happens */

        V(&w);
    }
}
```

第二类读者-写者问题：**写者优先**，一旦有写者准备好写，它就会优先使写者完成写操作。在写者后到达的读者必须等待，即便这个写者也在等待。

写序列通过 `r` 对读序列加锁，读序列的入列操作需要强制获取 `r`，从而使得写序列优先于读序列。依然通过 `w` 实现读写互斥和写者之间的互斥。

```c
int readcnt, writecnt;       /* Initially 0 */
sem_t rmutex, wmutex, r, w;  /* Initially 1 */

void reader(void) {
    while (1) {
        P(&r);
        P(&rmutex);
        readcnt++;
        if (readcnt == 1) /* First in */
            P(&w);
        V(&rmutex);
        V(&r)

        /* Reading happens here */

        P(&rmutex);
        readcnt--;
        if (readcnt == 0) /* Last out */
            V(&w);
        V(&rmutex);
    }
}

void writer(void) {
    while (1) {
        P(&wmutex);
        writecnt++;
        if (writecnt == 1) P(&r);
        V(&wmutex);

        P(&w);
        /* Writing here */
        V(&w);

        P(&wmutex);
        writecnt--;
        if (writecnt == 0) V(&r);
        V(&wmutex);
    }
}
```

对这两种读者-写者问题的正确解答可能造成**饥饿**（starvation），即某个线程被无限期地阻塞。例如，第一类读者-写者问题中，如果有读者不断地到达，写者就可能饥饿。

狒狒过峡谷问题: 狒狒分布在峡谷的东西侧，它们都想要到达对侧。峡谷之间有一条可供通过的绳索，但同时只能有一个方向的狒狒可以在绳索上，即如果绳索上同时有向西和向东的狒狒，则会死锁。

`mutex` 表示绳索资源，`Wcnt` 和 `Ecnt` 分别表示向东和向西的狒狒的数量。`W` 和 `E` 保护对 `Wcnt` 和 `Ecnt` 的访问。

```c
sem_t W, E;    /* Initially 1 */
sem_t mutex;   /* Initially 1 */
int Wcnt = 0, Ecnt = 0;

void West() {
    while (1) {
        P(&W);
        if (Wcnt == 0) P(&mutex);
        Wcnt++;
        V(&W);

        /* Cross the rope */

        P(&W);
        Wcnt--;
        if (Wcnt == 0) V(&mutex);
        V(&W);
    }
}

void East() {
    while (1) {
        P(&E);
        if (Ecnt == 0) P(&mutex);
        Ecnt++;
        V(&E);

        /* Cross the rope */

        P(&E);
        Ecnt--;
        if (Ecnt == 0) V(&mutex);
        V(&E);
    }
}
```

三个线程 PA, PB, PC. PA 将数据从磁盘复制到 buf1, PB 将数据从 buf1 复制到 buf2, PC 将数据从 buf2 打印出来. buf1 容量为 4, buf2 容量为 8.

信号量就是资源的数量.

`slot` `used` 是记录资源数量性质的信号量. `mutex` 是保护性质的信号量. 对前者的 P 操作要求一份资源, 必须在对后者的 P 操作之前, 否则会死锁.

```c
sem_t slot1 = 4, slot2 = 8;
sem_t used1 = 0, used2 = 0;
sem_t mutex1 = 1, mutex2 = 1;

void PA() {
    // read from disk
    P(&slot1);
    P(&mutex1);    // 顺序不能错!
    // write to buf1
    V(&mutex1);
    V(&used1);
}

void PB() {
    P(&used1);
    P(&mutex1);    // 顺序不能错!
    // read from buf1
    V(&mutex1);
    V(&slot1);

    P(&slot2);
    P(&mutex2);
    // write to buf2
    V(&mutex2);
    V(&used2);
}

void PC() {
    P(&used2);
    P(&mutex2);
    // read from buf2
    V(&mutex2);
    V(&slot2);
    // print
}
```

# 接口

```c
// 返回：子进程返回 0，父进程返回子进程的 PID，出错则返回 -1
pid_t fork(void);

// 返回：若成功，返回子进程的 PID；若 WNOHANG，返回 0；若出错，返回 -1
pid_t waitpid(pid_t pid, int* statusp, int options);
// pid: -1 for all child; > 0 for pid.
// options:  WNOHANG, WUNTRACED (+ stopped), WCONTINUED (+ recontinued)
// statusp:
// WIFEXITED(status) -> WEXITSTATUS(status);
// WIFSIGNALED(status) -> WTERMSIG(status);
// WIFSTOPPED(status) -> WSTOPSIG(status);
// WIFCONTINUED(status)

// no child, return -1, errno = ECHILD
// interrupted by signal, return -1, errno = EINTR

pid_t wait(int* statusp);  // waitpid(-1, statusp, 0)

// 返回：还要休眠的秒数
unsigned int sleep(unsigned int seconds);

// 返回：永远返回 -1
int pause(void);

// 返回：若成功，不返回；若出错，返回 -1
int execve(const char* filename, const char* argv[], const char* envp[]);

// 环境变量每个形如 name=value

// 返回：若找到，返回指向 value 的指针；未找到则返回空指针
char* getenv(const char* name);    // 找 value

// 返回：若成功，返回 0；若出错，返回 -1
int setenv(const char* name, const char* newvalue, int overwrite);
// 若 name 不存在, 添加. 若存在且 overwrite 为真, 覆写. 否则不做操作

// 返回：无
void unsetenv(const char* name);  // 删除

// 返回：若成功，返回 0；若出错，返回 -1
int kill(pid_t pid, int sig);
// pid > 0, send to pid
// pid = 0, send to all processes in caller process group
// pid < 0, send to all processes in process group -pid

// 返回：前一次的待处理的闹钟剩余的秒数（如果此次 alarm 调用没有发生的话），若没有待处理闹钟，返回 0
unsigned int alarm(unsigned int secs);  // 发 SIGALRM 给调用进程

// 返回：若成功，返回指向前次处理程序的指针；若出错，返回 SIG_ERR，不设置 errno
sighandler_t signal(int signum, sighandler_t handler);  // SIGSTOP, SIGKILL 不能被捕获或忽略, 除外
// handler 可以是信号处理程序的函数指针, 也可以是 SIG_IGN, SIG_DFL

// 返回：若成功，返回 0；若出错，返回 -1
int sigprocmask(int how, const sigset_t* set, sigset_t* oldset);
// how: SIG_BLOCK, SIG_UNBLOCK, SIG_SETMASK
// 加锁: SIG_BLOCK, 解锁: SIG_SETMASK

write 函数是唯一安全的在信号处理程序中产生输出的函数。

// 返回：若直接调用，返回 0；若从 longjmp 返回，返回非 0
int setjmp(jmp_buf env);

// 返回：返回到 setjmp
void longjmp(jmp_buf env, int val);

// setjmp 和 longjmp 的可以被信号处理程序使用的版本
int sigsetjmp(sigjmp_buf env, int savesigs);
void siglongjmp(sigjmp_buf env, int val);

// 创建新的虚拟内存区域，并将 fd 所指定对象 offset 偏移处起 length 字节长度的连续的片映射到此区域
// 返回：若成功，返回指向映射区域的指针；若出错，返回 MAP_FAILED（-1）
void* mmap(void* start, size_t length, int prot, int flags, int fd, off_t offset);
// start 起始地址, 建议
// prot: PROT_READ, PROT_WRITE, PROT_EXEC, PROT_NONE (不可访问)
// flags: MAP_SHARED, MAP_PRIVATE, MAP_ANON. 描述共享对象类型

// 删除 start 起 length 字节长度的区域
// 返回：若成功，返回 0；若出错，返回 -1
int munmap(void* start, size_t length);


// 打开路径名 filename 的文件
// 返回：若成功，返回文件描述符；若出错，返回 -1
int open(char* filename, int flags, mode_t mode);
// flags:
// O_RDONLY, O_WRONLY, O_RDWR
// 可以或上: O_CREAT, O_TRUNC, O_APPEND
// mode 指定创建文件时的访问权限
// 访问权限: S_I[R, W, X][USR, GRP, OTH]
umask(DEF_UMASK);
fd = open("foo", O_CREAT | O_TRUNC | O_WRONLY, DEF_MODE);
// permissions: DEF_MODE & ~DEF_UMASK

// 从 fd 的当前文件位置复制至多 n 字节到 buf
// 返回：实际读到的字节数。若 EOF 则返回 0，若出错则返回 -1
ssize_t read(int fd, void* buf, size_t n);

// 从 buf 复制至多 n 字节到 fd 的当前文件位置
// 返回：实际写入的字节数。若出错则返回 -1
ssize_t write(int fd, const void* buf, size_t n);

// 这两个函数分别以路径名和文件描述符为输入，将文件元数据填充在 buf 参数中
// 返回：若成功则返回 0，若出错则返回 -1
int stat(const char* filename, struct stat* buf);
int fstat(int fd, struct stat* buf);
// st_size 字节数大小
// st_mode
mode_t m = buf.st_mode;

// 确定 st_mode 指示的文件类型的宏谓词
S_ISREG(m)    // is this a regular file?
S_ISDIR(m)    // is this a directory file?
S_ISSOCK(m)    // is this a network socket?

// 打开目录
// 返回：若成功则返回指向目录流的指针，若出错则返回 NULL
DIR* opendir(const char* name);

// 目录的项
struct dirent {
    ino_t d_ino;       // inode number
    char d_name[256];  // null-terminated filename
    // ...
};

// 读取目录中的下一项
// 返回：指向目录中下一项的指针，若已到达目录尾或出错则返回 NULL
// 如果出错，还会设置 errno
struct dirent* readdir(DIR* dirp);

// 关闭目录流并释放资源
// 返回：若成功则返回 0，若出错则返回 -1
int closedir(DIR* dirp);

// 用描述表表项 oldfd 覆写描述符表表项 newfd
// 如果 newfd 已经打开，dup2 会在覆写前将其关闭
// 返回：若成功则返回 newfd，若出错则返回 -1 并设置 errno
int dup2(int oldfd, int newfd);

struct in_addr {
    uint32_t s_addr;  // 32-bit IPv4 address (big-endian)
};

uint32_t htonl(uint32_t hostint32);  // host to network long
uint16_t htons(uint16_t hostint16);  // host to network short

uint32_t ntohl(uint32_t netint32);   // network to host long
uint16_t ntohs(uint16_t netint16);   // network to host short

// there's no 64-bit versions of these functions

// IP 地址和点分十进制转换
// 返回：若成功则返回 1，若 src 非法则返回 0，若出错则返回 -1
int inet_pton(AF_INET, const char* src, void* dst);

// size 规定了向 dst 复制的字节数的上限，得到的字符串以 NULL 结尾
// 返回：若成功则返回指向 dst 的指针，若出错则返回 NULL
const char* inet_ntop(AF_INET, const void* src, char* dst, socklen_t size);

// IP socket address structure
struct sockaddr_in {
    uint16_t sin_family;  // Protocol family (always AF_INET)
    uint16_t sin_port;    // Port number in network byte order
    struct in_addr sin_addr;  // IP address in network byte order
    unsigned char sin_zero[8]; // Pad to sizeof(struct sockaddr)
};

// Generic socket address structure (for connect, bind, and accept)
struct sockaddr {
    uint16_t sa_family;  // Protocol family
    char sa_data[14];    // Address data
};

typedef struct sockaddr SA;

// 客户端和服务器用 socket 创建 socketfd
// 返回：若成功则为非负描述符，若出错则为 -1
int socket(int domain, int type, int protocol);
// domain: AF_INET for ipv4
// type: SOCK_STREAM for TCP connect, SOCK_DGRAM for UDP

// 服务器用 bind 绑定 socket 地址和 sockfd
// 返回：若成功则为 0，若出错则为 -1，并设置 errno
int bind(int sockfd, const struct sockaddr* addr, socklen_t addrlen);

// 返回：若成功则为 0，若出错则为 -1
int connect(int clientfd, const struct sockaddr* addr, socklen_t addrlen);

// 将 sockfd 由主动套接字转换为监听套接字
// 返回：若成功则为 0，若出错则为 -1
int listen(int sockfd, int backlog);
// backlog: 监听队列长度

// 返回：若成功则为非负描述符，若出错则为 -1
int accept(int listenfd, struct sockaddr* addr, socklen_t* addrlen);

// 返回：若成功则为 0，若出错则为 -1
int close(int fd);
// 如果 fd 是 TCP 套接字描述符，调用 close 会引起本地进程向远程进程发送关闭连接的消息，连接关闭后，相关的资源被释放。

struct addrinfo {
    int ai_flags;           // Hints argument flags
    int ai_family;          // First arg to socket function
    int ai_socktype;        // Second arg to socket function
    int ai_protocol;        // Third arg to socket function
    char* ai_canonname;     // Canonical hostname
    size_t ai_addrlen;      // Size of ai_addr struct
    struct sockaddr* ai_addr;   // Ptr to socket address structure
    struct addrinfo* ai_next;   // Next in linked list
};

// 返回：若成功则为 0，若出错则为非 0 的错误代码
int getaddrinfo(const char* host, const char* service,
                const struct addrinfo* hints, struct addrinfo** result);
// host 参数可以是域名，也可以是点分十进制的 IP 地址。service 参数可以是服务名（如 http），也可以是十进制端口号。如果不想将主机名转换成地址，可以将 host 设为 NULL；如果不想将服务名转换成端口号，也可以将 service 设为 NULL，但不能同时将两者都设为 NULL。
// hints 可选, 它只能设置以下字段:
// ai_family: AF_INET for ipv4, AF_INET6 for ipv6
// ai_socktype: SOCK_STREAM 每个地址至多返回一个对应于 TCP 连接的 addrinfo 结构体
// ai_flags:
// AI_ADDRCONFIG: 只返回适用于主机(ipv4/ipv6)的地址
// AI_CANONNAME: 将链表 中首个 addrinfo 的 ai_canonname 置为主机的规范名
// AI_NUMERICSERV: 强制 service 参数是端口号
// AI_PASSIVE: getaddrinfo 返回的默认为套接字地址，客户端可以在调用 connect 时将其用作主动套接字。设置此标志令 getaddrinfo 返回可以被服务器用作监听套接字的通配符地址（wildcard address），告诉内核这个服务器会接受发送到该主机所有 IP 地址的请求。此时，host 参数应为 NULL
// ai_protocol

// 返回：若成功则为 0，若出错则为非 0 的错误代码
int getnameinfo(const struct sockaddr* sa, socklen_t salen,
                char* host, size_t hostlen,
                char* service, size_t servlen,
                int flags);



// 返回：无
void freeaddrinfo(struct addrinfo* result);

// 返回：错误消息
const char* gai_strerror(int errcode);

// 返回：若成功则为描述符数目，若出错则为 -1
int select(int n, fd_set* fdset, NULL, NULL, NULL);
// n: 描述符集合位向量的长度, 描述符的最大值
// 阻塞直到 fdset 的某个 fd 准备好读, fdset 被修改成准备好集合, 返回准备好集合的基数

pthread_t tid;
Pthread_create(&tid, NULL, thread, NULL);
Pthread_join(tid, NULL);
// 返回：调用线程的 TID
pthread_t pthread_self(void);
// 无返回值
void pthread_exit(void* thread_return);
// 主线程调用 pthread_exit, 会等待所有对等线程终止, 然后终止主线程和整个进程
// 返回：若成功则为 0，若出错则非零
int pthread_detach(pthread_t tid);

pthread_once_t once_control = PTHREAD_ONCE_INIT;

// 返回：总是 0
int pthread_once(pthread_once_t* once_control, void (*init_routine)(void));
```

```c
// 返回：若成功则为套接字描述符，若出错则为 -1
int open_clientfd(char* hostname, char* port) {
    int clientfd;
    struct addrinfo hints
    struct addrinfo* listp;
    struct addrinfo* p;

    /* Get a list of potential server addresses */
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_socktype = SOCK_STREAM;  /* Open a connection */
    hints.ai_flags = AI_NUMERICSERV;  /* ... using a numeric port arg. */
    hints.ai_flags |= AI_ADDRCONFIG;  /* Recommended for connections */
    Getaddrinfo(hostname, port, &hints, &listp);

    /* Walk the list for one that we can successfully connect to */
    for (p = listp; p; p = p->ai_next) {
        /* Create a socket descriptor */
        if ((clientfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0)
            continue;  /* Socket failed, try the next */

        /* Connect to the server */
        if (connect(clientfd, p->ai_addr, p->ai_addrlen) != -1)
            break;  /* Success */

        Close(clientfd);  /* Connect failed, try another */
    }

    /* Clean up */
    Freeaddrinfo(listp);
    if (!p)  /* All connects failed */
        return -1;
    else  /* The last connect succeeded */
        return clientfd;
}

// 返回：若成功则为监听描述符，若出错则为 -1
int open_listenfd(char* port) {
    struct addrinfo hints;
    struct addrinfo* listp;
    struct addrinfo* p;
    int listenfd, optval = 1;

    /* Get a list of potential server addresses */
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_socktype = SOCK_STREAM;  /* Accept connections */
    hints.ai_flags = AI_PASSIVE | AI_ADDRCONFIG;  /* ... on any IP address */
    hints.ai_flags |= AI_NUMERICSERV;  /* ... using port number */
    Getaddrinfo(NULL, port, &hints, &listp);

    /* Walk the list for one that we can bind to */
    for (p = listp; p; p = p->ai_next) {
        /* Create a socket descriptor */
        if ((listenfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0)
            continue;  /* Socket failed, try the next */

        /* Eliminates "Address already in use" error from bind */
        Setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (const void*)&optval, sizeof(int));

        /* Bind the descriptor to the address */
        if (bind(listenfd, p->ai_addr, p->ai_addrlen) == 0)
            break;  /* Success */

        Close(listenfd);  /* Bind failed, try the next */
    }

    /* Clean up */
    Freeaddrinfo(listp);
    if (!p)  /* No address worked */
        return -1;

    /* Make it a listening socket ready to accept connection requests */
    if (listen(listenfd, LISTENQ) < 0) {
        Close(listenfd);
        return -1;
    }
    return listenfd;
}
```

# 网络

应用层: HTTP, DNS, SMTP
传输层: TCP, UDP
网络层: IP
网络接口层: MAC

A 类地址: 网络位第一个 bit 为 0. 0.0.0.0 ~ 127.255.255.255. 127 是回环地址.

B 类地址: 网络位前两个 bit 为 10. 128.0.0.0 ~ 191.255.255.255

C 类地址: 网络位前三个 bit 为 110. 192.0.0.0 ~ 223.255.255.255

私有 IP 地址的范围有：

A 类: 10.0.0.0-10.255.255.255. 10.0.0.0/8

B 类: 172.16.0.0—172.31.255.255. 172.16.0.0/12

C 类: 192.168.0.0-192.168.255.255. 192.168.0.0/16

0.0.0.0: 所有不清楚的主机和目的网络

255.255.255.255: 对本机来说, 这个地址指本网段内的所有主机. 这个地址不能被路由器所转发.

当主机号全为 0 时, 表示一个网络本身.
当主机号全为 1 时, 表示网络中的全部主机.
网络号代表一个网络本身, 同时也是一个网段的第一个地址. 广播地址是网段的最后一个地址,这两个地址是不能配在主机上的.

对于主机, 网络是网络适配器提供的 IO 设备 DMA 传送

LAN 局域网: 以太网是流行实现. 以太网段包括一些电缆和一个集线器. 主机的网络适配器通过电缆连接到集线器的端口. 集线器将一个端口接收到的每个比特都广播到其他端口.

每个以太网适配器有一个全球唯一 48 位地址, MAC 地址. 每个帧包括固定长头部标识源和目的和长度, 以及有效载荷. 每个主机适配器都能看到帧, 但只有目的主机读取它.

用电缆和网桥将以太网段连成大的局域网, 称为桥接以太网. 网桥有选择性.

多个不一定兼容的局域网可以用路由器连接, 组成 internet.

internet 协议给每个主机定义了至少一个 internet address, 异于每个主机的局域网地址. (命名机制)

internet 协议定义了**包**作为数据传送的格式. 包头+有效载荷 (传送机制)

![](images/11-7-internet主机通信.png)

主机 A 客户端向主机 B 服务器发送数据: A 的客户端进程进行系统调用, 传送数据到内核, 协议软件将数据附加 internet 包头和 LAN1 帧头, 传送到主机的 LAN1 网络适配器, 网络适配器将它复制到网络, 被路由器接收. 路由器的 LAN1 适配器将它传送到路由器的协议软件, 从 internet 包头读取目的, 剥去 LAN1 帧头, 附加 LAN2 帧头, 发到路由器的 LAN2 适配器, 它通过网络发到主机 B. 主机 B 适配器接收到它, 传送给协议软件, 剥落 LAN2 帧头, 从 internet 包头读取源, 得到数据.

Internet 是 internet 的实现.

每个 Internet 主机运行实现 TCP/IP 协议的软件. IP 的包也就数据报 datagram. IP 丢包不恢复, 所以不可靠. UDP 扩展了 IP, 使得包在进程间传送, 而不是主机之间. TCP 在 IP 之上, 提供了进程间的全双工连接.

localhost = 127.0.0.1

nslookup 命令查询域名的 IP 地址, hostname 命令查询主机域名

Internet 客户端和服务器通过连接来通信. 连接点对点 全双工 可靠.

套接字是连接的端点. 套接字地址: address:port

进程发起连接请求时, 内核分配临时端口 1024~5000

服务器套接字知名端口. Web 80, 知名服务名 http, 电子邮件 25, 知名服务名 smtp

连接: 两个套接字地址组成套接字对. (cliaddr:cliport, servaddr:servport)

MIME 类型:

text/html：HTML 页面
text/plain：纯文本
application/postscript：PostScript 文档
image/jpeg：JPEG 图像
image/gif：GIF 图像
image/png：PNG 图像

HTTP 响应状态码:

200 OK
301 Moved Permanently
400 Bad Request
403 Forbidden
404 Not Found
501 Not Implemented
505 HTTP Version Not Supported

GET /cgi-bin/adder?15000&213 HTTP/1.1
? 分隔文件名和参数, & 分隔参数

写已经关闭的连接, 第一次写正常返回, 第二次写触发 SIGPIPE, 如果捕获或忽略它, 返回-1, errno = EPIPE

线程上下文（thread context），包括线程 ID（Thread ID，TID）、栈、栈指针、程序计数器、通用目的寄存器和条件码。

并行程序的加速比：$S_p = \frac{T_1}{T_p}$. 单核时间除以 p 核时间. 单核时间是顺序版本时间时, 是绝对加速比

效率: 加速比除以核数.

线程不安全: 不保护共享变量, 利用跨越多个调用的状态, 返回静态变量指针, 调用线程不安全函数

返回静态变量指针的, 可以改成从参数传入结果指针, 也可以每次调用加锁 (但这得深复制)

可重入: 多线程调用时不访问任何共享数据. 所有参数值传递且所有变量都是自动局部变量, 则显式可重入.

printf, scanf, malloc, free 是线程安全的.

printf、sprintf、malloc、exit 不是异步信号安全的.

getaddrinfo, getnameinfo 是可重入的.

互斥锁加锁顺序规则：给定所有互斥操作的一个全序，如果每个线程都以同一种顺序加锁，并以相反的顺序解锁，那么就不会发生死锁。

# ECF

非负整数异常号.

异常表存 (异常号: 异常处理程序地址) 对.

异常表基址寄存器 + 异常号 \* 8 = 异常处理程序地址

中断是异步的. 陷阱, 故障, 终止是同步的.

中断和陷阱返回到下一条指令. 故障可能返回. 终止不返回.

0: divide error. 故障. Floating exception.

13: general protection fault. 故障. Segmentation fault.

14: page fault. 故障.

18: machine check. 致命硬件错误. 终止.

系统调用参数用寄存器传递. rax 存系统调用号, rdi, rsi, rdx, rcx, r8, r9 依次存参数. rcx r11 会被破坏

wait 函数!

```c
// 返回：若成功，返回子进程的 PID；若 WNOHANG，返回 0；若出错，返回 -1
pid_t waitpid(pid_t pid, int* statusp, int options);
```

pid: > 0, 等待集合是 pid 对应的子进程. -1, 等待集合是所有子进程.

options: 0: 挂起, 直到等待集合任意子进程终止, 返回终止子进程 pid; WNOHANG: 不挂起, 如果没有已经终止的子进程立即返回零; WUNTRACED: 挂起, 直到等待集合任意子进程终止或被停止; WCONTINUED: 挂起, 直到等待集合任意子进程终止或已停止子进程收到 SIGCONT 信号恢复执行

它们可以用或组合

statusp 如果非空, 会存放返回子进程的状态: WIFEXITED, WEXITSTATUS. WIFSIGNALED (由于未被捕获的信号终止), WTERMSIG (导致终止的信号编号). WIFSTOPPED (子进程是因为停止而被返回的), WSTOPSIG (导致停止的信号编号). WIFCONTINUED (子进程是因为收到 SIGCONT 信号而被返回的)

无子进程, 返回 -1, errno = ECHILD. waitpid 被信号中断, 返回 -1, errno = EINTR

wait 函数只有一个 statusp 参数, 等待集合为所有子进程, options 为 0

sleep 参数是秒.

main 开始执行时, 用户栈由上到下是: 环境变量字符串, 命令行参数字符串, 环境变量指针数组 (const char* envp[]), 命令行参数指针数组 (const char* argv[]), 一些别的东西, libc_start_main 的栈帧, main 的栈帧.

ctrl+c SIGINT, ctrl+\ SIGQUIT, ctrl+z SIGTSTP

SIGINT 默认终止. 子进程终止时, 内核发送 SIGCHLD 给父进程.

发送第一个信号: 信号处理程序. 信号处理程序中发第二遍: pending 置位. 信号处理程序中发第三遍: 被忽略.

信号行为: 终止, 终止并转储内存, 停止, 忽略, 捕获并执行信号处理程序

用 signal 函数更改信号行为. **SIGSTOP** 和 **SIGKILL** 除外, 它们不能被捕获或忽略.

处理程序里只应该调用异步信号安全函数: 可重入的, 或不能被信号处理程序中断的. printf, sprintf, malloc, exit 都不行. 输出只能用 write.

处理程序要保存和恢复 errno, 以免影响主程序.

访问全局数据结构时, 阻塞所有信号 (加锁)

volatile 声明全局变量, 令编译器强制从内存读取.

sig_atomic_t 声明标志. 处理程序写它记录收到信号, 主程序周期性读它响应信号, 并清除它. sig_atomic_t 保证原子性. 但是 flag++ 等操作可能不是原子的.

某些系统中, signal 函数语义不同, 信号处理程序返回后会把自己卸载.

某些系统中, 慢速系统调用 `read`, `write`, `accept` 被信号处理程序中断后, 可能不会继续, 而是返回 -1, errno = EINTR. 用户需要手动重启它.

用 sigaction 函数明确信号处理语义. 它被包装在 Signal 函数

进程用 sigsuspend 显式等待信号.

键入 Ctrl+C 会使内核向前台进程组中的每个进程发送一个 SIGINT 信号，默认地，这导致前台作业终止。

键入 Ctrl+Z 会使内核向前台进程组中的每个进程发送一个 SIGTSTP 信号，默认地，这导致前台作业停止（挂起）

# 虚存

core i7 PTE:

前三级:

P: present. 是否已缓存在物理内存中. 有效位

R/W: read/write. 是否可读写

U/S: user/supervisor. 用户/内核

WT: write-through. 直写还是写回

CD: cache disable. 禁止缓存

A: accessed. 是否被访问过

PS: page size. 页大小. 只对第一级 PTE 定义, 4kb/4mb

Base address: 页的物理地址的高 40 位

XD: execute disable. 禁止执行

前三级 PTE 没有 D 位, 但是有 G 位.

第四级

P: present. 是否已缓存在物理内存中. 有效位

R/W: read/write. 是否可读写

U/S: user/supervisor. 用户/内核

WT: write-through. 直写还是写回

CD: cache disabled. 禁止缓存

A: accessed. 是否被访问过

D: dirty. 是否被写过

G: global. 是否全局. 全局的话, 任务切换时不会被 TLB 驱逐

Base address: 页的物理地址的高 40 位

XD: execute disabled. 禁止执行

Linux 进程的虚拟内存 layout: 最上面是内核 VM, 有页表, task 结构体, mm 结构体, 内核栈, 物理内存, 被所有进程共享的内核数据和代码. 下面是用户栈 共享库 堆 .bss .data .text

每个进程有一个 task_struct, 里面有 pid, 用户栈指针, 可执行目标文件名, PC.

task_struct 里还有一个 mm_struct\* mm.

mm_struct 包括第一级页表基址 pgd. 还有一个 vm_area_struct\* mmap.

vm_area_struct 是一个链表. 包括 vm_end, vm_start, 区域内页的读写权限 vm_prot, 是否可共享等信息 vm_flags, vm_next.

MMU 触发缺页异常后, 内核缺页处理程序检查虚拟地址是否合法 (检查 vm_area_struct 链表, 是否包含在某个区域内), 是否有权限, 然后换入页

内存映射时, 区域如果比文件节更大，那么余下的部分被零初始化

匿名文件由内核创建，包含的全是二进制零。CPU 第一次引用被映射到匿名文件的区域时，内核会在物理内存中牺牲一个页（若修改过，写回它），用二进制零覆写它并更新页表，并令这个页驻留内存

虚拟页在交换文件之间换来换去

进程对私有对象的修改不影响磁盘的原始对象, 私有对象 COW. 私有区域的 PTE 只读, 区域结构体 vm_area_struct 被标记为私有的 COW. 进程写自己私有区域的页时, 触发保护故障, 处理程序创建一个新页, 从原始页复制内容, 更新页表, 恢复这个新页的写权限, 然后返回. 故障指令重新执行, 这一次会写成功.

fork 函数被调用时, 它创建一份父进程的 mm_struct, vm_area_struct 和页表的副本, 然后把两个进程的每个页都标记成只读, vm_area_struct 标记为 COW. 父进程和子进程各自的页表都指向原始页. 从而使得父子进程拥有独立且私有的虚拟地址空间

execve 函数被调用时, 它把父进程的用户部分的 vm_area_struct 丢弃, 创建新的私有, COW 的 vm_area_struct, 代码段和数据段分别是 .text 和 .data. .bss, 请求二进制零, 匿名文件大小包含在 a.out. 栈, 堆请求二进制零. 初始长度 0. 然后设置 PC 到 \_start

mmap 用户级内存映射

stdin: 0, stdout: 1, stderr: 2

描述符表 descriptor table: 每个进程都有独立的. 索引是文件描述符, 表项是文件表的指针

文件表 file table: 所有进程共享. 包括当前文件位置, 引用计数, 和 vnode 表项指针

v-node 表 所有进程共享 包括元信息 st_mode, st_size 等

fork 创建的子进程会继承父进程的 描述符表, 这导致文件表中的相应引用计数增加.

重复打开文件, 会得到不同的文件描述符, 它们指向不同的文件表项, 但是指向同一个 v-node 表项

dup2 函数可以把一个文件描述符复制到另一个文件描述符, 会导致文件表中的相应引用计数改变

# 链接

![](images/7-3典型ELF格式.png)

字符串常量, `switch` 跳转表在 `.rodata`

`.symtab` 是符号表.

`.rel.text` 和 `.rel.data` 是重定位表

`.debug` 里的条目包含局部变量, `typedef`, 定义和引用的全局变量, 原始的 C 源代码. 使用 `-g` 编译时出现

`.line` 原始 C 源代码的行号信息. 使用 `-g` 编译时出现

`.strtab` 字符串表. 包括符号表和 `.debug` 中的符号名, 节头部表的节名.

符号表条目包括名字字符串的偏移, 类型, 全局/局部, 节索引, 符号定义地址到其所在节的偏移 (value), 大小 (size).

`ABS` 文件名, `UNDEF` 引用但未定义, `COMMON` (默认没有) 试探性定义的全局符号 (value 字段对齐要求, size 最小大小).

只声明过但未调用的函数不会出现在符号表中

名字若在符号表中，则它一定也在 .strtab 字符串表中

符号表中，重名静态局部变量会被加上后缀 .0、.1、.2 等，以作区分

链接器使用静态库解析引用时只扫一遍, 所以库必须放在最后.

重定位合并节, 并为引用填入运行时地址.

重定位条目包括引用地址到其所在节的偏移, 类型 (PC32/绝对), 符号表索引, addend. 对于 PC32 引用, addend 补齐 "引用所在指令的下一条指令的地址与引用地址的差", 等于待填入地址的长度的相反数. 对于绝对引用, addend 是零

程序头部表描述加载时文件和内存之间的映射. ELF 头, 程序头部表, `.init`, `.text`, `.rodata` 都在内存的代码段 (code segment). `.data`, `.bss` 在数据段 (data segment).

对于任何段, 链接器为它所选择的内存地址 `vaddr` 必须和它在文件中的偏移 `off` 模 `align` 同余. `align` 是程序头部表中的段对齐要求, 一般是 2\*\*21 = 0x200000

![](images/7-14-程序头部表.png)

ELF 头中的 `e_entry` 字段指示程序入口地址.

启动: `_start`, `_libc_start_main`, `main`

`_start` 在系统文件 `ctrl.o` 定义. `_libc_start_main` 在 `libc.so` 中定义.

共享库: 后缀 `.so`

创建共享库 `gcc -shared -fpic -o libfoobar.so foo.c bar.c`

如果可执行文件 `my-prog` 需要链接一个动态链接库, 那么它会包含一个 `.intern` 节, 其中有动态链接器的路径名.

加载器不会像通常一样将控制传递给应用，而是加载和运行动态链接器，然后动态链接器完成对 `libfoobar.so` 和 `libc.so` 的重定位.

运行时动态链接: `dlopen`, `dlsym`, `dlclose`, `dlerror`.

`dlopen` 接受文件名和 `flag` 参数. RTLD_NOW 立即解析引用, RTLD_LAZY 延迟解析引用到首次执行库中代码, 它们可以与 RTLD_GLOBAL 取或, 使得库中的符号对后续打开的库可见.

gcc 编译选项 `-rdynamic` 使得可执行文件里的全局符号对于动态链接器可见

```bash
gcc -rdynamic -o my-prog dll.c -ldl
```

位置无关代码: 无需重定位即可加载的代码. 让多个进程共享内存中相同的一份代码.

gcc 编译选项 `-fpic` 生成位置无关代码.

数据段 data segment 中有一个 GOT. GOT 条目是目标的 8 字节绝对地址

数据引用: 对数据的引用指令中, 地址被重定位为对应 GOT 条目的地址, 形如 `movq foo(%rip), %rax`

```c
mov    0x2008b9(%rip),%rax        # 0x601018 <x>
```

函数调用: call 的参数是 PLT 表条目的地址, PLT 表条目是一段代码:

```c
// PLT[2]
jmpq    *GOT[4]     // 初始时 GOT[4] 就是这条指令的下一条指令 (pushq) 的地址, 此后 GOT[4] 被重定位为目标函数的地址
pushq   $0x1    // 这是目标函数的 ID
jmpq    0x4005a0   // 这是 PLT[0] 的地址
```

PLT[0] 跳转到动态链接器. PLT[1] 跳转到 `_libc_start_main`. PLT[2] 及以后跳转到用户函数

GOT[0] 和 GOT[1] 包含动态链接器在解析函数地址时会使用的信息. GOT[2] 是动态链接器在 ld-linux.so 模块中的入口点, 对应 PLT[0]. 其余的 GOT 条目是函数地址. 初始时, 它们指向对应的 PLT 条目的第二条指令.

打桩.

# 信息的表示和处理

## 数据长度

|   类型    | Intel IA32 | x86-64 |
| :-------: | :--------: | :----: |
|   char    |     1      |   1    |
|    int    |     4      |   4    |
|   long    |   **4**    | **8**  |
| long long |   **8**    | **8**  |
|   short   |     2      |   2    |
|   float   |     4      |   4    |
|  double   |     8      |   8    |
|  pointer  |   **4**    | **8**  |

Intel IA32 要求 `double` 和 `long long` 对齐到 4 字节边界，带 `double` 和 `long long` 的结构体也要对齐到 4 字节边界。

## 整数字面量

整数是以数字开头的。`-2147483647` 是一个常量表达式。

字面量推导的规则：

- ISO C99：
  - 十进制：`int`、`long`、`long long`（**永远都会是有符号数**）
  - 十六进制：`int`、`unsigned`、`long`、`unsigned long`、`long long`、`unsigned long long`
- ISO C90：
  - 十进制：`int`、`long`、`unsigned`、`unsigned long`
  - 十六进制：`int`、`unsigned`、`long`、`unsigned long`

`-2147483648` 中的 `2147483648` 参与类型推导。它在 ISO C90 32 位的机器上会被推导为 `unsigned`；在 ISO C90 64 位的机器上会被推导为 `long`；而在 ISO C99 的机器上被推导为 `long long`（32 位）和 `long`（64 位）。之后，`-2147483648` 作为一个常量表达式被运算。在被推导为 `unsigned` 的情况下，$-2147483648 == 2^32 - 2147483648 = 2147483648$。

`2147483647 + 1 < 2147483648` 在 ISO C90 32 位下为假，因为右边的 `2147483648` 被推导为 `unsigned` 类型。

x86-64 里，`char` 一般是有符号的。

## `double` 不一定比 `float` 精确

```c
float f1 = 0.1, f2 = 0.2, f3 = 0.3;
double d1 = 0.1, d2 = 0.2, d3 = 0.3;
f1 + f2 == f3;    // true
d1 + d2 == d3;    // false
```

1. `0.1` 和 `0.2` 和 `0.3` 在二进制下都是无限循环小数 循环节长是 4
2. `float` 和 `double` 的 `frac` 一个是 4 的倍数 一个不是
3. 因此在末尾的舍入可能导致不同的行为

## 运算符优先级

| 优先级 |                                       运算符                                       |  结合律  |
| :----: | :--------------------------------------------------------------------------------: | :------: |
|   1    |                   后缀运算符：`()`、`[]`、`.`、`->`、`++`、`--`                    | 从左到右 |
|   2    |     一元运算符：`++`、`--`、`!`、`~`、`+`、`-`、`*`、`&`、`sizeof`、`_Alignof`     | 从右到左 |
|   3    |                                   类型转换运算符                                   | 从右到左 |
|   4    |                            乘除法运算符：`*`、`/`、`%`                             | 从左到右 |
|   5    |                             **加减法运算符**：`+`、`-`                             | 从左到右 |
|   6    |                             **移位运算符**：`<<`、`>>`                             | 从左到右 |
|   7    |                        **关系运算符**：`<`、`<=`、`>`、`>=`                        | 从左到右 |
|   8    |                             **相等运算符**：`==`、`!=`                             | 从左到右 |
|   9    |                               **按位与运算符**：`&`                                | 从左到右 |
|   10   |                              **按位异或运算符**：`^`                               | 从左到右 |
|   11   |                               **按位或运算符**：`\|`                               | 从左到右 |
|   12   |                               **逻辑与运算符**：`&&`                               | 从左到右 |
|   13   |                              **逻辑或运算符**：`\|\|`                              | 从左到右 |
|   14   |                                **条件运算符**：`?:`                                | 从右到左 |
|   15   | **赋值运算符**：` =``+= `、`-=`、`*=`、`/=`、`%=`、`<<=`、`>>=`、`&=`、`^=`、`\|=` | 从右到左 |
|   16   |                                **逗号运算符**：`,`                                 | 从左到右 |

## 负数编码

原码（Sign-Magnitude）：最高位决定符号，其余位不变（IEEE 754 浮点数）
反码（One's Complement）：最高位决定符号，若最高位为 `1` 则其余位取反

## 取模

`a % b = a - a / b * b`

`-9 % 4 == -1`，而不是 3。

## 大端小端

**x86-64 是小端法**

```c
// Small endian
char* s = "2018";    // 0x32 0x30 0x31 0x38
int* p = (int*)s;
// *p == 0x38313032!
short s1 = (*p) >> 12;    // 3 hex digits!
unsigned u = s1;
printf("0x%x\n", u);    // 0xffff8313
```

## 运算

（`x` `y` 为 `int`，`ux` `uy` 为 `unsigned`）

**异或有交换律和结合律**

`x ^ y ^ x == y`
`x ^ y ^ x ^ y ^ x == x`
`x ^ y ^ (~x) - y == y ^ x ^ (~y) - x`

`((x >> 1) << 1) <= x` 恒为真

## 未定义行为

- 非良构（ill-formed）
- 非良构而不要求诊断（ill-formed no diagnostic required）
- 实现定义行为（implementation-def）
- 未定义行为（undefined behavior, ined behavior）
- 未指明行为（unspecified behaviorUB）

编译器假设程序永远不会包含未定义行为。

**`char` 类型有无符号是未定义行为**

**有符号整数溢出是未定义行为**，无符号整数溢出不是未定义行为

**解引用空指针是未定义行为**

**访问未初始化变量是未定义行为**

C 标准不强制 IEEE 754，（也不强制补码）因此除以零是未定义的。

IEEE 754 规定除以零得无穷，`0/0` 除外，结果为 NaN。

```c
char *p = "wikipedia"; // valid C, deprecated in C++98/C++03, ill-formed as of C++11
p[0] = 'W'; // undefined behavior
```

```c
// Meaningless checks from real code:
// pointer p; signed int x;
if (p + 100 < p)    // ptr overflow
if (x + 100 < x)    // signed int overflow
if (!(1 << x))      // oversized shift
*p; if (p)          // null pointer dereference
if (abs(x) < 0)     // Absolute value overflow
```

## 其他

当 $x=\operatorname{TMin}_{32}$ 时，$-x=x$. 这同时说明 $\operatorname{Tmin}+\operatorname{Tmin}=0$

IA32 下，`long` 和 `int` 都是 4 字节的，x86-64 中，`long` 是 8 字节的，`int` 是 4 字节的。

IA32 架构中，`((-9)>>1) + sizeof(long) > 0`

- **右移运算向下舍入**，`(-9)>>1 == -5`
- `sizeof(long) == 4`
- `sizeof` 的**返回值是无符号数，参与的运算结果必定非负**

```c
int x = 42;
char c = 'a';
&x - &c;    // error!
// &x 的类型是 int*, &c 的类型是 char*
// 它们不是 ptr to compatible type, 不允许相减

int y = 56;
&x - &y;    // 结果并不是地址差 而是 (地址差) / sizeof(int)！

void* ptr;
ptr + 2;    // 地址增加 2 字节, 即 void* 的 sizeof 是 1（未定义）
```

```c
// C 中, 字符字面量的类型是 int
printf("sizeof char: %lld\n", sizeof (char));    // 1
char a = 'a';
printf("sizeof a: %lld\n", sizeof a);   // 1
printf("sizeof 'a': %lld", sizeof 'a');    // 4
```

## 浮点

浮点数计算，先写成 $(-1)^s 2^{e-\operatorname{Bias}} M$ 的形式

**NaN 是无序的。有 NaN 参与的运算结果为 `false`**。NaN 和任何数不会比较相等，包括自身。

浮点加法有交换律，但是 `a + b == b + a` 在 `a` 和 `b` 都不是 NaN 的时候仍不一定成立。**因为 `-inf + inf == inf + (-inf)` 返回 `0`**。

若 `d < 0`，则 `d * 2` 也小于 `0`。但是 `d * d` 不一定大于 `0`，因为 `d * d` 可能为 `0`。

# 程序的机器级表示

| 四字（64 位） | 双字（32 位） | 字（16 位） | 字节（8 位） |     用途     |
| :-----------: | :-----------: | :---------: | :----------: | :----------: |
|    `%rax`     |    `%eax`     |    `%ax`    |    `%al`     |    返回值    |
|    `%rbx`     |    `%ebx`     |    `%bx`    |    `%bl`     | 被调用者保存 |
|    `%rcx`     |    `%ecx`     |    `%cx`    |    `%cl`     | 第 4 个参数  |
|    `%rdx`     |    `%edx`     |    `%dx`    |    `%dl`     | 第 3 个参数  |
|    `%rsi`     |    `%esi`     |    `%si`    |    `%sil`    | 第 2 个参数  |
|    `%rdi`     |    `%edi`     |    `%di`    |    `%dil`    | 第 1 个参数  |
|    `%rbp`     |    `%ebp`     |    `%bp`    |    `%bpl`    | 被调用者保存 |
|    `%rsp`     |    `%esp`     |    `%sp`    |    `%spl`    |    栈指针    |
|     `%r8`     |    `%r8d`     |   `%r8w`    |    `%r8b`    | 第 5 个参数  |
|     `%r9`     |    `%r9d`     |   `%r9w`    |    `%r9b`    | 第 6 个参数  |
|    `%r10`     |    `%r10d`    |   `%r10w`   |   `%r10b`    |  调用者保存  |
|    `%r11`     |    `%r11d`    |   `%r11w`   |   `%r11b`    |  调用者保存  |
|    `%r12`     |    `%r12d`    |   `%r12w`   |   `%r12b`    | 被调用者保存 |
|    `%r13`     |    `%r13d`    |   `%r13w`   |   `%r13b`    | 被调用者保存 |
|    `%r14`     |    `%r14d`    |   `%r14w`   |   `%r14b`    | 被调用者保存 |
|    `%r15`     |    `%r15d`    |   `%r15w`   |   `%r15b`    | 被调用者保存 |

`%rbx`、`%rbp`、`%r12`、`%r13`、`%r14`、`%r15` 是被调用者保存寄存器；`%rax`、`%rdi`、`%rsi`、`%rdx`、`%rcx`、`%r8`、`%r9`、`%r10`、`%r11` 都是调用者保存寄存器；`%rsp` 两者都不是。

- `CF`：进位标志。最近的操作使最高位产生了进位。可用于检查无符号数的溢出。
- `ZF`：零标志。最近的操作结果为 0.
- `SF`：符号标志。最近的操作结果为负数。
- `OF`：溢出标志。最近的操作导致补码溢出。

**逻辑操作置 `OF` 和 `CF` 为 `0`**。

**移位操作置 `OF` 为 `0`，置 `CF` 为最后移出的位**。

`SET` 指令根据条件码的某种组合设置一个字节的值。

|   指令    |  同义名  |   条件码组合   |        设置条件         |
| :-------: | :------: | :------------: | :---------------------: | --------------------- |
| `sete D`  |  `setz`  |      `ZF`      |        相等 / 零        |
| `setne D` | `setnz`  |     `~ZF`      |       不等 / 非零       |
| `sets D`  |          |      `SF`      |           负            |
| `setns D` |          |     `~SF`      |          非负           |
| `setg D`  | `setnle` | `~(SF^OF)&~ZF` |    大于（有符号 >）     |
| `setge D` | `setnl`  |   `~(SF^OF)`   |  大于等于（有符号 >=）  |
| `setl D`  | `setnge` |    `SF^OF`     |    小于（有符号 <）     |
| `setle D` | `setng`  |    `(SF^OF)    |           ZF`           | 小于等于（有符号 <=） |
| `seta D`  | `setnbe` |   `~CF&~ZF`    |    超过（无符号 >）     |
| `setae D` | `setnb`  |     `~CF`      | 超过或相等（无符号 >=） |
| `setb D`  | `setnae` |      `CF`      |    低于（无符号 <）     |
| `setbe D` | `setna`  |    `CF\|ZF`    | 低于或相等（无符号 <=） |

有符号比较使用 `SF` 和 `OF` 条件码，无符号比较使用 `CF` 和 `ZF` 条件码。

**`set D` 中的目标位置 `D` 必须是单字节寄存器**。

参数按顺序依次存储在 `%rdi`、`%rsi`、`%rdx`、`%rcx`、`%r8` 和 `%r9` 中，它们都是调用者保存寄存器。

在 64 位机器上，地址总是 64 位的，因此像 `(%eax)` 这样的操作数很有可能是错误的，我们认为它不合法。比例因子必须是 1、2、4 或 8，`(, %rax, 3)` 不合法。

`clto` 指令把 `%rax` 的值符号扩展，结果存放在 `%rdx`（高 64 位）和 `%rax`（低 64 位）两个寄存器中

右移也会改变进位标志

`sar %ebx`

**无条件跳转可以是直接跳转，也可以是间接跳转**。

**条件跳转只能是直接跳转**。

`call` 可以间接跳转

**通过栈传递参数时，所有数据大小向 8 的倍数对齐**

`for` 和对应 `while` 转写的区别：`continue` 语句被跳过

gcc 一次申请的栈空间大小必须是 8 的倍数，且大于 16 字节。

在调用新函数过程之前，`%rsp` 要 8 字节对齐（默认）或 16 字节对齐（ABI 规定，-fPIC 编译）

局部变量被引用前需要放置到栈上，这些变量的放置需要与自身字节对齐

`leave` 指令将帧指针恢复到原始值，释放整个栈帧。执行它如同执行 `movq %rbp, %rsp; popq %rbp`。

`cqto`
`idivq Src`
`divq Src`
`imulq Src` `mulq Src`

在 Intel 文档里，`cqto` 叫做 `cqo`。

x86-64 的 `char` 是有符号的

**寻址中的立即数不带 `$`**：`123(%rsp, %rbx, 4)`

## 其他

两个数组: 内存别名引用!
