//=====================================================================
//
// The platform independence poll wrapper, Skywind 2004
//
// HISTORY:
// Dec. 15 2004   skywind  created
// Jul. 25 2005   skywind  change poll desc. from int to apolld
// Jul. 27 2005   skywind  improve apr_poll_event method
//
// �첽I/O��װ��ʹ�ò�ͬ��ϵͳ�ж�����ʹ����ͬ��I/O�첽�ź�ģ�ͣ���
// ��ͨ�õ�select�������ڲ�ͬ��ƽ̨�����в�ͬ��ʵ�֣�unix��ʵ�ֵ�
// kqueue/epoll��windows��ʵ�ֵ��걸�˿ڣ�����ʹ�����е�ƽ̨�첽I/O��
// ����ͳһ�Ľӿ�
//
//=====================================================================

#include "aprpoll.h"
#include "aprpollh.h"

#ifdef APHAVE_POLL

#include <stdio.h>
#include <poll.h>

static int app_startup(void);
static int app_shutdown(void);
static int app_init_pd(apolld ipd, int param);
static int app_destroy_pd(apolld ipd);
static int app_poll_add(apolld ipd, int fd, int mask, void *user);
static int app_poll_del(apolld ipd, int fd);
static int app_poll_set(apolld ipd, int fd, int mask);
static int app_poll_wait(apolld ipd, int timeval);
static int app_poll_event(apolld ipd, int *fd, int *event, void **user);


//---------------------------------------------------------------------
// poll�������������ṹ
//---------------------------------------------------------------------
typedef struct 
{
	struct APOLLFV fv;
	struct IVECTOR vpollfd;
	struct IVECTOR vresultq;
	struct pollfd *pfds;
	struct pollfd *resultq;
	long fd_max;
	long fd_min;
	long pnum_max;
	long pnum_cnt;
	long result_num;
	long result_cur;
}	IPD_POLL;

//---------------------------------------------------------------------
// poll�豸������ͨ�������ṹ������һ��poll�����Ľӿ�
//---------------------------------------------------------------------
struct APOLL_DRIVER APOLL_POLL = {
	sizeof (IPD_POLL),
	APDEVICE_POLL,
	4,
	"POLL",
	app_startup,
	app_shutdown,
	app_init_pd,
	app_destroy_pd,
	app_poll_add,
	app_poll_del,
	app_poll_set,
	app_poll_wait,
	app_poll_event
};

#ifdef PSTRUCT
#undef PSTRUCT
#endif

#define PSTRUCT IPD_POLL



//---------------------------------------------------------------------
// ��ʼ��poll�豸
//---------------------------------------------------------------------
static int app_startup(void)
{
	return 0;
}

//---------------------------------------------------------------------
// ��ԭpoll�豸
//---------------------------------------------------------------------
static int app_shutdown(void)
{
	return 0;
}

//---------------------------------------------------------------------
// ��ʼ��poll������
//---------------------------------------------------------------------
static int app_init_pd(apolld ipd, int param)
{
	PSTRUCT *ps = PDESC(ipd);

	apr_poll_fvinit(&ps->fv, NULL);

	iv_init(&ps->vpollfd, NULL);
	iv_init(&ps->vresultq, NULL);
	ps->fd_max = 0;
	ps->fd_min = 0x7fffffff;
	ps->pnum_max = 0;
	ps->pnum_cnt = 0;
	ps->result_num = -1;
	ps->result_cur = -1;
	param = param;

	return 0;
}

//---------------------------------------------------------------------
// ����poll������
//---------------------------------------------------------------------
static int app_destroy_pd(apolld ipd)
{
	PSTRUCT *ps = PDESC(ipd);
	
	apr_poll_fvdestroy(&ps->fv);
	iv_destroy(&ps->vpollfd);
	iv_destroy(&ps->vresultq);
	ps->fd_max = 0;
	ps->fd_min = 0x7fffffff;
	ps->pnum_max = 0;
	ps->pnum_cnt = 0;
	ps->result_num = -1;
	ps->result_cur = -1;

	return 0;
}

//---------------------------------------------------------------------
// ��poll����һ���ļ�����
//---------------------------------------------------------------------
static int app_poll_add(apolld ipd, int fd, int mask, void *user)
{
	PSTRUCT *ps = PDESC(ipd);
	long ofd_max = (long)ps->fd_max;
	struct pollfd *pfd;
	int retval, index, i;

	if (fd > ps->fd_max) ps->fd_max = fd;
	if (fd < ps->fd_min) ps->fd_min = fd;

	if (apr_poll_fvresize(&ps->fv, ps->fd_max + 2)) return -1;

	for (i = ofd_max + 1; i <= ps->fd_max; i++) {
		ps->fv.fds[i].fd = -1;
		ps->fv.fds[i].user = NULL;
		ps->fv.fds[i].mask = 0;
	}

	// already added in
	if (ps->fv.fds[fd].fd >= 0) return 1;

	if (ps->pnum_cnt >= ps->pnum_max) {
		i = (ps->pnum_max <= 0)? 4 : ps->pnum_max * 2;
		retval = iv_resize(&ps->vpollfd, sizeof(struct pollfd) * i);
		if (retval) return -3;
		retval = iv_resize(&ps->vresultq, sizeof(struct pollfd) * i);
		if (retval) return -4;
		ps->pnum_max = i;
		ps->pfds = (struct pollfd*)ps->vpollfd.data;
		ps->resultq = (struct pollfd*)ps->vresultq.data;
	}

	index = ps->pnum_cnt++;
	pfd = &ps->pfds[index];
	pfd->fd = fd;
	pfd->events = 0;
	if (mask & APOLL_IN) pfd->events |= POLLIN;
	if (mask & APOLL_OUT)pfd->events |= POLLOUT;
	if (mask & APOLL_ERR)pfd->events |= POLLERR;
	pfd->revents = 0;

	ps->fv.fds[fd].fd = fd;
	ps->fv.fds[fd].index = index;
	ps->fv.fds[fd].user = user;
	ps->fv.fds[fd].mask = mask;

	return 0;
}

//---------------------------------------------------------------------
// ��poll��ɾ��һ���ļ�����
//---------------------------------------------------------------------
static int app_poll_del(apolld ipd, int fd)
{
	PSTRUCT *ps = PDESC(ipd);
	int index, last, lastfd;

	if (fd < ps->fd_min || fd > ps->fd_max) return -1;
	if (ps->fv.fds[fd].fd < 0) return 0;
	if (ps->fv.fds[fd].index < 0) return 0;
	if (ps->pnum_cnt <= 0) return -2;

	last = ps->pnum_cnt - 1;
	index = ps->fv.fds[fd].index;
	ps->pfds[index] = ps->pfds[last];
	lastfd = ps->pfds[index].fd;
	ps->fv.fds[lastfd].index = index;
	ps->fv.fds[fd].index = -1;

	ps->fv.fds[fd].fd = -1;
	ps->fv.fds[fd].mask = 0;
	ps->fv.fds[fd].user = NULL;

	ps->pnum_cnt--;

	
	return 0;
}

//---------------------------------------------------------------------
// ����poll��ĳ�ļ��������¼�
//---------------------------------------------------------------------
static int app_poll_set(apolld ipd, int fd, int mask)
{
	PSTRUCT *ps = PDESC(ipd);
	int index, events = 0;
	if (fd < ps->fd_min || fd > ps->fd_max) return -1;
	if (ps->fv.fds[fd].fd < 0) return 0;

	index = ps->fv.fds[fd].index;
	if (ps->pfds[index].fd != fd) return -3;
	if (mask & APOLL_IN) events |= POLLIN;
	if (mask & APOLL_OUT) events |= POLLOUT;
	if (mask & APOLL_ERR) events |= POLLERR;
	ps->pfds[index].events = events;
	ps->fv.fds[fd].mask = mask;

	return 0;
}

//---------------------------------------------------------------------
// ����poll����������׽�¼�
//---------------------------------------------------------------------
static int app_poll_wait(apolld ipd, int timeval)
{
	PSTRUCT *ps = PDESC(ipd);
	long retval, i, j;

	retval = poll(ps->pfds, ps->pnum_cnt, timeval);
	ps->result_num = -1;
	if (retval < 0) {
		return retval;
	}
	ps->result_num = 0;
	ps->result_cur = 0;
	for (i = 0; i < ps->pnum_cnt; i++) {
		if (ps->pfds[i].revents) {
			j = ps->result_num++;
			ps->resultq[j] = ps->pfds[i];
		}
	}
	return retval;
}

//---------------------------------------------------------------------
// �Ӳ�׽���¼��б��в�����һ���¼�
//---------------------------------------------------------------------
static int app_poll_event(apolld ipd, int *fd, int *event, void **user)
{
	PSTRUCT *ps = PDESC(ipd);
	int revents, eventx = 0, n;
	struct pollfd *pfd;
	if (ps->result_num < 0) return -1;
	if (ps->result_cur >= ps->result_num) return -2;
	pfd = &ps->resultq[ps->result_cur++];

	revents = pfd->revents;
	if (revents & POLLIN) eventx |= APOLL_IN;
	if (revents & POLLOUT)eventx |= APOLL_OUT;
	if (revents & POLLERR)eventx |= APOLL_ERR;

	n = pfd->fd;
	if (ps->fv.fds[n].fd < 0) eventx = 0;
	eventx &= ps->fv.fds[n].mask;

	if (fd) *fd = n;
	if (event) *event = eventx;
	if (user) *user = ps->fv.fds[n].user;

	return 0;
}



#endif

