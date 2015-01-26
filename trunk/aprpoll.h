//=====================================================================
//
// The platform independence poll wrapper, Skywind 2004
//
// HISTORY:
// Dec. 15 2004   skywind  created
// Jul. 25 2005   skywind  change poll desc. from int to apolld
// Jul. 27 2005   skywind  improve apr_poll_event method
//
// NOTE��
// �첽I/O��װ��ʹ�ò�ͬ��ϵͳ�ж�����ʹ����ͬ��I/O�첽�ź�ģ�ͣ���
// ��ͨ�õ�select�������ڲ�ͬ��ƽ̨�����в�ͬ��ʵ�֣�unix��ʵ�ֵ�
// kqueue/epoll��windows��ʵ�ֵ��걸�˿ڣ�����ʹ�����е�ƽ̨�첽I/O��
// ����ͳһ�Ľӿ�
//
//=====================================================================

#ifndef __APR_POLL_H__
#define __APR_POLL_H__

#include "aprsys.h"

// ֧�ֵ��豸��
#define APDEVICE_AUTO		0	// �Զ�ѡ������
#define APDEVICE_SELECT		1	// ʹ��select����
#define APDEVICE_POLL		2	// ʹ��poll����
#define APDEVICE_KQUEUE		3	// ʹ��kqueue����
#define APDEVICE_EPOLL		4	// ʹ��epoll����
#define APDEVICE_DEVPOLL	5	// ʹ��dev/poll����
#define APDEVICE_POLLSET	6	// ����pollset����
#define APDEVICE_WINCP		7	// ʹ����ɶ˿�����
#define APDEVICE_RTSIG		8	// ʹ��rtsig����

#define APOLL_IN	1	// �¼����ļ���������¼�
#define APOLL_OUT	2	// �¼����ļ��������¼�
#define APOLL_ERR	4	// �¼����ļ���������¼�


typedef void* apolld;

#ifdef __cplusplus
extern "C" {
#endif


// ��ʼ��POLL�豸�����������豸���еĺ궨��
int apr_poll_install(int device);

// ��ԭ�豸
int apr_poll_remove(void);

// ȡ�õ�ǰ�豸������
const char *apr_poll_devname(void);

// ��ʼ��һ��poll���о��
int apr_poll_init(apolld *ipd, int param);

// �ͷ�һ��poll�����豸
int apr_poll_destroy(apolld ipd);

// ���ļ���������poll����
int apr_poll_add(apolld ipd, int fd, int mask, void *udata);

// ���ļ�������poll����ɾ��
int apr_poll_del(apolld ipd, int fd);

// ����poll������ĳ�ļ��������¼�����
int apr_poll_set(apolld ipd, int fd, int mask);

// �ȴ���Ϣ��Ҳ������һ��poll
int apr_poll_wait(apolld ipd, int timeval);

// �Ӷ�����ȡ�õ����¼����ɹ��򷵻�0���¼��ӿ��򷵻ط��㣬��ʱ��Ҫ����wait
int apr_poll_event(apolld ipd, int *fd, int *event, void **udata);

#ifdef __cplusplus
}
#endif


#endif

