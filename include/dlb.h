/* NetHack 5.0	dlb.h	$NHDT-Date: 1781973079 2026/06/20 16:31:19 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.18 $ */
/* Copyright (c) Kenneth Lorber, Bethesda, Maryland, 1993. */
/* NetHack may be freely redistributed.  See license for details. */

/**
 * @file dlb.h
 * @brief Reading the game's data files whether they are files or an archive.
 *
 * A build may ship its data as loose files or bundled into one archive. Code that
 * reads a data file should not care which, so it opens by name through this layer
 * and the layer decides where to look.
 *
 * When an archive is in use it is opened once and its directory kept in memory, so
 * opening a member is a lookup and a seek rather than a file system operation. That
 * is why an archive member behaves like a file that cannot be written and whose
 * position is shared bookkeeping.
 *
 * @note Without @c DLB the whole layer reduces to ordinary file operations, so the
 *       calling code is identical either way.
 * @warning The Mac implementation uses resources rather than an archive file, which
 *          is why the two are selected between here rather than assumed.
 */

/**
 * @file dlb.h
 * @brief 게임의 데이터 파일을 읽기. 그것이 개별 파일이든 아카이브든.
 *
 * 빌드에 따라 데이터가 낱개 파일로 배포될 수도, 하나의 아카이브로 묶일 수도 있다. 데이터
 * 파일을 읽는 코드가 그 차이를 신경 쓸 필요는 없으므로, 이 계층을 통해 이름으로 열고 어디를
 * 볼지는 계층이 정한다.
 *
 * 아카이브를 쓰는 경우 한 번 열어 그 디렉토리를 메모리에 보관한다. 그래서 구성원을 여는 일이
 * 파일 시스템 연산이 아니라 조회와 위치 이동이 된다. 아카이브 구성원이 쓸 수 없는 파일처럼
 * 동작하고 그 위치가 공유되는 기록인 이유가 그것이다.
 *
 * @note @c DLB 없이 빌드하면 이 계층 전체가 평범한 파일 연산으로 줄어든다. 그래서 호출하는
 *       코드는 어느 쪽이든 동일하다.
 * @warning Mac 구현은 아카이브 파일 대신 리소스를 쓴다. 그래서 둘 중 무엇을 쓸지 가정하지 않고
 *          여기서 선택한다.
 */

#ifndef DLB_H
#define DLB_H
/* definitions for data library */

#ifdef DLB

/* implementations */
#if defined(MAC68K) && !defined(MAC68K_CROSS)
#define DLBRSRC /* use Mac resources */
#else
#define DLBLIB /* use a set of external files */
#endif

#ifdef DLBLIB
/* directory structure in memory */
typedef struct dlb_directory {
    char *fname;   /* file name as seen from calling code */
    long foffset;  /* offset in lib file to start of this file */
    long fsize;    /* file size */
    char handling; /* how to handle the file (compression, etc) */
} libdir;

/* information about each open library */
typedef struct dlb_library {
    FILE *fdata;   /* opened data file */
    long fmark;    /* current file mark */
    libdir *dir;   /* directory of library file */
    char *sspace;  /* pointer to string space */
    long nentries; /* # of files in directory */
    long rev;      /* dlb file revision */
    long strsize;  /* dlb file string size */
} library;

/* library definitions */
#ifndef DLBFILE
#ifndef VERSION_IN_DLB_FILENAME
#define DLBFILE "nhdat" /* name of library */
#else
#define MAX_DLB_FILENAME 256
#define DLBFILE dlbfilename
#define DLBBASENAME "nhdat"
extern char dlbfilename[MAX_DLB_FILENAME];
extern char *build_dlb_filename(const char *);
#endif
#endif
#ifndef FILENAME_CMP
#define FILENAME_CMP strcmp /* case sensitive */
#endif

#endif /* DLBLIB */

typedef struct dlb_handle {
    FILE *fp; /* pointer to an external file, use if non-null */
#ifdef DLBLIB
    library *lib; /* pointer to library structure */
    long start;   /* offset of start of file */
    long size;    /* size of file */
    long mark;    /* current file marker */
#endif
#ifdef DLBRSRC
    int fd; /* HandleFile file descriptor */
#endif
} dlb;

#if defined(ULTRIX_PROTO) && !defined(__STDC__)
/* buggy old Ultrix compiler wants this for the (*dlb_fread_proc)
   and (*dlb_fgets_proc) prototypes in struct dlb_procs (dlb.c);
   we'll use it in all the declarations for consistency */
#define DLB_P struct dlb_handle *
#else
#define DLB_P dlb *
#endif

boolean dlb_init(void);
void dlb_cleanup(void);

dlb *dlb_fopen(const char *, const char *);
int dlb_fclose(DLB_P);
int dlb_fread(char *, int, int, DLB_P);
int dlb_fseek(DLB_P, long, int);
char *dlb_fgets(char *, int, DLB_P);
int dlb_fgetc(DLB_P);
long dlb_ftell(DLB_P);

/* Resource DLB entry points */
#ifdef DLBRSRC
boolean rsrc_dlb_init(void);
void rsrc_dlb_cleanup(void);
boolean rsrc_dlb_fopen(dlb *dp, const char *name, const char *mode);
int rsrc_dlb_fclose(dlb *dp);
int rsrc_dlb_fread(char *buf, int size, int quan, dlb *dp);
int rsrc_dlb_fseek(dlb *dp, long pos, int whence);
char *rsrc_dlb_fgets(char *buf, int len, dlb *dp);
int rsrc_dlb_fgetc(dlb *dp);
long rsrc_dlb_ftell(dlb *dp);
#endif

#else /* DLB */

#define dlb FILE

#define dlb_init()
#define dlb_cleanup()

#define dlb_fopen fopen
#define dlb_fclose fclose
#define dlb_fread fread
#define dlb_fseek fseek
#define dlb_fgets fgets
#define dlb_fgetc fgetc
#define dlb_ftell ftell

#endif /* DLB */

/* various other I/O stuff we don't want to replicate everywhere */

#ifndef SEEK_SET
#define SEEK_SET 0
#endif
#ifndef SEEK_CUR
#define SEEK_CUR 1
#endif
#ifndef SEEK_END
#define SEEK_END 2
#endif

#define RDTMODE "r"
#if (defined(MSDOS) || defined(WIN32) || defined(TOS) || defined(OS2)) \
    && defined(DLB)
#define WRTMODE "w+b"
#else
#define WRTMODE "w+"
#endif
#if (defined(MICRO) && !defined(AMIGA)) || defined(THINK_C) \
    || defined(__MWERKS__) || defined(WIN32)
#define RDBMODE "rb"
#define WRBMODE "w+b"
#else
#define RDBMODE "r"
#define WRBMODE "w+"
#endif

#endif /* DLB_H */
