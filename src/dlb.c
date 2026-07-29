/* NetHack 5.0	dlb.c	$NHDT-Date: 1781973045 2026/06/20 16:30:45 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.30 $ */
/* Copyright (c) Kenneth Lorber, Bethesda, Maryland, 1993. */
/* NetHack may be freely redistributed.  See license for details. */

/**
 * @file dlb.c
 * @brief 데이터 라이브러리언(Data Librarian): 데이터 아카이브 접근 계층.
 *
 * 하나 이상의 "데이터 라이브러리"(아카이브 파일)를 다중화하여 NetHack 에
 * STDIO 유사 인터페이스(@c dlb_fopen/fread/fseek/fgets/fgetc/ftell)를
 * 제공한다. 라이브러리 내부에서 파일을 찾지 못하면 라이브러리 바깥의 실제
 * 파일을 찾는다. 백엔드는 @c #ifdef(@c DLBLIB, @c DLBRSRC)로 선택된다.
 *
 * @note 선언 순서가 조건부 컴파일에 의해 규정되므로 재배치하지 않는다.
 */

#include "config.h"
#include "dlb.h"

#ifdef __DJGPP__
#include <string.h>
#endif

#define DATAPREFIX 4        /* see decl.h */

#ifdef DLB
/*
 * Data librarian.  Present a STDIO-like interface to NetHack while
 * multiplexing on one or more "data libraries".  If a file is not found
 * in a given library, look for it outside the libraries.
 */

/** @brief DLB 백엔드 구현이 제공해야 하는 함수 포인터 집합. */
typedef struct dlb_procs {
    boolean (*dlb_init_proc)(void);
    void (*dlb_cleanup_proc)(void);
    boolean (*dlb_fopen_proc)(DLB_P, const char *, const char *);
    int (*dlb_fclose_proc)(DLB_P);
    int (*dlb_fread_proc)(char *, int, int, DLB_P);
    int (*dlb_fseek_proc)(DLB_P, long, int);
    char *(*dlb_fgets_proc)(char *, int, DLB_P);
    int (*dlb_fgetc_proc)(DLB_P);
    long (*dlb_ftell_proc)(DLB_P);
} dlb_procs_t;

#if defined(VERSION_IN_DLB_FILENAME)
char dlbfilename[MAX_DLB_FILENAME];
#endif

/* without extern.h via hack.h, these haven't been declared for us */
extern FILE *fopen_datafile(const char *, const char *, int);
#define FITSuint(x) FITSuint_((x), __func__, __LINE__)
/* implementation will be in either dlb_main.c or the core */
extern unsigned FITSuint_(unsigned long long, const char *, int);


#ifdef DLBLIB
/*
 * Library Implementation:
 *
 * When initialized, we open all library files and read in their tables
 * of contents.  The library files stay open all the time.  When
 * a open is requested, the libraries' directories are searched.  If
 * successful, we return a descriptor that contains the library, file
 * size, and current file mark.  This descriptor is used for all
 * successive calls.
 *
 * The ability to open more than one library is supported but used
 * only in the Amiga port (the second library holds the sound files).
 * For Unix, the idea would be to split the NetHack library
 * into text and binary parts, where the text version could be shared.
 */

#define MAX_LIBS 4
staticfn library dlb_libs[MAX_LIBS];

staticfn boolean readlibdir(library * lp);
staticfn boolean find_file(const char *name, library **lib, long *startp,
                         long *sizep);
staticfn boolean lib_dlb_init(void);
staticfn void lib_dlb_cleanup(void);
staticfn boolean lib_dlb_fopen(dlb *, const char *, const char *);
staticfn int lib_dlb_fclose(dlb *);
staticfn int lib_dlb_fread(char *, int, int, dlb *);
staticfn int lib_dlb_fseek(dlb *, long, int);
staticfn char *lib_dlb_fgets(char *, int, dlb *);
staticfn int lib_dlb_fgetc(dlb *);
staticfn long lib_dlb_ftell(dlb *);

/* not static because shared with dlb_main.c */
boolean open_library(const char *lib_name, library *lp);
void close_library(library * lp);

/* without extern.h via hack.h, these haven't been declared for us */
extern char *eos(char *);

/*
 * Read the directory out of the library.  Return 1 if successful,
 * 0 if it failed.
 *
 * NOTE: An improvement of the file structure should be the file
 * size as part of the directory entry or perhaps in place of the
 * offset -- the offset can be calculated by a running tally of
 * the sizes.
 *
 * Library file structure:
 *
 * HEADER:
 * %3ld library FORMAT revision (currently rev 1)
 * %1c  space
 * %8ld # of files in archive (includes 1 for directory)
 * %1c  space
 * %8ld size of allocation for string space for directory names
 * %1c  space
 * %8ld library offset - sanity check - lseek target for start of first file
 * %1c  space
 * %8ld size - sanity check - byte size of complete archive file
 *
 * followed by one DIRECTORY entry for each file in the archive, including
 *  the directory itself:
 * %1c  handling information (compression, etc.)  Always ' ' in rev 1.
 * %s   file name
 * %1c  space
 * %8ld offset in archive file of start of this file
 * %c   newline
 *
 * followed by the contents of the files
 */
#define DLB_MIN_VERS 1 /* min library version readable by this code */
#define DLB_MAX_VERS 1 /* max library version readable by this code */

/*
 * Read the directory from the library file.   This will allocate and
 * fill in our globals.  The file pointer is reset back to position
 * zero.  If any part fails, leave nothing that needs to be deallocated.
 *
 * Return TRUE on success, FALSE on failure.
 */
/**
 * @brief 라이브러리 파일에서 디렉터리(목차)를 읽어 구조체를 채운다.
 * @param[in,out] lp 채워 넣을 라이브러리 포인터.
 * @return 성공 시 TRUE, 실패 시 FALSE(할당물은 모두 정리됨).
 * @note 성공 후 파일 포인터를 위치 0으로 되돌린다.
 */
staticfn boolean
readlibdir(library *lp) /* library pointer to fill in */
{
    int i;
    char *sp;
    long liboffset, totalsize;

    if (fscanf(lp->fdata, "%ld %ld %ld %ld %ld\n", &lp->rev, &lp->nentries,
               &lp->strsize, &liboffset, &totalsize) != 5)
        return FALSE;
    if (lp->rev > DLB_MAX_VERS || lp->rev < DLB_MIN_VERS)
        return FALSE;

    lp->dir = (libdir *) alloc(FITSuint(lp->nentries * sizeof(libdir)));
    lp->sspace = (char *) alloc(FITSuint(lp->strsize));

    /* read in each directory entry */
    for (i = 0, sp = lp->sspace; i < lp->nentries; i++) {
        lp->dir[i].fname = sp;
        if (fscanf(lp->fdata, "%c%s %ld\n", &lp->dir[i].handling, sp,
                   &lp->dir[i].foffset) != 3) {
            free((genericptr_t) lp->dir);
            free((genericptr_t) lp->sspace);
            lp->dir = (libdir *) 0;
            lp->sspace = (char *) 0;
            return FALSE;
        }
        sp = eos(sp) + 1;
    }

    /* calculate file sizes using offset information */
    for (i = 0; i < lp->nentries; i++) {
        if (i == lp->nentries - 1)
            lp->dir[i].fsize = totalsize - lp->dir[i].foffset;
        else
            lp->dir[i].fsize = lp->dir[i + 1].foffset - lp->dir[i].foffset;
    }

    (void) fseek(lp->fdata, 0L, SEEK_SET); /* reset back to zero */
    lp->fmark = 0;

    return TRUE;
}

/*
 * Look for the file in our directory structure.  Return 1 if successful,
 * 0 if not found.  Fill in the size and starting position.
 */
/**
 * @brief 열린 라이브러리들의 디렉터리에서 파일을 찾는다.
 * @param[in]  name   찾을 파일 이름.
 * @param[out] lib    파일을 포함한 라이브러리 포인터(없으면 NULL).
 * @param[out] startp 아카이브 내 파일 시작 오프셋.
 * @param[out] sizep  파일 크기.
 * @return 찾으면 TRUE, 없으면 FALSE.
 */
staticfn boolean
find_file(const char *name, library **lib, long *startp, long *sizep)
{
    int i, j;
    library *lp;

    for (i = 0; i < MAX_LIBS && dlb_libs[i].fdata; i++) {
        lp = &dlb_libs[i];
        for (j = 0; j < lp->nentries; j++) {
            if (FILENAME_CMP(name, lp->dir[j].fname) == 0) {
                *lib = lp;
                *startp = lp->dir[j].foffset;
                *sizep = lp->dir[j].fsize;
                return TRUE;
            }
        }
    }
    *lib = (library *) 0;
    *startp = *sizep = 0;
    return FALSE;
}

/**
 * @brief 지정한 이름의 라이브러리 파일을 열고 디렉터리를 읽어 들인다.
 * @param[in]  lib_name 라이브러리 파일 이름.
 * @param[out] lp       채워 넣을 라이브러리 구조체.
 * @return 성공 시 TRUE, 실패 시 FALSE.
 * @note @c dlb_main.c 와 공유되므로 static 이 아니다.
 */
/*
 * Open the library of the given name and fill in the given library
 * structure.  Return TRUE if successful, FALSE otherwise.
 */
boolean
open_library(const char *lib_name, library *lp)
{
    boolean status = FALSE;

    lp->fdata = fopen_datafile(lib_name, RDBMODE, DATAPREFIX);
    if (lp->fdata) {
        if (readlibdir(lp)) {
            status = TRUE;
        } else {
            (void) fclose(lp->fdata);
            lp->fdata = (FILE *) 0;
        }
    }
    return status;
}

/**
 * @brief 라이브러리 파일을 닫고 관련 메모리를 해제한다.
 * @param[in,out] lp 닫을 라이브러리 구조체(호출 후 0으로 초기화됨).
 * @note @c dlb_main.c 와 공유되므로 static 이 아니다.
 */
void
close_library(library *lp)
{
    (void) fclose(lp->fdata);
    free((genericptr_t) lp->dir);
    free((genericptr_t) lp->sspace);

    (void) memset((char *) lp, 0, sizeof(library));
}

/**
 * @brief DLBLIB 백엔드 초기화: 라이브러리 파일(들)을 열어 둔다.
 * @return 성공 시 TRUE, 하나라도 실패하면 FALSE.
 */
/*
 * Open the library file once using stdio.  Keep it open, but
 * keep track of the file position.
 */
staticfn boolean
lib_dlb_init(void)
{
    /* zero out array */
    (void) memset((char *) &dlb_libs[0], 0, sizeof(dlb_libs));
#ifdef VERSION_IN_DLB_FILENAME
    build_dlb_filename((const char *) 0);
#endif
    /* To open more than one library, add open library calls here. */
    if (!open_library(DLBFILE, &dlb_libs[0]))
        return FALSE;
#ifdef DLBFILE2
    if (!open_library(DLBFILE2, &dlb_libs[1])) {
        close_library(&dlb_libs[0]);
        return FALSE;
    }
#endif
    return TRUE;
}

/**
 * @brief DLBLIB 백엔드 정리: 열려 있는 데이터 파일(들)을 닫는다.
 */
staticfn void
lib_dlb_cleanup(void)
{
    int i;

    /* close the data file(s) */
    for (i = 0; i < MAX_LIBS && dlb_libs[i].fdata; i++)
        close_library(&dlb_libs[i]);
}

/**
 * @brief 버전 번호가 포함된 DLB 파일 이름을 구성한다.
 * @param[in] lf 기본 이름(NULL 이면 @c DLBBASENAME 사용).
 * @return 구성된 파일 이름(정적 버퍼 @c dlbfilename).
 */
#ifdef VERSION_IN_DLB_FILENAME
char *
build_dlb_filename(const char *lf)
{
    Sprintf(dlbfilename, "%s%d%d%d",
            lf ? lf : DLBBASENAME, VERSION_MAJOR, VERSION_MINOR, PATCHLEVEL);
    return dlbfilename;
}
#endif

/**
 * @brief DLBLIB 백엔드: 라이브러리 내 파일을 연다(디스크립터 채우기).
 * @param[out] dp   채워 넣을 dlb 디스크립터.
 * @param[in]  name 열 파일 이름.
 * @param[in]  mode 모드(사용되지 않음).
 * @return 찾아서 열면 TRUE, 없으면 FALSE.
 */
/*ARGSUSED*/
staticfn boolean
lib_dlb_fopen(dlb *dp, const char *name, const char *mode UNUSED)
{
    long start, size;
    library *lp;

    /* look up file in directory */
    if (find_file(name, &lp, &start, &size)) {
        dp->lib = lp;
        dp->start = start;
        dp->size = size;
        dp->mark = 0;
        return TRUE;
    }

    return FALSE; /* failed */
}

/**
 * @brief DLBLIB 백엔드: 라이브러리 내 파일을 닫는다(할 일 없음).
 * @param[in] dp dlb 디스크립터(사용되지 않음).
 * @return 항상 0.
 */
/*ARGUSED*/
staticfn int
lib_dlb_fclose(dlb *dp UNUSED)
{
    /* nothing needs to be done */
    return 0;
}

/**
 * @brief DLBLIB 백엔드: 라이브러리 내 파일에서 데이터를 읽는다.
 * @param[out] buf  읽어들일 버퍼.
 * @param[in]  size 항목 하나의 바이트 크기.
 * @param[in]  quan 읽을 항목 수.
 * @param[in,out] dp dlb 디스크립터(파일 마크가 갱신됨).
 * @return 실제로 읽은 항목 수.
 * @note 다음 파일 영역을 침범하지 않도록 읽기 수량을 제한한다.
 */
staticfn int
lib_dlb_fread(char *buf, int size, int quan, dlb *dp)
{
    long pos, nread, nbytes;

    /* make sure we don't read into the next file */
    if ((dp->size - dp->mark) < (size * quan))
        quan = (int)((dp->size - dp->mark) / size);
    if (quan == 0)
        return 0;

    pos = dp->start + dp->mark;
    if (dp->lib->fmark != pos) {
        fseek(dp->lib->fdata, pos, SEEK_SET); /* check for error??? */
        dp->lib->fmark = pos;
    }

    nread = fread(buf, size, quan, dp->lib->fdata);
    nbytes = nread * size;
    dp->mark += nbytes;
    dp->lib->fmark += nbytes;

    return (int) nread;
}

/**
 * @brief DLBLIB 백엔드: 라이브러리 내 파일의 읽기 위치를 이동한다.
 * @param[in,out] dp     dlb 디스크립터.
 * @param[in]     pos    이동량/위치.
 * @param[in]     whence 기준(@c SEEK_CUR/SEEK_END/SEEK_SET).
 * @return 항상 0.
 * @note 위치는 파일 경계(0 ~ size) 내로 제한된다.
 */
staticfn int
lib_dlb_fseek(dlb *dp, long pos, int whence)
{
    long curpos;

    switch (whence) {
    case SEEK_CUR:
        curpos = dp->mark + pos;
        break;
    case SEEK_END:
        curpos = dp->size - pos;
        break;
    default: /* set */
        curpos = pos;
        break;
    }
    if (curpos < 0)
        curpos = 0;
    if (curpos > dp->size)
        curpos = dp->size;

    dp->mark = curpos;
    return 0;
}

/**
 * @brief DLBLIB 백엔드: 라이브러리 내 파일에서 한 줄을 읽는다.
 * @param[out]    buf 읽어들일 버퍼.
 * @param[in]     len 버퍼 크기.
 * @param[in,out] dp  dlb 디스크립터.
 * @return @p buf, EOF 이면 NULL.
 */
staticfn char *
lib_dlb_fgets(char *buf, int len, dlb *dp)
{
    int i;
    char *bp, c = 0;

    if (len <= 0)
        return buf; /* sanity check */

    /* return NULL on EOF */
    if (dp->mark >= dp->size)
        return (char *) 0;

    len--; /* save room for null */
    for (i = 0, bp = buf; i < len && dp->mark < dp->size && c != '\n';
         i++, bp++) {
        if (dlb_fread(bp, 1, 1, dp) <= 0)
            break; /* EOF or error */
        c = *bp;
    }
    *bp = '\0';

#if defined(MSDOS) || defined(WIN32)
    if ((bp = strchr(buf, '\r')) != 0) {
        *bp++ = '\n';
        *bp = '\0';
    }
#endif

    return buf;
}

/**
 * @brief DLBLIB 백엔드: 라이브러리 내 파일에서 한 문자를 읽는다.
 * @param[in,out] dp dlb 디스크립터.
 * @return 읽은 문자, EOF 이면 @c EOF.
 */
staticfn int
lib_dlb_fgetc(dlb *dp)
{
    char c;

    if (lib_dlb_fread(&c, 1, 1, dp) != 1)
        return EOF;
    return (int) c;
}

/**
 * @brief DLBLIB 백엔드: 라이브러리 내 파일의 현재 읽기 위치를 반환한다.
 * @param[in] dp dlb 디스크립터.
 * @return 파일 내 현재 위치.
 */
staticfn long
lib_dlb_ftell(dlb *dp)
{
    return dp->mark;
}

/** @brief DLBLIB 백엔드의 함수 포인터 테이블. */
static const dlb_procs_t lib_dlb_procs = { lib_dlb_init,  lib_dlb_cleanup,
                                    lib_dlb_fopen, lib_dlb_fclose,
                                    lib_dlb_fread, lib_dlb_fseek,
                                    lib_dlb_fgets, lib_dlb_fgetc,
                                    lib_dlb_ftell };

#endif /* DLBLIB */

#ifdef DLBRSRC
static const dlb_procs_t rsrc_dlb_procs = { rsrc_dlb_init,  rsrc_dlb_cleanup,
                                     rsrc_dlb_fopen, rsrc_dlb_fclose,
                                     rsrc_dlb_fread, rsrc_dlb_fseek,
                                     rsrc_dlb_fgets, rsrc_dlb_fgetc,
                                     rsrc_dlb_ftell };
#endif

/* Global wrapper functions ------------------------------------------------
 */

#define do_dlb_init (*dlb_procs->dlb_init_proc)
#define do_dlb_cleanup (*dlb_procs->dlb_cleanup_proc)
#define do_dlb_fopen (*dlb_procs->dlb_fopen_proc)
#define do_dlb_fclose (*dlb_procs->dlb_fclose_proc)
#define do_dlb_fread (*dlb_procs->dlb_fread_proc)
#define do_dlb_fseek (*dlb_procs->dlb_fseek_proc)
#define do_dlb_fgets (*dlb_procs->dlb_fgets_proc)
#define do_dlb_fgetc (*dlb_procs->dlb_fgetc_proc)
#define do_dlb_ftell (*dlb_procs->dlb_ftell_proc)

/** @brief 현재 선택된 DLB 백엔드의 함수 포인터 테이블. */
static const dlb_procs_t *dlb_procs;
/** @brief DLB 하위 시스템 초기화 완료 여부. */
static boolean dlb_initialized = FALSE;

/**
 * @brief DLB 하위 시스템을 초기화한다.
 * @return 초기화 성공(또는 이미 초기화됨) 시 TRUE, 실패 시 FALSE.
 */
boolean
dlb_init(void)
{
    if (!dlb_initialized) {
#ifdef DLBLIB
        dlb_procs = &lib_dlb_procs;
#endif
#ifdef DLBRSRC
        dlb_procs = &rsrc_dlb_procs;
#endif

        if (dlb_procs)
            dlb_initialized = do_dlb_init();
    }

    return dlb_initialized;
}

/**
 * @brief DLB 하위 시스템을 정리하고 자원을 해제한다.
 */
void
dlb_cleanup(void)
{
    if (dlb_initialized) {
        do_dlb_cleanup();
        dlb_initialized = FALSE;
    }
}

/**
 * @brief 데이터 파일을 연다(라이브러리 내부 우선, 없으면 외부 실제 파일).
 * @param[in] name 열 파일 이름.
 * @param[in] mode 열기 모드(읽기 전용만 지원).
 * @return dlb 디스크립터, 실패하면 NULL.
 * @note 반환된 디스크립터는 @c dlb_fclose() 로 닫아야 한다.
 */
dlb *
dlb_fopen(const char *name, const char *mode)
{
    FILE *fp;
    dlb *dp;

    if (!dlb_initialized)
        return (dlb *) 0;

    /* only support reading; ignore possible binary flag */
    if (!mode || mode[0] != 'r')
        return (dlb *) 0;

    dp = (dlb *) alloc(sizeof(dlb));
    if (do_dlb_fopen(dp, name, mode))
        dp->fp = (FILE *) 0;
    else if ((fp = fopen_datafile(name, mode, DATAPREFIX)) != 0)
        dp->fp = fp;
    else {
        /* can't find anything */
        free((genericptr_t) dp);
        dp = (dlb *) 0;
    }

    return dp;
}

/**
 * @brief @c dlb_fopen() 으로 연 데이터 파일을 닫는다.
 * @param[in,out] dp 닫을 dlb 디스크립터(해제됨).
 * @return 닫기 결과 코드.
 */
int
dlb_fclose(dlb *dp)
{
    int ret = 0;

    if (dlb_initialized) {
        if (dp->fp)
            ret = fclose(dp->fp);
        else
            ret = do_dlb_fclose(dp);

        free((genericptr_t) dp);
    }
    return ret;
}

/**
 * @brief 데이터 파일에서 데이터를 읽는다.
 * @param[out]    buf  읽어들일 버퍼.
 * @param[in]     size 항목 하나의 바이트 크기.
 * @param[in]     quan 읽을 항목 수.
 * @param[in,out] dp   dlb 디스크립터.
 * @return 실제로 읽은 항목 수.
 */
int
dlb_fread(char *buf, int size, int quan, dlb *dp)
{
    if (!dlb_initialized || size <= 0 || quan <= 0)
        return 0;
    if (dp->fp)
        return (int) fread(buf, size, quan, dp->fp);
    return do_dlb_fread(buf, size, quan, dp);
}

/**
 * @brief 데이터 파일의 읽기 위치를 이동한다.
 * @param[in,out] dp     dlb 디스크립터.
 * @param[in]     pos    이동량/위치.
 * @param[in]     whence 기준(@c SEEK_CUR/SEEK_END/SEEK_SET).
 * @return 성공 시 0, 미초기화 시 @c EOF.
 */
int
dlb_fseek(dlb *dp, long pos, int whence)
{
    if (!dlb_initialized)
        return EOF;
    if (dp->fp)
        return fseek(dp->fp, pos, whence);
    return do_dlb_fseek(dp, pos, whence);
}

/**
 * @brief 데이터 파일에서 한 줄을 읽는다.
 * @param[out]    buf 읽어들일 버퍼.
 * @param[in]     len 버퍼 크기.
 * @param[in,out] dp  dlb 디스크립터.
 * @return @p buf, EOF/미초기화 시 NULL.
 */
char *
dlb_fgets(char *buf, int len, dlb *dp)
{
    if (!dlb_initialized)
        return (char *) 0;
    if (dp->fp)
        return fgets(buf, len, dp->fp);
    return do_dlb_fgets(buf, len, dp);
}

/**
 * @brief 데이터 파일에서 한 문자를 읽는다.
 * @param[in,out] dp dlb 디스크립터.
 * @return 읽은 문자, EOF/미초기화 시 @c EOF.
 */
int
dlb_fgetc(dlb *dp)
{
    if (!dlb_initialized)
        return EOF;
    if (dp->fp)
        return fgetc(dp->fp);
    return do_dlb_fgetc(dp);
}

/**
 * @brief 데이터 파일의 현재 읽기 위치를 반환한다.
 * @param[in] dp dlb 디스크립터.
 * @return 파일 내 현재 위치, 미초기화 시 0.
 */
long
dlb_ftell(dlb *dp)
{
    if (!dlb_initialized)
        return 0;
    if (dp->fp)
        return ftell(dp->fp);
    return do_dlb_ftell(dp);
}

#endif /* DLB */

/*dlb.c*/
