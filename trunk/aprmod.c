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

#include "aprmod.h"

#include <stdio.h>

#ifdef __unix
#include <dlfcn.h>
#elif defined(_WIN32)
#include <windows.h>
#else
#error APR-MODULE only can be compiled under unix or win32 !!
#endif

APR_DECLARE(long) apr_module_open(apr_module *module, const char *mod_file)
{
	void *handle;

	if (module == NULL) return -1;
	#ifdef __unix
	handle = dlopen(mod_file, RTLD_LAZY);
	#else
	handle = (void*)LoadLibraryA(mod_file);
	#endif
	*module = handle;

	return 0;
}

APR_DECLARE(long) apr_module_close(apr_module module)
{
	int retval = 0;
	#ifdef __unix
	retval = dlclose(module);
	#else
	retval = FreeLibrary((HINSTANCE)module);
	#endif
	return (long)retval;
}

APR_DECLARE(void*) apr_module_symbol(apr_module module, const char *entry)
{
	void* startp = NULL;
	#ifdef __unix
	startp = dlsym(module, entry);
	#else
	startp = (void*)GetProcAddress((HINSTANCE)module, entry);
	#endif
	return startp;
}


