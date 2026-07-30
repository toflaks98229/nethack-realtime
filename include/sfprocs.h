/* NetHack 3.6 sfprocs.h	Tue Nov  6 19:38:48 2018 */
/* Copyright (c) NetHack Development Team 2025.                   */
/* NetHack may be freely redistributed.  See license for details. */

/**
 * @file sfprocs.h
 * @brief Making the save format something the game can choose at run time.
 *
 * There is more than one way to write a saved game. The historical format writes each structure as a block; another writes
 * every field separately as text. Neither is better in general -- one is compact, the other is readable and portable -- so the
 * game holds both and picks.
 *
 * Picking means the writing routines cannot be called by name. So the whole set of them is gathered into a table of function
 * pointers, one entry per saved type, and choosing a format is choosing which table to use. Every call to save something is a
 * call through that table.
 *
 * The file is written almost entirely with macros, and for one reason: the set of types appears four times over -- as
 * declarations, as table members, and once each for reading and writing -- and the lists have to agree exactly. Writing them
 * as a macro applied to a list of type names means the four lists cannot drift apart, because there is only one list.
 *
 * Three macros rather than one, because the types are not all shaped alike: a plain type is named on its own, a structure or
 * union needs its keyword given separately, and the character and bitfield routines take an extra size argument.
 *
 * @note Each type gets four routines, not two: read and write, in a plain form and an @c x form. The plain form is the
 *       historical whole-structure treatment; the @c x form is the field-by-field one. Two tables, two kinds of entry.
 * @note The macros are undefined again after use, deliberately -- they are scaffolding for this file rather than an interface.
 * @warning Adding a saved type means adding it to the declaration list and to the table, and the two are separate lists in
 *          this file even though they are generated. Adding to only one leaves either an unused routine or a table with a
 *          missing entry.
 */

/**
 * @file sfprocs.h
 * @brief 저장 형식을 게임이 실행 중에 고를 수 있는 것으로 만들기.
 *
 * 저장 게임을 기록하는 방식은 하나가 아니다. 역사적 형식은 각 구조체를 덩어리로 쓰고, 다른 형식은 모든 필드를 따로 텍스트로 쓴다. 어느 쪽도 일반적으로 더 낫지 않다. 하나는 작고, 다른 하나는 읽을 수
 * 있고 이식 가능하다. 그래서 게임이 둘 다를 지니고 고른다.
 *
 * 고른다는 것은 쓰기 루틴을 이름으로 호출할 수 없다는 뜻이다. 그래서 그 전체 묶음이 함수 포인터의 표로 모이며, 저장되는 타입마다 항목 하나씩이고, 형식을 고르는 것은 어느 표를 쓸지 고르는 일이 된다.
 * 무엇을 저장하는 모든 호출이 그 표를 통한 호출이다.
 *
 * 이 파일은 거의 전부가 매크로로 쓰여 있고, 이유는 하나다. 타입의 묶음이 네 번 나타난다. 선언으로, 표의 멤버로, 그리고 읽기와 쓰기에 각각 한 번씩. 그리고 그 목록들이 정확히 일치해야 한다. 타입 이름
 * 목록에 적용되는 매크로로 쓰면 그 네 목록이 서로 어긋날 수 없다. 목록이 하나뿐이기 때문이다.
 *
 * 하나가 아니라 세 매크로인 것은 타입들의 모양이 다 같지 않기 때문이다. 평범한 타입은 그 자체로 지칭되고, 구조체나 공용체는 그 키워드를 따로 줘야 하며, 문자와 비트필드 루틴은 크기 인자를 하나 더
 * 받는다.
 *
 * @note 각 타입은 둘이 아니라 네 루틴을 갖는다. 읽기와 쓰기가 각각 평범한 형태와 @c x 형태로. 평범한 형태는 역사적인 구조체 통째 처리이고, @c x 형태는 필드 하나하나 처리다. 두 표, 두 종류의 항목.
 * @note 매크로들은 쓰인 뒤 다시 정의 해제된다. 의도적이다. 인터페이스가 아니라 이 파일을 위한 비계이기 때문이다.
 * @warning 저장되는 타입을 더하는 것은 선언 목록과 표에 그것을 더하는 일이며, 생성되는 것이라 해도 이 파일에서 그 둘은 별개의 목록이다. 한쪽에만 더하면 쓰이지 않는 루틴이 남거나 항목이 빠진 표가
 *          남는다.
 */

#ifndef SFPROCS_H
#define SFPROCS_H

/**
 * @name Type shapes
 * @brief Whether a saved type is a plain value or something with parts.
 * @note The distinction matters to the field-by-field format: a plain value is one item of output, and something with parts has
 *       to be taken apart first.
 * @{
 */
/**
 * @name 타입의 모양
 * @brief 저장되는 타입이 평범한 값인지 부분을 가진 것인지.
 * @note 이 구별이 필드 단위 형식에 중요하다. 평범한 값은 출력 한 항목이고, 부분을 가진 것은 먼저 분해되어야 한다.
 * @{
 */
#define NHTYPE_SIMPLE    1
#define NHTYPE_COMPLEX   2
/** @} */

/**
 * @def SF_PROTO
 * @brief Declare all four routines for one plain type.
 * @param dtyp the type's name, used both as the type and to build the routine names
 * @note Four rather than two: read and write, each in the whole-structure form and the field-by-field @c x form.
 */
/**
 * @def SF_PROTO
 * @brief 평범한 타입 하나에 대한 네 루틴을 모두 선언한다.
 * @param dtyp 타입의 이름. 타입으로도 쓰이고 루틴 이름을 만드는 데도 쓰인다
 * @note 둘이 아니라 넷이다. 읽기와 쓰기가 각각 구조체 통째 형태와 필드 단위 @c x 형태로.
 */
#define SF_PROTO(dtyp) \
    extern void sfo_##dtyp(NHFILE *, dtyp *, const char *);   \
    extern void sfi_##dtyp(NHFILE *, dtyp *, const char *);   \
    extern void sfo_x_##dtyp(NHFILE *, dtyp *, const char *); \
    extern void sfi_x_##dtyp(NHFILE *, dtyp *, const char *)
/**
 * @def SF_PROTO_C
 * @brief Declare all four routines for a structure or union.
 * @param keyw the keyword -- @c struct or @c union
 * @param dtyp the tag name, used to build the routine names
 * @note The keyword is a separate argument because the routine names are built from the tag alone, so the two cannot be given
 *       as one token.
 */
/**
 * @def SF_PROTO_C
 * @brief 구조체나 공용체에 대한 네 루틴을 모두 선언한다.
 * @param keyw 키워드. @c struct 또는 @c union
 * @param dtyp 태그 이름. 루틴 이름을 만드는 데 쓰인다
 * @note 키워드가 별개의 인자인 것은 루틴 이름이 태그만으로 만들어지기 때문이다. 그래서 그 둘을 하나의 토큰으로 줄 수 없다.
 */
#define SF_PROTO_C(keyw, dtyp) \
    extern void sfo_##dtyp(NHFILE *, keyw dtyp *, const char *);   \
    extern void sfi_##dtyp(NHFILE *, keyw dtyp *, const char *);   \
    extern void sfo_x_##dtyp(NHFILE *, keyw dtyp *, const char *); \
    extern void sfi_x_##dtyp(NHFILE *, keyw dtyp *, const char *)
/**
 * @def SF_PROTO_X
 * @brief Declare all four routines for a type whose routines need a size.
 * @param xxx the actual type of the data
 * @param dtyp the name used to build the routine names, which need not match the type
 * @note Used for characters and bitfields, where how many is not implied by the pointer. That extra argument is why they cannot
 *       share the shape of the others.
 */
/**
 * @def SF_PROTO_X
 * @brief 루틴에 크기가 필요한 타입에 대한 네 루틴을 모두 선언한다.
 * @param xxx 데이터의 실제 타입
 * @param dtyp 루틴 이름을 만드는 데 쓰이는 이름. 타입과 일치할 필요는 없다
 * @note 문자와 비트필드에 쓰인다. 그것들은 몇 개인지가 포인터에 함축되어 있지 않다. 그 추가 인자가 그들이 나머지의 모양을 공유할 수 없는 이유다.
 */
#define SF_PROTO_X(xxx, dtyp) \
    extern void sfo_##dtyp(NHFILE *, xxx *, const char *, int bfsz);   \
    extern void sfi_##dtyp(NHFILE *, xxx *, const char *, int bfsz);   \
    extern void sfo_x_##dtyp(NHFILE *, xxx *, const char *, int bfsz); \
    extern void sfi_x_##dtyp(NHFILE *, xxx *, const char *, int bfsz)

SF_PROTO_C(struct, arti_info);
SF_PROTO_C(struct, nhrect);
SF_PROTO_C(struct, branch);
SF_PROTO_C(struct, bubble);
SF_PROTO_C(struct, cemetery);
SF_PROTO_C(struct, context_info);
SF_PROTO_C(struct, nhcoord);
SF_PROTO_C(struct, damage);
SF_PROTO_C(struct, dest_area);
SF_PROTO_C(struct, dgn_topology);
SF_PROTO_C(struct, dungeon);
SF_PROTO_C(struct, d_level);
SF_PROTO_C(struct, ebones);
SF_PROTO_C(struct, edog);
SF_PROTO_C(struct, egd);
SF_PROTO_C(struct, emin);
SF_PROTO_C(struct, engr);
SF_PROTO_C(struct, epri);
SF_PROTO_C(struct, eshk);
SF_PROTO_C(struct, fe);
SF_PROTO_C(struct, flag);
SF_PROTO_C(struct, fruit);
SF_PROTO_C(struct, gamelog_line);
SF_PROTO_C(struct, kinfo);
SF_PROTO_C(struct, levelflags);
SF_PROTO_C(struct, ls_t);
SF_PROTO_C(struct, linfo);
SF_PROTO_C(struct, mapseen_feat);
SF_PROTO_C(struct, mapseen_flags);
SF_PROTO_C(struct, mapseen_rooms);
SF_PROTO_C(struct, mkroom);
SF_PROTO_C(struct, monst);
SF_PROTO_C(struct, mvitals);
SF_PROTO_C(struct, obj);
SF_PROTO_C(struct, objclass);
SF_PROTO_C(struct, q_score);
SF_PROTO_C(struct, rm);
SF_PROTO_C(struct, spell);
SF_PROTO_C(struct, stairway);
SF_PROTO_C(struct, s_level);
SF_PROTO_C(struct, trap);
SF_PROTO_C(struct, version_info);
SF_PROTO_C(struct, you);
SF_PROTO_C(union, any);
#ifdef DEMO_UPLIFTS
SF_PROTO_C(struct, mystruct);
SF_PROTO_C(struct, mystruct_rev0);
#endif
SF_PROTO(int16);
SF_PROTO(int32);
SF_PROTO(int64);
SF_PROTO(uchar);
SF_PROTO(uint16);
SF_PROTO(uint32);
SF_PROTO(uint64);
SF_PROTO(long);
SF_PROTO(ulong);
SF_PROTO(xint8);
SF_PROTO(boolean);
SF_PROTO(schar);
SF_PROTO(aligntyp);
SF_PROTO(genericptr);
SF_PROTO(size_t);
SF_PROTO(time_t);
SF_PROTO(int);
SF_PROTO(unsigned);
SF_PROTO(coordxy);
SF_PROTO(short);
SF_PROTO(xint16);
SF_PROTO(ushort);
SF_PROTO_X(uint8_t, bitfield);
SF_PROTO_X(char, char);

#undef SF_PROTO
#undef SF_PROTO_C
#undef SF_PROTO_X

/**
 * @def SF_ENTRY
 * @brief Declare the table slot for one plain type.
 * @param dtyp the type's name, used both as the type and to build the member name
 * @note Deliberately mirrors @c SF_PROTO so that the declarations and the table are generated from the same list of type names
 *       and cannot disagree about a type's signature.
 */
/**
 * @def SF_ENTRY
 * @brief 평범한 타입 하나에 대한 표의 칸을 선언한다.
 * @param dtyp 타입의 이름. 타입으로도 쓰이고 멤버 이름을 만드는 데도 쓰인다
 * @note @c SF_PROTO 를 의도적으로 그대로 따른다. 그래서 선언들과 표가 같은 타입 이름 목록에서 생성되고 어떤 타입의 서명에 대해 서로 어긋날 수 없다.
 */
#define SF_ENTRY(dtyp)                                 \
    void (*sf_##dtyp)(NHFILE *, dtyp *, const char *)
/**
 * @def SF_ENTRY_C
 * @brief Declare the table slot for a structure or union.
 * @param keyw the keyword -- @c struct or @c union
 * @param dtyp the tag name, used to build the member name
 */
/**
 * @def SF_ENTRY_C
 * @brief 구조체나 공용체에 대한 표의 칸을 선언한다.
 * @param keyw 키워드. @c struct 또는 @c union
 * @param dtyp 태그 이름. 멤버 이름을 만드는 데 쓰인다
 */
#define SF_ENTRY_C(keyw, dtyp)                              \
    void (*sf_##dtyp)(NHFILE *, keyw dtyp *, const char *)
/**
 * @def SF_ENTRY_X
 * @brief Declare the table slot for a type whose routine needs a size.
 * @param xxx the actual type of the data
 * @param dtyp the name used to build the member name
 */
/**
 * @def SF_ENTRY_X
 * @brief 루틴에 크기가 필요한 타입에 대한 표의 칸을 선언한다.
 * @param xxx 데이터의 실제 타입
 * @param dtyp 멤버 이름을 만드는 데 쓰이는 이름
 */
#define SF_ENTRY_X(xxx, dtyp)                                   \
    void (*sf_##dtyp)(NHFILE *, xxx *, const char *, int bfsz)

/**
 * @brief One complete set of routines: everything needed to write, or to read, in one format.
 *
 * Every saved type has a slot. Choosing a save format is choosing which of these to call through, so the table has to be
 * complete -- a null slot is not a format that omits that type, it is a crash when something of that type is saved.
 *
 * @note Structures and unions come first, then the plain types, then the two that take a size. The grouping is by which macro
 *       declares them and carries no meaning beyond that.
 * @warning The member order must match the order in which each format's table is initialised. Since the slots are all function
 *          pointers of similar shape, a table built in the wrong order compiles.
 */
/**
 * @brief 하나의 완전한 루틴 묶음. 한 형식으로 쓰거나 읽는 데 필요한 모든 것.
 *
 * 저장되는 모든 타입에 칸이 하나씩 있다. 저장 형식을 고르는 것은 이 중 어느 것을 통해 호출할지 고르는 일이므로, 표는 완전해야 한다. 빈 칸은 그 타입을 생략하는 형식이 아니라, 그 타입의 무언가를 저장할
 * 때의 충돌이다.
 *
 * @note 구조체와 공용체가 먼저, 그다음 평범한 타입, 그다음 크기를 받는 둘이 온다. 그 묶음은 어느 매크로가 선언하는지에 따른 것이며 그 이상의 의미는 없다.
 * @warning 멤버 순서는 각 형식의 표가 초기화되는 순서와 맞아야 한다. 칸들이 모두 모양이 비슷한 함수 포인터이므로, 잘못된 순서로 만든 표도 컴파일된다.
 */
struct sf_procs {
    SF_ENTRY_C(struct, arti_info);
    SF_ENTRY_C(struct, nhrect);
    SF_ENTRY_C(struct, branch);
    SF_ENTRY_C(struct, bubble);
    SF_ENTRY_C(struct, cemetery);
    SF_ENTRY_C(struct, context_info);
    SF_ENTRY_C(struct, nhcoord);
    SF_ENTRY_C(struct, damage);
    SF_ENTRY_C(struct, dest_area);
    SF_ENTRY_C(struct, dgn_topology);
    SF_ENTRY_C(struct, dungeon);
    SF_ENTRY_C(struct, d_level);
    SF_ENTRY_C(struct, ebones);
    SF_ENTRY_C(struct, edog);
    SF_ENTRY_C(struct, egd);
    SF_ENTRY_C(struct, emin);
    SF_ENTRY_C(struct, engr);
    SF_ENTRY_C(struct, epri);
    SF_ENTRY_C(struct, eshk);
    SF_ENTRY_C(struct, fe);
    SF_ENTRY_C(struct, flag);
    SF_ENTRY_C(struct, fruit);
    SF_ENTRY_C(struct, gamelog_line);
    SF_ENTRY_C(struct, kinfo);
    SF_ENTRY_C(struct, levelflags);
    SF_ENTRY_C(struct, ls_t);
    SF_ENTRY_C(struct, linfo);
    SF_ENTRY_C(struct, mapseen_feat);
    SF_ENTRY_C(struct, mapseen_flags);
    SF_ENTRY_C(struct, mapseen_rooms);
    SF_ENTRY_C(struct, mkroom);
    SF_ENTRY_C(struct, monst);
    SF_ENTRY_C(struct, mvitals);
    SF_ENTRY_C(struct, obj);
    SF_ENTRY_C(struct, objclass);
    SF_ENTRY_C(struct, q_score);
    SF_ENTRY_C(struct, rm);
    SF_ENTRY_C(struct, spell);
    SF_ENTRY_C(struct, stairway);
    SF_ENTRY_C(struct, s_level);
    SF_ENTRY_C(struct, trap);
    SF_ENTRY_C(struct, version_info);
    SF_ENTRY_C(struct, you);
    SF_ENTRY_C(union, any);
#ifdef DEMO_UPLIFTS
    SF_ENTRY_C(struct, mystruct);
    SF_ENTRY_C(struct, mystruct_rev0);
#endif

    SF_ENTRY(aligntyp);
    SF_ENTRY(boolean);
    SF_ENTRY(coordxy);
    SF_ENTRY(genericptr);
    SF_ENTRY(int);
    SF_ENTRY(int16);
    SF_ENTRY(int32);
    SF_ENTRY(int64);
    SF_ENTRY(long);
    SF_ENTRY(schar);
    SF_ENTRY(short);
    SF_ENTRY(size_t);
    SF_ENTRY(time_t);
    SF_ENTRY(uchar);
    SF_ENTRY(uint16);
    SF_ENTRY(uint32);
    SF_ENTRY(uint64);
    SF_ENTRY(ulong);
    SF_ENTRY(unsigned);
    SF_ENTRY(ushort);
    SF_ENTRY(xint16);
    SF_ENTRY(xint8);
    SF_ENTRY_X(char, char);
    SF_ENTRY_X(uint8_t, bitfield);
};

#undef SF_ENTRY
#undef SF_ENTRY_C
#undef SF_ENTRY_X

/**
 * @brief A save format that treats each structure as a single block.
 * @note The historical way: a structure is written whole, so the file is compact and its contents are only meaningful to a build
 *       with the same structure layouts.
 */
/**
 * @brief 각 구조체를 하나의 덩어리로 다루는 저장 형식.
 * @note 역사적인 방식이다. 구조체가 통째로 기록되므로 파일이 작고, 그 내용은 같은 구조체 배치를 가진 빌드에게만 의미가 있다.
 */
struct sf_structlevel_procs {
    /**
     * @brief The file name extension this format uses.
     * @note Kept with the routines so that the format and the name of the file it produces cannot get out of step.
     */
    /**
     * @brief 이 형식이 쓰는 파일 이름 확장자.
     * @note 루틴들과 함께 보관되어, 형식과 그것이 만드는 파일의 이름이 서로 어긋날 수 없게 한다.
     */
    const char *ext;
    /**
     * @brief The routines themselves, one per saved type.
     * @note Called for whole-structure saving -- the historical treatment.
     */
    /**
     * @brief 루틴들 자체. 저장되는 타입마다 하나씩.
     * @note 구조체 통째 저장에 대해 호출된다. 역사적인 처리다.
     */
    struct sf_procs fn;    /* called for structlevel (historical) */
};
/**
 * @brief A save format that writes every field separately.
 * @note Larger but not tied to any build's structure layout, which is what makes a file written this way readable elsewhere.
 */
/**
 * @brief 모든 필드를 따로 기록하는 저장 형식.
 * @note 더 크지만 어떤 빌드의 구조체 배치에도 묶이지 않는다. 이 방식으로 기록된 파일이 다른 곳에서도 읽힐 수 있게 하는 것이 그것이다.
 */
struct sf_fieldlevel_procs {
    /**
     * @brief The file name extension this format uses.
     */
    /**
     * @brief 이 형식이 쓰는 파일 이름 확장자.
     */
    const char *ext;
    /**
     * @brief The routines themselves, one per saved type.
     * @note Called for field-by-field saving. The same table shape as the whole-structure form, filled with the @c x routines
     *       instead.
     */
    /**
     * @brief 루틴들 자체. 저장되는 타입마다 하나씩.
     * @note 필드 단위 저장에 대해 호출된다. 구조체 통째 형태와 같은 표 모양이며, 대신 @c x 루틴들로 채워진다.
     */
    struct sf_procs fn_x; /* called for fieldlevel */
};

/**
 * @name The registered formats
 * @brief Every save format the build knows how to write and read, indexed by format number.
 * @note Writing and reading are separate arrays because a build may be able to read a format it does not write.
 * @warning Declared twice in this file. The repetition is harmless but is repetition, not two different arrays.
 * @{
 */
/**
 * @name 등록된 형식들
 * @brief 이 빌드가 쓰고 읽을 수 있는 모든 저장 형식. 형식 번호로 색인된다.
 * @note 쓰기와 읽기가 별개의 배열인 것은, 어떤 빌드가 쓰지는 못하면서 읽을 수는 있는 형식이 있을 수 있기 때문이다.
 * @warning 이 파일에 두 번 선언되어 있다. 그 되풀이는 해롭지 않지만 되풀이이며, 서로 다른 두 배열이 아니다.
 * @{
 */
extern struct sf_structlevel_procs sfoprocs[NUM_SAVEFORMATS], sfiprocs[NUM_SAVEFORMATS];
/**
 * @brief Register a whole-structure format's writing and reading tables under a format number.
 * @note Both directions are registered together, so a format cannot be half-installed.
 */
/**
 * @brief 구조체 통째 형식의 쓰기와 읽기 표를 형식 번호 아래에 등록한다.
 * @note 두 방향이 함께 등록되므로, 형식이 절반만 설치될 수는 없다.
 */
extern void sf_setprocs(int, struct sf_structlevel_procs *, struct sf_structlevel_procs *);
/**
 * @brief The same, for a field-by-field format.
 */
/**
 * @brief 같은 일을 필드 단위 형식에 대해 한다.
 */
extern void sf_setflprocs(int, struct sf_fieldlevel_procs *, struct sf_fieldlevel_procs *);
extern struct sf_structlevel_procs sfoprocs[NUM_SAVEFORMATS], sfiprocs[NUM_SAVEFORMATS];
extern struct sf_fieldlevel_procs sfoflprocs[NUM_SAVEFORMATS], sfiflprocs[NUM_SAVEFORMATS];
/** @} */

/**
 * @name The two formats that exist
 * @brief The tables each format defines, which are what gets registered.
 * @note The historical pair is the compact whole-structure format; the exported pair writes readable text field by field. They
 *       are named separately rather than found by number because the code that installs them names them directly.
 * @{
 */
/**
 * @name 실제로 존재하는 두 형식
 * @brief 각 형식이 정의하는 표들. 등록되는 것이 바로 이것이다.
 * @note 역사적 짝은 작은 구조체 통째 형식이고, 내보내기 짝은 필드 하나하나를 읽을 수 있는 텍스트로 쓴다. 번호로 찾아지는 것이 아니라 따로 이름 붙어 있는 것은, 그것들을 설치하는 코드가 직접 이름으로
 *       지칭하기 때문이다.
 * @{
 */
extern struct sf_structlevel_procs historical_sfo_procs;
extern struct sf_structlevel_procs historical_sfi_procs;
extern struct sf_fieldlevel_procs exportascii_sfo_procs;
extern struct sf_fieldlevel_procs exportascii_sfi_procs;
/** @} */

#endif /* SFPROCS_H */

