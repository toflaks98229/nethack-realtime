/* NetHack 5.0	nh_savefile.h	$NHDT-Date: 0 $ */
/* NetHack may be freely redistributed.  See license for details. */
/* Extracted 2026-07 from hack.h for the real-time fork (see MODIFICATIONS.md).
   Content is unchanged; hack.h includes this header at the point the
   definitions used to occupy, so preprocessing order is identical. */

/**
 * @file nh_savefile.h
 * @brief File handle and access modes shared by the save, level, and bones
 *        serializers.
 *
 * One handle type, @c NHFILE, serves all three kinds of persistent file, and
 * one set of mode bits describes what a serialization pass is doing. Because
 * the same routine both writes and frees depending on those bits, the mode
 * predicates below are what distinguish a save pass from a teardown pass.
 *
 * @warning @c release_data() is how the serializers decide to deallocate what
 *          they just walked; a pass whose mode has @c FREEING set destroys the
 *          structures it visits.
 * @note Extracted verbatim from @c hack.h; include @c hack.h rather than this
 *       header directly, which is what every existing source file does.
 */

/**
 * @file nh_savefile.h
 * @brief 세이브·레벨·본즈 직렬화기가 공유하는 파일 핸들과 접근 모드.
 *
 * 세 종류의 영속 파일 모두를 하나의 핸들 타입 @c NHFILE 로 다루며, 직렬화
 * 패스가 무엇을 하는 중인지는 하나의 모드 비트 집합으로 서술한다. 같은 루틴이
 * 이 비트에 따라 쓰기도 하고 해제도 하므로, 아래의 모드 술어가 저장 패스와
 * 해제 패스를 가르는 기준이 된다.
 *
 * @warning @c release_data() 는 직렬화기가 방금 순회한 대상을 해제할지
 *          결정하는 수단이다. 모드에 @c FREEING 이 설정된 패스는 방문하는
 *          구조체들을 파괴한다.
 * @note @c hack.h 에서 그대로 추출했다. 기존 모든 소스 파일이 그렇듯 이 헤더를
 *       직접 포함하지 말고 @c hack.h 를 포함할 것.
 */

#ifndef NH_SAVEFILE_H
#define NH_SAVEFILE_H

/* NetHack ftypes */
#define NHF_LEVELFILE       1
#define NHF_SAVEFILE        2
#define NHF_BONESFILE       3
/* modes */
#define READING      0x0
#define COUNTING     0x01
#define WRITING      0x02
#define FREEING      0x04
#define CONVERTING   0x08
#define UNCONVERTING 0x10
#if 0
/* operations of the various saveXXXchn & co. routines */
#define perform_bwrite(nhfp) ((nhfp)->mode & (COUNTING | WRITING))
#define release_data(nhfp) ((nhfp)->mode & FREEING)
#endif

/* operations of the various saveXXXchn & co. routines */
#define update_file(nhfp) ((nhfp)->mode & (COUNTING | WRITING))
#define release_data(nhfp) ((nhfp)->mode & FREEING)

enum saveformats {
    invalid = 0,
    historical = 1,     /* entire struct, binary, as-is */
    exportascii = 2,    /* each field written out as ascii text */
    NUM_SAVEFORMATS
};

/* Content types for fieldlevel files */
struct fieldlevel_content {
    boolean deflt;        /* individual fields */
    boolean binary;       /* binary rather than text */
};

struct nh_file {
    int fd;               /* for traditional structlevel binary writes */
    int mode;             /* holds READING, WRITING, FREEING, CONVERTING modes  */
    int ftype;            /* NHF_LEVELFILE, NHF_SAVEFILE, or NHF_BONESFILE */
    int fnidx;            /* index of procs for fieldlevel saves */
    long rcount,          /* read count since opening */
         wcount;          /* write count since opening */
    boolean structlevel;  /* traditional structure binary saves */
    boolean fieldlevel;   /* fieldlevel saves each field individually */
    boolean addinfo;      /* if set, some additional context info from core */
    boolean eof;          /* place to mark eof reached */
    boolean bendian;      /* set to true if executing on big-endian machine */
    FILE *fpdef;          /* file pointer for fieldlevel default style */
    FILE *fpdefmap;       /* file pointer mapfile for def format */
    FILE *fplog;          /* file pointer logfile */
    FILE *fpdebug;        /* file pointer debug info */
    struct fieldlevel_content style;
    struct nh_file *nhfpconvert;
};

typedef struct nh_file NHFILE;

#endif /* NH_SAVEFILE_H */
