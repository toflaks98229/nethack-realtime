/* NetHack 5.0	sfstruct.c	$NHDT-Date: 1781973066 2026/06/20 16:31:06 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.28 $ */
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/*-Copyright (c) Michael Allison, 2025. */
/* NetHack may be freely redistributed.  See license for details. */

/**
 * @file sfstruct.c
 * @brief The historical save format: structures written to disk as they sit
 *        in memory.
 *
 * Each type gets a matched pair of routines, one to write and one to read, and
 * they do essentially nothing -- the bytes of the object go straight out and
 * straight back in. That is what makes this format fast, and also what ties a
 * save file to the machine and the build that produced it.
 *
 * The pairs exist so that the save code can name what it is writing rather
 * than counting bytes, and so a different format can be substituted by
 * swapping the table of routines rather than editing every call site.
 *
 * @warning Reading a file written by a build with different integer sizes or
 *          alignment produces garbage rather than an error; the version and
 *          data-model checks performed before restoring are what prevent it.
 */

/**
 * @file sfstruct.c
 * @brief 전통 저장 형식. 구조체를 메모리에 놓인 모습 그대로 디스크에 쓴다.
 *
 * 타입마다 쓰기와 읽기가 짝을 이루며, 하는 일은 사실상 없다. 객체의 바이트가
 * 그대로 나가고 그대로 들어온다. 이 형식이 빠른 이유이자, 저장 파일이 그것을
 * 만든 기계와 빌드에 묶이는 이유이기도 하다.
 *
 * 짝을 이루어 두는 것은 저장 코드가 바이트를 세는 대신 무엇을 쓰는지 이름으로
 * 말할 수 있게 하고, 모든 호출 지점을 고치는 대신 루틴 표만 바꿔 다른 형식으로
 * 교체할 수 있게 하기 위함이다.
 *
 * @warning 정수 크기나 정렬이 다른 빌드가 쓴 파일을 읽으면 오류가 아니라 쓰레기가
 *          나온다. 복원 전에 수행하는 버전·데이터 모델 검사가 그것을 막는다.
 */

#include "hack.h"
#include "sfprocs.h"

/* #define SFLOGGING */          /* debugging */

staticfn void sfstruct_read_error(void);

/* historical full struct savings */

#ifdef SAVEFILE_DEBUGGING
#if defined(__GNUC__)
#define DEBUGFORMATSTR64 "%s %s %ld %ld %d\n"
#elif defined(_MSC_VER)
#define DEBUGFORMATSTR64 "%s %s %lld %ld %d\n"
#endif
#endif

#ifdef SFLOGGING
staticfn void logging_finish(void);
#endif

/**
 * @name The bodies of the write and read routines
 * @brief What every routine in this format actually does, written once.
 *
 * There are hundreds of routines here and only four bodies, because in this format every type is handled identically: the bytes go out, or the bytes come in. The routines exist to give the operation a name and a
 * type, not to do anything different from one another.
 *
 * @note The reading body checks for having already reached the end of the file before reading and again after. Both are needed: reading past the end must not be attempted, and a read that fails has to mark the file
 *       so the next one does not attempt it either. Without that a truncated save file would be read as an unbounded stream of zeroes.
 * @note The variants ending in a compound body exist for types holding pointers. A pointer cannot be written as it stands, so those types are normalised before writing and again after reading -- which is why the
 *       normalising routine is declared alongside.
 * @warning Nothing here converts anything. The type's own bytes are the format, which is what makes this fast and what makes a file unreadable by a build whose types are laid out differently.
 * @{
 */
/**
 * @name 쓰기와 읽기 루틴의 본문
 * @brief 이 형식의 모든 루틴이 실제로 하는 일. 한 번만 적힌 것.
 *
 * 여기에는 수백 개의 루틴이 있고 본문은 넷뿐이다. 이 형식에서는 모든 타입이 똑같이 다뤄지기 때문이다. 바이트가 나가거나, 바이트가 들어온다. 루틴들이 존재하는 것은 그 연산에 이름과 타입을 주기 위해서이고, 서로 다른 일을 하기 위해서가 아니다.
 *
 * @note 읽기 본문은 읽기 전에 이미 파일 끝에 이르렀는지 검사하고 읽은 뒤에 다시 검사한다. 둘 다 필요하다. 끝을 넘어 읽는 것이 시도되어서는 안 되고, 실패한 읽기는 다음 읽기도 그것을 시도하지 않도록 그 파일을 표시해야 한다. 그것 없이는 잘린 저장 파일이 끝없는 0의 흐름으로 읽힌다.
 * @note 복합 본문으로 끝나는 변종은 포인터를 담은 타입을 위해 존재한다. 포인터는 그대로 기록될 수 없으므로, 그런 타입은 쓰기 전에 그리고 읽은 뒤에 다시 정규화된다. 그것이 정규화 루틴이 나란히 선언되는 이유다.
 * @warning 여기의 어느 것도 무엇을 변환하지 않는다. 타입 자신의 바이트가 그 형식이며, 그것이 이것을 빠르게 만들고 타입 배치가 다른 빌드가 그 파일을 읽을 수 없게 만드는 것이다.
 * @{
 */
#define SFO_BODY(dt) \
{                                                                                   \
    bwrite(nhfp->fd, (genericptr_t) d_##dt, sizeof *d_##dt);                        \
}

#define SFI_BODY(dt) \
{                                                                                   \
    if (nhfp->eof) {                                                                \
       sfstruct_read_error();                                                       \
    }                                                                               \
    mread(nhfp->fd, (genericptr_t) d_##dt, sizeof *d_##dt);                         \
    if (restoreinfo.mread_flags == -1)                                              \
        nhfp->eof = TRUE;                                                           \
}

/**
 * @name Generating a routine pair
 * @brief Define the write and read routines for one type, from one line.
 *
 * Three forms because the types are not shaped alike: a plain type, a structure or union needing its keyword separately, and one whose routines take a size. They mirror the declarations in the save interface exactly,
 * which is deliberate -- the declarations and the definitions are generated from the same list of type names, so they cannot disagree about a signature.
 *
 * @note Each also declares the routines it is about to define. That is not redundant: it is what lets the compiler check the definition against a prototype when nothing else has declared it.
 * @note The name argument every routine takes is unused in this format and marked as such. It exists because the other format uses it, and both must have the same signature to be interchangeable through the table.
 * @{
 */
/**
 * @name 루틴 짝 만들기
 * @brief 한 줄에서 한 타입의 쓰기와 읽기 루틴을 정의한다.
 *
 * 세 형태인 것은 타입들의 모양이 같지 않기 때문이다. 평범한 타입, 키워드를 따로 필요로 하는 구조체나 공용체, 그리고 루틴이 크기를 받는 것. 이들은 저장 인터페이스의 선언을 정확히 그대로 따르며 그것은 의도적이다. 선언과 정의가 같은 타입 이름 목록에서 생성되므로 서명에 대해 서로
 * 어긋날 수 없다.
 *
 * @note 각각이 자신이 곧 정의할 루틴을 선언하기도 한다. 그것은 잉여가 아니다. 다른 무엇도 그것을 선언하지 않았을 때 컴파일러가 정의를 프로토타입과 맞춰 볼 수 있게 하는 것이다.
 * @note 모든 루틴이 받는 이름 인자는 이 형식에서 쓰이지 않고 그렇게 표시되어 있다. 다른 형식이 그것을 쓰기 때문에 존재하며, 표를 통해 바꿔 쓸 수 있으려면 둘이 같은 서명을 가져야 한다.
 * @{
 */
#define SF_A(dtyp) \
void historical_sfo_##dtyp(NHFILE *, dtyp *d_##dtyp, const char *);                 \
void historical_sfi_##dtyp(NHFILE *, dtyp *d_##dtyp, const char *);                 \
                                                                                    \
void historical_sfo_##dtyp(NHFILE *nhfp, dtyp *d_##dtyp,                            \
                           const char *myname UNUSED)                               \
    SFO_BODY(dtyp)                                                                  \
                                                                                    \
void historical_sfi_##dtyp(NHFILE *nhfp, dtyp *d_##dtyp,                            \
                  const char *myname UNUSED)                                        \
    SFI_BODY(dtyp)

#define SFO_CBODY(dt)                                                               \
    {                                                                               \
        norm_ptrs_##dt(d_##dt);                                            \
        bwrite(nhfp->fd, (genericptr_t) d_##dt, sizeof *d_##dt);                    \
    }

#define SFI_CBODY(dt)                                                               \
    {                                                                               \
        if (nhfp->eof) {                                                            \
            sfstruct_read_error();                                                  \
        }                                                                           \
        mread(nhfp->fd, (genericptr_t) d_##dt, sizeof *d_##dt);                     \
        norm_ptrs_##dt(d_##dt);                                            \
        if (restoreinfo.mread_flags == -1)                                          \
            nhfp->eof = TRUE;                                                       \
    }

#define SF_C(keyw, dtyp) \
void historical_sfo_##dtyp(NHFILE *, keyw dtyp *d_##dtyp,                           \
                           const char *);                                           \
void historical_sfi_##dtyp(NHFILE *, keyw dtyp *d_##dtyp,                           \
                           const char *);                                           \
extern void norm_ptrs_##dtyp(keyw dtyp *d_##dtyp);                         \
                                                                                    \
void historical_sfo_##dtyp(NHFILE *nhfp, keyw dtyp *d_##dtyp,                       \
                           const char *myname UNUSED)                               \
    SFO_CBODY(dtyp)                                                                 \
                                                                                    \
void historical_sfi_##dtyp(NHFILE *nhfp, keyw dtyp *d_##dtyp,                       \
                           const char *myname UNUSED)                               \
    SFI_CBODY(dtyp)

#define SF_X(xxx, dtyp) \
void historical_sfo_##dtyp(NHFILE *, xxx *d_##dtyp, const char *, int);             \
void historical_sfi_##dtyp(NHFILE *, xxx *d_##dtyp, const char *, int);             \
                                                                                    \
void historical_sfo_##dtyp(NHFILE *nhfp, xxx *d_##dtyp,                             \
                           const char *myname UNUSED, int bflen UNUSED)             \
    SFO_BODY(dtyp)                                                                  \
                                                                                    \
void historical_sfi_##dtyp(NHFILE *nhfp, xxx *d_##dtyp,                             \
                           const char *myname UNUSED, int bflen UNUSED)             \
    SFI_BODY(dtyp)


/** @} */

/**
 * @note Reading the type list generates every routine pair at once. That is the whole of this file's bulk, and it is why adding a saved type here is one line rather than two functions.
 */
/**
 * @note 타입 목록을 읽으면 모든 루틴 짝이 한꺼번에 생성된다. 그것이 이 파일 분량의 전부이며, 여기에 저장되는 타입을 더하는 것이 두 함수가 아니라 한 줄인 이유다.
 */
#include "sfmacros.h"
/**
 * @note The version structure's pair is generated separately, after the list. It has to be: the version is what is checked before deciding whether the rest of the file can be read at all, so it cannot be part of the
 *       list of things whose readability depends on that check.
 */
/**
 * @note 버전 구조체의 짝은 목록 뒤에 따로 생성된다. 그래야 한다. 버전은 파일의 나머지를 아예 읽을 수 있는지 정하기 전에 검사되는 것이므로, 그 검사에 읽을 수 있는지가 달려 있는 것들의 목록에 속할 수 없다.
 */
SF_C(struct, version_info)

void historical_sfo_char(NHFILE *, char *d_char, const char *, int);
void historical_sfi_char(NHFILE *, char *d_char, const char *, int);

void
historical_sfo_char(NHFILE *nhfp, char *d_char,
                    const char *myname UNUSED, int cnt)
{
    bwrite(nhfp->fd, (genericptr_t) d_char, cnt * sizeof (char));
}

void
historical_sfi_char(NHFILE *nhfp, char *d_char,
                    const char *myname UNUSED, int cnt)
{
    mread(nhfp->fd, (genericptr_t) d_char, cnt * sizeof (char));
    if (restoreinfo.mread_flags == -1)
        nhfp->eof = TRUE;
}
//extern void sfo_genericptr(NHFILE *, void **, const char *);
//extern void sfi_genericptr(NHFILE *, void **, const char *);
//extern void sfo_x_genericptr(NHFILE *, void **, const char *);
//extern void sfi_x_genericptr(NHFILE *, void **, const char *);

void historical_sfo_genericptr_t(NHFILE *, genericptr_t *d_genericptr_t,
                                 const char *);
void historical_sfi_genericptr_t(NHFILE *, genericptr_t *d_genericptr_t,
                                 const char *);
void
historical_sfo_genericptr_t(NHFILE *nhfp, genericptr_t *d_genericptr_t,
                            const char *myname UNUSED)
{
    bwrite(nhfp->fd, (genericptr_t) d_genericptr_t, sizeof *d_genericptr_t);
}
void
historical_sfi_genericptr_t(NHFILE *nhfp, genericptr_t *d_genericptr_t,
                            const char *myname UNUSED)
{
    if (nhfp->eof) {
        sfstruct_read_error();
    }
    mread(nhfp->fd, (genericptr_t) d_genericptr_t, sizeof *d_genericptr_t);
    if (restoreinfo.mread_flags == -1)
        nhfp->eof = TRUE;
}

SF_X(uint8_t, bitfield)

struct sf_structlevel_procs historical_sfo_procs = {
    "",
    /* sf */
    {
        historical_sfo_arti_info,
        historical_sfo_nhrect,
        historical_sfo_branch,
        historical_sfo_bubble,
        historical_sfo_cemetery,
        historical_sfo_context_info,
        historical_sfo_nhcoord,
        historical_sfo_damage,
        historical_sfo_dest_area,
        historical_sfo_dgn_topology,
        historical_sfo_dungeon,
        historical_sfo_d_level,
        historical_sfo_ebones,
        historical_sfo_edog,
        historical_sfo_egd,
        historical_sfo_emin,
        historical_sfo_engr,
        historical_sfo_epri,
        historical_sfo_eshk,
        historical_sfo_fe,
        historical_sfo_flag,
        historical_sfo_fruit,
        historical_sfo_gamelog_line,
        historical_sfo_kinfo,
        historical_sfo_levelflags,
        historical_sfo_ls_t,
        historical_sfo_linfo,
        historical_sfo_mapseen_feat,
        historical_sfo_mapseen_flags,
        historical_sfo_mapseen_rooms,
        historical_sfo_mkroom,
        historical_sfo_monst,
        historical_sfo_mvitals,
        historical_sfo_obj,
        historical_sfo_objclass,
        historical_sfo_q_score,
        historical_sfo_rm,
        historical_sfo_spell,
        historical_sfo_stairway,
        historical_sfo_s_level,
        historical_sfo_trap,
        historical_sfo_version_info,
        historical_sfo_you,
        historical_sfo_any,
        historical_sfo_aligntyp,
        historical_sfo_boolean,
        historical_sfo_coordxy,
        historical_sfo_genericptr_t,
        historical_sfo_int,
        historical_sfo_int16,
        historical_sfo_int32,
        historical_sfo_int64,
        historical_sfo_long,
        historical_sfo_schar,
        historical_sfo_short,
        historical_sfo_size_t,
        historical_sfo_time_t,
        historical_sfo_uchar,
        historical_sfo_uint16,
        historical_sfo_uint32,
        historical_sfo_uint64,
        historical_sfo_ulong,
        historical_sfo_unsigned,
        historical_sfo_ushort,
        historical_sfo_xint16,
        historical_sfo_xint8,
        historical_sfo_char,
        historical_sfo_bitfield,
#ifdef DEMO_UPLIFTS
        historical_sfo_mystruct,
        historical_sfo_mystruct_rev0,
#endif
    }
};

struct sf_structlevel_procs historical_sfi_procs = {
    "",
    /* sf */
    {
        historical_sfi_arti_info,
        historical_sfi_nhrect,
        historical_sfi_branch,
        historical_sfi_bubble,
        historical_sfi_cemetery,
        historical_sfi_context_info,
        historical_sfi_nhcoord,
        historical_sfi_damage,
        historical_sfi_dest_area,
        historical_sfi_dgn_topology,
        historical_sfi_dungeon,
        historical_sfi_d_level,
        historical_sfi_ebones,
        historical_sfi_edog,
        historical_sfi_egd,
        historical_sfi_emin,
        historical_sfi_engr,
        historical_sfi_epri,
        historical_sfi_eshk,
        historical_sfi_fe,
        historical_sfi_flag,
        historical_sfi_fruit,
        historical_sfi_gamelog_line,
        historical_sfi_kinfo,
        historical_sfi_levelflags,
        historical_sfi_ls_t,
        historical_sfi_linfo,
        historical_sfi_mapseen_feat,
        historical_sfi_mapseen_flags,
        historical_sfi_mapseen_rooms,
        historical_sfi_mkroom,
        historical_sfi_monst,
        historical_sfi_mvitals,
        historical_sfi_obj,
        historical_sfi_objclass,
        historical_sfi_q_score,
        historical_sfi_rm,
        historical_sfi_spell,
        historical_sfi_stairway,
        historical_sfi_s_level,
        historical_sfi_trap,
        historical_sfi_version_info,
        historical_sfi_you,
        historical_sfi_any,

        historical_sfi_aligntyp,
        historical_sfi_boolean,
        historical_sfi_coordxy,
        historical_sfi_genericptr_t,
        historical_sfi_int,
        historical_sfi_int16,
        historical_sfi_int32,
        historical_sfi_int64,
        historical_sfi_long,
        historical_sfi_schar,
        historical_sfi_short,
        historical_sfi_size_t,
        historical_sfi_time_t,
        historical_sfi_uchar,
        historical_sfi_uint16,
        historical_sfi_uint32,
        historical_sfi_uint64,
        historical_sfi_ulong,
        historical_sfi_unsigned,
        historical_sfi_ushort,
        historical_sfi_xint16,
        historical_sfi_xint8,
        historical_sfi_char,
        historical_sfi_bitfield,
#ifdef DEMO_UPLIFTS
        historical_sfi_mystruct,
        historical_sfi_mystruct_rev0,
#endif
    }
};

/*
 * The historical bwrite() and mread() functions follow
 */

#ifdef SAVEFILE_DEBUGGING
static long floc = 0L;
#endif

/*
 * historical structlevel savefile writing and reading routines follow.
 * These were moved here from save.c and restore.c between 3.6.3 and 5.0.0,.
 */

staticfn int getidx(int, int);

#if defined(UNIX) || defined(WIN32)
#define USE_BUFFERING
#endif

struct restore_info restoreinfo = {
    "externalcomp", 0,
};

#define MAXFD 5
enum {NOFLG = 0, NOSLOT = 1};
static int bw_sticky[MAXFD] = {-1,-1,-1,-1,-1};
static int bw_buffered[MAXFD] = {0,0,0,0,0};
#ifdef USE_BUFFERING
static FILE *bw_FILE[MAXFD] = {0,0,0,0,0};
#endif

#ifdef SFLOGGING
static FILE *ofp[20] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                         0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    char ofnamebuf[80];
    static long ocnt = 0L;

    static FILE *ifp[20] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                             0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    char ifnamebuf[80];
    static long icnt = 0L;
#endif


/*
 * Presumably, the fdopen() to allow use of stdio fwrite()
 * over write() was done for performance or functionality
 * reasons to help some particular platform long ago.
 *
 * There have been some issues being encountered with the
 * implementation due to having an individual set of
 * tracking variables, even though there were nested
 * sets of open fd (like INSURANCE).
 *
 * This uses an individual tracking entry for each fd
 * being used.
 *
 * Some notes:
 *
 * Once buffered IO (stdio) has been enabled on the file
 * associated with a descriptor via fdopen():
 *
 *    1. If you use bufoff and bufon to try to toggle the
 *       use of write vs fwrite; the code just tracks which
 *       routine is to be called through the tracking
 *       variables and acts accordingly.
 *             bw_sticky[]    -  used to find the index number for
 *                               the fd that is stored in it, or -1
 *                               if it is a free slot.
 *             bw_buffered[]  -  indicator that buffered IO routines
 *                               are available for use.
 *             bw_FILE[]      -  the non-zero FILE * for use in calling
 *                               fwrite() when bw_buffered[] is also
 *                               non-zero.
 *
 *    2. It is illegal to call close(fd) after fdopen(), you
 *       must always use fclose() on the FILE * from
 *       that point on, so care must be taken to never call
 *       close(fd) on the underlying fd or bad things will
 *       happen.
 */

/**
 * @brief Find the slot tracking a file, optionally claiming a free one.
 *
 * The buffering machinery keeps a small fixed table of files it knows about, and this is how a file is looked up in it. The flag decides whether a file not already there gets a slot -- which is how the same lookup serves
 * both "is this file buffered" and "start tracking this file".
 *
 * @param fd the file
 * @param flg whether to claim a free slot if the file is not already tracked
 * @return the slot, or negative if the file is not tracked and none was claimed
 * @warning The table has a fixed size, so claiming can fail. A caller that ignores a negative result will index the table with it.
 */
/**
 * @brief 어떤 파일을 추적하는 칸을 찾고, 선택적으로 빈 칸을 차지한다.
 *
 * 버퍼링 기제는 자신이 아는 파일에 대한 작은 고정 표를 보관하며, 이것이 그 안에서 파일을 조회하는 방식이다. 그 플래그가 아직 거기 없는 파일이 칸을 받는지를 정한다. 그것이 같은 조회가 "이 파일이 버퍼링되는가"와 "이 파일을 추적하기 시작하라" 둘 다를 맡는 방식이다.
 *
 * @param fd 그 파일
 * @param flg 그 파일이 아직 추적되지 않을 때 빈 칸을 차지할지
 * @return 그 칸. 파일이 추적되지 않고 아무것도 차지되지 않았으면 음수
 * @warning 그 표는 크기가 고정되어 있으므로 차지하기가 실패할 수 있다. 음수 결과를 무시하는 호출자는 그것으로 표를 색인하게 된다.
 */
staticfn int
getidx(int fd, int flg)
{
    int i, retval = -1;

    for (i = 0; i < MAXFD; ++i)
        if (bw_sticky[i] == fd)
            return i;
    if (flg == NOSLOT)
        return retval;
    for (i = 0; i < MAXFD; ++i)
        if (bw_sticky[i] < 0) {
            bw_sticky[i] = fd;
            retval = i;
            break;
        }
    return retval;
}

/**
 * @brief Whether this file must be closed through the buffering machinery rather than directly.
 *
 * The question exists because of the hazard the commentary above spells out: once a file has been given a buffered stream, closing the underlying file directly is not permitted and doing so causes trouble. So a caller
 * that closes files has to ask first.
 *
 * @param fd the file
 * @return true if the buffered close must be used
 * @warning Answering this wrongly is not a leak but corruption -- the buffered stream's own close would then never happen, and whatever it had not yet written is lost.
 */
/**
 * @brief 이 파일이 직접이 아니라 버퍼링 기제를 통해 닫혀야 하는지.
 *
 * 이 질문이 존재하는 것은 위의 해설이 적어 놓은 위험 때문이다. 파일에 버퍼링된 스트림이 주어진 뒤에는 그 바탕 파일을 직접 닫는 것이 허용되지 않고, 그렇게 하면 문제가 생긴다. 그래서 파일을 닫는 호출자는 먼저 물어야 한다.
 *
 * @param fd 그 파일
 * @return 버퍼링된 닫기를 써야 하면 참
 * @warning 이것에 잘못 답하는 것은 누수가 아니라 손상이다. 그러면 버퍼링된 스트림 자신의 닫기가 결코 일어나지 않고, 그것이 아직 쓰지 않은 것이 사라진다.
 */
/* Let caller know that bclose() should handle it (TRUE) */
boolean
close_check(int fd)
{
    int idx = getidx(fd, NOSLOT);
    boolean retval = FALSE;

    if (idx >= 0)
        retval = TRUE;
    return retval;
}

/**
 * @brief Begin buffering writes to a file.
 *
 * Saving writes a great many small pieces, and doing each as a separate system call is slow. Buffering gathers them, which is why saving is switched to buffered writing for the bulk of a save and back for the parts that
 * must reach the disk in order.
 *
 * @param fd the file
 * @warning Enabling buffering twice on one file is a programming error and is reported as one rather than being tolerated, because the second stream would have its own buffer and the two would interleave unpredictably.
 * @note Where the platform has no buffered writing the tracking still happens but no stream is created, so the rest of the code is unaffected and simply gains nothing.
 */
/**
 * @brief 파일에 대한 쓰기를 버퍼링하기 시작한다.
 *
 * 저장은 아주 많은 작은 조각을 쓰며, 각각을 별도의 시스템 호출로 하는 것은 느리다. 버퍼링이 그것들을 모으며, 그래서 저장의 대부분에 대해 버퍼링된 쓰기로 전환되고 순서대로 디스크에 닿아야 하는 부분에 대해 되돌려진다.
 *
 * @param fd 그 파일
 * @warning 한 파일에 버퍼링을 두 번 켜는 것은 프로그래밍 오류이며 용인되는 대신 그렇게 알려진다. 두 번째 스트림이 자기 버퍼를 가지게 되고 그 둘이 예측할 수 없게 섞이기 때문이다.
 * @note 플랫폼에 버퍼링된 쓰기가 없는 곳에서는 추적은 여전히 일어나고 스트림은 만들어지지 않는다. 그래서 나머지 코드는 영향을 받지 않고 그저 아무것도 얻지 않는다.
 */
void
bufon(int fd)
{
    int idx = getidx(fd, NOFLG);

    if (idx >= 0) {
        bw_sticky[idx] = fd;
#ifdef USE_BUFFERING
        if (bw_buffered[idx])
            panic("buffering already enabled");
        if (!bw_FILE[idx]) {
            if ((bw_FILE[idx] = fdopen(fd, "w")) == 0)
                panic("buffering of file %d failed", fd);
        }
        bw_buffered[idx] = (bw_FILE[idx] != 0);
#else
        bw_buffered[idx] = 1;
#endif
    }
}

/**
 * @brief Stop buffering writes to a file.
 * @param fd the file
 * @note Flushes before switching, so nothing already gathered is lost. That ordering is what makes turning buffering off safe to do partway through writing a file.
 * @warning Does not close the buffered stream, only stops using it -- so a file switched off and on again does not create a second stream. That is deliberate, since creating one twice is an error.
 */
/**
 * @brief 파일에 대한 쓰기 버퍼링을 멈춘다.
 * @param fd 그 파일
 * @note 전환하기 전에 비운다. 그래서 이미 모인 것이 사라지지 않는다. 그 순서가 파일을 쓰는 도중에 버퍼링을 끄는 일을 안전하게 만드는 것이다.
 * @warning 버퍼링된 스트림을 닫지 않고 그것을 쓰기만 멈춘다. 그래서 끄고 다시 켠 파일이 두 번째 스트림을 만들지 않는다. 의도적이다. 그것을 두 번 만드는 것이 오류이기 때문이다.
 */
void
bufoff(int fd)
{
    int idx = getidx(fd, NOFLG);

    if (idx >= 0) {
        bflush(fd);
        bw_buffered[idx] = 0;     /* just a flag that says "use write(fd)" */
    }
}

/**
 * @brief Close a file, through its buffered stream if it has one.
 *
 * The only correct way to close a file this machinery has touched. It closes the stream where one exists and the file directly where one does not, and returns the tracking slot -- so a long-running game that saves many
 * times does not exhaust the table.
 *
 * @param fd the file
 * @note Stops buffering first, which flushes. So closing is not a way to discard buffered output; there is no way to discard it.
 */
/**
 * @brief 파일을 닫는다. 버퍼링된 스트림이 있으면 그것을 통해서.
 *
 * 이 기제가 손댄 파일을 닫는 유일하게 올바른 방법이다. 스트림이 있는 곳에서는 그 스트림을 닫고 없는 곳에서는 파일을 직접 닫으며, 추적 칸을 되돌려준다. 그래서 여러 번 저장하는 오래 도는 게임이 그 표를 소진하지 않는다.
 *
 * @param fd 그 파일
 * @note 먼저 버퍼링을 멈추며, 그것이 비운다. 그래서 닫는 것이 버퍼링된 출력을 버리는 방법은 아니다. 그것을 버릴 방법은 없다.
 */
void
bclose(int fd)
{
    int idx = getidx(fd, NOSLOT);

    bufoff(fd);     /* sets bw_buffered[idx] = 0 */
    if (idx >= 0) {
#ifdef USE_BUFFERING
        if (bw_FILE[idx]) {
            (void) fclose(bw_FILE[idx]);
            bw_FILE[idx] = 0;
        } else
#endif
            close(fd);
        /* return the idx to the pool */
        bw_sticky[idx] = -1;
    }
#ifdef SFLOGGING
    if (fd >= 0 && fd <= SIZE(ofp)) {
        if (ofp[fd]) {
            fclose(ofp[fd]);
            ofp[fd] = 0;
        } else if (ifp[fd]) {
            fclose(ifp[fd]);
            ifp[fd] = 0;
        }
    }
#endif
    return;
}

/**
 * @brief Force everything gathered so far out to the file.
 * @param fd the file
 * @warning A failed flush ends the game rather than being reported, and that is the right severity: it means the save file on disk is incomplete, and continuing would leave the player believing they had saved.
 */
/**
 * @brief 지금까지 모인 모든 것을 파일로 억지로 내보낸다.
 * @param fd 그 파일
 * @warning 실패한 비우기는 알려지는 대신 게임을 끝낸다. 그것이 알맞은 심각도다. 디스크의 저장 파일이 불완전하다는 뜻이고, 계속하는 것은 플레이어가 저장했다고 믿게 남기는 일이다.
 */
void
bflush(int fd)
{
    int idx = getidx(fd, NOFLG);

    if (idx >= 0) {
#ifdef USE_BUFFERING
        if (bw_FILE[idx]) {
           if (fflush(bw_FILE[idx]) == EOF)
               panic("flush of savefile failed!");
        }
#endif
    }
    return;
}

/**
 * @brief Write bytes to a file, buffered or not, and end the game if it fails.
 *
 * Everything this format writes goes through here. It writes through whichever mechanism the file is currently using, and treats a short write as a failure -- because a save file that is partly written is not a save file.
 *
 * @param fd the file
 * @param loc the bytes
 * @param num how many
 * @warning Does not return on failure. A write that cannot complete ends the game, and where the game is already ending because the connection was lost it exits immediately rather than trying to report anything.
 * @note Writing nothing returns early, and the accompanying comment gives the reason: one platform's buffered writing does not reliably report success when asked to write zero bytes, so the case is excluded rather than
 *       interpreted.
 */
/**
 * @brief 파일에 바이트를 쓴다. 버퍼링되든 아니든. 그리고 실패하면 게임을 끝낸다.
 *
 * 이 형식이 쓰는 모든 것이 여기를 지난다. 그 파일이 지금 쓰고 있는 기제가 무엇이든 그것을 통해 쓰고, 짧은 쓰기를 실패로 취급한다. 일부만 기록된 저장 파일은 저장 파일이 아니기 때문이다.
 *
 * @param fd 그 파일
 * @param loc 그 바이트들
 * @param num 몇 개인지
 * @warning 실패하면 반환하지 않는다. 완료될 수 없는 쓰기는 게임을 끝내며, 연결이 끊겨 게임이 이미 끝나는 중인 곳에서는 무엇을 알리려 하는 대신 즉시 나간다.
 * @note 아무것도 쓰지 않는 것은 일찍 반환하며, 딸린 주석이 그 이유를 밝힌다. 어떤 플랫폼의 버퍼링된 쓰기가 0바이트를 쓰라고 요청받았을 때 성공을 믿을 만하게 알리지 않으므로, 그 경우가 해석되는 대신 제외된다.
 */
void
bwrite(int fd, const genericptr_t loc, unsigned num)
{
    boolean failed;
    int idx = getidx(fd, NOFLG);

#ifdef SFLOGGING
    if (fd >= 0 && fd < SIZE(ofp)) {
        if (!ofp[fd]) {
            Snprintf(ofnamebuf, sizeof ofnamebuf, "bwrite_%02d.log", fd);
            ofp[fd] = fopen(ofnamebuf, "w");
        }
        if (ofp[fd]) {
            fprintf(ofp[fd], "%08ld, %08ld, %d\n", ocnt,
                    ftell(ofp[fd]), num);
            ocnt++;
        }
    }
#endif

    if (idx >= 0) {
        if (num == 0) {
            /* nothing to do; we need a special case to exit early
               because glibc fwrite doesn't give reliable
               success/failure indication when writing 0 bytes */
            return;
        }

#ifdef USE_BUFFERING
        if (bw_buffered[idx] && bw_FILE[idx]) {
            failed = (fwrite(loc, (int) num, 1, bw_FILE[idx]) != 1);
        } else
#endif /* UNIX */
        {
            /* lint wants 3rd arg of write to be an int; lint -p an unsigned */
#if defined(BSD) || defined(ULTRIX) || defined(WIN32) || defined(_MSC_VER)
            failed = ((long) write(fd, loc, (int) num) != (long) num);
#else /* e.g. SYSV, __TURBOC__ */
            failed = ((long) write(fd, loc, num) != (long) num);
#endif
        }
        if (failed) {
#if defined(HANGUPHANDLING)
            if (program_state.done_hup)
                nh_terminate(EXIT_FAILURE);
            else
#endif
                panic("cannot write %u bytes to file #%d", num, fd);
        }
    } else
        impossible("fd not in list (%d)?", fd);
}

/*  ===================================================== */

/**
 * @brief Read bytes from a file, and decide what to do about getting fewer than asked for.
 *
 * The reading counterpart of writing, and the more delicate of the two. A short read means the file is truncated, and what should happen then is not always the same: while restoring a game it is fatal, and while checking
 * whether a file is worth restoring at all it is an answer.
 *
 * So the caller establishes in advance which it wants, and this obeys that rather than deciding for itself.
 *
 * @param fd the file
 * @param buf where to put the bytes
 * @param len how many are wanted
 * @warning On a short read the buffer holds whatever was read and the rest is untouched -- so a caller told to carry on must not assume the buffer was filled. The routine's own callers mark the file as finished, which is
 *          what stops a truncated file being read as an endless supply of stale bytes.
 * @note The length type differs between platforms and is named locally for that reason, with the accompanying comment conceding that the ideal type is not available.
 */
/**
 * @brief 파일에서 바이트를 읽고, 요청한 것보다 적게 받은 일에 대해 무엇을 할지 정한다.
 *
 * 쓰기의 읽기 짝이며, 둘 중 더 섬세한 것이다. 짧은 읽기는 파일이 잘렸다는 뜻이고, 그때 무슨 일이 일어나야 하는지는 항상 같지 않다. 게임을 복원하는 중에는 치명적이고, 파일이 아예 복원할 가치가 있는지 확인하는 중에는 하나의 답이다.
 *
 * 그래서 호출자가 자신이 어느 쪽을 원하는지 미리 확정하고, 이것은 스스로 정하는 대신 그것을 따른다.
 *
 * @param fd 그 파일
 * @param buf 바이트를 넣을 곳
 * @param len 몇 개를 원하는지
 * @warning 짧은 읽기에서 버퍼는 읽힌 것만을 담고 나머지는 건드려지지 않는다. 그래서 계속하라고 들은 호출자는 버퍼가 채워졌다고 가정해서는 안 된다. 이 루틴 자신의 호출자들이 그 파일을 끝난 것으로 표시하며, 그것이 잘린 파일이 끝없는 낡은 바이트 공급으로 읽히는 것을 막는 것이다.
 * @note 길이 타입은 플랫폼마다 다르며 그 이유로 지역에서 이름 붙는다. 딸린 주석은 이상적인 타입을 쓸 수 없다고 인정하고 있다.
 */
void
mread(int fd, genericptr_t buf, unsigned len)
{

#ifdef SFLOGGING
    if (fd >= 0 && fd < 9) {
        if (!ifp[fd]) {
            Snprintf(ifnamebuf, sizeof ifnamebuf, "mread_%02d.log", fd);
            ifp[fd] = fopen(ifnamebuf, "w");
        }
        if (ifp[fd]) {
            fprintf(ifp[fd], "%08ld, %08ld, %d\n", icnt,
                    ftell(ifp[fd]), (int) len);
            icnt++;
        }
    }
#endif

#if defined(BSD) || defined(ULTRIX) || defined(WIN32)
#define readLenType int
#else /* e.g. SYSV, __TURBOC__ */
#define readLenType unsigned
#endif
    readLenType rlen;
        /* Not perfect, but we don't have ssize_t available. */
    rlen = (readLenType) read(fd, buf, (readLenType) len);
    if ((readLenType) rlen != (readLenType) len) {
        if ((restoreinfo.mread_flags == 1) /* means "return anyway" */
            || (program_state.reading_bonesfile == 1)) {
            restoreinfo.mread_flags = -1;
            return;
        } else {
#ifndef SFCTOOL
            pline("Read %d instead of %u bytes.", (int) rlen, len);
            display_nhwindow(WIN_MESSAGE, TRUE); /* flush before error() */
            if (program_state.restoring) {
                (void) nhclose(fd);
                (void) delete_savefile();
                error("Error restoring old game.");
            }
            panic("Error reading level file.");
#else
            printf("Read %d instead of %u bytes.\n", (int) rlen, len);
#endif
        }
    }
}

/**
 * @brief The place a read past the end of the file is noticed.
 * @warning Deliberately does nothing at present, as its body's comment concedes. It exists as the single point where that condition arrives, so that whatever should be done about it can be done in one place rather than at
 *          every reading routine -- and every generated routine already calls it.
 * @note So it is a hook rather than dead code: the routines that call it are correct in doing so, and what is missing is the response and not the call.
 */
/**
 * @brief 파일 끝을 넘어선 읽기가 알아채지는 곳.
 * @warning 그 본문의 주석이 인정하듯 현재 의도적으로 아무것도 하지 않는다. 그 조건이 도착하는 단 하나의 지점으로 존재하며, 그래서 그것에 대해 해야 할 일을 읽기 루틴마다가 아니라 한곳에서 할 수 있다. 그리고 생성된 모든 루틴이 이미 그것을 호출한다.
 * @note 그래서 이것은 죽은 코드가 아니라 훅이다. 그것을 호출하는 루틴들은 그렇게 하는 것이 옳으며, 빠진 것은 그 호출이 아니라 그 대응이다.
 */
staticfn void
sfstruct_read_error(void)
{
    /* problem */;
}

/**
 * @brief Close the per-file trace logs and reset their counters.
 * @note Only exists in a build with the tracing switch enabled. The tracing writes one log per file being saved or restored, recording the size and position of every read and write -- which is how a save file's structure is
 *       compared against what was expected when a format change goes wrong.
 */
/**
 * @brief 파일별 추적 로그를 닫고 그 계수기를 초기화한다.
 * @note 추적 스위치가 켜진 빌드에서만 존재한다. 그 추적은 저장되거나 복원되는 파일마다 로그 하나를 쓰며, 모든 읽기와 쓰기의 크기와 위치를 기록한다. 그것이 형식 변경이 잘못되었을 때 저장 파일의 구조를 기대되던 것과 견주어 보는 방식이다.
 */
#ifdef SFLOGGING
staticfn void
logging_finish(void)
{
    int i;

    for (i = 0; i < SIZE(ofp); ++i) {
        if (ofp[i]) {
            fclose(ofp[i]);
            ofp[i] = 0;
        }
    }
    ocnt = 0L;

    for (i = 0; i < SIZE(ifp); ++i) {
        if (ifp[i]) {
            fclose(ifp[i]);
            ifp[i] = 0;
        }
    }
    icnt = 0L;
}
#endif  /* SFLOGGING */

#undef SF_X
#undef SF_C
#undef SF_A

/* end of sfstruct.c */

