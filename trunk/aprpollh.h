//=====================================================================
//
// The platform independence poll wrapper, Skywind 2004
//
// HISTORY:
// Dec. 15 2004   skywind  created
// Jul. 25 2005   skywind  change poll desc. from int to apolld
// Jul. 27 2005   skywind  improve apr_poll_event method
//
// NOTE�����ļ�ΪPOLL ʵ���ڲ�ͷ�ļ�
// �첽I/O��װ��ʹ�ò�ͬ��ϵͳ�ж�����ʹ����ͬ��I/O�첽�ź�ģ�ͣ���
// ��ͨ�õ�select�������ڲ�ͬ��ƽ̨�����в�ͬ��ʵ�֣�unix��ʵ�ֵ�
// kqueue/epoll��windows��ʵ�ֵ��걸�˿ڣ�����ʹ�����е�ƽ̨�첽I/O��
// ����ͳһ�Ľӿ�
//
//=====================================================================

#ifndef __APR_POLLH_H__
#define __APR_POLLH_H__

#include "icvector.h"
#include "impoold.h"
#include "aprpoll.h"

#if defined(_WIN32) || defined(__unix)
#define APHAVE_SELECT
#endif
#if defined(__unix)
#define APHAVE_POLL
#endif
#if defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__) \
	|| (defined(__APPLE__) && defined(__MACH__))
#define APHAVE_KQUEUE
#endif
#if defined(linux)
#define APHAVE_EPOLL
//#define APHAVE_RTSIG
#endif
#if defined(_WIN32)
//#define APHAVE_WINCP
#endif
#if defined(sun) || defined(__sun) || defined(__sun__)
#define APHAVE_DEVPOLL
#endif

#ifdef __cplusplus
extern "C" {
#endif

//---------------------------------------------------------------------
// POLL �豸����
//---------------------------------------------------------------------
struct APOLL_DRIVER
{
	int pdsize;								// poll��������С
	int id;									// �豸id
	int performance;						// �豸Ч��
	const char *name;						// �豸����
	int (*startup)(void);					// �豸��ʼ��
	int (*shutdown)(void);					// �豸�ر�
	int (*init_pd)(apolld ipd, int param);	// ��ʼ��poll������
	int (*destroy_pd)(apolld ipd);			// ����poll������
	int (*poll_add)(apolld ipd, int fd, int mask, void *udata);  // ����
	int (*poll_del)(apolld ipd, int fd);						 // ɾ��
	int (*poll_set)(apolld ipd, int fd, int mask);		// �����¼�
	int (*poll_wait)(apolld ipd, int timeval);			// �ȴ���׽�¼�
	int (*poll_event)(apolld ipd, int *fd, int *event, void **udata);
};

extern struct IPOLL_DRIVER IPOLLDRV;	// ��ǰ���豸����
extern struct IMPOOL  apr_polld_list;	// poll�������ṹ�б�

#define PSTRUCT void					// ��������ṹ��
#define PDESC(pd) ((PSTRUCT*)(pd))		// ����ṹ��ת��


//---------------------------------------------------------------------
// �����������ڵ�
//---------------------------------------------------------------------
struct APOLLFD	
{
	int fd;		// �ļ�������
	int mask;	// �����¼�mask
	int event;	// �������¼�
	int index;	// ��Ӧ�������
	void*user;	// �û���������
};

//---------------------------------------------------------------------
// �����������б�
//---------------------------------------------------------------------
struct APOLLFV 
{
	struct APOLLFD *fds;	// �ļ���������
	struct IVECTOR  vec;	// �ļ�����ʸ��
	unsigned long count;	// �ļ�������С
};

// ��ʼ�������б�
void apr_poll_fvinit(struct APOLLFV *fv, struct IALLOCATOR *allocator);

// ���������б�
void apr_poll_fvdestroy(struct APOLLFV *fv);

// �ı������б��С
int apr_poll_fvresize(struct APOLLFV *fv, int size);



#ifdef __cplusplus
}
#endif

#endif

