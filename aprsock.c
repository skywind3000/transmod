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

#include "aprsock.h"

#include <stdlib.h>
#include <string.h>

#ifdef __unix
#include <netdb.h>
#include <sched.h>
#include <poll.h>
#endif

//---------------------------------------------------------------------
// 设置内部宏定义
//---------------------------------------------------------------------
#if defined(TCP_CORK) && !defined(TCP_NOPUSH)
#define TCP_NOPUSH TCP_CORK
#endif

#ifdef __unix
typedef socklen_t DSOCKLEN_T;
#else
typedef int DSOCKLEN_T;
#endif



//=====================================================================
// 基本函数接口声明
//=====================================================================

//---------------------------------------------------------------------
// 开始网络
//---------------------------------------------------------------------
int apr_netstart(int v)
{
	int retval = 0;
	#ifdef _WIN32
	struct WSAData wsa;
	retval = (int)WSAStartup((unsigned short)v, &wsa);	
	#endif
	return retval;
}

//---------------------------------------------------------------------
// 结束网络
//---------------------------------------------------------------------
int apr_netclose(void)
{
	int retval = 0;
	#ifdef _WIN32
	retval = (int)WSACleanup();
	#endif
	return retval;
}

//---------------------------------------------------------------------
// 初始化套接字
//---------------------------------------------------------------------
int apr_socket(int family, int type, int protocol)
{
	return (int)socket(family, type, protocol);
}

//---------------------------------------------------------------------
// 关闭套接字
//---------------------------------------------------------------------
int apr_close(int sock)
{
	int retval = 0;
	if (sock < 0) return 0;
	#ifdef __unix
	retval = close(sock);
	#else
	retval = closesocket((SOCKET)sock);
	#endif
	return retval;
}

//---------------------------------------------------------------------
// 连接目标地址
//---------------------------------------------------------------------
int apr_connect(int sock, const struct sockaddr *addr)
{
	DSOCKLEN_T len = sizeof(struct sockaddr);
	return connect(sock, addr, len);
}

//---------------------------------------------------------------------
// 停止套接字
//---------------------------------------------------------------------
int apr_shutdown(int sock, int mode)
{
	return shutdown(sock, mode);
}

//---------------------------------------------------------------------
// 绑定端口
//---------------------------------------------------------------------
int apr_bind(int sock, const struct sockaddr *addr)
{
	DSOCKLEN_T len = sizeof(struct sockaddr);
	return bind(sock, addr, len);
}

//---------------------------------------------------------------------
// 监听端口
//---------------------------------------------------------------------
int apr_listen(int sock, int count)
{
	return listen(sock, count);
}

//---------------------------------------------------------------------
// 接收连接
//---------------------------------------------------------------------
int apr_accept(int sock, struct sockaddr *addr)
{
	DSOCKLEN_T len = sizeof(struct sockaddr);
	return (int)accept(sock, addr, &len);
}

//---------------------------------------------------------------------
// 返回上一个错误
//---------------------------------------------------------------------
int apr_errno(void)
{
	int retval;
	#ifdef __unix
	retval = errno;
	#else
	retval = (int)WSAGetLastError();
	#endif
	return retval;
}

//---------------------------------------------------------------------
// 发送数据
//---------------------------------------------------------------------
int apr_send(int sock, const void *buf, long size, int mode)
{
	return send(sock, (char*)buf, size, mode);
}

//---------------------------------------------------------------------
// 接收数据
//---------------------------------------------------------------------
int apr_recv(int sock, void *buf, long size, int mode)
{
	return recv(sock, (char*)buf, size, mode);
}

//---------------------------------------------------------------------
// 非连接套接字发送
//---------------------------------------------------------------------
int apr_sendto(int sock, const void *buf, long size, int mode, const struct sockaddr *addr)
{
	DSOCKLEN_T len = sizeof(struct sockaddr);
	return sendto(sock, (char*)buf, size, mode, addr, len);
}

//---------------------------------------------------------------------
// 非连接套接字接收
//---------------------------------------------------------------------
int apr_recvfrom(int sock, void *buf, long size, int mode, struct sockaddr *addr)
{
	DSOCKLEN_T len = sizeof(struct sockaddr);
	return recvfrom(sock, (char*)buf, size, mode, addr, &len);
}

//---------------------------------------------------------------------
// 调用ioctlsocket，设置输出输入参数
//---------------------------------------------------------------------
int apr_ioctl(int sock, long cmd, unsigned long *argp)
{
	int retval;
	#ifdef __unix
	retval = ioctl(sock, cmd, argp);
	#else
	retval = ioctlsocket((SOCKET)sock, cmd, argp);
	#endif
	return retval;	
}

//---------------------------------------------------------------------
// 设置套接字参数
//---------------------------------------------------------------------
int apr_setsockopt(int sock, int level, int optname, const char *optval, int optlen)
{
	DSOCKLEN_T len = optlen;
	return setsockopt(sock, level, optname, optval, len);
}

//---------------------------------------------------------------------
// 读取套接字参数
//---------------------------------------------------------------------
int apr_getsockopt(int sock, int level, int optname, char *optval, int *optlen)
{
	DSOCKLEN_T len = (optlen)? *optlen : 0;
	int retval;
	retval = getsockopt(sock, level, optname, optval, &len);
	if (optlen) *optlen = len;

	return retval;
}

//---------------------------------------------------------------------
// 取得套接字地址
//---------------------------------------------------------------------
int apr_sockname(int sock, struct sockaddr *addr)
{
	DSOCKLEN_T len = sizeof(struct sockaddr);
	return getsockname(sock, addr, &len);
}

//---------------------------------------------------------------------
// 取得套接字所连接地址
//---------------------------------------------------------------------
int apr_peername(int sock, struct sockaddr *addr)
{
	DSOCKLEN_T len = sizeof(struct sockaddr);
	return getpeername(sock, addr, &len);
}



//=====================================================================
// 功能函数接口声明
//=====================================================================

//---------------------------------------------------------------------
// 允许功能选项：APR_NOBLOCK / APR_NODELAY等
//---------------------------------------------------------------------
int apr_enable(int fd, int mode)
{
	long value = 1;
	long retval = 0;

	switch (mode)
	{
	case APR_NOBLOCK:
		retval = apr_ioctl(fd, (int)FIONBIO, (unsigned long*)(void*)&value);
		break;
	case APR_REUSEADDR:
		retval = apr_setsockopt(fd, (int)SOL_SOCKET, SO_REUSEADDR, (char*)&value, sizeof(value));
		break;
	case APR_NODELAY:
		retval = apr_setsockopt(fd, (int)IPPROTO_TCP, TCP_NODELAY, (char*)&value, sizeof(value));
		break;
	case APR_NOPUSH:
		#ifdef TCP_NOPUSH
		retval = apr_setsockopt(fd, (int)IPPROTO_TCP, TCP_NOPUSH, (char*)&value, sizeof(value));
		#else
		retval = -1000;
		#endif
		break;
	}

	return retval;
}

//---------------------------------------------------------------------
// 禁止功能选项：APR_NOBLOCK / APR_NODELAY等
//---------------------------------------------------------------------
int apr_disable(int fd, int mode)
{
	long value = 0;
	long retval = 0;

	switch (mode)
	{
	case APR_NOBLOCK:
		retval = apr_ioctl(fd, (int)FIONBIO, (unsigned long*)&value);
		break;
	case APR_REUSEADDR:
		retval = apr_setsockopt(fd, (int)SOL_SOCKET, SO_REUSEADDR, (char*)&value, sizeof(value));
		break;
	case APR_NODELAY:
		retval = apr_setsockopt(fd, (int)IPPROTO_TCP, TCP_NODELAY, (char*)&value, sizeof(value));
		break;
	case APR_NOPUSH:
		#ifdef TCP_NOPUSH
		retval = apr_setsockopt(fd, (int)IPPROTO_TCP, TCP_NOPUSH, (char*)&value, sizeof(value));
		#else
		retval = -1000;
		#endif
		break;
	}
	return retval;
}

//---------------------------------------------------------------------
// 捕捉套接字的事件：APR_ESEND / APR_ERECV / APR_ERROR 
//---------------------------------------------------------------------
int apr_pollfd(int sock, int event, long millsec)
{
	int retval = 0;

	#ifdef __unix
	struct pollfd pfd = { sock, 0, 0 };
	int POLLERC = POLLERR | POLLHUP | POLLNVAL;

	pfd.events |= (event & APR_ERECV)? POLLIN : 0;
	pfd.events |= (event & APR_ESEND)? POLLOUT : 0;
	pfd.events |= (event & APR_ERROR)? POLLERC : 0;

	poll(&pfd, 1, millsec);

	if ((event & APR_ERECV) && (pfd.revents & POLLIN)) retval |= APR_ERECV;
	if ((event & APR_ESEND) && (pfd.revents & POLLOUT)) retval |= APR_ESEND;
	if ((event & APR_ERROR) && (pfd.revents & POLLERC)) retval |= APR_ERROR;

	#else
	struct timeval tmx = { 0, 0 };
	union { void *ptr; fd_set *fds; } p[3];
	int fdr[2], fdw[2], fde[2];

	tmx.tv_sec = millsec / 1000;
	tmx.tv_usec = (millsec % 1000) * 1000;
	fdr[0] = fdw[0] = fde[0] = 1;
	fdr[1] = fdw[1] = fde[1] = sock;

	p[0].ptr = (event & APR_ERECV)? fdr : NULL;
	p[1].ptr = (event & APR_ESEND)? fdw : NULL;
	p[2].ptr = (event & APR_ESEND)? fde : NULL;

	retval = select( sock + 1, p[0].fds, p[1].fds, p[2].fds, 
					(millsec >= 0)? &tmx : 0);
	retval = 0;

	if ((event & APR_ERECV) && fdr[0]) retval |= APR_ERECV;
	if ((event & APR_ESEND) && fdw[0]) retval |= APR_ESEND;
	if ((event & APR_ERROR) && fde[0]) retval |= APR_ERROR;
	#endif

	return retval;
}

//---------------------------------------------------------------------
// 尽可能的发送数据
//---------------------------------------------------------------------
int apr_sendall(int sock, const void *buf, long size)
{
	unsigned char *lptr = (unsigned char*)buf;
	int total = 0, retval = 0, c;

	for (; size > 0; lptr += retval, size -= (long)retval) {
		retval = apr_send(sock, lptr, size, 0);
		if (retval == 0) {
			retval = -1;
			break;
		}
		if (retval == -1) {
			c = apr_errno();
			if (c != IEAGAIN) {
				retval = -1000 - c;
				break;
			}
			retval = 0;
			break;
		}
		total += retval;
	}

	return (retval < 0)? retval : total;
}

//---------------------------------------------------------------------
// 尽可能的接收数据
//---------------------------------------------------------------------
int apr_recvall(int sock, void *buf, long size)
{
	unsigned char *lptr = (unsigned char*)buf;
	int total = 0, retval = 0, c;

	for (; size > 0; lptr += retval, size -= (long)retval) {
		retval = apr_recv(sock, lptr, size, 0);
		if (retval == 0) {
			retval = -1;
			break;
		}
		if (retval == -1) {
			c = apr_errno();
			if (c != IEAGAIN) {
				retval = -1000 - c;
				break;
			}
			retval = 0;
			break;
		}
		total += retval;
	}

	return (retval < 0)? retval : total;
}

#ifdef _MSC_VER
#pragma warning(disable:4996)
#endif

//---------------------------------------------------------------------
// 将错误码转换成对应字符串
//---------------------------------------------------------------------
char *apr_errstr(int errnum, char *msg, int size)
{
	static char buffer[1025];
	char *lptr = (msg == NULL)? buffer : msg;
	long length = (msg == NULL)? 1024 : size;
#ifdef __unix
	#ifdef __CYGWIN__
	strncpy(lptr, strerror(errnum), length);
	#else
	strerror_r(errnum, lptr, length);
	#endif
#else
	LPVOID lpMessageBuf;
	fd_set fds;
	FD_ZERO(&fds);
	FD_CLR(0, &fds);
	size = (long)FormatMessage( 
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, errnum, MAKELANGID(LANG_ENGLISH, SUBLANG_DEFAULT), (LPTSTR) &lpMessageBuf,
		0, NULL);
	strncpy(lptr, (char*)lpMessageBuf, length);
	LocalFree(lpMessageBuf);
#endif
	return lptr;
}



