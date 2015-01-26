//=====================================================================
//
// The platform independence system call wrapper, Skywind 2004
// Unix/Windows ��׼ϵͳ���ñ��ͨ�ýӿ�
//
// HISTORY:
// Nov. 15 2004   skywind  created
//
// NOTE��
// �ṩʹ Unix���� Windows��ͬ��ϵͳ���ñ�̽ӿ���Ҫ�м�������İ�װ��
// ��һ��ʱ�ӡ��ڶ����̡߳������ǻ��⣬��VC������Ҫ��/MT���أ�ȡ��
// apr����ģ��apache��aprlib
//
//=====================================================================


#ifndef __APR_SYS_H__
#define __APR_SYS_H__

#include <stddef.h>
#include <stdlib.h>


//=====================================================================
// 32BIT INTEGER DEFINITION 
//=====================================================================
#ifndef __INTEGER_32_BITS__
#define __INTEGER_32_BITS__
#if defined(__UINT32_TYPE__) && defined(__UINT32_TYPE__)
	typedef __UINT32_TYPE__ ISTDUINT32;
	typedef __INT32_TYPE__ ISTDINT32;
#elif defined(__UINT_FAST32_TYPE__) && defined(__INT_FAST32_TYPE__)
	typedef __UINT_FAST32_TYPE__ ISTDUINT32;
	typedef __INT_FAST32_TYPE__ ISTDINT32;
#elif defined(_WIN64) || defined(WIN64) || defined(__amd64__) || \
	defined(__x86_64) || defined(__x86_64__) || defined(_M_IA64) || \
	defined(_M_AMD64)
	typedef unsigned int ISTDUINT32;
	typedef int ISTDINT32;
#elif defined(_WIN32) || defined(WIN32) || defined(__i386__) || \
	defined(__i386) || defined(_M_X86)
	typedef unsigned long ISTDUINT32;
	typedef long ISTDINT32;
#elif defined(__MACOS__)
	typedef UInt32 ISTDUINT32;
	typedef SInt32 ISTDINT32;
#elif defined(__APPLE__) && defined(__MACH__)
	#include <sys/types.h>
	typedef u_int32_t ISTDUINT32;
	typedef int32_t ISTDINT32;
#elif defined(__BEOS__)
	#include <sys/inttypes.h>
	typedef u_int32_t ISTDUINT32;
	typedef int32_t ISTDINT32;
#elif (defined(_MSC_VER) || defined(__BORLANDC__)) && (!defined(__MSDOS__))
	typedef unsigned __int32 ISTDUINT32;
	typedef __int32 ISTDINT32;
#elif defined(__GNUC__) && (__GNUC__ > 3)
	#include <stdint.h>
	typedef uint32_t ISTDUINT32;
	typedef int32_t ISTDINT32;
#else 
	typedef unsigned long ISTDUINT32; 
	typedef long ISTDINT32;
#endif
#endif


//=====================================================================
// DETECTION WORD ORDER
//=====================================================================
#ifndef IWORDS_BIG_ENDIAN
    #ifdef _BIG_ENDIAN_
        #if _BIG_ENDIAN_
            #define IWORDS_BIG_ENDIAN 1
        #endif
    #endif
    #ifndef IWORDS_BIG_ENDIAN
        #if defined(__hppa__) || \
            defined(__m68k__) || defined(mc68000) || defined(_M_M68K) || \
            (defined(__MIPS__) && defined(__MISPEB__)) || \
            defined(__ppc__) || defined(__POWERPC__) || defined(_M_PPC) || \
            defined(__sparc__) || defined(__powerpc__) || \
            defined(__mc68000__) || defined(__s390x__) || defined(__s390__)
            #define IWORDS_BIG_ENDIAN 1
        #endif
    #endif
    #ifndef IWORDS_BIG_ENDIAN
        #define IWORDS_BIG_ENDIAN  0
    #endif
#endif


//=====================================================================
// inline definition
//=====================================================================
#ifndef INLINE
#ifdef __GNUC__
#if (__GNUC__ > 3) || ((__GNUC__ == 3) && (__GNUC_MINOR__ >= 1))
    #define INLINE         __inline__ __attribute__((always_inline))
#else
    #define INLINE         __inline__
#endif
#elif defined(_MSC_VER)
	#define INLINE __forceinline
#elif (defined(__BORLANDC__) || defined(__WATCOMC__))
    #define INLINE __inline
#else
    #define INLINE 
#endif
#endif

#ifndef inline
    #define inline INLINE
#endif


//=====================================================================
// Integer Type Decleration
//=====================================================================

/* Typedefs that APR needs. */
typedef  unsigned char     apr_byte;
typedef  short             apr_int16;
typedef  unsigned short    apr_uint16;
typedef  ISTDINT32         apr_int32;
typedef  ISTDUINT32        apr_uint32;


#if !(defined(_MSC_VER) || defined(__LCC__))
typedef long long apr_int64;
typedef unsigned long long apr_uint64;
#else
typedef __int64 apr_int64;
typedef unsigned __int64 apr_uint64;
#endif

#if defined(__APPLE__) && (!defined(__unix))
	#define __unix 1
#endif

#if defined(__unix__) || defined(unix) || defined(__linux)
	#ifndef __unix
		#define __unix 1
	#endif
#endif

#ifdef __cplusplus
extern "C" {
#endif


//=====================================================================
// Interfaces
//=====================================================================

#define APR_THREAD_FUNC
#define APR_DECLARE(type)  type 

#define APR_SUCCESS        0


// �̼߳������ߣ���λΪ ms
APR_DECLARE(void) apr_sleep(unsigned long time);

// ��ȡ 1000Hz��һ��ʱ�ӣ���Сʱ�ظ�һ��
APR_DECLARE(long) apr_clock(void);

// ��ȡ��1970��1��1�գ�00:00�����ľ�ȷʱ��
APR_DECLARE(void) apr_timeofday(long *sec, long *usec);

// ��ȡ 1MHz��ʱ��
APR_DECLARE(apr_int64) apr_timex(void);

typedef void (APR_THREAD_FUNC *apr_thread_start)(void*);

// �̵߳Ĵ������˳����жϺ���
APR_DECLARE(long) apr_thread_create(long* tid, const apr_thread_start tproc, const void *tattr, void *targs);
APR_DECLARE(long) apr_thread_exit(long retval);
APR_DECLARE(long) apr_thread_join(long threadid);
APR_DECLARE(long) apr_thread_detach(long threadid);
APR_DECLARE(long) apr_thread_kill(long threadid);

// ��������
typedef void* apr_mutex;

#ifndef APR_MAX_MUTEX
#define APR_MAX_MUTEX 0x10000
#endif

// �������͵Ĵ�������������������������
APR_DECLARE(long)      apr_mutex_init(apr_mutex *m);
APR_DECLARE(long)      apr_mutex_lock(apr_mutex m);
APR_DECLARE(long)      apr_mutex_unlock(apr_mutex m);
APR_DECLARE(long)      apr_mutex_trylock(apr_mutex m);
APR_DECLARE(long)      apr_mutex_destroy(apr_mutex m);


#ifdef __cplusplus
}
#endif

#endif

