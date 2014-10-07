//=====================================================================
//
// TML <Transmod Library>, by skywind 2004, transmod.h
//
// NOTES�� 
// ���紫��� TML<����ģ��>������ �ͻ�/Ƶ����ͨ��ģʽ���ṩ���ڶ�Ƶ��
// multi-channelͨ�ŵ� TCP/UDPͨ�Ż��ƣ�����/�ڴ������ʱ���ƵȻ���
// �ķ���ӿڣ��Ե�ϵͳռ����ɼ�ʱ�����˼�ͨ������
//
//=====================================================================


#ifndef __TRANSMOD_H__
#define __TRANSMOD_H__

#if defined(__APPLE__) && (!defined(__unix))
    #define __unix
#endif

#ifndef APR_MODULE
#ifdef __unix
#define APR_MODULE(type)  type 
#elif defined(_WIN32)
#define APR_MODULE(type)  __declspec(dllexport) type
#else
#error APR-MODULE only can be compiled under unix or win32 !!
#endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

//---------------------------------------------------------------------
// Service Status Definition
//---------------------------------------------------------------------

#define CTMS_SERVICE		0	// ����״̬
#define CTMS_CUCOUNT		1	// �����ⲿ������
#define CTMS_CHCOUNT		2	// ����Ƶ��������
#define CTMS_WTIME			3	// ȡ�÷�������ʱ��
#define CTMS_STIME			4	// ȡ�ÿ�ʼ����ʱ��
#define CTMS_CSEND			5	// ��������
#define CTMS_CRECV			6	// ��������
#define CTMS_CDISCARD		7	// ��������

#define CTM_STOPPED		0	// ����״̬��ֹͣ
#define CTM_STARTING	1	// ����״̬��������
#define CTM_RUNNING		2	// ����״̬������
#define CTM_STOPPING	3	// ����״̬��ֹͣ��

#define CTM_OK			0	// û�д���
#define CTM_ERROR		1	// ��������


//---------------------------------------------------------------------
// Service Method Definition
//---------------------------------------------------------------------

// ��������
APR_MODULE(int) ctm_startup(void);

// �رշ���
APR_MODULE(int) ctm_shutdown(void);

// ȡ�÷���״̬
APR_MODULE(long) ctm_status(int item);

// ȡ�ô������
APR_MODULE(long) ctm_errno(void);


//---------------------------------------------------------------------
// Options Method Definition
//---------------------------------------------------------------------
#define CTMO_HEADER		0	// ͷ��ģʽ
#define CTMO_WTIME		1	// ����ʱ��
#define CTMO_PORTU4		2	// IPv4 �ⲿ�˿�
#define CTMO_PORTC4		3	// IPv4 �ڲ��˿�
#define CTMO_PORTU6		4	// IPv6 �ⲿ�˿�
#define CTMO_PORTC6		5	// IPv6 �ڲ��˿�
#define CTMO_PORTD4		6	// IPv4 ���ݱ��˿�
#define CTMO_PORTD6		7	// IPv6 ���ݱ��˿�
#define CTMO_HTTPSKIP	8	// ���� HTTPͷ��
#define CTMO_DGRAM		9	// ���ݱ�����ģʽ

#define CTMO_MAXCU		20	// �ⲿ�������
#define CTMO_MAXCC		21	// �ڲ��������
#define CTMO_TIMEU		22	// �ⲿ���ӳ�ʱ
#define CTMO_TIMEC		23	// �ڲ����ӳ�ʱ
#define CTMO_LIMIT		24	// Ƶ���������
#define CTMO_LIMTU		25	// �ⲿ��󻺴�
#define CTMO_LIMTC		26	// �ڲ���󻺴�
#define CTMO_ADDRC4		27	// IPv4 �ڲ��󶨵�ַ
#define CTMO_ADDRC6		28	// IPv6 �ڲ��󶨵�ַ

#define CTMO_DATMAX		40	// �������
#define CTMO_DHCPBASE	41	// ��͵ķ���
#define CTMO_DHCPHIGH	42	// ��ߵķ���
#define CTMO_PSIZE		43	// ҳ���СĬ��4K

#define CTMO_PLOGP		80	// ������־��ӡ����ָ��
#define	CTMO_PENCP		81	// ���ü��ܺ���ָ��
#define CTMO_LOGMK		82	// ��־��������
#define CTMO_INTERVAL	83	// �������ģʽ
#define CTMO_UTIME		84	// �ͻ��˼�ʱģʽ
#define CTMO_REUSEADDR	85	// �׽��ֵ�ַ���ã�0�Զ� 1���� 2��ֹ
#define CTMO_REUSEPORT	86	// �׽��ֶ˿����ã�0�Զ� 1���� 2��ֹ
#define CTMO_SOCKSNDO	90	// �ⲿ�׽��ַ��ͻ���
#define CTMO_SOCKRCVO	91	// �ⲿ�׽��ֽ��ջ���
#define CTMO_SOCKSNDI	92	// �ڲ��׽��ַ��ͻ���
#define CTMO_SOCKRCVI	93	// �ڲ��׽��ֽ��ջ���
#define CTMO_SOCKUDPB	94	// ���ݱ��׽��ֻ���



// ���÷������
APR_MODULE(int) ctm_config(int item, long value, const char *text);


typedef int (*CTM_LOG_HANDLE)(const char *);
typedef int (*CTM_ENCRYPT_HANDLE)(void*, const void *, int, int, int);
typedef int (*CTM_VALIDATE_HANDLE)(const void *sockaddr);

// ������־����Handle
APR_MODULE(int) ctm_handle_log(CTM_LOG_HANDLE handle);

// ���ü��ܺ���Handle
APR_MODULE(int) ctm_handle_encrypt(CTM_ENCRYPT_HANDLE handle);

// ������֤����Handle
APR_MODULE(int) ctm_handle_validate(CTM_VALIDATE_HANDLE handle);

// ����Ĭ����־����ӿ�
// mode=0�ǹر�, 1���ļ���2�Ǳ�׼���룬4�Ǳ�׼����
APR_MODULE(int) cmt_handle_logout(int mode, const char *fn_prefix);


//---------------------------------------------------------------------
// ������Ϣ
//---------------------------------------------------------------------

// ȡ�ñ���汾��
APR_MODULE(int) ctm_version(void);

// ȡ�ñ�������
APR_MODULE(const char*) ctm_get_date(void);

// ȡ�ñ���ʱ��
APR_MODULE(const char*) ctm_get_time(void);

// ͳ����Ϣ���õ������˶��ٰ����յ����ٰ����������ٰ������Ʒ��ͻ���ģʽ��
// ע�⣺����ָ��ᱻ��� 64λ������
APR_MODULE(void) ctm_get_stat(void *stat_send, void *stat_recv, void *stat_discard);


#ifdef __cplusplus
}
#endif

#endif



