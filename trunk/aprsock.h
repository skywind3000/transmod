//=====================================================================
//
// The common interface of socket for Unix and Windows
// Unix/Windows 标准 socket编程通用接口
//
// HISTORY:
// Nov. 15 2004   skywind  created this file
// Dec. 17 2005   skywind  support apr_*able, apr_poll
//
// NOTE：
// 提供使 Unix或者 Windows相同的 socket编程接口主要有许多地方不一样，
// 这里提供相同的访问方式，起名apr_sock意再模仿apache的aprlib
//
//=====================================================================

#ifndef __APR_SOCK_H__
#define __APR_SOCK_H__

#if defined(__APPLE__) && (!defined(__unix))
    #define __unix
#endif

//---------------------------------------------------------------------
// Unix 头部定义
//---------------------------------------------------------------------
#if defined(__unix)
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <netinet/in.h>

#define IESOCKET		-1
#define IEAGAIN			EAGAIN

//---------------------------------------------------------------------
// Windows 头部定义
//---------------------------------------------------------------------
#elif defined(_WIN32)
#include <windows.h>
#include <winsock.h>

#define IESOCKET		SOCKET_ERROR
#define IEAGAIN			WSAEWOULDBLOCK

#else
#error Unknow platform, only support unix and win32
#endif


//---------------------------------------------------------------------
// 全局相关宏定义
//---------------------------------------------------------------------
#define APR_NOBLOCK		1		// 标志：非阻塞
#define APR_REUSEADDR	2		// 标志：地址复用
#define APR_NODELAY		3		// 标志：立即发送
#define APR_NOPUSH		4		// 标志：塞子操作

#define APR_ERECV		1		// 网络事件：捕获输入
#define APR_ESEND		2		// 网络事件：捕获输出
#define APR_ERROR		4		// 网络事件：捕获错误


//---------------------------------------------------------------------
// 基本函数接口声明
//---------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif

// 开始网络
int apr_netstart(int v);

// 结束网络
int apr_netclose(void);

// 初始化套接字
int apr_socket(int family, int type, int protocol);

// 关闭套接字
int apr_close(int sock);

// 连接目标地址
int apr_connect(int sock, const struct sockaddr *addr);

// 停止套接字
int apr_shutdown(int sock, int mode);

// 绑定端口
int apr_bind(int sock, const struct sockaddr *addr);

// 监听消息
int apr_listen(int sock, int count);

// 接收连接
int apr_accept(int sock, struct sockaddr *addr);

// 获取错误信息
int apr_errno(void);

// 发送消息
int apr_send(int sock, const void *buf, long size, int mode);

// 接收消息
int apr_recv(int sock, void *buf, long size, int mode);

// 非连接套接字发送消息
int apr_sendto(int sock, const void *buf, long size, int mode, const struct sockaddr *addr);

// 非连接套接字接收消息
int apr_recvfrom(int sock, void *buf, long size, int mode, struct sockaddr *addr);

// 调用ioctlsocket，设置输出输入参数
int apr_ioctl(int sock, long cmd, unsigned long *argp);

// 设置套接字参数
int apr_setsockopt(int sock, int level, int optname, const char *optval, int optlen);

// 读取套接字参数
int apr_getsockopt(int sock, int level, int optname, char *optval, int *optlen);

// 取得套接字地址
int apr_sockname(int sock, struct sockaddr *addr);

// 取得套接字所连接地址
int apr_peername(int sock, struct sockaddr *addr);


//---------------------------------------------------------------------
// 功能函数接口声明
//---------------------------------------------------------------------
// 允许功能选项：APR_NOBLOCK / APR_NODELAY ...
int apr_enable(int fd, int mode);

// 禁止功能选项：APR_NOBLOCK / APR_NODELAY ...
int apr_disable(int fd, int mode);

// 捕捉套接字的事件：APR_ESEND / APR_ERECV / APR_ERROR 
int apr_pollfd(int sock, int event, long millsec);

// 尽可能的发送数据
int apr_sendall(int sock, const void *buf, long size);

// 尽可能的接收数据
int apr_recvall(int sock, void *buf, long size);

// 将错误码转换成对应字符串
char *apr_errstr(int errnum, char *msg, int size);


#ifdef __cplusplus
}
#endif


#endif



