//=====================================================================
//
// The common interface of socket for Unix and Windows
// Unix/Windows ��׼ socket���ͨ�ýӿ�
//
// HISTORY:
// Nov. 15 2004   skywind  created this file
// Dec. 17 2005   skywind  support apr_*able, apr_poll
//
// NOTE��
// �ṩʹ Unix���� Windows��ͬ�� socket��̽ӿ���Ҫ�����ط���һ����
// �����ṩ��ͬ�ķ��ʷ�ʽ������apr_sock����ģ��apache��aprlib
//
//=====================================================================

#ifndef __APR_SOCK_H__
#define __APR_SOCK_H__


/*-------------------------------------------------------------------*/
/* C99 Compatible                                                    */
/*-------------------------------------------------------------------*/
#if defined(linux) || defined(__linux) || defined(__linux__)
#ifdef _POSIX_C_SOURCE
#if _POSIX_C_SOURCE < 200112L
#undef _POSIX_C_SOURCE
#endif
#endif

#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200112L
#endif

#ifdef _GNU_SOURCE
#undef _GNU_SOURCE
#endif

#ifdef _BSD_SOURCE
#undef _BSD_SOURCE
#endif

#ifdef __BSD_VISIBLE
#undef __BSD_VISIBLE
#endif

#ifdef _XOPEN_SOURCE
#undef _XOPEN_SOURCE
#endif

#define _GNU_SOURCE 1
#define _BSD_SOURCE 1
#define __BSD_VISIBLE 1
#define _XOPEN_SOURCE 600

#endif

#include <stddef.h>
#include <stdlib.h>


/*-------------------------------------------------------------------*/
/* Unix Platform                                                     */
/*-------------------------------------------------------------------*/
#if defined(__unix) || defined(unix) || defined(__MACH__)
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>

#if defined(__sun) || defined(__sun__)
#include <sys/filio.h>
#endif

#ifndef __unix
#define __unix
#endif


#if defined(__MACH__) && (!defined(_SOCKLEN_T)) && (!defined(HAVE_SOCKLEN))
typedef int socklen_t;
#endif

#include <errno.h>


#define IESOCKET		-1
#define IEAGAIN			EAGAIN

/*-------------------------------------------------------------------*/
/* Windows Platform                                                  */
/*-------------------------------------------------------------------*/
#elif defined(_WIN32) || defined(WIN32) || defined(_WIN64) || defined(WIN64)
#if ((!defined(_M_PPC)) && (!defined(_M_PPC_BE)) && (!defined(_XBOX)))
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0500
#endif
#ifndef WIN32_LEAN_AND_MEAN  
#define WIN32_LEAN_AND_MEAN  
#endif
#include <windows.h>
#include <winsock2.h>
#else
#ifndef _XBOX
#define _XBOX
#endif
#include <xtl.h>
#include <winsockx.h>
#endif

#include <errno.h>

#define IESOCKET		SOCKET_ERROR
#define IEAGAIN			WSAEWOULDBLOCK

#ifndef _WIN32
#define _WIN32
#endif

#ifdef AF_INET6
#include <ws2tcpip.h>
#endif

typedef int socklen_t;
typedef SOCKET Socket;

#ifndef EWOULDBLOCK
#define EWOULDBLOCK             WSAEWOULDBLOCK
#endif
#ifndef EAGAIN
#define EAGAIN                  WSAEWOULDBLOCK
#endif
#ifndef EINPROGRESS
#define EINPROGRESS             WSAEINPROGRESS
#endif
#ifndef EALREADY
#define EALREADY                WSAEALREADY
#endif
#ifndef ENOTSOCK
#define ENOTSOCK                WSAENOTSOCK
#endif
#ifndef EDESTADDRREQ
#define EDESTADDRREQ            WSAEDESTADDRREQ
#endif
#ifndef EMSGSIZE
#define EMSGSIZE                WSAEMSGSIZE
#endif 
#ifndef EPROTOTYPE
#define EPROTOTYPE              WSAEPROTOTYPE
#endif
#ifndef ENOPROTOOPT
#define ENOPROTOOPT             WSAENOPROTOOPT
#endif
#ifndef EPROTONOSUPPORT
#define EPROTONOSUPPORT         WSAEPROTONOSUPPORT
#endif
#ifndef ESOCKTNOSUPPORT
#define ESOCKTNOSUPPORT         WSAESOCKTNOSUPPORT
#endif
#ifndef EOPNOTSUPP
#define EOPNOTSUPP              WSAEOPNOTSUPP
#endif
#ifndef EPFNOSUPPORT
#define EPFNOSUPPORT            WSAEPFNOSUPPORT
#endif
#ifndef EAFNOSUPPORT
#define EAFNOSUPPORT            WSAEAFNOSUPPORT
#endif
#ifndef EADDRINUSE
#define EADDRINUSE              WSAEADDRINUSE
#endif
#ifndef EADDRNOTAVAIL
#define EADDRNOTAVAIL           WSAEADDRNOTAVAIL
#endif

#ifndef ENETDOWN
#define ENETDOWN                WSAENETDOWN
#endif
#ifndef ENETUNREACH
#define ENETUNREACH             WSAENETUNREACH
#endif
#ifndef ENETRESET
#define ENETRESET               WSAENETRESET
#endif
#ifndef ECONNABORTED
#define ECONNABORTED            WSAECONNABORTED
#endif
#ifndef ECONNRESET
#define ECONNRESET              WSAECONNRESET
#endif
#ifndef ENOBUFS
#define ENOBUFS                 WSAENOBUFS
#endif
#ifndef EISCONN
#define EISCONN                 WSAEISCONN
#endif
#ifndef ENOTCONN
#define ENOTCONN                WSAENOTCONN
#endif
#ifndef ESHUTDOWN
#define ESHUTDOWN               WSAESHUTDOWN
#endif
#ifndef ETOOMANYREFS
#define ETOOMANYREFS            WSAETOOMANYREFS
#endif
#ifndef ETIMEDOUT
#define ETIMEDOUT               WSAETIMEDOUT
#endif
#ifndef ECONNREFUSED
#define ECONNREFUSED            WSAECONNREFUSED
#endif
#ifndef ELOOP
#define ELOOP                   WSAELOOP
#endif
#ifndef EHOSTDOWN
#define EHOSTDOWN               WSAEHOSTDOWN
#endif
#ifndef EHOSTUNREACH
#define EHOSTUNREACH            WSAEHOSTUNREACH
#endif
#define EPROCLIM                WSAEPROCLIM
#define EUSERS                  WSAEUSERS
#define EDQUOT                  WSAEDQUOT
#define ESTALE                  WSAESTALE
#define EREMOTE                 WSAEREMOTE

#else
#error Unknow platform, only support unix and win32
#endif

#ifndef EINVAL
#define EINVAL          22
#endif

#ifdef IHAVE_CONFIG_H
#include "config.h"
#endif

#if defined(i386) || defined(__i386__) || defined(__i386) || \
	defined(_M_IX86) || defined(_X86_) || defined(__THW_INTEL)
	#ifndef __i386__
	#define __i386__
	#endif
#endif

#if (defined(__MACH__) && defined(__APPLE__)) || defined(__MACOS__)
	#ifndef __imac__
	#define __imac__
	#endif
#endif

#if defined(linux) || defined(__linux) || defined(__linux__)
	#ifndef __linux
		#define __linux 1
	#endif
	#ifndef __linux__
		#define __linux__ 1
	#endif
#endif

#if defined(__sun) || defined(__sun__)
	#ifndef __sun
		#define __sun 1
	#endif
	#ifndef __sun__
		#define __sun__ 1
	#endif
#endif


/* check ICLOCK_TYPE_REALTIME for using pthread_condattr_setclock */
#if defined(__CYGWIN__) || defined(__imac__)
	#define ICLOCK_TYPE_REALTIME
#endif

#if defined(ANDROID) && (!defined(__ANDROID__))
	#define __ANDROID__
#endif

#if defined(__APPLE__) && (!defined(__unix))
    #define __unix
#endif



//---------------------------------------------------------------------
// ȫ����غ궨��
//---------------------------------------------------------------------
#define APR_NOBLOCK		1		// ��־��������
#define APR_REUSEADDR	2		// ��־����ַ����
#define APR_NODELAY		3		// ��־����������
#define APR_NOPUSH		4		// ��־�����Ӳ���
#define APR_CLOEXEC		5		// ��־��ִ�йر�

#define APR_ERECV		1		// �����¼�����������
#define APR_ESEND		2		// �����¼����������
#define APR_ERROR		4		// �����¼����������


//---------------------------------------------------------------------
// ���������ӿ�����
//---------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif

// ��ʼ����
int apr_netstart(void);

// ��������
int apr_netclose(void);

// ��ʼ���׽���
int apr_socket(int family, int type, int protocol);

// �ر��׽���
int apr_close(int sock);

// ����Ŀ���ַ
int apr_connect(int sock, const struct sockaddr *addr, int addrlen);

// ֹͣ�׽���
int apr_shutdown(int sock, int mode);

// �󶨶˿�
int apr_bind(int sock, const struct sockaddr *addr, int addrlen);

// ������Ϣ
int apr_listen(int sock, int count);

// ��������
int apr_accept(int sock, struct sockaddr *addr, int *addrlen);

// ��ȡ������Ϣ
int apr_errno(void);

// ������Ϣ
int apr_send(int sock, const void *buf, long size, int mode);

// ������Ϣ
int apr_recv(int sock, void *buf, long size, int mode);

// �������׽��ַ�����Ϣ
int apr_sendto(int sock, const void *buf, long size, int mode, const struct sockaddr *addr, int addrlen);

// �������׽��ֽ�����Ϣ
int apr_recvfrom(int sock, void *buf, long size, int mode, struct sockaddr *addr, int *addrlen);

// ����ioctlsocket����������������
int apr_ioctl(int sock, unsigned long cmd, unsigned long *argp);

// �����׽��ֲ���
int apr_setsockopt(int sock, int level, int optname, const char *optval, int optlen);

// ��ȡ�׽��ֲ���
int apr_getsockopt(int sock, int level, int optname, char *optval, int *optlen);

// ȡ���׽��ֵ�ַ
int apr_sockname(int sock, struct sockaddr *addr, int *addrlen);

// ȡ���׽��������ӵ�ַ
int apr_peername(int sock, struct sockaddr *addr, int *addrlen);


//---------------------------------------------------------------------
// ���ܺ����ӿ�����
//---------------------------------------------------------------------
// ������ѡ�APR_NOBLOCK / APR_NODELAY ...
int apr_enable(int fd, int mode);

// ��ֹ����ѡ�APR_NOBLOCK / APR_NODELAY ...
int apr_disable(int fd, int mode);

// ��׽�׽��ֵ��¼���APR_ESEND / APR_ERECV / APR_ERROR 
int apr_pollfd(int sock, int event, long millsec);

// �����ܵķ�������
int apr_sendall(int sock, const void *buf, long size);

// �����ܵĽ�������
int apr_recvall(int sock, void *buf, long size);

// Win32ģʽ��ʼ��
int apr_win32_init(int sock);

// ��������ת���ɶ�Ӧ�ַ���
char *apr_errstr(int errnum, char *msg, int size);


//---------------------------------------------------------------------
// IPV4/IPV6 ��ַ����
//---------------------------------------------------------------------

// ת����ʾ��ʽ�������ʽ 
// ���� 0��ʾ�ɹ���֧�� AF_INET/AF_INET6 
int apr_pton(int af, const char *src, void *dst);

// ת�������ʽ����ʾ��ʽ 
// ֧�� AF_INET/AF_INET6 
const char *apr_ntop(int af, const void *src, char *dst, size_t size);



#ifdef __cplusplus
}
#endif


#endif



