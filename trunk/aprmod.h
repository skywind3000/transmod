//=====================================================================
//
// The platform independence system call wrapper, Skywind 2004
// Unix/Windows ��׼ϵͳ��̬���ӿ���ͨ�ýӿ�
//
// HISTORY:
// Nov. 15 2004   skywind  created
//
// NOTE��
// �ṩʹ Unix���� Windows��ͬ��ϵͳ���ñ�̽ӿ���Ҫ�м�������İ�װ��
// ��һ��ʱ�ӡ��ڶ����̡߳������ǻ��⣬�����ǽ��̣������ǽ���ͨ�ţ�
// ����// �Ƕ�̬�ҽӶ�̬���ӣ��˲���ʵ�ֶ�̬���ӣ�ȡ��apr����ģ��
// apache��aprlib
//
//=====================================================================

#ifndef __APR_MOD_H__
#define __APR_MOD_H__

#include "aprsys.h"

// ��̬���ӿ�Ĳ���
typedef void* apr_module;

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

APR_DECLARE(long)  apr_module_open(apr_module *module, const char *mod_file);
APR_DECLARE(long)  apr_module_close(apr_module module);
APR_DECLARE(void*) apr_module_symbol(apr_module module, const char *entry);

#ifdef __cplusplus
}
#endif

#endif

