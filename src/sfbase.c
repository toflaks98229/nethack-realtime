/* NetHack 5.0	sfbase.c.template $NHDT-Date$  $NHDT-Branch$:$NHDT-Revision$ */
/* Copyright (c) Michael Allison, 2025. */
/* NetHack may be freely redistributed.  See license for details. */

/**
 * @file sfbase.c
 * @brief Choosing how a game is written, and routing every read and write
 *        through that choice.
 *
 * The save code says what it wants to store; this decides how. A table of
 * routines per format sits behind that, so the historical binary layout and
 * the field-by-field text export are interchangeable without the save code
 * knowing which is in use.
 *
 * That indirection is also what makes conversion between formats possible: a
 * file can be read through one set of routines and written through another.
 *
 * @note The mode bits on the handle still decide whether a traversal writes or
 *       frees; the format only decides what the writing looks like.
 */

/**
 * @file sfbase.c
 * @brief 게임을 어떤 방식으로 쓸지 고르고, 모든 읽기와 쓰기를 그 선택으로
 *        흘려보내기.
 *
 * 저장 코드는 무엇을 담을지 말하고, 어떻게 담을지는 여기서 정한다. 그 뒤에 형식별
 * 루틴 표가 있어서, 전통적인 이진 배치와 필드 단위 텍스트 내보내기를 저장 코드가
 * 어느 쪽인지 모른 채 바꿔 쓸 수 있다.
 *
 * 이 간접화 덕분에 형식 간 변환도 가능해진다. 한 루틴 집합으로 읽고 다른 집합으로
 * 쓰면 되기 때문이다.
 *
 * @note 순회가 쓰기인지 해제인지는 여전히 핸들의 모드 비트가 정한다. 형식은 그
 *       쓰기가 어떤 모습인지만 정한다.
 */

#include "hack.h"
#include "sfprocs.h"

#ifdef SFCTOOL
//#include "sfproto.h"
#endif

/* #define DO_DEBUG */

/**
 * @def TURN_OFF_LOGGING
 * @brief The mode bit that suppresses logging for the duration of a nested call.
 * @note Defined as the bit above the last real mode bit rather than as a number, so adding a mode bit moves this one with it. The commented-out literal above records what it currently comes to -- and is exactly what would go stale.
 * @warning It is set in the same field as the genuine mode bits, so it is not private. Anything examining the mode sees it, and code that copies the mode carries it along.
 */
/**
 * @def TURN_OFF_LOGGING
 * @brief 겹친 호출 동안 기록을 억제하는 모드 비트.
 * @note 숫자가 아니라 마지막 진짜 모드 비트 위의 비트로 정의되므로, 모드 비트를 더하면 이것이 그와 함께 옮겨 간다. 위의 주석 처리된 상수가 그것이 현재 무엇에 이르는지 적으며, 그것이 바로 낡아질 것이다.
 * @warning 진짜 모드 비트와 같은 항목에 설정되므로 사적인 것이 아니다. 모드를 살펴보는 무엇이든 그것을 보며, 모드를 복사하는 코드가 그것을 함께 지고 간다.
 */
//#define TURN_OFF_LOGGING 0x20
#define TURN_OFF_LOGGING (UNCONVERTING << 1)

/**
 * @name The dispatch tables
 *
 * Four tables of routine pointers, one per format: writing and reading, each in a whole-structure and a field-by-field variety. Every routine in this file reaches its format through one of these.
 *
 * The pairs of zeroed copies beside them are not spare slots but the value used to blank an entry. Assigning a whole table of null pointers is how a format is marked unavailable, and it is done by copying one of these rather than by clearing in place.
 *
 * @note Which pair a routine uses is decided by the handle, not the format, because the same format could in principle be written either way -- the handle says how this particular file is laid out.
 * @note Sized by the number of formats, and indexed by the handle's format number. Nothing bounds that index, so the tables' size is a promise the callers must keep.
 * @{
 */
/**
 * @name 갈라 보내기 표
 *
 * 형식마다 하나씩인 네 개의 함수 포인터 표. 쓰기와 읽기, 각각이 구조체 전체와 필드 단위의 두 종류로. 이 파일의 모든 함수가 이 가운데 하나를 통해 자기 형식에 닿는다.
 *
 * 그 옆의 영으로 된 사본 짝은 남는 자리가 아니라 항목을 비우는 데 쓰이는 값이다. 널 포인터로 된 표 전체를 배정하는 것이 형식을 쓸 수 없는 것으로 표시하는 방법이며, 그것은 제자리에서 지움으로써가 아니라 이 가운데 하나를 복사함으로써 이루어진다.
 *
 * @note 함수가 어느 짝을 쓰는지는 형식이 아니라 핸들이 정한다. 원칙적으로 같은 형식이 어느 방식으로든 쓰일 수 있기 때문이다. 핸들이 이 특정한 파일이 어떻게 놓였는지 말한다.
 * @note 형식의 개수로 크기가 정해지고, 핸들의 형식 번호로 색인된다. 무엇도 그 색인을 제한하지 않으므로, 표의 크기는 호출자들이 지켜야 하는 약속이다.
 * @{
 */

struct sf_structlevel_procs sfoprocs[NUM_SAVEFORMATS], sfiprocs[NUM_SAVEFORMATS],
                zerosfoprocs = {0}, zerosfiprocs = {0};
struct sf_fieldlevel_procs sfoflprocs[NUM_SAVEFORMATS], sfiflprocs[NUM_SAVEFORMATS],
                zerosfoflprocs = {0}, zerosfiflprocs = {0};

/** @} */

char *sfvalue_aligntyp(aligntyp *a);
char *sfvalue_any(anything *a);
char *sfvalue_genericptr(genericptr a);
char *sfvalue_int16(int16 *a);
char *sfvalue_int32(int32 *a);
char *sfvalue_int64(int64 *a);
char *sfvalue_uchar(uchar *a);
char *sfvalue_uint16(uint16 *a);
char *sfvalue_uint32(uint32 *a);
char *sfvalue_uint64(uint64 *a);
char *sfvalue_size_t(size_t *a);
char *sfvalue_time_t(time_t *a);
char *sfvalue_short(short *a);
char *sfvalue_ushort(ushort *a);
char *sfvalue_int(int *a);
char *sfvalue_unsigned(unsigned *a);
char *sfvalue_long(long *a);
char *sfvalue_ulong(ulong *a);
char *sfvalue_xint8(xint8 *a);
char *sfvalue_xint16(xint16 *a);
char *sfvalue_char(char *a, int n);
char *sfvalue_boolean(boolean *a);
char *sfvalue_schar(schar *a);
char *sfvalue_bitfield(uint8 *a);
char *complex_dump(uchar *a);
char *bitfield_dump(uint8 *a);

void sf_log(NHFILE *, const char *, size_t, int, char *);

#if NH_C < 202300L
#define Sfvalue_aligntyp(a) sfvalue_aligntyp(a)
#define Sfvalue_any(a) sfvalue_any(a)
#define Sfvalue_genericptr(a) sfvalue_genericptr(a)
#define Sfvalue_coordxy(a) sfvalue_int16(a)
#define Sfvalue_int16(a) sfvalue_int16(a)
#define Sfvalue_int32(a) sfvalue_int32(a)
#define Sfvalue_int64(a) sfvalue_int64(a)
#define Sfvalue_uchar(a) sfvalue_uchar(a)
#define Sfvalue_uint16(a) sfvalue_uint16(a)
#define Sfvalue_uint32(a) sfvalue_uint32(a)
#define Sfvalue_uint64(a) sfvalue_uint64(a)
#define Sfvalue_size_t(a) sfvalue_size_t(a)
#define Sfvalue_time_t(a) sfvalue_time_t(a)
#define Sfvalue_short(a) sfvalue_short(a)
#define Sfvalue_ushort(a) sfvalue_ushort(a)
#define Sfvalue_int(a) sfvalue_int(a)
#define Sfvalue_unsigned(a) sfvalue_unsigned(a)
#define Sfvalue_xint8(a) sfvalue_xint8(a)
#define Sfvalue_xint16(a) sfvalue_xint16(a)

#else

#define sfvalue(x)                          \
    _Generic( (x),                          \
        anything *: sfvalue_any,            \
        genericptr_t *: sfvalue_genericptr, \
        int16_t *: sfvalue_int16,           \
        int32_t *: sfvalue_int32,           \
        int64_t *: sfvalue_int64,           \
        uchar *: sfvalue_uchar,             \
        uint16_t *: sfvalue_uint16,         \
        uint32_t *: sfvalue_uint32,         \
        uint64_t *: sfvalue_uint64,         \
        xint8 *: sfvalue_xint8              \
    )(x)

#define Sfvalue_any(a) sfvalue(a)
#define Sfvalue_aligntyp(a) sfvalue(a)
#define Sfvalue_genericptr(a) sfvalue(a)
#define Sfvalue_coordxy(a) sfvalue(a)
#define Sfvalue_int16(a) sfvalue(a)
#define Sfvalue_int32(a) sfvalue(a)
#define Sfvalue_int64(a) sfvalue(a)
#define Sfvalue_uchar(a) sfvalue(a)
#define Sfvalue_unsigned(a) sfvalue(a)
#define Sfvalue_uchar(a) sfvalue(a)
#define Sfvalue_uint16(a) sfvalue(a)
#define Sfvalue_uint32(a) sfvalue(a)
#define Sfvalue_uint64(a) sfvalue(a)
#define Sfvalue_short(a) sfvalue(a)
#define Sfvalue_ushort(a) sfvalue(a)
#define Sfvalue_int(a) sfvalue(a)
#define Sfvalue_unsigned(a) sfvalue(a)
#define Sfvalue_xint8(a) sfvalue(a)
#define Sfvalue_xint16(a) sfvalue(a)
#endif

/* not in _Generic */
#define Sfvalue_long(a) sfvalue_long(a)
#define Sfvalue_ulong(a) sfvalue_ulong(a)
#define Sfvalue_char(a, d) sfvalue_char(a, d)
#define Sfvalue_boolean(a) sfvalue_boolean(a)
#define Sfvalue_schar(a) sfvalue_schar(a)
#define Sfvalue_bitfield(a) sfvalue_bitfield(a)
#define Sfvalue_time_t(a) sfvalue_time_t(a)
#define Sfvalue_size_t(a) sfvalue_size_t(a)

/**
 * @name The read and write routine generators
 *
 * Three macros, each producing a matched pair of routines -- one to write a value and one to read it -- for a whole family of types. Nearly every routine in this file comes from one of them, expanded by including a header that names every type the save
 * code handles.
 *
 * Each generated routine does the same four things in the same order, and understanding that shape once explains the whole file:
 *
 * First, it chooses between two entirely separate dispatch tables. A file written whole structures at a time and a file written field by field need different routines, and the handle says which it is.
 *
 * Second, it calls through the table for the format in use. Neither the caller nor these routines know what that format does.
 *
 * Third, on reading, it may write the same value straight back out through a second handle. That is the whole conversion mechanism: reading through one format's routines while writing through another's turns one file into the other, and no
 * conversion code exists beyond this.
 *
 * Fourth, it may log the value in readable form, which is how a save file is inspected without being loaded.
 *
 * @note The three differ only in what they can express. One handles plain types whose value can be printed; one handles structures, which are logged as raw bytes because there is nothing sensible to print; one handles types needing an extra size
 *       argument.
 * @note Logging is disabled around the field-by-field call and restored afterwards. That format logs for itself, and leaving it enabled would record every value twice.
 * @note The conversion bits are cleared for the duration of that same call, so a nested traversal does not start a second conversion inside the one already running.
 * @warning The write and read halves differ in when they log: writing logs before the value goes out, reading logs after it comes in. That is necessary rather than inconsistent -- there is nothing to log before a read -- but it means the log's order
 *          reflects the traversal, not the file.
 * @warning Nothing is logged or converted once the handle reports end of file. A truncated file therefore produces a log that simply stops, with no note of why.
 * @{
 */
/**
 * @name 읽기와 쓰기 함수 생성기
 *
 * 세 매크로이며, 각각이 온전한 형 가족을 위한 짝 지어진 함수 한 벌을 낸다. 값을 쓰는 것 하나와 그것을 읽는 것 하나. 이 파일의 거의 모든 함수가 그중 하나에서 오며, 저장 코드가 다루는 모든 형을 이름 짓는 헤더를 포함함으로써 펼쳐진다.
 *
 * 생성된 각 함수가 같은 순서로 같은 네 가지를 하며, 그 모양을 한 번 이해하는 것이 파일 전체를 설명한다.
 *
 * 첫째, 그것은 완전히 따로 된 두 갈라 보내기 표 사이에서 고른다. 구조체 전체를 한 번에 쓰는 파일과 필드 하나하나 쓰는 파일은 다른 함수를 필요로 하며, 핸들이 그것이 어느 쪽인지 말한다.
 *
 * 둘째, 그것은 쓰이고 있는 형식을 위한 표를 통해 호출한다. 호출자도 이 함수들도 그 형식이 무엇을 하는지 모른다.
 *
 * 셋째, 읽을 때 그것은 같은 값을 두 번째 핸들을 통해 곧바로 되쓸 수 있다. 그것이 변환 방식 전부다. 한 형식의 함수를 통해 읽으면서 다른 것의 함수를 통해 쓰는 것이 한 파일을 다른 것으로 바꾸며, 이것 너머에 어떤 변환 코드도 존재하지 않는다.
 *
 * 넷째, 그것은 값을 읽을 수 있는 형태로 기록할 수 있으며, 그것이 저장 파일을 불러오지 않고 살펴보는 방법이다.
 *
 * @note 그 셋은 무엇을 나타낼 수 있는지에서만 다르다. 하나는 값을 인쇄할 수 있는 맨 형을 다루고, 하나는 구조체를 다루는데 그것은 인쇄할 분별 있는 것이 없으므로 날바이트로 기록되고, 하나는 여분의 크기 인자를 필요로 하는 형을 다룬다.
 * @note 필드 단위 호출 둘레에서 기록이 꺼지고 뒤에 되돌려진다. 그 형식은 스스로 기록하며, 그것을 켜 둔 채로 두면 모든 값을 두 번 적을 것이다.
 * @note 그 같은 호출 동안 변환 비트가 지워지므로, 겹친 순회가 이미 돌고 있는 변환 안에서 두 번째 변환을 시작하지 않는다.
 * @warning 쓰기와 읽기 반쪽이 언제 기록하는지에서 다르다. 쓰기는 값이 나가기 전에 기록하고, 읽기는 그것이 들어온 뒤에 기록한다. 그것은 어긋남이 아니라 필요한 것이지만. 읽기 전에 기록할 것이 없다. 그러나 그것은 기록의 순서가 파일이 아니라 순회를 비춘다는 뜻이다.
 * @warning 핸들이 파일 끝을 알린 뒤로는 무엇도 기록되거나 변환되지 않는다. 그러므로 잘린 파일은 왜인지에 대한 적바림 없이 그저 멈추는 기록을 낸다.
 * @{
 */

/**
 * @def SF_A
 * @brief Generate the pair for a plain type whose value can be shown as text.
 * @param dtyp the type
 * @note The commonest of the three. The logged form comes from a value-rendering routine chosen by the type's name, so adding a type means providing that as well.
 */
/**
 * @def SF_A
 * @brief 값을 글로 보일 수 있는 맨 형을 위한 한 벌을 생성한다.
 * @param dtyp 그 형
 * @note 셋 중 가장 흔한 것이다. 기록되는 형태가 형의 이름으로 골라진 값 그리기 함수에서 오므로, 형을 더하는 것은 그것을 내주는 일도 뜻한다.
 */
#define SF_A(dtyp) \
void sfo_##dtyp(NHFILE *nhfp, dtyp *d_##dtyp, const char *myname)               \
{                                                                               \
    if (nhfp->fplog)                                                            \
        sf_log(nhfp, myname, sizeof *d_##dtyp, 1, Sfvalue_##dtyp(d_##dtyp));    \
    if (nhfp->structlevel) {                                                    \
        (*sfoprocs[nhfp->fnidx].fn.sf_##dtyp)(nhfp, d_##dtyp, myname);          \
    } else {                                                                    \
        FILE *save_fplog = nhfp->fplog;                                         \
                                                                                \
        nhfp->fplog = 0;                                                        \
        (*sfoflprocs[nhfp->fnidx].fn_x.sf_##dtyp)(nhfp, d_##dtyp, myname);      \
        nhfp->fplog = save_fplog;                                               \
    }                                                                           \
}                                                                               \
                                                                                \
void sfi_##dtyp(NHFILE *nhfp, dtyp *d_##dtyp, const char *myname)               \
{                                                                               \
    if (nhfp->structlevel) {                                                    \
        (*sfiprocs[nhfp->fnidx].fn.sf_##dtyp)(nhfp, d_##dtyp, myname);          \
    } else {                                                                    \
        int save_mode = nhfp->mode;                                             \
                                                                                \
        nhfp->mode &= ~(CONVERTING | UNCONVERTING);                             \
        nhfp->mode |= TURN_OFF_LOGGING;                                         \
        (*sfiflprocs[nhfp->fnidx].fn_x.sf_##dtyp)(nhfp, d_##dtyp, myname);      \
        nhfp->mode = save_mode;                                                 \
    }                                                                           \
    if (!nhfp->eof) {                                                           \
        if ((((nhfp->mode & CONVERTING) != 0)                                   \
            || ((nhfp->mode & UNCONVERTING) != 0)) && nhfp->nhfpconvert)  {     \
            sfo_##dtyp(nhfp->nhfpconvert, d_##dtyp, myname);                    \
        }                                                                       \
        if (nhfp->fplog)                                                        \
            sf_log(nhfp, myname, sizeof *d_##dtyp, 1,                           \
                   Sfvalue_##dtyp(d_##dtyp));                                   \
    }                                                                           \
}

/**
 * @def SF_C
 * @brief Generate the pair for a structure or union.
 * @param keyw the keyword introducing the type -- "struct" or "union"
 * @param dtyp the type's name
 * @note The keyword is a separate argument because the type's name alone does not say which it is, and the generated declarations need both.
 * @note Logged as raw bytes rather than as a value, since a structure has no single readable form. That makes the log of a structure far less useful than the log of a plain value, which is why the field-by-field format exists at all.
 */
/**
 * @def SF_C
 * @brief 구조체나 공용체를 위한 한 벌을 생성한다.
 * @param keyw 그 형을 들이는 낱말. "struct" 또는 "union"
 * @param dtyp 그 형의 이름
 * @note 그 낱말이 따로 된 인자인 것은 형의 이름만으로는 그것이 어느 쪽인지 말하지 않고, 생성되는 선언이 둘 다를 필요로 하기 때문이다.
 * @note 값이 아니라 날바이트로 기록된다. 구조체는 하나의 읽을 수 있는 형태를 갖지 않기 때문이다. 그것이 구조체의 기록을 맨 값의 기록보다 훨씬 덜 쓸모 있게 만들며, 그것이 필드 단위 형식이 애초에 존재하는 이유다.
 */
#define SF_C(keyw, dtyp) \
void sfo_##dtyp(NHFILE *nhfp, keyw dtyp *d_##dtyp, const char *myname)          \
{                                                                               \
    if (nhfp->fplog)                                                            \
        sf_log(nhfp, myname, sizeof *d_##dtyp, 1,                               \
               complex_dump((uchar *) d_##dtyp));                               \
    if (nhfp->structlevel) {                                                    \
        (*sfoprocs[nhfp->fnidx].fn.sf_##dtyp)(nhfp, d_##dtyp, myname);          \
    } else {                                                                    \
        FILE *save_fplog = nhfp->fplog;                                         \
                                                                                \
        nhfp->fplog = 0;                                                        \
        (*sfoflprocs[nhfp->fnidx].fn_x.sf_##dtyp)(nhfp, d_##dtyp, myname);      \
        nhfp->fplog = save_fplog;                                               \
    }                                                                           \
}                                                                               \
                                                                                \
void sfi_##dtyp(NHFILE *nhfp, keyw dtyp *d_##dtyp, const char *myname)          \
{                                                                               \
    if (nhfp->structlevel) {                                                    \
        (*sfiprocs[nhfp->fnidx].fn.sf_##dtyp)(nhfp, d_##dtyp, myname);          \
    } else {                                                                    \
        int save_mode = nhfp->mode;                                             \
                                                                                \
        nhfp->mode &= ~(CONVERTING | UNCONVERTING);                             \
        nhfp->mode |= TURN_OFF_LOGGING;                                         \
        (*sfiflprocs[nhfp->fnidx].fn_x.sf_##dtyp)(nhfp, d_##dtyp, myname);      \
        nhfp->mode = save_mode;                                                 \
    }                                                                           \
    if (!nhfp->eof) {                                                           \
        if ((((nhfp->mode & CONVERTING) != 0)                                   \
            || ((nhfp->mode & UNCONVERTING) != 0))                              \
            && nhfp->nhfpconvert) {                                             \
            sfo_##dtyp(nhfp->nhfpconvert, d_##dtyp, myname);                    \
        }                                                                       \
        if (nhfp->fplog)                                                        \
            sf_log(nhfp, myname, sizeof *d_##dtyp, 1,                           \
                       complex_dump((uchar *) d_##dtyp));                       \
    }                                                                           \
}

/**
 * @def SF_X
 * @brief Generate the pair for a type needing a size passed alongside it.
 * @param xxx the underlying type the value is stored in
 * @param dtyp the name the routines are known by
 * @note Two names rather than one, because what the value is stored in and what it is called are different here. Bit fields are the case it was written for: they are carried in a byte but their width must be passed, since the storage does not record it.
 * @warning The write half logs twice -- once before dispatching and once after -- unlike the other two, which log once. The second is conditional on not having reached end of file, so a normal write records every such value twice in the log.
 */
/**
 * @def SF_X
 * @brief 크기가 함께 넘겨져야 하는 형을 위한 한 벌을 생성한다.
 * @param xxx 값이 저장되는 밑의 형
 * @param dtyp 그 함수들이 불리는 이름
 * @note 하나가 아니라 두 이름인 것은, 값이 무엇에 저장되는지와 그것이 무엇으로 불리는지가 여기서 다르기 때문이다. 비트 필드가 이것이 쓰인 경우다. 그것들은 한 바이트에 담기지만 그 너비가 넘겨져야 한다. 저장이 그것을 기록하지 않기 때문이다.
 * @warning 한 번 기록하는 다른 둘과 달리 쓰기 반쪽이 두 번 기록한다. 갈라 보내기 전에 한 번, 뒤에 한 번. 두 번째는 파일 끝에 이르지 않았을 것을 조건으로 하므로, 보통의 쓰기가 그런 값마다 기록에 두 번 적는다.
 */
#define SF_X(xxx, dtyp) \
void sfo_##dtyp(NHFILE *nhfp, xxx *d_##dtyp, const char *myname, int bfsz)      \
{                                                                               \
    if (nhfp->fplog)                                                            \
            sf_log(nhfp, myname, sizeof *d_##dtyp, 1,                           \
                   Sfvalue_##dtyp(d_##dtyp));                                   \
    if (nhfp->structlevel) {                                                    \
        (*sfoprocs[nhfp->fnidx].fn.sf_##dtyp)(nhfp, d_##dtyp, myname, bfsz);    \
    } else {                                                                    \
        FILE *save_fplog = nhfp->fplog;                                         \
                                                                                \
        nhfp->fplog = 0;                                                        \
        (*sfoflprocs[nhfp->fnidx].fn_x.sf_##dtyp)(nhfp, d_##dtyp,               \
                                                  myname, bfsz);                \
        nhfp->fplog = save_fplog;                                               \
    }                                                                           \
    if (nhfp->fplog && !nhfp->eof)                                              \
        sf_log(nhfp, myname, sizeof *d_##dtyp, 1, Sfvalue_##dtyp(d_##dtyp));    \
}                                                                               \
                                                                                \
void sfi_##dtyp(NHFILE *nhfp, xxx *d_##dtyp, const char *myname, int bfsz)      \
{                                                                               \
    if (nhfp->structlevel) {                                                    \
        (*sfiprocs[nhfp->fnidx].fn.sf_##dtyp)(nhfp, d_##dtyp, myname, bfsz);    \
    } else {                                                                    \
        int save_mode = nhfp->mode;                                             \
                                                                                \
        nhfp->mode &= ~(CONVERTING | UNCONVERTING);                             \
        nhfp->mode |= TURN_OFF_LOGGING;                                         \
        (*sfiflprocs[nhfp->fnidx].fn_x.sf_##dtyp)(nhfp, d_##dtyp,               \
                                                  myname, bfsz);                \
        nhfp->mode = save_mode;                                                 \
    }                                                                           \
    if (!nhfp->eof) {                                                           \
        if ((((nhfp->mode & CONVERTING) != 0)                                   \
            || ((nhfp->mode & UNCONVERTING) != 0))                              \
                       && nhfp->nhfpconvert) {                                  \
            sfo_##dtyp(nhfp->nhfpconvert, d_##dtyp, myname, bfsz);              \
        }                                                                       \
        if (nhfp->fplog)                                                        \
            sf_log(nhfp, myname, sizeof *d_##dtyp, 1,                           \
                   dtyp##_dump(d_##dtyp));                                      \
    }                                                                           \
}

/** @} */

/**
 * @brief Expand the generators over every type the save code handles.
 * @note This one line is where most of this file's contents come from. The header names each type and which generator applies to it, so a new saved type is added there rather than here.
 */
/**
 * @brief 저장 코드가 다루는 모든 형에 걸쳐 생성기를 펼친다.
 * @note 이 한 줄이 이 파일 내용 대부분이 오는 곳이다. 그 헤더가 각 형과 그것에 어느 생성기가 적용되는지를 이름 짓므로, 새로 저장되는 형은 여기가 아니라 거기에 더해진다.
 */
#include "sfmacros.h"

/** Bit fields, expanded separately because the header does not name them. */
/** 비트 필드. 헤더가 그것을 이름 짓지 않으므로 따로 펼쳐진다. */
SF_X(uint8_t, bitfield)

/**
 * @brief Write a run of characters.
 *
 * Written out rather than generated because it takes a count, and the count means a length of text rather than a number of separate values -- which is why the logged form is the whole run as a string rather than one entry per character.
 *
 * @param nhfp the file handle
 * @param d_char the characters
 * @param myname the field's name, for logging and for the field-by-field format
 * @param cnt how many characters
 */
/**
 * @brief 이어진 문자들을 쓴다.
 *
 * 개수를 취하기 때문에 생성되는 대신 풀어 쓰여 있으며, 그 개수가 따로 된 값의 수가 아니라 글의 길이를 뜻한다. 그것이 기록되는 형태가 문자마다 한 항목이 아니라 이어진 전체가 하나의 문자열인 이유다.
 *
 * @param nhfp 파일 핸들
 * @param d_char 그 문자들
 * @param myname 그 필드의 이름. 기록과 필드 단위 형식을 위해
 * @param cnt 몇 문자인지
 */
void
sfo_char(NHFILE *nhfp, char *d_char, const char *myname, int cnt)
{
    if (nhfp->fplog)
        sf_log(nhfp, myname, sizeof(char), cnt, Sfvalue_char(d_char, cnt));
    if (nhfp->structlevel) {
        (*sfoprocs[nhfp->fnidx].fn.sf_char)(nhfp, d_char, myname, cnt);
    } else {
        FILE *save_fplog = nhfp->fplog;

        nhfp->fplog = 0;
        (*sfoflprocs[nhfp->fnidx].fn_x.sf_char)(nhfp, d_char, myname, cnt);
        nhfp->fplog = save_fplog;
    }
}

/**
 * @brief Read a run of characters.
 * @param nhfp the file handle
 * @param d_char where to put them
 * @param myname the field's name
 * @param cnt how many characters
 * @warning The count is the caller's assertion, not something read from the file. A count larger than the buffer overruns it, and the file cannot contradict it.
 */
/**
 * @brief 이어진 문자들을 읽는다.
 * @param nhfp 파일 핸들
 * @param d_char 그것을 둘 곳
 * @param myname 그 필드의 이름
 * @param cnt 몇 문자인지
 * @warning 그 개수는 파일에서 읽힌 것이 아니라 호출자의 주장이다. 버퍼보다 큰 개수는 그것을 넘쳐 흐르며, 파일이 그것을 반박할 수 없다.
 */
void
sfi_char(NHFILE *nhfp, char *d_char, const char *myname, int cnt)
{
    if (nhfp->structlevel) {
        (*sfiprocs[nhfp->fnidx].fn.sf_char)(nhfp, d_char, myname, cnt);
    } else {
        int save_mode = nhfp->mode;

        nhfp->mode &= ~(CONVERTING | UNCONVERTING);
        nhfp->mode |= TURN_OFF_LOGGING;
        (*sfiflprocs[nhfp->fnidx].fn_x.sf_char)(nhfp, d_char, myname, cnt);
        nhfp->mode = save_mode;
    }
    if (!nhfp->eof) {
        if ((((nhfp->mode & CONVERTING) != 0)
             || ((nhfp->mode & UNCONVERTING) != 0))
            && nhfp->nhfpconvert) {
            sfo_char(nhfp->nhfpconvert, d_char, myname, cnt);
        }
        if (nhfp->fplog)
            sf_log(nhfp, myname, sizeof(char), cnt,
                   Sfvalue_char(d_char, cnt));
    }
}

/**
 * @name Untyped pointers
 *
 * Written out rather than generated because a pointer's declaration cannot be produced by the same substitution as a value's -- the generated form would give a pointer to the type, and here the type is already a pointer.
 *
 * @warning What is stored is the pointer, not what it points at. A restored pointer is meaningless as an address, so whatever reads it must replace it -- the value exists only so that the field's presence and whether it was set survive the file.
 * @{
 */

/**
 * @brief Write an untyped pointer.
 * @param nhfp the file handle
 * @param d_genericptr the pointer
 * @param myname the field's name
 */
/**
 * @brief 형 없는 포인터를 쓴다.
 * @param nhfp 파일 핸들
 * @param d_genericptr 그 포인터
 * @param myname 그 필드의 이름
 */
void
sfo_genericptr(NHFILE *nhfp, void **d_genericptr, const char *myname)
{
    if (nhfp->fplog)
        sf_log(nhfp, myname, sizeof *d_genericptr, 1,
               Sfvalue_genericptr(d_genericptr));
    if (nhfp->structlevel) {
        (*sfoprocs[nhfp->fnidx].fn.sf_genericptr)(nhfp, d_genericptr, myname);
    } else {
        FILE *save_fplog = nhfp->fplog;
        nhfp->fplog = 0;
        (*sfoflprocs[nhfp->fnidx].fn_x.sf_genericptr)(nhfp, d_genericptr,
                                                    myname);
        nhfp->fplog = save_fplog;
    }
}
/**
 * @brief Read an untyped pointer.
 * @param nhfp the file handle
 * @param d_genericptr where to put it
 * @param myname the field's name
 * @warning The value read is an address from another run of the program and must not be followed. The routines named for normalising pointers are what clear such fields before a structure is written; on reading, the caller is responsible.
 */
/**
 * @brief 형 없는 포인터를 읽는다.
 * @param nhfp 파일 핸들
 * @param d_genericptr 그것을 둘 곳
 * @param myname 그 필드의 이름
 * @warning 읽힌 값은 프로그램의 다른 실행에서 온 주소이며 따라가서는 안 된다. 포인터를 표준화하는 이름의 함수들이 구조체가 쓰이기 전에 그런 필드를 지우는 것이다. 읽을 때는 호출자가 책임을 진다.
 */
void
sfi_genericptr(NHFILE *nhfp, void **d_genericptr, const char *myname)
{
    if (nhfp->structlevel) {
        (*sfiprocs[nhfp->fnidx].fn.sf_genericptr)(nhfp, d_genericptr, myname);
    } else {
        int save_mode = nhfp->mode;
        nhfp->mode &= ~(CONVERTING | UNCONVERTING);
        nhfp->mode |= TURN_OFF_LOGGING;
        (*sfiflprocs[nhfp->fnidx].fn_x.sf_genericptr)(nhfp, d_genericptr,
                                                    myname);
        nhfp->mode = save_mode;
    }
    if (!nhfp->eof) {
        if ((((nhfp->mode & CONVERTING) != 0) || ((nhfp->mode & UNCONVERTING) != 0))
            && nhfp->nhfpconvert) {
            sfo_genericptr(nhfp->nhfpconvert, d_genericptr, myname);
        }
        if (nhfp->fplog)
            sf_log(nhfp, myname, sizeof *d_genericptr, 1,
                   Sfvalue_genericptr(d_genericptr));
    }
}

/** @} */

/**
 * @name The version stamp
 *
 * Written out rather than generated because reading it does something no other field does: it marks the version as having passed through the conversion tool.
 * @{
 */

/**
 * @brief Write the version stamp.
 * @param nhfp the file handle
 * @param d_version_info the version numbers
 * @param myname the field's name
 */
/**
 * @brief 판본 표식을 쓴다.
 * @param nhfp 파일 핸들
 * @param d_version_info 그 판본 숫자들
 * @param myname 그 필드의 이름
 */
void
sfo_version_info(NHFILE *nhfp, struct version_info *d_version_info,
                 const char *myname)
{
    if (nhfp->fplog)
        sf_log(nhfp, myname, sizeof *d_version_info, 1,
               complex_dump((uchar *) d_version_info));
    if (nhfp->structlevel) {
        (*sfoprocs[nhfp->fnidx].fn.sf_version_info)(nhfp, d_version_info,
                                                    myname);
    } else {
        FILE *save_fplog = nhfp->fplog;
        nhfp->fplog = 0;
        (*sfoflprocs[nhfp->fnidx].fn_x.sf_version_info)(nhfp, d_version_info,
                                                      myname);
        nhfp->fplog = save_fplog;
    }
}
/**
 * @brief Read the version stamp, marking it when converting.
 *
 * The one place the conversion leaves a trace in what it writes. When a file is being converted, a bit is set in the feature set before the stamp goes out, saying the file came from the tool rather than from a game.
 *
 * That bit is why the game must disregard it when deciding whether a file is compatible: a converted file is otherwise identical to a native one, and refusing it over a mark the tool itself added would make conversion useless. The routine that
 * lists disregarded options is the other half of this arrangement.
 *
 * @param nhfp the file handle
 * @param d_version_info where to put the version numbers
 * @param myname the field's name
 * @warning The mark is written into the caller's own copy of the version, not into a temporary. So after a conversion the version in memory carries the bit too, and anything reading it afterwards sees a version that claims to have come from the tool.
 */
/**
 * @brief 판본 표식을 읽으며, 변환할 때 그것을 표시한다.
 *
 * 변환이 자기가 쓰는 것에 자취를 남기는 단 하나의 자리다. 파일이 변환되고 있을 때, 표식이 나가기 전에 기능 묶음에 비트 하나가 설정되어 그 파일이 게임이 아니라 도구에서 왔다고 말한다.
 *
 * 그 비트가 게임이 파일이 호환되는지 정할 때 그것을 무시해야 하는 이유다. 변환된 파일은 그것 말고는 원래의 것과 똑같으며, 도구 자신이 더한 표시 때문에 그것을 거부하는 것은 변환을 쓸모없게 만들 것이다. 무시되는 선택지를 나열하는 함수가 이 배치의 다른 반쪽이다.
 *
 * @param nhfp 파일 핸들
 * @param d_version_info 판본 숫자를 둘 곳
 * @param myname 그 필드의 이름
 * @warning 그 표시가 임시가 아니라 호출자 자신의 판본 사본에 쓰인다. 그래서 변환 뒤에 메모리의 판본도 그 비트를 지니며, 그 뒤에 그것을 읽는 무엇이든 도구에서 왔다고 주장하는 판본을 본다.
 */
void
sfi_version_info(NHFILE *nhfp, struct version_info *d_version_info,
                 const char *myname)
{
    if (nhfp->structlevel) {
        (*sfiprocs[nhfp->fnidx].fn.sf_version_info)(nhfp, d_version_info,
                                                    myname);
    } else {
        int save_mode = nhfp->mode;
        nhfp->mode &= ~(CONVERTING | UNCONVERTING);
        nhfp->mode |= TURN_OFF_LOGGING;
        (*sfiflprocs[nhfp->fnidx].fn_x.sf_version_info)(nhfp, d_version_info,
                                                      myname);
        nhfp->mode = save_mode;
    }
    if (!nhfp->eof) {
        if ((((nhfp->mode & CONVERTING) != 0) || ((nhfp->mode & UNCONVERTING) != 0))
            && nhfp->nhfpconvert) {
            d_version_info->feature_set |= SFCTOOL_BIT;
            sfo_version_info(nhfp->nhfpconvert, d_version_info, myname);
        }
        if (nhfp->fplog)
            sf_log(nhfp, myname, sizeof *d_version_info, 1,
                   complex_dump((uchar *) d_version_info));
    }
}

/** @} */

/* ---------------------------------------------------------------*/

/**
 * @brief Record one field in the readable log.
 *
 * The log exists so that a save file can be examined without being loaded, and compared against another file field by field. Each line carries the running byte position, the field's name, its size and count, and its value -- which is enough to find
 * where two files first diverge.
 *
 * @param nhfp the file handle
 * @param t1 the field's name
 * @param sz the size of one value
 * @param cnt how many values
 * @param txtvalue the value rendered as text
 * @note Reads the position from whichever counter matches the direction, so the position logged is a position in the file being read or written rather than a count of log lines.
 * @note The size is printed with a different conversion on one system, because the type used for sizes is not printable the same way everywhere. That is a portability accommodation rather than a difference in what is logged.
 * @note Logging can be suppressed by a mode bit as well as by there being no log file. That bit is what the generated routines set around a nested call, so a nested traversal does not log what the outer one already did.
 * @note There is disabled code that counted lines and stopped the debugger at a particular one. It records how this log is actually used -- run to the point where two files diverge, then break there.
 */
/**
 * @brief 읽을 수 있는 기록에 필드 하나를 적는다.
 *
 * 그 기록은 저장 파일이 불러와지지 않고 살펴질 수 있도록, 그리고 다른 파일과 필드 하나하나 비교될 수 있도록 존재한다. 각 줄이 진행 중인 바이트 위치, 필드의 이름, 그 크기와 개수, 그리고 그 값을 지닌다. 그것이 두 파일이 처음 갈라지는 곳을 찾기에 충분하다.
 *
 * @param nhfp 파일 핸들
 * @param t1 그 필드의 이름
 * @param sz 값 하나의 크기
 * @param cnt 몇 개의 값인지
 * @param txtvalue 글로 그려진 그 값
 * @note 방향에 맞는 계수기에서 위치를 읽으므로, 기록되는 위치는 기록 줄의 개수가 아니라 읽히거나 쓰이고 있는 파일 안의 위치다.
 * @note 크기가 한 시스템에서 다른 변환으로 인쇄되는데, 크기에 쓰이는 형이 어디서나 같은 방식으로 인쇄할 수 있는 것이 아니기 때문이다. 그것은 기록되는 것의 차이가 아니라 이식성을 위한 맞춤이다.
 * @note 기록은 기록 파일이 없음으로써뿐 아니라 모드 비트로도 억제될 수 있다. 그 비트가 생성된 함수들이 겹친 호출 둘레에서 설정하는 것이며, 그래서 겹친 순회가 바깥 것이 이미 한 것을 기록하지 않는다.
 * @note 줄을 세고 특정한 줄에서 디버거를 멈추던 비활성화된 코드가 있다. 그것은 이 기록이 실제로 어떻게 쓰이는지 적는다. 두 파일이 갈라지는 지점까지 실행하고, 그다음 거기서 멈춘다.
 */
void
sf_log(NHFILE *nhfp, const char *t1, size_t sz, int cnt, char *txtvalue)
{
    FILE *fp = nhfp->fplog;
    long *iocount;
    boolean dolog = ((nhfp->mode & TURN_OFF_LOGGING) == 0);

    if (fp && dolog) {
        iocount = ((nhfp->mode & WRITING) == 0) ? &nhfp->rcount : &nhfp->wcount;
        (void) fprintf(fp,
#ifndef VMS
                       "%08ld %s sz=%zu cnt=%d |%s|\n",
#else
                       "%08ld %s sz=%lu cnt=%d |%s|\n",
#endif
                       *iocount,
                       t1,
#ifndef VMS
                       sz,
#else
                       (unsigned long) sz,
#endif
                       cnt, txtvalue);
//        (*iocount)++;
//        if (*iocount == 87)
//            __debugbreak();
        fflush(fp);
    }
}

/**
 * @name Rendering values for the log
 *
 * One routine per type, each producing a readable form of a value for the log. They exist only for logging -- nothing here affects what goes into a save file.
 *
 * @warning Each keeps its own single buffer, overwritten on the next call to that same routine. That is safe only because every caller passes the result straight to the logging routine and does not hold it. Two values of the same type rendered into one
 *          log line would show the second twice.
 * @note Most are one formatting call and differ only in the conversion used, which must match the type's actual width rather than the width it usually has -- printing a wide value with a narrow conversion reads whatever follows it.
 * @{
 */

/**
 * @brief Render a run of characters as text.
 * @param a the characters
 * @param n how many
 * @return the text
 * @note Copies rather than printing, because the characters need not be a terminated string -- a fixed-width name field is padded, not terminated.
 * @note Truncates to the buffer, so a run longer than the buffer is logged short. The log is for inspection, so a shortened value is acceptable where an overrun would not be.
 * @warning Writes the terminator twice, once at the requested length before copying and once after. The first is the truncation guard and the second is the ordinary ending; they coincide only when the run fits exactly.
 */
/**
 * @brief 이어진 문자들을 글로 그린다.
 * @param a 그 문자들
 * @param n 몇 개인지
 * @return 그 글
 * @note 인쇄하는 대신 복사한다. 그 문자들이 종료된 문자열이어야 하지 않기 때문이다. 고정 너비 이름 필드는 종료되는 것이 아니라 채워진다.
 * @note 버퍼에 맞게 자르므로, 버퍼보다 긴 것은 짧게 기록된다. 기록은 살펴보기 위한 것이므로, 넘쳐 흐름은 받아들일 수 없어도 짧아진 값은 받아들일 수 있다.
 * @warning 종료 문자를 두 번 쓴다. 복사하기 전에 요청된 길이에 한 번, 뒤에 한 번. 첫 번째는 자르기 방벽이고 두 번째는 보통의 끝이다. 그것들은 이어진 것이 정확히 들어맞을 때만 겹친다.
 */
char *sfvalue_char(char *a, int n)
{
    int i;
    static char buf[120];
    char *cp;

    cp = &buf[0];
    if (n < (int) (sizeof buf - 1))
        buf[n] = '\0';
    else
        buf[(int) (sizeof buf - 1)] = '\0';
    for (i = 0; i < n; ++i, ++cp, ++a)
        *cp = *a;
    *cp = '\0';
    return buf;
}

/**
 * @brief Render a truth value as a word.
 * @param a the value
 * @return "true" or "false"
 * @note Words rather than digits, because a log read by a person should not require knowing which way round the numbers go.
 * @warning Any non-zero value reads as true, so a truth field holding some other number is logged as though it held one. That matters: such a field is a sign of corruption, and the log conceals it.
 */
/**
 * @brief 참거짓 값을 낱말로 그린다.
 * @param a 그 값
 * @return "true" 또는 "false"
 * @note 숫자가 아니라 낱말인 것은, 사람이 읽는 기록이 그 숫자가 어느 쪽인지 아는 것을 요구해서는 안 되기 때문이다.
 * @warning 영이 아닌 어떤 값이든 참으로 읽히므로, 다른 수를 담은 참거짓 필드가 일을 담은 것처럼 기록된다. 그것이 중요하다. 그런 필드는 손상의 표시이며, 기록이 그것을 감춘다.
 */
char *sfvalue_boolean(boolean *a)
{
    static char buf[20];

    Snprintf(buf, sizeof buf, "%s",
             (*a == 0) ? "false" : "true");
    return buf;
}

char *sfvalue_schar(schar *a)
{
    static char buf[20];

    Snprintf(buf, sizeof buf, "%d", (int) *a);
    return buf;
}

char * sfvalue_aligntyp(aligntyp *a)
{
    static char buf[20];

    Snprintf(buf, sizeof buf, "%d", (int) *a);
    return buf;
}

/**
 * @brief Render the general-purpose value union.
 * @param a the value
 * @return the text
 * @warning Prints the union's widest integer member regardless of which member actually holds the value. That is the only choice that shows every bit, but it means the logged number is meaningless as a quantity when the union holds something narrower --
 *          it is useful for comparing two files, not for reading a value.
 */
/**
 * @brief 여러 쓰임의 값 공용체를 그린다.
 * @param a 그 값
 * @return 그 글
 * @warning 실제로 어느 구성원이 값을 담고 있는지와 무관하게 공용체의 가장 넓은 정수 구성원을 인쇄한다. 그것이 모든 비트를 보이는 유일한 선택이지만, 공용체가 더 좁은 것을 담을 때 기록되는 수가 양으로서는 뜻이 없다는 뜻이다. 그것은 두 파일을 비교하는 데 쓸모 있고, 값을 읽는 데는 아니다.
 */
char *
sfvalue_any(anything *a)
{
    static char buf[20];

    Snprintf(buf, sizeof buf,
             "%" PRId64,
             a->a_int64);
    return buf;
}

/**
 * @brief Render a pointer as whether it was set, not as its value.
 * @param a the pointer
 * @return "0" for a null pointer, a fixed nonsense word for any other
 * @note Deliberately does not print the address. An address differs between runs, so logging it would make two logs of the same game differ everywhere a pointer appears -- and comparing two logs is what the log is for.
 * @note The nonsense word is nonsense on purpose: it cannot be mistaken for a value, so a reader is not tempted to interpret it.
 */
/**
 * @brief 포인터를 그 값이 아니라 설정되었는지로 그린다.
 * @param a 그 포인터
 * @return 널 포인터에는 "0", 그 밖의 어떤 것에든 고정된 뜻 없는 낱말
 * @note 의도적으로 주소를 인쇄하지 않는다. 주소는 실행 사이에서 다르므로, 그것을 기록하는 것은 같은 게임의 두 기록이 포인터가 나타나는 모든 곳에서 다르게 만들 것이다. 그리고 두 기록을 비교하는 것이 기록의 목적이다.
 * @note 그 뜻 없는 낱말은 일부러 뜻이 없다. 그것은 값으로 오인될 수 없으므로, 읽는 사람이 그것을 해석하려 들지 않는다.
 */
char *
sfvalue_genericptr(genericptr a)
{
    static char buf[20];

    Snprintf(buf, sizeof buf, "%s",
             (a == 0) ? "0" : "glorkum");
    return buf;
}

char * sfvalue_int16(int16 *a)
{
    static char buf[20];

    Snprintf(buf, sizeof buf, "%d", *a);
    return buf;
}

char * sfvalue_int32(int32 *a)
{
    static char buf[20];

    Snprintf(buf, sizeof buf, "%" PRId32, *a);
    return buf;
}

char * sfvalue_int64(int64 *a)
{
    static char buf[20];
    Snprintf(buf, sizeof buf, "%" PRId64, *a);
    return buf;
}

char * sfvalue_uchar(uchar *a)
{
    static char buf[20];
    unsigned x;

    x = *a;
    Snprintf(buf, sizeof buf, "%03u", x);
    return buf;
}

char * sfvalue_uint16(uint16 *a)
{
    static char buf[20];

    Snprintf(buf, sizeof buf, "%u", (uint) *a);
    return buf;
}

char * sfvalue_uint32(uint32 *a)
{
    static char buf[20];

    Snprintf(buf, sizeof buf, "%" PRIu32, *a);
    return buf;
}

char * sfvalue_uint64(uint64 *a)
{
    static char buf[20];

    Snprintf(buf, sizeof buf, "%" PRIu64, *a);
    return buf;
}

char * sfvalue_size_t(size_t *a UNUSED)
{
    static char buf[20];

    Snprintf(buf, sizeof buf, "%s", (char *) "");
    return buf;
}

/**
 * @brief Render a time value -- as nothing.
 * @param a the time, deliberately unused
 * @return an empty string
 * @warning Logs nothing at all for a time field, so the field's name appears in the log with no value. That is presumably because a time cannot be rendered portably by a single conversion, but the effect is that two files differing only in their
 *          timestamps produce identical logs -- and the log is used to find where two files differ.
 */
/**
 * @brief 시간 값을 그린다. 아무것도 아닌 것으로.
 * @param a 그 시간. 의도적으로 쓰이지 않는다
 * @return 빈 문자열
 * @warning 시간 필드에 대해 아무것도 기록하지 않으므로, 그 필드의 이름이 값 없이 기록에 나타난다. 그것은 아마 시간이 하나의 변환으로 이식성 있게 그려질 수 없기 때문이겠지만, 그 결과는 시간표만 다른 두 파일이 똑같은 기록을 낸다는 것이다. 그리고 그 기록은 두 파일이 어디서 다른지 찾는 데 쓰인다.
 */
char * sfvalue_time_t(time_t *a UNUSED)
{
    static char buf[20];

    Snprintf(buf, sizeof buf, "%s", (char *) "");
    return buf;
}

char * sfvalue_short(short *a)
{
    static char buf[20];

    Snprintf(buf, sizeof buf, "%d", (int) *a);
    return buf;
}

char * sfvalue_ushort(ushort *a)
{
    static char buf[20];

    Snprintf(buf, sizeof buf, "%u", (unsigned) *a);
    return buf;
}

char * sfvalue_int(int *a)
{
    static char buf[20];

    Snprintf(buf, sizeof buf, "%d", *a);
    return buf;
}

char * sfvalue_unsigned(unsigned *a)
{
    static char buf[20];

    Snprintf(buf, sizeof buf, "%u", *a);
    return buf;
}

char * sfvalue_long(long *a)
{
    static char buf[20];

    Snprintf(buf, sizeof buf, "%ld", *a);
    return buf;
}

char * sfvalue_ulong(ulong *a)
{
    static char buf[20];

    Snprintf(buf, sizeof buf, "%lu", *a);
    return buf;
}

char * sfvalue_xint8(xint8 *a)
{
    static char buf[20];

    Snprintf(buf, sizeof buf, "%d", (int) *a);
    return buf;
}

char * sfvalue_xint16(xint16 *a)
{
    static char buf[20];


    Snprintf(buf, sizeof buf, "%d", (int) *a);
    return buf;
}

char *
sfvalue_bitfield(uint8 *a)
{
    static char buf[20];

    Snprintf(buf, sizeof buf, "%u", (uint) *a);
    return buf;
}

/** @} */

/**
 * @name Dumping raw storage
 * @{
 */

/**
 * @brief Render a bit field's storage byte.
 * @param a the byte
 * @return the text
 * @note Identical to the value-rendering routine for the same type. It exists separately because one of the generators names its dumping routine after the type, so the name must exist even where the two would be the same.
 */
/**
 * @brief 비트 필드의 저장 바이트를 그린다.
 * @param a 그 바이트
 * @return 그 글
 * @note 같은 형을 위한 값 그리기 함수와 똑같다. 따로 존재하는 것은, 생성기 중 하나가 자기 출력 함수를 형의 이름에 따라 이름 짓기 때문이며, 그래서 그 둘이 같을 곳에서도 그 이름이 존재해야 한다.
 */
char *
bitfield_dump(uint8 *a)
{
    static char buf[20];

    Snprintf(buf, sizeof buf, "%u", (uint) *a);
    return buf;
}

/**
 * @brief Render the beginning of a structure's raw bytes.
 *
 * What a structure looks like in the log. There is no readable form for a structure as a whole, so its first bytes are shown in hexadecimal -- enough to tell whether two files hold the same structure without pretending to interpret it.
 *
 * @param a the structure's bytes
 * @return the text
 * @warning Always reads ten bytes, whatever the structure's actual size. A structure smaller than that is read past its end, and what the extra bytes show is whatever happens to follow it in memory. So the log can differ between two runs for a structure
 *          that is genuinely identical.
 * @note Shows only the beginning, so two structures differing only in a later field log identically. The field-by-field format is the answer to that, not a longer dump here.
 */
/**
 * @brief 구조체의 날바이트 앞부분을 그린다.
 *
 * 기록에서 구조체가 어떻게 보이는지다. 구조체 전체를 위한 읽을 수 있는 형태가 없으므로, 그 첫 바이트들이 십육진수로 보인다. 그것을 해석하는 척하지 않으면서 두 파일이 같은 구조체를 담는지 말할 만큼이다.
 *
 * @param a 그 구조체의 바이트들
 * @return 그 글
 * @warning 구조체의 실제 크기가 무엇이든 언제나 열 바이트를 읽는다. 그보다 작은 구조체는 그 끝을 넘어 읽히며, 그 여분 바이트가 보이는 것은 마침 메모리에서 그것을 뒤따르는 무엇이든이다. 그래서 진짜로 똑같은 구조체에 대해 기록이 두 실행 사이에서 다를 수 있다.
 * @note 앞부분만 보이므로, 뒤쪽 필드만 다른 두 구조체가 똑같이 기록된다. 필드 단위 형식이 그것에 대한 답이며, 여기서 더 길게 출력하는 것이 아니다.
 */
char *
complex_dump(uchar *a)
{
    int i;
    uchar *uc = a;
    static char buf[50];
    unsigned x[10];

    for (i = 0; i < SIZE(x); ++i) {
        x[i] = *uc++;
    }
    Snprintf(buf, sizeof buf,
             "%03x %03x %03x %03x %03x %03x %03x %03x %03x %03x",
             x[0], x[1], x[2], x[3], x[4], x[5], x[6], x[7], x[8], x[9]);
    buf[40] = '\0';
    return buf;
}
/*
 *----------------------------------------------------------------------------
 * initialize the function pointers. These are called from initoptions_init().
 *----------------------------------------------------------------------------
 */

/** @} */

/**
 * @brief Install the routine tables for the formats that are built in.
 *
 * Run during option initialisation, as the existing comment records, which is early enough that nothing has tried to read or write a file yet.
 *
 * @note The invalid entry is filled with zeroes deliberately, not left uninitialised. A traversal that reached it would call through a null pointer and stop immediately, which is a better failure than calling through whatever the memory held.
 * @note The field-by-field format's tables are also zeroed here rather than filled. That format installs its own routines separately, so this only guarantees they are safely empty until it does.
 * @note The historical format is the only one installed with real routines here, because it is the only one compiled into the game itself.
 */
/**
 * @brief 빌드에 들어 있는 형식들을 위한 함수 표를 설치한다.
 *
 * 기존 주석이 기록하듯 옵션 초기화 중에 실행되며, 그것은 아직 무엇도 파일을 읽거나 쓰려 하지 않았을 만큼 이르다.
 *
 * @note 유효하지 않은 항목이 초기화되지 않은 채로 남겨지는 대신 의도적으로 영으로 채워진다. 그것에 이른 순회는 널 포인터를 통해 호출하고 곧바로 멈출 것이며, 그것은 메모리가 담고 있던 무엇이든을 통해 호출하는 것보다 나은 실패다.
 * @note 필드 단위 형식의 표도 채워지는 대신 여기서 영으로 된다. 그 형식은 자기 함수를 따로 설치하므로, 이것은 그것이 그럴 때까지 그것들이 안전하게 비어 있음만 보장한다.
 * @note 역사적 형식이 여기서 실제 함수와 함께 설치되는 유일한 것인데, 그것이 게임 자신에 컴파일되어 들어가는 유일한 것이기 때문이다.
 */
void
sf_init(void)
{
    sfoprocs[invalid] = zerosfoprocs;
    sfiprocs[invalid] = zerosfiprocs;
    sfoprocs[historical] = historical_sfo_procs;
    sfiprocs[historical] = historical_sfi_procs;
    sfoflprocs[exportascii] = zerosfoflprocs;
    sfiflprocs[exportascii] = zerosfiflprocs;
}

/**
 * @brief Install a format's whole-structure routines.
 * @param idx which format
 * @param sfi the reading routines
 * @param sfo the writing routines
 * @note Exists so a format can be added without this file knowing about it: the conversion tool installs its own tables at startup. That is what makes the tool able to write a format the game itself cannot.
 * @note The tables are copied, not pointed at, so the caller need not keep them.
 * @warning The index is not checked against the number of formats. An out-of-range index writes past the tables.
 */
/**
 * @brief 어떤 형식의 구조체 전체 함수를 설치한다.
 * @param idx 어느 형식인지
 * @param sfi 읽기 함수들
 * @param sfo 쓰기 함수들
 * @note 이 파일이 그것에 대해 알지 않고도 형식이 더해질 수 있도록 존재한다. 변환 도구가 시작할 때 자기 표를 설치한다. 그것이 그 도구가 게임 자신이 쓸 수 없는 형식을 쓸 수 있게 하는 것이다.
 * @note 표가 가리켜지는 것이 아니라 복사되므로, 호출자가 그것을 지녀야 하지 않는다.
 * @warning 그 색인이 형식의 개수와 견주어 검사되지 않는다. 범위를 벗어난 색인은 표를 넘어 쓴다.
 */
void
sf_setprocs(int idx, struct sf_structlevel_procs *sfi, struct sf_structlevel_procs *sfo)
{
    sfoprocs[idx] = *sfo;
    sfiprocs[idx] = *sfi;
}
/**
 * @brief Install a format's field-by-field routines.
 * @param idx which format
 * @param flsfi the reading routines
 * @param flsfo the writing routines
 * @note The counterpart for the other kind of table. A format needs one kind or the other, not both -- which one it gets decides how the generated routines dispatch for it.
 */
/**
 * @brief 어떤 형식의 필드 단위 함수를 설치한다.
 * @param idx 어느 형식인지
 * @param flsfi 읽기 함수들
 * @param flsfo 쓰기 함수들
 * @note 다른 종류의 표를 위한 짝이다. 형식은 한 종류나 다른 종류를 필요로 하며 둘 다는 아니다. 어느 것을 얻는지가 생성된 함수들이 그것을 위해 어떻게 갈라 보내는지를 정한다.
 */
void
sf_setflprocs(int idx, struct sf_fieldlevel_procs *flsfi,
            struct sf_fieldlevel_procs *flsfo)
{
    sfoflprocs[idx] = *flsfo;
    sfiflprocs[idx] = *flsfi;
}

/**
 * @name Pointer normalisation hooks
 *
 * One routine per saved structure, called before that structure is written so it can clear or rewrite any pointer field. A stored pointer is an address from this run and is meaningless in the file, so a structure holding one has to say what should be
 * stored instead.
 *
 * Every one of them is empty. That is not an oversight but the current state of the design: the hooks exist so that a structure needing this has somewhere to put it, and none does yet -- the structures that hold pointers are handled by the save code
 * itself rather than here.
 *
 * @note Each takes its structure and ignores it, marked as deliberately unused so the compiler does not object. That is what an empty hook has to look like.
 * @note Declared just above rather than in a header, because nothing outside this file calls them -- they are reached only through the tables.
 * @note Absent from the conversion tool's build. The tool never writes a structure from live memory, so it has no pointers to normalise.
 * @warning Being empty, they provide no protection. A structure whose pointer field is written as-is produces a file containing an address, and nothing here or elsewhere reports that.
 * @{
 */
/**
 * @name 포인터 표준화 갈고리
 *
 * 저장되는 구조체마다 한 함수이며, 그 구조체가 쓰이기 전에 호출되어 어떤 포인터 필드든 지우거나 다시 쓸 수 있게 한다. 저장된 포인터는 이번 실행의 주소이고 파일에서는 뜻이 없으므로, 그것을 담은 구조체는 대신 무엇이 저장되어야 하는지 말해야 한다.
 *
 * 그것들 모두가 비어 있다. 그것은 빠뜨림이 아니라 설계의 현재 상태다. 그 갈고리는 이것을 필요로 하는 구조체가 그것을 둘 곳을 갖도록 존재하며, 아직 어느 것도 그렇지 않다. 포인터를 담은 구조체는 여기가 아니라 저장 코드 자신이 다룬다.
 *
 * @note 각각이 자기 구조체를 취하고 그것을 무시하며, 컴파일러가 반대하지 않도록 의도적으로 쓰이지 않는 것으로 표시된다. 그것이 빈 갈고리가 보여야 하는 모습이다.
 * @note 헤더가 아니라 바로 위에 선언되는데, 이 파일 밖의 무엇도 그것을 호출하지 않기 때문이다. 그것들은 표를 통해서만 닿아진다.
 * @note 변환 도구의 빌드에는 없다. 그 도구는 살아 있는 메모리에서 구조체를 쓰는 일이 결코 없으므로, 표준화할 포인터가 없다.
 * @warning 비어 있으므로 어떤 보호도 내주지 않는다. 포인터 필드가 그대로 쓰인 구조체는 주소를 담은 파일을 내며, 여기의 무엇도 다른 곳의 무엇도 그것을 알리지 않는다.
 * @{
 */

#ifndef SFCTOOL
void norm_ptrs_any(union any *d_any);
void norm_ptrs_align(struct align *d_align);
void norm_ptrs_arti_info(struct arti_info *d_arti_info);
void norm_ptrs_attribs(struct attribs *d_attribs);
void norm_ptrs_bill_x(struct bill_x *d_bill_x);
void norm_ptrs_branch(struct branch *d_branch);
void norm_ptrs_bubble(struct bubble *d_bubble);
void norm_ptrs_cemetery(struct cemetery *d_cemetery);
void norm_ptrs_context_info(struct context_info *d_context_info);
void norm_ptrs_achievement_tracking(
    struct achievement_tracking *d_achievement_tracking);
void norm_ptrs_book_info(struct book_info *d_book_info);
void norm_ptrs_dig_info(struct dig_info *d_dig_info);
void norm_ptrs_engrave_info(struct engrave_info *d_engrave_info);
void norm_ptrs_obj_split(struct obj_split *d_obj_split);
void norm_ptrs_polearm_info(struct polearm_info *d_polearm_info);
void norm_ptrs_takeoff_info(struct takeoff_info *d_takeoff_info);
void norm_ptrs_tin_info(struct tin_info *d_tin_info);
void norm_ptrs_tribute_info(struct tribute_info *d_tribute_info);
void norm_ptrs_victual_info(struct victual_info *d_victual_info);
void norm_ptrs_warntype_info(struct warntype_info *d_warntype_info);
void norm_ptrs_d_flags(struct d_flags *d_d_flags);
void norm_ptrs_d_level(struct d_level *d_d_level);
void norm_ptrs_damage(struct damage *d_damage);
void norm_ptrs_dest_area(struct dest_area *d_dest_area);
void norm_ptrs_dgn_topology(struct dgn_topology *d_dgn_topology);
void norm_ptrs_dungeon(struct dungeon *d_dungeon);
void norm_ptrs_ebones(struct ebones *d_ebones);
void norm_ptrs_edog(struct edog *d_edog);
void norm_ptrs_egd(struct egd *d_egd);
void norm_ptrs_emin(struct emin *d_emin);
void norm_ptrs_engr(struct engr *d_engr);
void norm_ptrs_epri(struct epri *d_epri);
void norm_ptrs_eshk(struct eshk *d_eshk);
void norm_ptrs_fakecorridor(struct fakecorridor *d_fakecorridor);
void norm_ptrs_fe(struct fe *d_fe);
void norm_ptrs_flag(struct flag *d_flag);
void norm_ptrs_fruit(struct fruit *d_fruit);
void norm_ptrs_gamelog_line(struct gamelog_line *d_gamelog_line);
void norm_ptrs_kinfo(struct kinfo *d_kinfo);
void norm_ptrs_levelflags(struct levelflags *d_levelflags);
void norm_ptrs_linfo(struct linfo *d_linfo);
void norm_ptrs_ls_t(struct ls_t *d_ls_t);
void norm_ptrs_mapseen_feat(struct mapseen_feat *d_mapseen_feat);
void norm_ptrs_mapseen_flags(struct mapseen_flags *d_mapseen_flags);
void norm_ptrs_mapseen_rooms(struct mapseen_rooms *d_mapseen_rooms);
void norm_ptrs_mapseen(struct mapseen *d_mapseen);
void norm_ptrs_mextra(struct mextra *d_mextra);
void norm_ptrs_mkroom(struct mkroom *d_mkroom);
void norm_ptrs_monst(struct monst *d_monst);
void norm_ptrs_mvitals(struct mvitals *d_mvitals);
void norm_ptrs_nhcoord(struct nhcoord *d_nhcoord);
void norm_ptrs_nhrect(struct nhrect *d_nhrect);
void norm_ptrs_novel_tracking(struct novel_tracking *d_novel_tracking);
void norm_ptrs_obj(struct obj *d_obj);
void norm_ptrs_objclass(struct objclass *d_objclass);
void norm_ptrs_oextra(struct oextra *d_oextra);
void norm_ptrs_prop(struct prop *d_prop);
void norm_ptrs_q_score(struct q_score *d_q_score);
void norm_ptrs_rm(struct rm *d_rm);
void norm_ptrs_s_level(struct s_level *d_s_level);
void norm_ptrs_skills(struct skills *d_skills);
void norm_ptrs_spell(struct spell *d_spell);
void norm_ptrs_stairway(struct stairway *d_stairway);
void norm_ptrs_trap(struct trap *d_trap);
void norm_ptrs_u_conduct(struct u_conduct *d_u_conduct);
void norm_ptrs_u_event(struct u_event *d_u_event);
void norm_ptrs_u_have(struct u_have *d_u_have);
void norm_ptrs_u_realtime(struct u_realtime *d_u_realtime);
void norm_ptrs_u_roleplay(struct u_roleplay *d_u_roleplay);
void norm_ptrs_version_info(struct version_info *d_version_info);
void norm_ptrs_vlaunchinfo(union vlaunchinfo *d_vlaunchinfo);
void norm_ptrs_vptrs(union vptrs *d_vptrs);
void norm_ptrs_you(struct you *d_you);
#ifdef DEMO_UPLIFTS
void norm_ptrs_mystruct(struct mystruct *d_mystruct);
void norm_ptrs_mystruct_rev0(struct mystruct_rev0 *d_mystruct_rev0);
#endif

void
norm_ptrs_any(union any *d_any UNUSED)
{
}
void
norm_ptrs_align(struct align *d_align UNUSED)
{
}

void
norm_ptrs_arti_info(struct arti_info *d_arti_info UNUSED)
{
}

void
norm_ptrs_attribs(struct attribs *d_attribs UNUSED)
{
}

void
norm_ptrs_bill_x(struct bill_x *d_bill_x UNUSED)
{
}

void
norm_ptrs_branch(struct branch *d_branch UNUSED)
{
}

void
norm_ptrs_bubble(struct bubble *d_bubble UNUSED)
{
}

void
norm_ptrs_cemetery(struct cemetery *d_cemetery UNUSED)
{
}

void
norm_ptrs_context_info(struct context_info *d_context_info UNUSED)
{
}

void
norm_ptrs_achievement_tracking(struct achievement_tracking *d_achievement_tracking UNUSED)
{
}

void
norm_ptrs_book_info(struct book_info *d_book_info UNUSED)
{
}

void
norm_ptrs_dig_info(struct dig_info *d_dig_info UNUSED)
{
}

void
norm_ptrs_engrave_info(struct engrave_info *d_engrave_info UNUSED)
{
}

void
norm_ptrs_obj_split(struct obj_split *d_obj_split UNUSED)
{
}

void
norm_ptrs_polearm_info(struct polearm_info *d_polearm_info UNUSED)
{
}

void
norm_ptrs_takeoff_info(struct takeoff_info *d_takeoff_info UNUSED)
{
}

void
norm_ptrs_tin_info(struct tin_info *d_tin_info UNUSED)
{
}

void
norm_ptrs_tribute_info(struct tribute_info *d_tribute_info UNUSED)
{
}

void
norm_ptrs_victual_info(struct victual_info *d_victual_info UNUSED)
{
}

void
norm_ptrs_warntype_info(struct warntype_info *d_warntype_info UNUSED)
{
}

void
norm_ptrs_d_flags(struct d_flags *d_d_flags UNUSED)
{
}

void
norm_ptrs_d_level(struct d_level *d_d_level UNUSED)
{
}

void
norm_ptrs_damage(struct damage *d_damage UNUSED)
{
}

void
norm_ptrs_dest_area(struct dest_area *d_dest_area UNUSED)
{
}

void
norm_ptrs_dgn_topology(struct dgn_topology *d_dgn_topology UNUSED)
{
}

void
norm_ptrs_dungeon(struct dungeon *d_dungeon UNUSED)
{
}

void
norm_ptrs_ebones(struct ebones *d_ebones UNUSED)
{
}

void
norm_ptrs_edog(struct edog *d_edog UNUSED)
{
}

void
norm_ptrs_egd(struct egd *d_egd UNUSED)
{
}

void
norm_ptrs_emin(struct emin *d_emin UNUSED)
{
}

void
norm_ptrs_engr(struct engr *d_engr UNUSED)
{
}

void
norm_ptrs_epri(struct epri *d_epri UNUSED)
{
}

void
norm_ptrs_eshk(struct eshk *d_eshk UNUSED)
{
}

void
norm_ptrs_fakecorridor(struct fakecorridor *d_fakecorridor UNUSED)
{
}

void
norm_ptrs_fe(struct fe *d_fe UNUSED)
{
}

void
norm_ptrs_flag(struct flag *d_flag UNUSED)
{
}

void
norm_ptrs_fruit(struct fruit *d_fruit UNUSED)
{
}

void
norm_ptrs_gamelog_line(struct gamelog_line *d_gamelog_line UNUSED)
{
}

void
norm_ptrs_kinfo(struct kinfo *d_kinfo UNUSED)
{
}

void
norm_ptrs_levelflags(struct levelflags *d_levelflags UNUSED)
{
}

void
norm_ptrs_linfo(struct linfo *d_linfo UNUSED)
{
}

void
norm_ptrs_ls_t(struct ls_t *d_ls_t UNUSED)
{
}

void
norm_ptrs_mapseen_feat(struct mapseen_feat *d_mapseen_feat UNUSED)
{
}

void
norm_ptrs_mapseen_flags(struct mapseen_flags *d_mapseen_flags UNUSED)
{
}

void
norm_ptrs_mapseen_rooms(struct mapseen_rooms *d_mapseen_rooms UNUSED)
{
}

void
norm_ptrs_mapseen(struct mapseen *d_mapseen UNUSED)
{
}

void
norm_ptrs_mextra(struct mextra *d_mextra UNUSED)
{
}

void
norm_ptrs_mkroom(struct mkroom *d_mkroom UNUSED)
{
}

void
norm_ptrs_monst(struct monst *d_monst UNUSED)
{
}

void
norm_ptrs_mvitals(struct mvitals *d_mvitals UNUSED)
{
}

void
norm_ptrs_nhcoord(struct nhcoord *d_nhcoord UNUSED)
{
}

void
norm_ptrs_nhrect(struct nhrect *d_nhrect UNUSED)
{
}

void
norm_ptrs_novel_tracking(struct novel_tracking *d_novel_tracking UNUSED)
{
}

void
norm_ptrs_obj(struct obj *d_obj UNUSED)
{
}

void
norm_ptrs_objclass(struct objclass *d_objclass UNUSED)
{
}

void
norm_ptrs_oextra(struct oextra *d_oextra UNUSED)
{
}

void
norm_ptrs_prop(struct prop *d_prop UNUSED)
{
}

void
norm_ptrs_q_score(struct q_score *d_q_score UNUSED)
{
}

void
norm_ptrs_rm(struct rm *d_rm UNUSED)
{
}

#ifdef DEMO_UPLIFTS
void
norm_ptrs_mystruct(struct mystruct *d_mystruct UNUSED)
{
}
void
norm_ptrs_mystruct_rev0(struct mystruct_rev0 *d_mystruct_rev0 UNUSED)
{
}
#endif

void
norm_ptrs_s_level(struct s_level *d_s_level UNUSED)
{
}

void
norm_ptrs_skills(struct skills *d_skills UNUSED)
{
}

void
norm_ptrs_spell(struct spell *d_spell UNUSED)
{
}

void
norm_ptrs_stairway(struct stairway *d_stairway UNUSED)
{
}

void
norm_ptrs_trap(struct trap *d_trap UNUSED)
{
}

void
norm_ptrs_u_conduct(struct u_conduct *d_u_conduct UNUSED)
{
}

void
norm_ptrs_u_event(struct u_event *d_u_event UNUSED)
{
}

void
norm_ptrs_u_have(struct u_have *d_u_have UNUSED)
{
}

void
norm_ptrs_u_realtime(struct u_realtime *d_u_realtime UNUSED)
{
}

void
norm_ptrs_u_roleplay(struct u_roleplay *d_u_roleplay UNUSED)
{
}

void
norm_ptrs_version_info(struct version_info *d_version_info UNUSED)
{
}

void
norm_ptrs_vlaunchinfo(union vlaunchinfo *d_vlaunchinfo UNUSED)
{
}

void
norm_ptrs_vptrs(union vptrs *d_vptrs UNUSED)
{
}

void
norm_ptrs_you(struct you *d_you UNUSED)
{
}
#endif  /* SFCTOOL */

/** @} */

/**
 * @name Retiring the generators
 * @brief Undefined so nothing outside this file can expand them.
 * @note They generate function definitions, so expanding one anywhere else would define the same routines twice. Undefining them makes that a compilation failure rather than a duplicate-symbol error at link time.
 * @{
 */
/**
 * @name 생성기 물리기
 * @brief 이 파일 밖의 무엇도 그것을 펼칠 수 없도록 정의 해제된다.
 * @note 그것들은 함수 정의를 생성하므로, 다른 어디서든 하나를 펼치는 것은 같은 함수를 두 번 정의할 것이다. 그것을 정의 해제하는 것은 그것을 이을 때의 중복 기호 오류가 아니라 컴파일 실패로 만든다.
 * @{
 */

#undef SF_X
#undef SF_C
#undef SF_A

/** @} */

/* end of sfbase.c */

