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

/**
 * @brief Which consistency checks @c uptodate() should perform, and how
 *        loudly to complain.
 * @note These relax the check rather than tighten it: a caller that already
 *       knows a file is unusual skips the parts that would reject it.
 */
/**
 * @brief @c uptodate() 가 어떤 정합성 검사를 수행할지, 그리고 얼마나 요란하게
 *        알릴지.
 * @note 검사를 강화하는 것이 아니라 완화하는 쪽이다. 파일이 특수하다는 것을
 *       이미 아는 호출자가 그것을 거부할 검사 부분을 건너뛴다.
 */
/* Flags for controlling uptodate */
#define UTD_CHECKSIZES                 0x01
#define UTD_CHECKFIELDCOUNTS           0x02
#define UTD_SKIP_SANITY1               0x04
#define UTD_SKIP_SAVEFILEINFO          0x08
#define UTD_WITHOUT_WAITSYNCH_PERFILE  0x10
#define UTD_QUIETLY                    0x20

/**
 * @brief Why a savefile was rejected, distinguishing version drift from
 *        machine mismatch.
 *
 * Most of these values name a specific pair of data models -- the one that
 * wrote the file and the one now reading it. A save written by a 64-bit build
 * is not merely "wrong version"; its integers are laid out differently, and
 * saying which combination occurred lets the player be told something useful.
 */
/**
 * @brief 세이브 파일이 거부된 이유. 버전 차이와 기계 불일치를 구분한다.
 *
 * 대부분의 값은 특정한 데이터 모델 쌍을 지칭한다. 파일을 쓴 쪽과 지금 읽는
 * 쪽이다. 64비트 빌드가 쓴 저장 파일은 단지 "버전이 다른" 것이 아니라 정수
 * 배치 자체가 다르며, 어떤 조합이 발생했는지 밝혀야 플레이어에게 쓸모 있는
 * 안내를 할 수 있다.
 */
/* Values for savefile status */
#define SF_UPTODATE                     0
#define SF_OUTDATED                     1
#define SF_CRITICAL_BYTE_COUNT_MISMATCH 2
#define SF_DM_IL32LLP64_ON_ILP32LL64    3  /* Wind x64 savefile on x86     */
#define SF_DM_I32LP64_ON_ILP32LL64      4  /* Unix 64 savefile on x86      */
#define SF_DM_ILP32LL64_ON_I32LP64      5  /* x86 savefile on Unix 64      */
#define SF_DM_ILP32LL64_ON_IL32LLP64    6  /* x86 savefile on Wind x64     */
#define SF_DM_I32LP64_ON_IL32LLP64      7  /* Unix 64 savefile on Wind x64 */
#define SF_DM_IL32LLP64_ON_I32LP64      8  /* Wind x64 savefile on Unix 64 */
#define SF_DM_MISMATCH                  9  /* generic savefile byte mismatch */

/**
 * @brief Which of the three persistent file kinds a handle refers to.
 * @note All three share one handle type and one set of routines; this is what
 *       distinguishes them.
 */
/**
 * @brief 핸들이 가리키는 세 가지 영속 파일 종류 중 무엇인지.
 * @note 셋 모두 하나의 핸들 타입과 하나의 루틴 집합을 공유하며, 이 값이 그것들을
 *       구분한다.
 */
/* NetHack ftypes */
#define NHF_LEVELFILE       1
#define NHF_SAVEFILE        2
#define NHF_BONESFILE       3

/**
 * @brief What a serialization pass is doing as it walks the game's structures.
 *
 * The save routines are written once and reused for counting, writing, and
 * tearing down, so the same traversal serves several purposes depending on
 * these bits.
 *
 * @warning @c FREEING means the pass destroys what it visits. A traversal is
 *          not read-only just because it is not writing a file.
 */
/**
 * @brief 직렬화 패스가 게임 구조체들을 순회하며 무엇을 하는 중인지.
 *
 * 저장 루틴은 한 번 작성되어 개수 세기, 쓰기, 해제에 재사용된다. 따라서 같은
 * 순회가 이 비트들에 따라 여러 목적을 수행한다.
 *
 * @warning @c FREEING 은 패스가 방문하는 대상을 파괴한다는 뜻이다. 파일에 쓰지
 *          않는다고 해서 읽기 전용 순회인 것은 아니다.
 */
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

/**
 * @brief How a save is laid out on disk.
 *
 * The historical format dumps whole structures verbatim, which is fast but
 * ties the file to the machine that wrote it; the ascii export writes field by
 * field, which is portable and inspectable.
 */
/**
 * @brief 저장 파일이 디스크에 놓이는 방식.
 *
 * 전통 형식은 구조체 전체를 그대로 덤프하므로 빠르지만 파일이 그것을 쓴 기계에
 * 묶인다. ascii 내보내기는 필드 단위로 기록하므로 이식 가능하고 들여다볼 수 있다.
 */
enum saveformats {
    invalid = 0,
    historical = 1,     /* entire struct, binary, as-is */
    exportascii = 2,    /* each field written out as ascii text */
    NUM_SAVEFORMATS
};

/**
 * @brief Style selection for a field-level save.
 * @note Applies only when the handle has @c fieldlevel set; ignored for
 *       traditional structure-level writes.
 */
/**
 * @brief 필드 단위 저장의 방식 선택.
 * @note 핸들에 @c fieldlevel 이 설정된 경우에만 적용되며, 전통적인 구조체 단위
 *       쓰기에서는 무시된다.
 */
/* Content types for fieldlevel files */
struct fieldlevel_content {
    boolean deflt;        /* individual fields */
    boolean binary;       /* binary rather than text */
};

/**
 * @brief A handle onto a save, level, or bones file.
 *
 * Carries both possible representations at once: a raw descriptor for
 * structure-level binary writes and a set of stdio streams for field-level
 * ones, with @c structlevel and @c fieldlevel saying which is in use. The
 * counters exist so a pass can report how much it moved, which is how the
 * counting mode sizes a file before writing it.
 *
 * @note @c nhfpconvert points at a second handle when a file is being
 *       converted between formats, so both are open at once.
 * @warning @c mode determines whether operations through this handle also free
 *          the structures they traverse; see the mode bits above.
 */
/**
 * @brief 저장·레벨·본즈 파일에 대한 핸들.
 *
 * 가능한 두 표현을 동시에 지닌다. 구조체 단위 이진 쓰기를 위한 원시 디스크립터와
 * 필드 단위 쓰기를 위한 stdio 스트림들이며, @c structlevel 과 @c fieldlevel 이
 * 어느 쪽을 쓰는지 알려 준다. 카운터는 패스가 얼마나 이동했는지 보고하기 위한
 * 것으로, 개수 세기 모드가 쓰기 전에 파일 크기를 산정하는 방식이다.
 *
 * @note 파일을 형식 간 변환하는 중에는 @c nhfpconvert 가 두 번째 핸들을 가리켜
 *       둘이 동시에 열려 있게 된다.
 * @warning @c mode 는 이 핸들을 통한 작업이 순회하는 구조체를 해제하기도 하는지
 *          결정한다. 위의 모드 비트를 참고할 것.
 */
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
