/* NetHack 5.0	alloc.c	$NHDT-Date: 1781973040 2026/06/20 16:30:40 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.42 $ */
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/*-Copyright (c) Robert Patrick Rankin, 2012. */
/* NetHack may be freely redistributed.  See license for details. */

/**
 * @file alloc.c
 * @brief NetHack 의 메모리 할당 래퍼 및 관련 유틸리티.
 *
 * 실패 시 @c panic() 으로 중단하는 @c alloc()/re_alloc(), 문자열 복제
 * @c dupstr(), 포인터 포맷팅, 정수 범위 검사(@c FITSint_/FITSuint_) 등을
 * 제공한다. @c MONITOR_HEAP 빌드에서는 호출자 추적과 힙 로깅을 지원하는
 * @c nhalloc/nhrealloc/nhfree/nhdupstr 변형을 제공한다.
 *
 * @note 보조 프로그램에서도 사용되므로 nethack 전체 선언을 포함하지 않으며,
 *       선언 순서는 조건부 컴파일에 의해 규정되어 재배치하지 않는다.
 */

#define ALLOC_C /* comment line for pre-compiled headers */
/* since this file is also used in auxiliary programs, don't include all the
   function declarations for all of nethack */
#define EXTERN_H /* comment line for pre-compiled headers */

#include "config.h"
#ifndef LUA_INTEGER
#include "nhlua.h"
#endif


/*#define FITSint(x) FITSint_(x, __func__, __LINE__)*/
extern int FITSint_(LUA_INTEGER, const char *, int) NONNULLARG2;
/*#define FITSuint(x) FITSuint_(x, __func__, __LINE__)*/
extern unsigned FITSuint_(unsigned long long, const char *, int) NONNULLARG2;

char *fmt_ptr(const genericptr) NONNULL;

#ifdef MONITOR_HEAP
#undef alloc
#undef re_alloc
#undef free
extern void free(genericptr_t);
staticfn void heapmon_init(void);

static FILE *heaplog = 0;
static boolean tried_heaplog = FALSE;
#endif

/*
 * For historical reasons, nethack's alloc() returns 'long *' rather
 * than 'void *' or 'char *'.
 *
 * Some static analysis complains if it can't deduce that the number
 * of bytes being allocated is a multiple of 'sizeof (long)'.  It
 * recognizes that the following manipulation overcomes that via
 * rounding the requested length up to the next long.  NetHack doesn't
 * make a lot of tiny allocations, so this shouldn't waste much memory
 * regardless of whether malloc() does something similar.  NetHack
 * isn't expected to call alloc(0), but if that happens treat it as
 * alloc(sizeof (long)) instead.
 */
#define ForceAlignedLength(LTH) \
    do {                                                        \
        if (!(LTH) || (LTH) % sizeof (long) != 0)               \
            (LTH) += sizeof (long) - (LTH) % sizeof (long);     \
    } while (0)

#ifndef MONITOR_HEAP
long *alloc(unsigned int) NONNULL;
long *re_alloc(long *, unsigned int) NONNULL;
#else
    /* for #if MONITOR_HEAP, alloc() might return Null but only nhalloc()
       should be calling it; nhalloc() never returns Null */
long *alloc(unsigned int);
long *re_alloc(long *, unsigned int);
long *nhalloc(unsigned int, const char *, int) NONNULL;
long *nhrealloc(long *, unsigned int, const char *, int) NONNULL;
#endif
ATTRNORETURN extern void panic(const char *, ...) PRINTF_F(1, 2) NORETURN;

/**
 * @brief 메모리를 할당하고 실패 시 프로그램을 중단한다.
 * @param[in] lth 요청 바이트 수(0이면 @c sizeof(long) 로 취급).
 * @return 할당된 메모리 포인터(@c long* 형식으로 반환).
 * @warning 비-@c MONITOR_HEAP 빌드에서 할당 실패 시 @c panic() 으로 중단한다.
 */
long *
alloc(unsigned int lth)
{
    genericptr_t ptr;

    ForceAlignedLength(lth);
    ptr = malloc(lth);
#ifndef MONITOR_HEAP
    if (!ptr)
        panic("Memory allocation failure; cannot get %u bytes", lth);
#else
    /* for #if MONITOR_HEAP, failure is handled in nhalloc() */
#endif
    return (long *) ptr;
}

/**
 * @brief 메모리 블록의 크기를 조정하고 확장 실패 시 중단한다.
 * @param[in] oldptr 기존 메모리 포인터(NULL 가능).
 * @param[in] newlth 새 크기(바이트).
 * @return 재조정된 메모리 포인터.
 * @warning 비-@c MONITOR_HEAP 빌드에서 확장 실패 시 @c panic() 으로 중단한다
 *          (축소는 실패하지 않는다고 가정).
 */
/* realloc() call that might get substituted by nhrealloc(p,n,file,line) */
long *
re_alloc(long *oldptr, unsigned int newlth)
{
    long *newptr;

    ForceAlignedLength(newlth);
    newptr = (long *) realloc((genericptr_t) oldptr, (size_t) newlth);
#ifndef MONITOR_HEAP
    /* "extend to":  assume it won't ever fail if asked to shrink */
    if (newlth && !newptr)
        panic("Memory allocation failure; cannot extend to %u bytes", newlth);
#else
    /* for #if MONITOR_HEAP, failure is handled in nhrealloc() */
#endif
    return newptr;
}

#ifdef HAS_PTR_FMT
#define PTR_FMT "%p"
#define PTR_TYP genericptr_t
#else
#define PTR_FMT "%06lx"
#define PTR_TYP unsigned long
#endif

/* A small pool of static formatting buffers.
 * PTRBUFSIZ:  We assume that pointers will be formatted as integers in
 * hexadecimal, requiring at least 16+1 characters for each buffer to handle
 * 64-bit systems, but the standard doesn't mandate that encoding and an
 * implementation could do something different for %p, so we make some
 * extra room.
 * PTRBUFCNT:  Number of formatted values which can be in use at the same
 * time.  To have more, callers need to make copies of them as they go.
 */
#define PTRBUFCNT 4
#define PTRBUFSIZ 32
static char ptrbuf[PTRBUFCNT][PTRBUFSIZ];
static int ptrbufidx = 0;

/**
 * @brief 포인터를 표시용 문자열로 포맷팅한다.
 * @param[in] ptr 포맷팅할 포인터.
 * @return 포맷팅된 문자열(정적 버퍼).
 * @warning 소수의 정적 버퍼를 순환 사용하므로, 동시에 유효한 결과 수는
 *          @c PTRBUFCNT 개로 제한된다. 더 필요하면 호출자가 복사해야 한다.
 */
/* format a pointer for display purposes; returns a static buffer */
char *
fmt_ptr(const genericptr ptr)
{
    char *buf;

    buf = ptrbuf[ptrbufidx];
    if (++ptrbufidx >= PTRBUFCNT)
        ptrbufidx = 0;

    Sprintf(buf, PTR_FMT, (PTR_TYP) ptr);
    return buf;
}

#ifdef MONITOR_HEAP

/**
 * @brief 힙 모니터링 로그 파일을 초기화한다.
 * @note 환경 변수 @c NH_HEAPLOG 가 설정되어 있으면 해당 이름의 파일을 열어
 *       할당/해제 정보를 기록한다. 최초 할당 시 한 번만 시도된다.
 */
/* If ${NH_HEAPLOG} is defined and we can create a file by that name,
   then we'll log the allocation and release information to that file. */
staticfn void
heapmon_init(void)
{
    char *logname = getenv("NH_HEAPLOG");

    if (logname && *logname)
        heaplog = fopen(logname, "w");
    tried_heaplog = TRUE;
}

/**
 * @brief 호출자 추적과 힙 로깅을 지원하는 @c alloc() 변형.
 * @param[in] lth  요청 바이트 수.
 * @param[in] file 호출 위치의 소스 파일명.
 * @param[in] line 호출 위치의 소스 라인 번호.
 * @return 할당된 메모리 포인터(절대 NULL 을 반환하지 않음).
 * @warning 할당 실패 시 @c panic() 으로 중단한다.
 */
long *
nhalloc(unsigned int lth, const char *file, int line)
{
    long *ptr = alloc(lth);

    if (!tried_heaplog)
        heapmon_init();
    if (heaplog)
        (void) fprintf(heaplog, "+%5u %s %4d %s\n", lth,
                       fmt_ptr((genericptr_t) ptr), line, file);
    /* potential panic in alloc() was deferred til here */
    if (!ptr)
        panic("Cannot get %u bytes, line %d of %s", lth, line, file);

    return ptr;
}

/**
 * @brief 호출자 추적과 힙 로깅을 지원하는 @c re_alloc() 변형.
 * @param[in] oldptr 기존 메모리 포인터(NULL 가능).
 * @param[in] newlth 새 크기(바이트).
 * @param[in] file   호출 위치의 소스 파일명.
 * @param[in] line   호출 위치의 소스 라인 번호.
 * @return 재조정된 메모리 포인터.
 * @warning 확장 실패 시 @c panic() 으로 중단한다.
 */
/* re_alloc() with heap logging; we lack access to the old alloc size  */
long *
nhrealloc(
    long *oldptr,
    unsigned int newlth,
    const char *file,
    int line)
{
    long *newptr = re_alloc(oldptr, newlth);

    if (!tried_heaplog)
        heapmon_init();
    if (heaplog) {
        char op = '*'; /* assume realloc() will change size of previous
                        * allocation rather than make a new one */

        if (newptr != oldptr) {
            /* if oldptr wasn't Null, realloc() freed it */
            if (oldptr)
                (void) fprintf(heaplog, "%c%5s %s %4d %s\n", '<', "",
                               fmt_ptr((genericptr_t) oldptr), line, file);
            op = '>'; /* new allocation rather than size-change of old one */
        }
        (void) fprintf(heaplog, "%c%5u %s %4d %s\n", op, newlth,
                           fmt_ptr((genericptr_t) newptr), line, file);
    }
    /* potential panic in re_alloc() was deferred til here;
       "extend to":  assume it won't ever fail if asked to shrink;
       even if that assumption happens to be wrong, we lack access to
       the old size so can't use alternate phrasing for that case */
    if (newlth && !newptr)
        panic("Cannot extend to %u bytes, line %d of %s", newlth, line, file);

    return newptr;
}

/**
 * @brief 호출자 추적과 힙 로깅을 지원하는 @c free() 변형.
 * @param[in] ptr  해제할 메모리 포인터.
 * @param[in] file 호출 위치의 소스 파일명.
 * @param[in] line 호출 위치의 소스 라인 번호.
 */
void
nhfree(genericptr_t ptr, const char *file, int line)
{
    if (!tried_heaplog)
        heapmon_init();
    if (heaplog)
        (void) fprintf(heaplog, "-      %s %4d %s\n",
                       fmt_ptr((genericptr_t) ptr), line, file);

    free(ptr);
}

/**
 * @brief 호출자 추적을 지원하는 @c strdup() 변형(우리 @c alloc() 사용).
 * @param[in] string 복제할 널 종료 문자열.
 * @param[in] file   호출 위치의 소스 파일명.
 * @param[in] line   호출 위치의 소스 라인 번호.
 * @return 복제된 문자열 포인터.
 * @warning 문자열 길이가 오버플로하면 @c panic() 으로 중단한다.
 */
/* strdup() which uses our alloc() rather than libc's malloc(),
   with caller tracking */
char *
nhdupstr(const char *string, const char *file, int line)
{
    /* we've got some info about the caller, so use it instead of __func__ */
    unsigned len = FITSuint_(strlen(string), file, line);

    if (FITSuint_(len + 1, file, line) < len)
        panic("nhdupstr: string length overflow, line %d of %s",
              line, file);

    return strcpy((char *) nhalloc(len + 1, file, line), string);
}
#undef dupstr

#endif /* MONITOR_HEAP */

/**
 * @brief libc 의 malloc 대신 우리 @c alloc() 을 쓰는 @c strdup() 구현.
 * @param[in] string 복제할 널 종료 문자열.
 * @return 복제된 문자열 포인터.
 * @warning 문자열 길이가 오버플로하면 @c panic() 으로 중단한다.
 */
/* strdup() which uses our alloc() rather than libc's malloc();
   not used when MONITOR_HEAP is enabled, but included unconditionally
   in case utility programs get built using a different setting for that */
char *
dupstr(const char *string)
{
    size_t len = strlen(string);

    /* make sure len+1 doesn't overflow plain unsigned (for alloc()) */
    if (len > (unsigned) (~0U - 1U))
        panic("dupstr: string length overflow");

    return strcpy((char *) alloc(len + 1), string);
}

#if 0   /* suppress this; if included, it will need a MONITOR_HEAP edition */

/* similar for reasonable size strings, but return length of input as well */
char *
dupstr_n(const char *string, unsigned int *lenout)
{
    size_t len = strlen(string);

    if (len >= LARGEST_INT)
        panic("dupstr_n: string too long");
    *lenout = (unsigned int) len;
    return strcpy((char *) alloc(len + 1), string);
}
#endif

/**
 * @brief 값을 @c int 로 캐스팅하되 오버플로 시 중단한다(매크로로 호출).
 * @param[in] i    변환할 값.
 * @param[in] file 호출 위치의 소스 파일명.
 * @param[in] line 호출 위치의 소스 라인 번호.
 * @return @c int 로 변환된 값.
 * @warning 값이 @c int 범위를 벗어나면 @c panic() 으로 중단한다.
 */
/* cast to int or panic on overflow; use via macro */
int
FITSint_(LUA_INTEGER i, const char *file, int line)
{
    int iret = (int) i;

    if (iret != i)
        panic("Overflow at %s:%d", file, line);
    return iret;
}

/**
 * @brief 값을 @c unsigned 로 캐스팅하되 오버플로 시 중단한다(매크로로 호출).
 * @param[in] ull  변환할 값.
 * @param[in] file 호출 위치의 소스 파일명.
 * @param[in] line 호출 위치의 소스 라인 번호.
 * @return @c unsigned 로 변환된 값.
 * @warning 값이 @c unsigned 범위를 벗어나면 @c panic() 으로 중단한다.
 */
unsigned
FITSuint_(unsigned long long ull, const char *file, int line)
{
    unsigned uret = (unsigned) ull;

    if (uret != ull)
        panic("Overflow at %s:%d", file, line);
    return uret;
}

/*alloc.c*/
