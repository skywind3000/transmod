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

#include "aprpoll.h"
#include "aprpollh.h"

#include <stdio.h>

struct APOLL_DRIVER APOLLDRV;
struct IMPOOL apr_polld_list;


//---------------------------------------------------------------------
// ���ܹ��ṩ���豸����
//---------------------------------------------------------------------
#ifdef APHAVE_SELECT
extern struct APOLL_DRIVER APOLL_SELECT;
#endif
#ifdef APHAVE_POLL
extern struct APOLL_DRIVER APOLL_POLL;
#endif
#ifdef APHAVE_KQUEUE
extern struct APOLL_DRIVER APOLL_KQUEUE;
#endif
#ifdef APHAVE_EPOLL
extern struct APOLL_DRIVER APOLL_EPOLL;
#endif
#ifdef APHAVE_DEVPOLL
extern struct APOLL_DRIVER APOLL_DEVPOLL;
#endif
#ifdef APHAVE_POLLSET
extern struct APOLL_DRIVER APOLL_POLLSET;
#endif
#ifdef APHAVE_RTSIG
extern struct APOLL_DRIVER APOLL_RTSIG;
#endif
#ifdef APHAVE_WINCP
extern struct APOLL_DRIVER APOLL_WINCP;
#endif

//---------------------------------------------------------------------
// ��ǰ֧�ֵ��豸�����б�
//---------------------------------------------------------------------
static struct APOLL_DRIVER *drivers[] = {
#ifdef APHAVE_SELECT
	&APOLL_SELECT,
#endif
#ifdef APHAVE_POLL
	&APOLL_POLL,
#endif
#ifdef APHAVE_KQUEUE
	&APOLL_KQUEUE,
#endif
#ifdef APHAVE_EPOLL
	&APOLL_EPOLL,
#endif
#ifdef APHAVE_DEVPOLL
	&APOLL_DEVPOLL,
#endif
#ifdef APHAVE_POLLSET
	&APOLL_POLLSET,
#endif
#ifdef APHAVE_RTSIG
	&APOLL_RTSIG,
#endif
#ifdef APHAVE_WINCP
	&APOLL_WINCP,
#endif
	NULL
};

static int init_flag = 0;

static apr_mutex pmutex;
static apr_mutex tmutex;

//---------------------------------------------------------------------
// ��ʼ��POLL�豸�����������豸���еĺ궨��
//---------------------------------------------------------------------
int apr_poll_install(int device)
{
	int i, bp;
	long v;

	if (init_flag) return 1;

	if (device > 0) {
		for (i = 0; drivers[i]; i++) if (drivers[i]->id == device) break;
		if (drivers[i] == NULL) return -1;
		APOLLDRV = *drivers[i];
	}	else {
		for (i = 0, bp = -1, device = -1; drivers[i]; i++) {
			if (drivers[i]->performance > bp)
				bp = drivers[i]->performance,
				device = i;
		}
		if (device == -1) return -2;
		APOLLDRV = *drivers[device];
	}

	imp_init(&apr_polld_list, APOLLDRV.pdsize + (int)sizeof(int), NULL);

	i = APOLLDRV.startup();

	if (i) {
		imp_destroy(&apr_polld_list);
		return -10 + i;
	}
	
	v = apr_mutex_init(&pmutex);
	v = apr_mutex_init(&tmutex);

	init_flag = 1;

	return (int)v;
}

//---------------------------------------------------------------------
// ��ԭ�豸
//---------------------------------------------------------------------
int apr_poll_remove(void)
{
	int retval = 0;
	if (init_flag == 0) return 0;
	retval = APOLLDRV.shutdown();
	imp_destroy(&apr_polld_list);
	retval = (int)apr_mutex_destroy(tmutex);
	retval = (int)apr_mutex_destroy(pmutex);
	init_flag = 0;

	return retval;
}

//---------------------------------------------------------------------
// ȡ�õ�ǰ�豸������
//---------------------------------------------------------------------
const char *apr_poll_devname(void)
{
	return APOLLDRV.name;
}

//---------------------------------------------------------------------
// ����һ���µ�poll���к�
//---------------------------------------------------------------------
static apolld apoll_new(void)
{
	int retval = -1;
	char *lptr = NULL;
	retval = apr_mutex_lock(pmutex);
	retval = imp_newnode(&apr_polld_list);
	if (retval >= 0) {
		lptr = (char*)IMP_DATA(&apr_polld_list, retval);
		*(int*)((void*)lptr) = retval;
		lptr += sizeof(int);
	}
	retval = apr_mutex_unlock(pmutex);
	return lptr;
}

//---------------------------------------------------------------------
// ɾ��һ�����е�poll���к�
//---------------------------------------------------------------------
static int apoll_del(apolld ipd)
{
	int retval = 0;
	char *lptr = NULL;
	retval = apr_mutex_lock(pmutex);
	lptr = (char*)ipd;
	lptr -= sizeof(int);
	imp_delnode(&apr_polld_list, *(int*)((void*)lptr));
	retval = apr_mutex_unlock(pmutex);
	return retval;
}


//---------------------------------------------------------------------
// ��ʼ��һ��poll���о��
//---------------------------------------------------------------------
int apr_poll_init(apolld *ipd, int param)
{
	apolld pd;
	int retval = 0;

	if (ipd == NULL) return -1;
	pd = apoll_new();
	if (pd == NULL) return -2; 
	retval = APOLLDRV.init_pd(pd, param);
	if (retval) {
		retval = apoll_del(pd);
		return -3;
	}
	*ipd = pd;
	return retval;
}

//---------------------------------------------------------------------
// �ͷ�һ��poll�����豸
//---------------------------------------------------------------------
int apr_poll_destroy(apolld ipd)
{
	int retval = 0;
	retval = APOLLDRV.destroy_pd(ipd);
	retval = apoll_del(ipd);
	return retval;
}

//---------------------------------------------------------------------
// ���ļ���������poll����
//---------------------------------------------------------------------
int apr_poll_add(apolld ipd, int fd, int mask, void *udata)
{
	return APOLLDRV.poll_add(ipd, fd, mask, udata);
}

//---------------------------------------------------------------------
// ���ļ�������poll����ɾ��
//---------------------------------------------------------------------
int apr_poll_del(apolld ipd, int fd)
{
	return APOLLDRV.poll_del(ipd, fd);
}

//---------------------------------------------------------------------
// ����poll������ĳ�ļ��������¼�����
//---------------------------------------------------------------------
int apr_poll_set(apolld ipd, int fd, int mask)
{
	return APOLLDRV.poll_set(ipd, fd, mask);
}

//---------------------------------------------------------------------
// �ȴ���Ϣ��Ҳ������һ��poll
//---------------------------------------------------------------------
int apr_poll_wait(apolld ipd, int timeval)
{
	return APOLLDRV.poll_wait(ipd, timeval);
}

//---------------------------------------------------------------------
// �Ӷ�����ȡ�õ����¼����ɹ��򷵻�0���¼��ӿ��򷵻ط��㣬��ʱ����wait
//---------------------------------------------------------------------
int apr_poll_event(apolld ipd, int *fd, int *event, void **udata)
{
	int retval;
	do {
		retval = APOLLDRV.poll_event(ipd, fd, event, udata);
	}	while (*event == 0 && retval == 0);
	return retval;
}


//---------------------------------------------------------------------
// ��ʼ�������б�
//---------------------------------------------------------------------
void apr_poll_fvinit(struct APOLLFV *fv, struct IALLOCATOR *allocator)
{
	iv_init(&fv->vec, allocator);
	fv->count = 0;
	fv->fds = NULL;
}

//---------------------------------------------------------------------
// ���������б�
//---------------------------------------------------------------------
void apr_poll_fvdestroy(struct APOLLFV *fv)
{
	iv_destroy(&fv->vec);
	fv->count = 0;
	fv->fds = NULL;
}

//---------------------------------------------------------------------
// �ı������б��С
//---------------------------------------------------------------------
int apr_poll_fvresize(struct APOLLFV *fv, int size)
{
	int retval = 0;

	if (size <= (int)fv->count) return 0;
	retval = iv_resize(&fv->vec, (int)sizeof(struct APOLLFD) * size);
	if (retval) return -1;
	fv->fds = (struct APOLLFD*)((void*)fv->vec.data);
	fv->count = (unsigned long)size;

	return 0;
}

