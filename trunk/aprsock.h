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

#if defined(__APPLE__) && (!defined(__unix))
    #define __unix
#endif

//---------------------------------------------------------------------
// Unix ͷ������
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
// Windows ͷ������
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
// ȫ����غ궨��
//---------------------------------------------------------------------
#define APR_NOBLOCK		1		// ��־��������
#define APR_REUSEADDR	2		// ��־����ַ����
#define APR_NODELAY		3		// ��־����������
#define APR_NOPUSH		4		// ��־�����Ӳ���

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
int apr_netstart(int v);

// ��������
int apr_netclose(void);

// ��ʼ���׽���
int apr_socket(int family, int type, int protocol);

// �ر��׽���
int apr_close(int sock);

// ����Ŀ���ַ
int apr_connect(int sock, const struct sockaddr *addr);

// ֹͣ�׽���
int apr_shutdown(int sock, int mode);

// �󶨶˿�
int apr_bind(int sock, const struct sockaddr *addr);

// ������Ϣ
int apr_listen(int sock, int count);

// ��������
int apr_accept(int sock, struct sockaddr *addr);

// ��ȡ������Ϣ
int apr_errno(void);

// ������Ϣ
int apr_send(int sock, const void *buf, long size, int mode);

// ������Ϣ
int apr_recv(int sock, void *buf, long size, int mode);

// �������׽��ַ�����Ϣ
int apr_sendto(int sock, const void *buf, long size, int mode, const struct sockaddr *addr);

// �������׽��ֽ�����Ϣ
int apr_recvfrom(int sock, void *buf, long size, int mode, struct sockaddr *addr);

// ����ioctlsocket����������������
int apr_ioctl(int sock, long cmd, unsigned long *argp);

// �����׽��ֲ���
int apr_setsockopt(int sock, int level, int optname, const char *optval, int optlen);

// ��ȡ�׽��ֲ���
int apr_getsockopt(int sock, int level, int optname, char *optval, int *optlen);

// ȡ���׽��ֵ�ַ
int apr_sockname(int sock, struct sockaddr *addr);

// ȡ���׽��������ӵ�ַ
int apr_peername(int sock, struct sockaddr *addr);


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

// ��������ת���ɶ�Ӧ�ַ���
char *apr_errstr(int errnum, char *msg, int size);


#ifdef __cplusplus
}
#endif


#endif



