//=====================================================================
//
// TML <Transmod Library>, by skywind 2004, itransmod.h
//
// HISTORY:
// Apr. 27 2010   skywind  fixed: when sys-timer changed, maybe error
// Mar. 15 2011   skywind  64bit support, header size configurable
// Jun. 25 2011   skywind  implement channel subscribe
// Sep. 09 2011   skywind  new: socket buf resize, congestion ctrl.
// Nov. 30 2011   skywind  new: channel broadcasting (v2.40)
// Dec. 23 2011   skywind  new: rc4 crypt (v2.43)
// Dec. 28 2011   skywind  rc4 enchance (v2.44)
// Mar. 03 2012   skywind  raw data header (v2.45)
// Dec. 01 2013   skywind  solaris /dev/poll supported (v2.64)
// Apr. 01 2014   skywind  new ITMH_LINESPLIT (v2.65)
//
// NOTES�� 
// ���紫��� TML<����ģ��>������ �ͻ�/Ƶ����ͨ��ģʽ���ṩ���ڶ�Ƶ��
// multi-channelͨ�ŵ� TCP/UDPͨ�Ż��ƣ�����/�ڴ������ʱ���ƵȻ���
// �ķ���ӿڣ��Ե�ϵͳռ����ɼ�ʱ�����˼�ͨ������
// 
//=====================================================================

#ifndef __I_TRANSMOD_H__
#define __I_TRANSMOD_H__

#include "aprsys.h"			// ����ƽ̨ϵͳ����ģ��
#include "aprsock.h"		// ����ƽ̨�׽��ַ�װģ��
#include "aprpoll.h"		// ����ƽ̨POLL��װģ��

#include "icvector.h"		// ͨ����������ģ��
#include "impoold.h"		// ͨ���ڴ����ģ��
#include "imstream.h"		// ͨ�û������ģ��
#include "idtimeout.h"		// ��ʱ����ģ��
#include "icqueue.h"		// �ȴ����п���ģ��

#ifdef __unix
#include <netinet/tcp.h>	// �������ӵ�ͷ�ļ�
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define ITMV_VERSION 0x268	// ����ģ��汾��

//=====================================================================
// Global Variables Definition
//=====================================================================

extern int itm_outer_port4;	// IPv4 ��������˿�
extern int itm_inner_port4;	// IPv4 ���ڼ����˿�
extern int itm_dgram_port4;	// IPv4 ���ݱ��˿�
extern int itm_outer_port6;	// IPv6 ��������˿� 
extern int itm_inner_port6;	// IPv6 ���ڼ����˿�
extern int itm_dgram_port6;	// IPv6 ���ݱ��˿�
extern int itm_outer_sock4;	// ��������׽���
extern int itm_inner_sock4;	// ���ڼ����׽���
extern int itm_dgram_sock4;	// ���ݱ��׽���
extern int itm_outer_sock6;	// IPv6 ��������׽���
extern int itm_inner_sock6;	// IPv6 ���ڼ����׽���
extern int itm_dgram_sock6;	// IPv6 ���ݱ��׽���
extern int itm_outer_max;	// �����������
extern int itm_inner_max;	// �����������
extern int itm_outer_cnt;	// ���⵱ǰ����
extern int itm_inner_cnt;	// ���ڵ�ǰ����
extern apolld itm_polld;	// �ṩ��APR_POLL���¼���׽������

extern int itm_state;		// ��ǰ״̬
extern int itm_error;		// �������
extern int itm_psize;		// �ڴ�ҳ���С
extern int itm_backlog;		// ��������backlog
extern int itm_counter;		// ����HID������
extern int itm_udpmask;		// ���ݱ�����

extern int itm_headmod;		// ͷ��ģʽ
extern int itm_headint;		// ͷ������
extern int itm_headinc;		// ͷ������
extern int itm_headlen;		// ͷ������
extern int itm_headmsk;		// ͷ������
extern int itm_hdrsize;		// ���ȵĳ���

extern long itm_outer_time;	// �ⲿ��������ʱ��
extern long itm_inner_time;	// �ڲ���������ʱ��
extern long itm_wtime;		// ����ʱ��
extern long itm_datamax;	// �������
extern long itm_limit;		// ���ͻ��泬���ͶϿ�

extern long itm_logmask;		// ��־����0 Ϊ�������־
extern char itm_msg[];			// ��Ϣ�ַ���
extern long itm_inner_addr4;	// �ڲ������󶨵�IP
extern char itm_inner_addr6[];	// �ڲ������󶨵� IPv6��ַ

extern long itm_outer_blimit;	// �ⲿ�׽��ֻ��漫��
extern long itm_inner_blimit;	// �ڲ�Ƶ�����漫��
extern long itm_dgram_blimit;	// ���ݱ��׽��ֻ����С
extern long itm_socksndi;		// �ڲ��׽��ַ��ͻ����С
extern long itm_sockrcvi;		// �ڲ��׽��ֽ��ܻ����С
extern long itm_socksndo;		// �ⲿ�׽��ַ��ͻ����С
extern long itm_sockrcvo;		// �ⲿ�׽��ַ��ͻ����С

extern long  itm_hostc;			// �ڲ�Channel����
extern long  itm_datac;			// �ڲ����ݳ���

extern apr_int64 itm_time_start;	// ������ʱ��
extern apr_int64 itm_time_current;	// ��ǰ��ʱ��
extern apr_int64 itm_time_slap;		// ʱ������ʱ��

extern apr_int64 itm_notice_slap;	// Ƶ��ʱ���źŶ���
extern apr_int64 itm_notice_cycle;	// Ƶ��ʱ������
extern apr_int64 itm_notice_count;	// Ƶ��ʼ���źż�ʱ
extern apr_int64 itm_notice_saved;	// Ƶ����ʱ��ʼ

extern long itm_dropped;			// �����Ĳ��������ݱ�
extern long itm_utiming;			// �ͻ��˼�ʱģʽ

extern long itm_interval;			// ʱ����


#define ITMD_TIME_CYCLE		10		// ����ʱ�Ӷ���

extern long itm_fastmode;			// �Ƿ�����д�б�
extern long itm_httpskip;			// �Ƿ������� httpͷ��

extern int itm_dhcp_index;			// Ƶ����������
extern int itm_dhcp_base;			// Ƶ�������ַ
extern int itm_dhcp_high;			// Ƶ����������

extern apr_int64 itm_stat_send;			// ͳ�ƣ������˶��ٰ�
extern apr_int64 itm_stat_recv;			// ͳ�ƣ������˶��ٰ�
extern apr_int64 itm_stat_discard;		// ͳ�ƣ������˶��ٸ����ݰ�

extern int itm_reuseaddr;			// ��ַ���ã�0�Զ���1���� 2��ֹ
extern int itm_reuseport;			// �˿ڸ��ã�0�Զ���1���� 2��ֹ	


//=====================================================================
// ITM Connection Description Definition
//=====================================================================
#define ITMD_OUTER_HOST4	0	// �׽���ģʽ��IPv4 �ⲿ�������׽���
#define ITMD_INNER_HOST4	1	// �׽���ģʽ��IPv4 �ڲ��������׽���
#define ITMD_OUTER_HOST6	2	// �׽���ģʽ��IPv6 �ⲿ�������׽���
#define ITMD_INNER_HOST6	3	// �׽���ģʽ��IPv6 �ڲ��������׽���
#define ITMD_DGRAM_HOST4	4	// �׽���ģʽ��IPv4 ���ݱ����׽���
#define ITMD_DGRAM_HOST6	5	// �׽���ģʽ��IPv6 ���ݱ����׽���
#define ITMD_OUTER_CLIENT	6	// �׽���ģʽ���ⲿ���ӵ��׽���
#define ITMD_INNER_CLIENT	7	// �׽���ģʽ���ڲ����ӵ��׽���

#define ITMD_HOST_IS_OUTER(mode)	(((mode) & 1) == 0)
#define ITMD_HOST_IS_INNER(mode)	(((mode) & 1) == 1)
#define ITMD_HOST_IS_IPV4(mode)		(((mode) <= 1) || (mode) == ITMD_DGRAM_HOST4)
#define ITMD_HOST_IS_IPV6(mode)		(!ITMD_HOST_IS_IPV4(mode))

struct ITMD
{
	long mode;					// ģʽ��0��4
	long node;					// ��itm_fds�еĽڵ���
	long fd;					// �ļ�����
	long hid;					// ���
	long tag;					// �����û�����
	long mask;					// ��дmask
	long channel;				// Ƶ��
	long timeid;				// ��ʱ����е�����
	long initok;				// �Ƿ��������
	long ccnum;					// Ƶ���ڵ��û�����
	long session;				// �Ự���
	long touched;				// ���ݱ������Ƿ�����
	long dropped;				// һ�����������ݰ�
	long inwlist;				// �Ƿ��ڷ����б���
	long disable;				// �Ƿ��ֹ
	struct IVQNODE wnode;		// �ȴ������еĽڵ�
	struct IVQUEUE waitq;		// �ȴ�����
	struct IMSTREAM lstream;	// �зָ���
	struct IMSTREAM rstream;	// ������
	struct IMSTREAM wstream;	// д����
	unsigned long cnt_tcpr;		// TCP���ռ�����
	unsigned long cnt_tcpw;		// TCP���ͼ�����
	unsigned long cnt_udpr;		// UDP���ռ�����
	unsigned long cnt_udpw;		// UDP���ͼ�����
	unsigned char skipped;		// ���� httpͷ����
	unsigned char history1;		// ��ʷ�ַ���
	unsigned char history2;		// ��ʷ�ַ���
	unsigned char history3;		// ��ʷ�ַ���
	unsigned char history4;		// ��ʷ�ַ���
	struct sockaddr_in remote4;		// Զ�̵�ַ
	struct sockaddr_in dgramp4;		// ���ݱ���ַ
#ifdef AF_INET6
	struct sockaddr_in6 remote6;	// IPv6 Զ�˵�ַ
	struct sockaddr_in6 dgramp6;	// IPv6 ���ݱ���ַ
#endif
	int IsIPv6;					// �Ƿ�ʹ�� IPv6
#ifndef IDISABLE_RC4			// �ж� RC4�����Ƿ񱻽�ֹ
	int rc4_send_x;				// RC4 ���ͼ���λ��1
	int rc4_send_y;				// RC4 ���ͼ���λ��2
	int rc4_recv_x;				// RC4 ���ռ���λ��1
	int rc4_recv_y;				// RC4 ���ռ���λ��2
	unsigned char rc4_send_box[256];	// RC4 ���ͼ���BOX
	unsigned char rc4_recv_box[256];	// RC4 ���ռ���BOX
#endif
};


//=====================================================================
// Endian Transform Definition
//=====================================================================
#ifndef ITMHTONS			// �����ֽ���ת����ʽ��Ĭ�ϲ��ı�
#define ITMHTONS(x) (x)	
#endif

#ifndef ITMHTONL			// �����ֽ���ת����ʽ��Ĭ�ϲ��ı�
#define ITMHTONL(x) (x)
#endif

#ifndef ITMNTOHS			// �����ֽ���ת����ʽ��Ĭ�ϲ��ı�
#define ITMNTOHS(x) (x)
#endif

#ifndef ITMNTOHL			// �����ֽ���ת����ʽ��Ĭ�ϲ��ı�
#define ITMNTOHL(x) (x)
#endif

#define ITMDINCD(x) ((x)=((unsigned long)((x) + 1) & 0x7fffffff))



//=====================================================================
// Main Container Definition
//=====================================================================
extern struct IMPOOL itm_fds;	// �׽��������ṹ������
extern struct IMPOOL itm_mem;	// �ڴ�ҳ�������

extern struct IVECTOR itm_datav;	// �ڲ�����ʸ��
extern struct IVECTOR itm_hostv;	// �ڲ�Channel�б�ʸ��
extern struct ITMD **itm_host;		// �ڲ�Channel�б�ָ��
extern char *itm_data;				// �ڲ������ֽ�ָ��
extern char *itm_crypt;				// �ڲ����ݼ���ָ��

extern char *itm_document;			// �ĵ�����
extern long itm_docsize;			// �ĵ�����
extern unsigned int itm_version;	// �ĵ��汾

extern struct ITMD itmd_inner4;		// IPv4 �ڲ�������ITMD(�׽�������)
extern struct ITMD itmd_outer4;		// IPv4 �ⲿ������ITMD(�׽�������)
extern struct ITMD itmd_dgram4;		// IPv4 ���ݱ��׽��ֵ�ITMD
extern struct ITMD itmd_inner6;		// IPv6 �ڲ�������ITMD(�׽�������)
extern struct ITMD itmd_outer6;		// IPv6 �ⲿ������ITMD(�׽�������)
extern struct ITMD itmd_dgram6;		// IPv6 ���ݱ��׽��ֵ�ITMD

extern struct IMSTREAM itm_dgramdat4;	// IPv4 ���ݱ�����
extern struct IMSTREAM itm_dgramdat6;	// IPv6 ���ݱ�����


//=====================================================================
// Public Method Definition
//=====================================================================
int itm_startup(void);	// ��������
int itm_shutdown(void);	// �رշ���
int itm_process(long timeval);	// ����һ���¼�


//=====================================================================
// Timeout Control Definition
//=====================================================================
extern struct IDTIMEV itm_timeu;	// �ⲿ���ӳ�ʱ������
extern struct IDTIMEV itm_timec;	// �ڲ����ӳ�ʱ������
int itm_timer(void);				// ʱ�ӿ���


//=====================================================================
// Private Method Definition
//=====================================================================
long itm_trysend(struct ITMD *itmd);	// ���Է��� wstream
long itm_tryrecv(struct ITMD *itmd);	// ���Խ��� rstream
long itm_trysendto(int af);				// ���Է������ݱ�

extern long itm_local1;		// �ֲ�����������ʱ��������1
extern long itm_local2;		// �ֲ�����������ʱ��������2

// ��ȡƵ��
#define itm_rchannel(x) ((x >= 0 && x < itm_hostc)? itm_host[x] : NULL)

int itm_wchannel(int index, struct ITMD *itmd);	// ����Ƶ��
long itm_dsize(long length);					// ���û������ݴ�С
int itm_permitr(struct ITMD *itmd);				// ����һ��READ�¼�

#define ITM_READ	1			// �׽����¼���ʽ����
#define ITM_WRITE	2			// �׽����¼���ʽ��д

#define ITM_BUFSIZE 0x10000		// ���շ�����ʱ����
#define ITM_ADDRSIZE4 (sizeof(struct sockaddr))
#define ITM_ADDRSIZE6 (sizeof(struct sockaddr_in6))

extern char itm_zdata[];		// �ײ������շ�����

extern short *itm_book[512];	// ÿ���¼���ע��Ƶ���б�
extern int itm_booklen[512];	// ÿ���¼���ע��Ƶ������

int itm_mask(struct ITMD *itmd, int enable, int disable);	// �����¼���׽
int itm_send(struct ITMD *itmd, const void *data, long length);	// ��������
int itm_bcheck(struct ITMD *itmd);				// ��黺��

char *itm_epname4(const struct sockaddr *ep);	// ȡ�ö˵�����
char *itm_epname6(const struct sockaddr *ep);	// ȡ�ö˵�����
char *itm_epname(const struct ITMD *itmd);
char *itm_ntop(int af, const struct sockaddr *remote);

int itm_book_add(int category, int channel);	// ���ӹ�ע
int itm_book_del(int category, int channel);	// ȡ����ע
int itm_book_reset(int channel);				// Ƶ��ȫ��������ע
int itm_book_empty(void);						// ��ע��λ

// RC4 ��ʼ��������
void itm_rc4_init(unsigned char *box, int *x, int *y, const unsigned char *key, int keylen);
void itm_rc4_crypt(unsigned char *box, int *x, int *y, const unsigned char *src, unsigned char *dst, long size);

// �ⲿ�¼�
long itm_msg_put(int id, const char *data, long size);
long itm_msg_get(int id, char *data, long maxsize);


//---------------------------------------------------------------------
// ���ݱ�����
//---------------------------------------------------------------------
struct ITMHUDP
{
	apr_uint32 order;
	apr_uint32 index;
	apr_int32 hid;
	apr_int32 session;
};

#define ITMU_MWORK	0x0001		// ���ݱ����룺���ݱ�ϵͳ����
#define ITMU_MDUDP	0x0002		// �������룺udp�������ݴ���
#define ITMU_MDTCP	0x0004		// �������룺tcp������ݴ���

#define ITMU_TOUCH	0x6000		// ���UDP��ʼ������
#define ITMU_ECHO	0x6001		// �����������
#define ITMU_MIRROR	0x6002		// �����������
#define ITMU_DELIVER 0x6003		// ���ת�ƴ���
#define ITMU_FORWARD 0x6004		// ���ת��


int itm_sendudp(int af, struct sockaddr *remote, struct ITMHUDP *head, const void *data, long len);
int itm_sendto(struct ITMD *itmd, const void *data, long length);	// �������ݱ�
int itm_optitmd(struct ITMD *itmd, int flags);						// �������Ӳ���

//---------------------------------------------------------------------
// ��Ϣ��Ƶ������
//---------------------------------------------------------------------
static inline struct ITMD* itm_hid_itmd(long hid)
{
	struct ITMD *itmd;
	long c = (hid & 0xffff);
	if (c < 0 || c >= itm_fds.node_max) return NULL;
	itmd = (struct ITMD*)IMP_DATA(&itm_fds, c);
	if (hid != itmd->hid) return NULL;
	return itmd;
}

//---------------------------------------------------------------------
// �����б���ز���
//---------------------------------------------------------------------
int itm_wlist_record(long hid);		// ��¼�������б�

//---------------------------------------------------------------------
// ���ݼ��ܺ�IP��֤ģ�麯��ָ��
//---------------------------------------------------------------------
typedef int (*ITM_ENCRYPT_HANDLE)(void*, const void *, int, int, int);
typedef int (*ITM_VALIDATE_HANDLE)(const void *sockaddr);
extern int (*itm_encrypt)(void *output, const void *input, int length, int fd, int mode);	
extern int (*itm_validate)(const void *sockaddr);

//---------------------------------------------------------------------
// ����ģ��
//---------------------------------------------------------------------
typedef int (*ITM_LOG_HANDLE)(const char *);
extern int (*itm_logv)(const char *text);

int itm_log(int mask, const char *argv, ...);
void itm_lltoa(char *dst, apr_int64 x);


//=====================================================================
// �����¼�������
//=====================================================================
#define ITMT_NEW		0	// �½��ⲿ���ӣ�(id,tag) ip/d,port/w
#define ITMT_LEAVE		1	// �Ͽ��ⲿ���ӣ�(id,tag)
#define ITMT_DATA		2	// �ⲿ���ݵ��(id,tag) data...
#define ITMT_CHANNEL	3	// Ƶ��ͨ�ţ�(channel,tag)
#define ITMT_CHNEW		4	// Ƶ��������(channel,id)
#define ITMT_CHSTOP		5	// Ƶ���Ͽ���(channel,tag)
#define ITMT_SYSCD		6	// ϵͳ��Ϣ��(subtype, v) data...
#define ITMT_TIMER		7	// ϵͳʱ�ӣ�(timesec,timeusec)
#define ITMT_UNRDAT		10	// ���ɿ����ݰ���(id,tag)
#define ITMT_NOOP		80	// ��ָ�(wparam, lparam)

#define ITMC_DATA		0	// �ⲿ���ݷ��ͣ�(id,*) data...
#define ITMC_CLOSE		1	// �ر��ⲿ���ӣ�(id,code)
#define ITMC_TAG		2	// ����TAG��(id,tag)
#define ITMC_CHANNEL	3	// ���ͨ�ţ�(channel,*) data...
#define ITMC_MOVEC		4	// �ƶ��ⲿ���ӣ�(channel,id) data...
#define ITMC_SYSCD		5	// ϵͳ������Ϣ��(subtype, v) data...
#define ITMC_BROADCAST	6	// �㲥���ݣ�(count, limit) data, hidlist...
#define ITMC_UNRDAT		10	// ���ɿ����ݰ���(id,tag)
#define ITMC_IOCTL		11	// ���ӿ���ָ�(id,flag)
#define ITMC_NOOP		80	// ��ָ�(*,*)

#define ITMS_CONNC		0	// ������������(st,0) cu/d,cc/d
#define ITMS_LOGLV		1	// ������־����(st,level)
#define ITMS_LISTC		2	// ����Ƶ����Ϣ(st,cn) d[ch,id,tag],w[t,c]
#define ITMS_RTIME		3	// ϵͳ����ʱ��(st,wtime)
#define ITMS_TMVER		4	// ����ģ��汾(st,tmver)
#define ITMS_REHID		5	// ����Ƶ����(st,ch)
#define ITMS_QUITD		6	// �����Լ��˳�
#define ITMS_TIMER		8	// ����Ƶ�����ʱ��(st,timems)
#define ITMS_INTERVAL	9	// �����Ƿ�Ϊ���ģʽ(st,isinterval)
#define ITMS_FASTMODE	10	// �����Ƿ�ʹ�����Ӷ����㷨
#define ITMS_CHID		11	// ȡ���Լ���channel���(st, ch)
#define ITMS_BOOKADD	12	// ���Ӷ���
#define ITMS_BOOKDEL	13	// ȡ������
#define ITMS_BOOKRST	14	// ��ն���
#define ITMS_STATISTIC	15	// ͳ����Ϣ
#define ITMS_RC4SKEY	16	// ���÷���KEY (st, hid) key
#define ITMS_RC4RKEY	17	// ���ý���KEY (st, hid) key
#define ITMS_DISABLE	18	// ��ֹ���ո��û���Ϣ
#define ITMS_ENABLE		19	// ������ո��û���Ϣ
#define ITMS_SETDOC		20	// �ĵ�����
#define ITMS_GETDOC		21	// �ĵ���ȡ
#define ITMS_MESSAGE	22	// �ⲿ�����¼� 
#define ITMS_NODELAY	1	// ���ӿ��ƣ�������������ģʽ
#define ITMS_NOPUSH		2	// ���ӿ��ƣ���������������
#define ITMS_PRIORITY	3	// SO_PRIORITY
#define ITMS_TOS		4	// IP_TOS

#define ITMH_WORDLSB	0		// ͷ����־��2�ֽ�LSB
#define ITMH_WORDMSB	1		// ͷ����־��2�ֽ�MSB
#define ITMH_DWORDLSB	2		// ͷ����־��4�ֽ�LSB
#define ITMH_DWORDMSB	3		// ͷ����־��4�ֽ�MSB
#define ITMH_BYTELSB	4		// ͷ����־�����ֽ�LSB
#define ITMH_BYTEMSB	5		// ͷ����־�����ֽ�MSB
#define ITMH_EWORDLSB	6		// ͷ����־��2�ֽ�LSB���������Լ���
#define ITMH_EWORDMSB	7		// ͷ����־��2�ֽ�MSB���������Լ���
#define ITMH_EDWORDLSB	8		// ͷ����־��4�ֽ�LSB���������Լ���
#define ITMH_EDWORDMSB	9		// ͷ����־��4�ֽ�MSB���������Լ���
#define ITMH_EBYTELSB	10		// ͷ����־�����ֽ�LSB���������Լ���
#define ITMH_EBYTEMSB	11		// ͷ����־�����ֽ�MSB���������Լ���
#define ITMH_DWORDMASK	12		// ͷ����־��4�ֽ�LSB�������Լ������룩
#define ITMH_RAWDATA	13		// ͷ����־��������ͷ��������4�ֽ�LSB
#define ITMH_LINESPLIT	14		// ͷ����־��������ͷ�������зָ����4�ֽ�LSB


#define ITML_BASE		0x01	// ��־���룺����
#define ITML_INFO		0x02	// ��־���룺��Ϣ
#define ITML_ERROR		0x04	// ��־���룺����
#define ITML_WARNING	0x08	// ��־���룺����
#define ITML_DATA		0x10	// ��־���룺����
#define ITML_CHANNEL	0x20	// ��־���룺Ƶ��
#define ITML_EVENT		0x40	// ��־���룺�¼�
#define ITML_LOST		0x80	// ��־���룺������¼


//---------------------------------------------------------------------
// �������봦��
//---------------------------------------------------------------------
int itm_data_inner(struct ITMD *itmd);					// �ڲ����ݰ�����
int itm_data_outer(struct ITMD *itmd);					// �ⲿ���ݰ�����

//---------------------------------------------------------------------
// �����¼�����
//---------------------------------------------------------------------
int itm_event_accept(int hmode);						// �¼�������������
int itm_event_close(struct ITMD *itmd, int code);		// �¼����ر�����
int itm_event_recv(struct ITMD *itmd);					// �¼�������
int itm_event_send(struct ITMD *itmd);					// �¼�������
int itm_event_dgram(int af);							// �¼������ݱ�����

int itm_dgram_data(int af, struct sockaddr *remote, struct ITMHUDP *head, void *data, long size);
int itm_dgram_cmd(int af, struct sockaddr *remote, struct ITMHUDP *head, void *data, long size);

//---------------------------------------------------------------------
// Ƶ���¼�����
//---------------------------------------------------------------------
int itm_on_logon(struct ITMD *itmd);
int itm_on_data(struct ITMD *itmd, long wparam, long lparam, long length);
int itm_on_close(struct ITMD *itmd, long wparam, long lparam, long length);
int itm_on_tag(struct ITMD *itmd, long wparam, long lparam, long length);
int itm_on_channel(struct ITMD *itmd, long wparam, long lparam, long length);
int itm_on_movec(struct ITMD *itmd, long wparam, long lparam, long length);
int itm_on_syscd(struct ITMD *itmd, long wparam, long lparam, long length);
int itm_on_dgram(struct ITMD *itmd, long wparam, long lparam, long length);
int itm_on_ioctl(struct ITMD *itmd, long wparam, long lparam, long length);
int itm_on_broadcast(struct ITMD *itmd, long wparam, long lparam, long length);


//---------------------------------------------------------------------
// ���ú���
//---------------------------------------------------------------------
static inline char *iencode16u_lsb(char *p, unsigned short w)
{
#if IWORDS_BIG_ENDIAN
	*(unsigned char*)(p + 0) = (w & 255);
	*(unsigned char*)(p + 1) = (w >> 8);
#else
	*(unsigned short*)(p) = w;
#endif
	p += 2;
	return p;
}

static inline char *idecode16u_lsb(const char *p, unsigned short *w)
{
#if IWORDS_BIG_ENDIAN
	*w = *(const unsigned char*)(p + 1);
	*w = *(const unsigned char*)(p + 0) + (*w << 8);
#else
	*w = *(const unsigned short*)p;
#endif
	p += 2;
	return (char*)p;
}


static inline char *iencode16u_msb(char *p, unsigned short w)
{
#if IWORDS_BIG_ENDIAN
	*(unsigned short*)(p) = w;
#else
	*(unsigned char*)(p + 0) = (w >> 8);
	*(unsigned char*)(p + 1) = (w & 255);
#endif
	p += 2;
	return p;
}

static inline char *idecode16u_msb(const char *p, unsigned short *w)
{
#if IWORDS_BIG_ENDIAN
	*w = *(const unsigned short*)p;
#else
	*w = *(const unsigned char*)(p + 0);
	*w = *(const unsigned char*)(p + 1) + (*w << 8);
#endif
	p += 2;
	return (char*)p;
}

static inline char *iencode32u_lsb(char *p, apr_uint32 l)
{
#if IWORDS_BIG_ENDIAN
	*(unsigned char*)(p + 0) = (unsigned char)((l >>  0) & 0xff);
	*(unsigned char*)(p + 1) = (unsigned char)((l >>  8) & 0xff);
	*(unsigned char*)(p + 2) = (unsigned char)((l >> 16) & 0xff);
	*(unsigned char*)(p + 3) = (unsigned char)((l >> 24) & 0xff);
#else
	*(apr_uint32*)p = l;
#endif
	p += 4;
	return p;
}

static inline char *idecode32u_lsb(const char *p, apr_uint32 *l)
{
#if IWORDS_BIG_ENDIAN
	*l = *(const unsigned char*)(p + 3);
	*l = *(const unsigned char*)(p + 2) + (*l << 8);
	*l = *(const unsigned char*)(p + 1) + (*l << 8);
	*l = *(const unsigned char*)(p + 0) + (*l << 8);
#else 
	*l = *(const apr_uint32*)p;
#endif
	p += 4;
	return (char*)p;
}

static inline char *iencode32u_msb(char *p, apr_uint32 l)
{
#if IWORDS_BIG_ENDIAN
	*(apr_uint32*)p = l;
#else
	*(unsigned char*)(p + 0) = (unsigned char)((l >> 24) & 0xff);
	*(unsigned char*)(p + 1) = (unsigned char)((l >> 16) & 0xff);
	*(unsigned char*)(p + 2) = (unsigned char)((l >>  8) & 0xff);
	*(unsigned char*)(p + 3) = (unsigned char)((l >>  0) & 0xff);
#endif
	p += 4;
	return p;
}

static inline char *idecode32u_msb(const char *p, apr_uint32 *l)
{
#if IWORDS_BIG_ENDIAN
	*l = *(const apr_uint32*)p;
#else 
	*l = *(const unsigned char*)(p + 0);
	*l = *(const unsigned char*)(p + 1) + (*l << 8);
	*l = *(const unsigned char*)(p + 2) + (*l << 8);
	*l = *(const unsigned char*)(p + 3) + (*l << 8);
#endif
	p += 4;
	return (char*)p;
}

/* encode 8 bits unsigned int */
static inline char *iencode8u(char *p, unsigned char c)
{
	*(unsigned char*)p++ = c;
	return p;
}

/* decode 8 bits unsigned int */
static inline char *idecode8u(const char *p, unsigned char *c)
{
	*c = *(unsigned char*)p++;
	return (char*)p;
}

static inline long itm_size_get(const void *p)
{
	unsigned char cbyte;
	apr_uint16 cshort;
	apr_uint32 cint;
	long length = 0;

	switch (itm_headint)
	{
	case ITMH_WORDLSB:
		idecode16u_lsb((const char*)p, &cshort);
		length = cshort;
		break;
	case ITMH_WORDMSB:
		idecode16u_msb((const char*)p, &cshort);
		length = cshort;
		break;
	case ITMH_DWORDLSB:
		idecode32u_lsb((const char*)p, &cint);
		length = cint;
		break;
	case ITMH_DWORDMSB:
		idecode32u_msb((const char*)p, &cint);
		length = cint;
		break;
	case ITMH_BYTELSB:
	case ITMH_BYTEMSB:
		idecode8u((const char*)p, &cbyte);
		length = cbyte;
		break;
	}
	if (itm_headmsk) {
		length &= 0xffffff;
	}
	return length + itm_headinc;
}

static inline void itm_size_set(void *p, long size)
{
	size -= itm_headinc;
	switch (itm_headint)
	{
	case ITMH_WORDLSB:
		iencode16u_lsb((char*)p, (apr_uint16)size);
		break;
	case ITMH_WORDMSB:
		iencode16u_msb((char*)p, (apr_uint16)size);
		break;
	case ITMH_DWORDLSB:
		iencode32u_lsb((char*)p, (apr_uint32)size);
		break;
	case ITMH_DWORDMSB:
		iencode32u_msb((char*)p, (apr_uint32)size);
		break;
	case ITMH_BYTELSB:
	case ITMH_BYTEMSB:
		iencode8u((char*)p, (unsigned char)size);
		break;
	}
}

static inline int itm_cate_get(void *p)
{
	apr_uint32 length;
	if (itm_headmsk == 0) return 0;
	idecode32u_lsb((const char*)p, &length);
	return (int)((length >> 24) & 0xff);
}

static inline int itm_param_get(const void *ptr, long *length, short *cmd, long *wparam, long *lparam)
{
	const char *lptr = (const char*)ptr;
	unsigned char cbyte;
	apr_uint16 cshort;
	apr_uint32 cint;
	int size = 12;
	long x = 0;

	switch (itm_headint)
	{
	case ITMH_WORDLSB: 
		lptr = idecode16u_lsb(lptr, &cshort); x = cshort;
		lptr = idecode16u_lsb(lptr, (apr_uint16*)cmd); 
		lptr = idecode32u_lsb(lptr, &cint); *wparam = cint;
		lptr = idecode32u_lsb(lptr, &cint); *lparam = cint;
		break;
	case ITMH_WORDMSB: 
		lptr = idecode16u_msb(lptr, &cshort); x = cshort;
		lptr = idecode16u_msb(lptr, (apr_uint16*)cmd); 
		lptr = idecode32u_msb(lptr, &cint); *wparam = cint;
		lptr = idecode32u_msb(lptr, &cint); *lparam = cint;
		break;
	case ITMH_DWORDLSB: 
		lptr = idecode32u_lsb(lptr, &cint); x = cint;
		lptr = idecode16u_lsb(lptr, (apr_uint16*)cmd); 
		lptr = idecode32u_lsb(lptr, &cint); *wparam = cint;
		lptr = idecode32u_lsb(lptr, &cint); *lparam = cint;
		size = 14;
		break;
	case ITMH_DWORDMSB: 
		lptr = idecode32u_msb(lptr, &cint); x = cint;
		lptr = idecode16u_msb(lptr, (apr_uint16*)cmd); 
		lptr = idecode32u_msb(lptr, &cint); *wparam = cint;
		lptr = idecode32u_msb(lptr, &cint); *lparam = cint;
		size = 14;
		break;
	case ITMH_BYTELSB: 
		lptr = idecode8u(lptr, &cbyte); x = cbyte;
		lptr = idecode16u_lsb(lptr, (apr_uint16*)cmd); 
		lptr = idecode32u_lsb(lptr, &cint); *wparam = cint;
		lptr = idecode32u_lsb(lptr, &cint); *lparam = cint;
		size = 11;
		break;
	case ITMH_BYTEMSB: 
		lptr = idecode8u(lptr, &cbyte); x = cbyte;
		lptr = idecode16u_msb(lptr, (apr_uint16*)cmd); 
		lptr = idecode32u_msb(lptr, &cint); *wparam = cint;
		lptr = idecode32u_msb(lptr, &cint); *lparam = cint;
		size = 11;
		break;
	}
	*length = x + itm_headinc;
	return size;
}

static inline int itm_param_set(int offset, long length, short cmd, long wparam, long lparam)
{
	char *lptr = itm_data + offset;
	int size = 12;

	length -= itm_headinc;
	switch (itm_headint)
	{
	case ITMH_WORDLSB: 
		lptr = iencode16u_lsb(lptr, (apr_uint16)length);
		lptr = iencode16u_lsb(lptr, (apr_uint16)cmd);
		lptr = iencode32u_lsb(lptr, (apr_uint32)wparam);
		lptr = iencode32u_lsb(lptr, (apr_uint32)lparam);
		break;
	case ITMH_WORDMSB:
		lptr = iencode16u_msb(lptr, (apr_uint16)length);
		lptr = iencode16u_msb(lptr, (apr_uint16)cmd);
		lptr = iencode32u_msb(lptr, (apr_uint32)wparam);
		lptr = iencode32u_msb(lptr, (apr_uint32)lparam);
		break;
	case ITMH_DWORDLSB:
		lptr = iencode32u_lsb(lptr, (apr_uint32)length);
		lptr = iencode16u_lsb(lptr, (apr_uint16)cmd);
		lptr = iencode32u_lsb(lptr, (apr_uint32)wparam);
		lptr = iencode32u_lsb(lptr, (apr_uint32)lparam);
		size = 14;
		break;
	case ITMH_DWORDMSB:
		lptr = iencode32u_msb(lptr, (apr_uint32)length);
		lptr = iencode16u_msb(lptr, (apr_uint16)cmd);
		lptr = iencode32u_msb(lptr, (apr_uint32)wparam);
		lptr = iencode32u_msb(lptr, (apr_uint32)lparam);
		size = 14;
		break;
	case ITMH_BYTELSB:
		lptr = iencode8u(lptr, (unsigned char)length);
		lptr = iencode16u_lsb(lptr, (apr_uint16)cmd);
		lptr = iencode32u_lsb(lptr, (apr_uint32)wparam);
		lptr = iencode32u_lsb(lptr, (apr_uint32)lparam);
		size = 11;
		break;
	case ITMH_BYTEMSB:
		lptr = iencode8u(lptr, (unsigned char)length);
		lptr = iencode16u_msb(lptr, (apr_uint16)cmd);
		lptr = iencode32u_msb(lptr, (apr_uint32)wparam);
		lptr = iencode32u_msb(lptr, (apr_uint32)lparam);
		size = 11;
		break;
	}
	return size;
}


static inline long itm_dataok(struct ITMD *itmd)
{
	struct IMSTREAM *stream = &(itmd->rstream);
	long len;
	char buf[4];
	void *ptr = (void*)buf;

	if (itmd->mode == ITMD_OUTER_CLIENT) {
		if (itm_headmod == ITMH_RAWDATA) {
			long size = stream->size;
			long limit = (ITM_BUFSIZE < itm_datamax)? ITM_BUFSIZE : itm_datamax;
			if (size < limit) return size;
			return limit;
		}
	}

	len = ims_peek(stream, buf, itm_hdrsize);
	if (len < itm_hdrsize) return 0;

	len = (long)(itm_size_get(ptr));

	if (len < itm_hdrsize) return -1;
	if (len > itm_datamax) return -2;
	if (stream->size < len) return 0;

	return len;
}

static inline void itm_write_dword(void *ptr, apr_uint32 dword)
{
	if (itm_headint & 1) {
		iencode32u_msb((char*)ptr, dword);
	}	else {
		iencode32u_lsb((char*)ptr, dword);
	}
}

static inline void itm_write_word(void *ptr, apr_uint16 word)
{
	if (itm_headmod & 1) {
		iencode16u_msb((char*)ptr, word);
	}	else {
		iencode16u_lsb((char*)ptr, word);
	}
}

static inline unsigned short itm_read_word(const void *ptr)
{
	unsigned short word;
	if (itm_headmod & 1) {
		idecode16u_msb((const char*)ptr, &word);
	}	else {
		idecode16u_lsb((const char*)ptr, &word);
	}
	return word;
}

static inline apr_uint32 itm_read_dword(const void *ptr)
{
	apr_uint32 dword;
	if (itm_headmod & 1) {
		idecode32u_msb((const char*)ptr, &dword);
	}	else {
		idecode32u_lsb((const char*)ptr, &dword);
	}
	return dword;
}


#ifdef __cplusplus
}
#endif


#endif



