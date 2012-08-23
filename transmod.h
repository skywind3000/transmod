//=====================================================================
//
// TML <Transmod Library>, by skywind 2004, transmod.h
//
// NOTES： 
// 网络传输库 TML<传输模块>，建立 客户/频道的通信模式，提供基于多频道
// multi-channel通信的 TCP/UDP通信机制，缓存/内存管理，超时控制等机制
// 的服务接口，以低系统占用完成即时的万人级通信任务
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

#define CTMS_SERVICE		0	// 服务状态
#define CTMS_CUCOUNT		1	// 现在外部连接数
#define CTMS_CHCOUNT		2	// 现在频道连接数
#define CTMS_WTIME			3	// 取得服务运行时间
#define CTMS_STIME			4	// 取得开始服务时间
#define CTMS_CSEND			5	// 发送数量
#define CTMS_CRECV			6	// 接受数量
#define CTMS_CDISCARD		7	// 错误数量

#define CTM_STOPPED		0	// 服务状态：停止
#define CTM_STARTING	1	// 服务状态：启动中
#define CTM_RUNNING		2	// 服务状态：服务
#define CTM_STOPPING	3	// 服务状态：停止中

#define CTM_OK			0	// 没有错误
#define CTM_ERROR		1	// 发生错误


//---------------------------------------------------------------------
// Service Method Definition
//---------------------------------------------------------------------

// 开启服务
APR_MODULE(int) ctm_startup(void);

// 关闭服务
APR_MODULE(int) ctm_shutdown(void);

// 取得服务状态
APR_MODULE(long) ctm_status(int item);

// 取得错误代码
APR_MODULE(long) ctm_errno(void);


//---------------------------------------------------------------------
// Options Method Definition
//---------------------------------------------------------------------
#define CTMO_WTIME	0	// 世界事件
#define CTMO_PORTU	1	// 外部端口
#define CTMO_PORTC	2	// 内部端口
#define CTMO_MAXCU	3	// 外部最大连接
#define CTMO_MAXCC	4	// 内部最大连接
#define CTMO_TIMEU	5	// 外部连接超时
#define CTMO_TIMEC	6	// 内部连接超时
#define CTMO_ADDRC	7	// 内部绑定地址
#define CTMO_LOGMK	8	// 日志报告掩码
#define CTMO_LIMTU	9	// 外部最大缓存
#define CTMO_LIMTC	10	// 内部最大缓存
#define CTMO_PLOGP	11	// 设置日志打印函数指针
#define	CTMO_PENCP	12	// 设置加密函数指针
#define CTMO_HEADER	13	// 头部模式
#define CTMO_DATMAX 14	// 最大数据
#define CTMO_SOCKSNDO	15	// 外部套接字发送缓存
#define CTMO_SOCKRCVO	16	// 外部套接字接收缓存
#define CTMO_SOCKSNDI	17	// 内部套接字发送缓存
#define CTMO_SOCKRCVI	18	// 内部套接字接收缓存
#define CTMO_SOCKUDPB	19	// 数据报套接字缓存
#define CTMO_PORTD	20	// 数据报监听端口
#define CTMO_DGRAM	21	// 数据报启动模式
#define CTMO_UTIME	22	// 客户端计时模式
#define CTMO_INTERVAL	23	// 启动间隔模式
#define CTMO_DHCPBASE	24	// 最低的分配
#define CTMO_DHCPHIGH	25	// 最高的分配
#define CTMO_NOREUSE	26	// 禁用套接字地址重用
#define CTMO_HTTPSKIP	27	// 跳过 HTTP头部


// 设置服务参数
APR_MODULE(int) ctm_option(int item, long value);


typedef int (*CTM_LOG_HANDLE)(const char *);
typedef int (*CTM_ENCRYPT_HANDLE)(void*, const void *, int, int, int);
typedef int (*CTM_VALIDATE_HANDLE)(const void *sockaddr);

// 设置日志函数Handle
APR_MODULE(int) ctm_handle_log(CTM_LOG_HANDLE handle);

// 设置加密函数Handle
APR_MODULE(int) ctm_handle_encrypt(CTM_ENCRYPT_HANDLE handle);

// 设置验证函数Handle
APR_MODULE(int) ctm_handle_validate(CTM_VALIDATE_HANDLE handle);

// 设置默认日志输出接口
// mode=0是关闭, 1是文件，2是标准输入，4是标准错误
APR_MODULE(int) cmt_handle_logout(int mode, const char *fn_prefix);


//---------------------------------------------------------------------
// 编译信息
//---------------------------------------------------------------------

// 取得编译版本号
APR_MODULE(int) ctm_version(void);

// 取得编译日期
APR_MODULE(const char*) ctm_get_date(void);

// 取得编译时间
APR_MODULE(const char*) ctm_get_time(void);

// 统计信息：得到发送了多少包，收到多少包，丢弃多少包（限制发送缓存模式）
// 注意：三个指针会被填充 64位整数。
APR_MODULE(void) ctm_get_stat(void *stat_send, void *stat_recv, void *stat_discard);


#ifdef __cplusplus
}
#endif

#endif



