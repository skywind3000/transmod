//=====================================================================
//
// transmod.h
//
// TCP Transfer Cache Module, by skywind 2004
//
//=====================================================================



#include "transmod.h"

#include "aprsys.h"
#include "itransmod.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#if defined(_WIN32) || defined(WIN32)
#include <io.h>
#endif

//used by writev
#ifdef __unix
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#endif


volatile static int _ctm_status = CTM_STOPPED;
volatile static int _ctm_errno = CTM_OK;
static long _ctm_threadid = -1;

static void ctm_thread_working(void*);


#ifdef _MSC_VER
#pragma comment(lib,"wsock32.lib")
#endif


//---------------------------------------------------------------------
// 开启服务
//---------------------------------------------------------------------
APR_MODULE(int) ctm_startup(void)
{
	if (_ctm_status != CTM_STOPPED) return -1;
	if (_ctm_threadid >= 0) return -2;
	if (apr_thread_create(&_ctm_threadid, ctm_thread_working, NULL, NULL)) 
		return -4;
	
	return 0;
}


//---------------------------------------------------------------------
// 关闭服务
//---------------------------------------------------------------------
APR_MODULE(int) ctm_shutdown(void)
{
	if (_ctm_status == CTM_STOPPED) return 0;
	if (_ctm_status == CTM_STOPPING) return 0;

	_ctm_status = CTM_STOPPING;

	return 0;
}

//---------------------------------------------------------------------
// 取得服务状态
//---------------------------------------------------------------------
APR_MODULE(long) ctm_status(int item)
{
	long retval = 0;

	switch (item)
	{
	case CTMS_SERVICE:
		retval = _ctm_status;
		break;
	case CTMS_CUCOUNT:
		retval = itm_outer_cnt;
		break;
	case CTMS_CHCOUNT:
		retval = itm_inner_cnt;
		break;
	case CTMS_WTIME:
		retval = itm_wtime;
		break;
	}

	return retval;
}

//---------------------------------------------------------------------
// 服务线程
//---------------------------------------------------------------------
static void ctm_thread_working(void*p)
{
	long retval;

	_ctm_status = CTM_STARTING;

	itm_wtime = 0;
	retval = itm_startup();
	if (retval) {
		_ctm_errno = retval;
		goto _exitp;
	}

	_ctm_status = CTM_RUNNING;

	while (_ctm_status == CTM_RUNNING) {
		itm_process(ITMD_TIME_CYCLE);
	}

_exitp:
	itm_shutdown();
	_ctm_threadid = -1;
	_ctm_status = CTM_STOPPED;
}

//---------------------------------------------------------------------
// ctm_option
//---------------------------------------------------------------------
APR_MODULE(int) ctm_option(int item, long value)
{
	switch (item)
	{
	case CTMO_WTIME:	itm_wtime = value;			break;
	case CTMO_PORTU:	itm_outer_port = value;		break;
	case CTMO_PORTC:	itm_inner_port = value;		break;
	case CTMO_MAXCU:	itm_outer_max = value;		break;
	case CTMO_MAXCC:	itm_inner_max = value;		break;
	case CTMO_TIMEU:	itm_outer_time = value;		break;
	case CTMO_TIMEC:	itm_inner_time = value;		break;
	case CTMO_ADDRC:	itm_inner_addr = value;		break;
	case CTMO_LOGMK:	itm_logmask = value;		break;
	case CTMO_LIMTU:	itm_outer_blimit = value;	break;
	case CTMO_LIMTC:	itm_inner_blimit = value;	break;
	case CTMO_PORTD:	itm_dgram_port = value;		break;
	case CTMO_UTIME:	itm_utiming = value;		break;
	case CTMO_INTERVAL:	itm_interval = value;		break;
	case CTMO_DGRAM:
		switch (value)
		{
		case 0:	itm_udpmask = 0;	break;
		case 1: itm_udpmask = ITMU_MWORK | ITMU_MDUDP | ITMU_MDTCP; break;
		case 2: itm_udpmask = ITMU_MWORK | ITMU_MDUDP; break;
		case 3: itm_udpmask = ITMU_MWORK; break;
		}
		break;
	case CTMO_HEADER:
		if (_ctm_status == CTM_STOPPED) itm_headmod = value; 
		else return -1;
		break;
	case CTMO_DATMAX:
		if (_ctm_status == CTM_STOPPED) itm_datamax = value; 
		else return -1;
		break;
	case CTMO_SOCKSNDI:  itm_socksndi = (long)value; break;
	case CTMO_SOCKRCVI:  itm_sockrcvi = (long)value; break;
	case CTMO_SOCKSNDO:  itm_socksndo = (long)value; break;
	case CTMO_SOCKRCVO:  itm_sockrcvo = (long)value; break;
	case CTMO_DHCPBASE:  itm_dhcp_base = (int)value; break;
	case CTMO_DHCPHIGH:  itm_dhcp_high = (int)value; break;
	case CTMO_NOREUSE:   itm_noreuse = (int)value; break;
	}
	return 0;
}

//---------------------------------------------------------------------
// 取得错误代码
//---------------------------------------------------------------------
APR_MODULE(long) ctm_errno(void)
{
	return _ctm_errno;
}


//---------------------------------------------------------------------
// 设置日志函数Handle
//---------------------------------------------------------------------
APR_MODULE(int) ctm_handle_log(CTM_LOG_HANDLE handle)
{
	itm_logv = handle;
	return 0;
}

//---------------------------------------------------------------------
// 设置加密函数Handle
//---------------------------------------------------------------------
APR_MODULE(int) ctm_handle_encrypt(CTM_ENCRYPT_HANDLE handle)
{
	return 0;
}

// 设置验证函数Handle
APR_MODULE(int) ctm_handle_validate(CTM_VALIDATE_HANDLE handle)
{
	itm_validate = handle;
	return 0;
}



//---------------------------------------------------------------------
// 设置日志默认接口
//---------------------------------------------------------------------
static int  log_mode = 0;
static char log_prefix[1024] = "";

#ifdef _MSC_VER
#pragma warning(disable:4996)
#endif

static int ctm_logout_proc(const char *text)
{
	static time_t tt_save = 0, tt_now = 0;
	static struct tm tm_time, *tmx = &tm_time;
	static char timetxt[64] = "";
	static char xtext[4096] = "";
	static char fname[2048] = "";
	static int olday = -1;
	static FILE *fp = NULL;
	int size;

	// 更新时间
	tt_now = time(NULL);
	if (tt_now != tt_save) {
		tt_save = tt_now;
		memcpy(&tm_time, localtime(&tt_now), sizeof(tm_time));	
		sprintf(timetxt, "%04d-%02d-%02d %02d:%02d:%02d", tmx->tm_year + 1900, 
			tmx->tm_mon + 1, tmx->tm_mday, tmx->tm_hour, tmx->tm_min, tmx->tm_sec);
		if (olday != tmx->tm_mday) olday = tmx->tm_mday, fname[0] = 0;
	}

	if (log_mode == 0) 
		return 0;
	
	// 组成日志
	sprintf(xtext, "[%s] %s\n", timetxt, text);
	size = strlen(xtext);

	if (fname[0] == 0 && (log_mode & 1)) {
		if (fp) fclose(fp);
		sprintf(fname, "%s%04d%02d%02d.log", log_prefix, tmx->tm_year + 1900, 
			tmx->tm_mon + 1, tmx->tm_mday);
		if ((fp = fopen(fname, "a"))) {
			fseek(fp, 0, SEEK_END);
		}
	}

	if ((log_mode & 1) == 0) {
		if (fp) fclose(fp);
		fp = NULL;
		fname[0] = 0;
	}

	if (log_mode & 1) {
		if (fp) {
			fwrite(xtext, 1, size, fp);
			fflush(fp);
		}
	}

	if (log_mode & 2) {
		write(1, xtext, size);
	}

	if (log_mode & 4) {
		write(2, xtext, size);
	}

	return 0;
}

APR_MODULE(int) cmt_handle_logout(int mode, const char *fn_prefix)
{
	int len;

	if (fn_prefix == NULL) {
		fn_prefix = "";
	};

	len = strlen(fn_prefix);

	if (len == 0) mode &= ~1;

	if (mode != 0) {
		if (len > 1023) len = 1023;
		memcpy(log_prefix, fn_prefix, len);
	}

	log_mode = mode;
	ctm_handle_log(ctm_logout_proc);

	return 0;
}



//---------------------------------------------------------------------
// 编译信息
//---------------------------------------------------------------------

// 取得编译版本号
APR_MODULE(int) ctm_version(void)
{
	return ITMV_VERSION;
}

// 取得编译日期
APR_MODULE(const char*) ctm_get_date(void)
{
	return __DATE__;
}

// 取得编译时间
APR_MODULE(const char*) ctm_get_time(void)
{
	return __TIME__;
}

