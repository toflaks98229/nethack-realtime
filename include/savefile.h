/* NetHack 5.0	savefile.h	$NHDT-Date: 1781973087 2026/06/20 16:31:27 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.5 $ */
/* Copyright (c) Michael Allison, 2025.                           */
/* NetHack may be freely redistributed.  See license for details. */

/**
 * @file savefile.h
 * @brief Writing and reading every kind of thing the game needs to save.
 *
 * A saved game is not a memory image. Pointers mean nothing once written, structure padding differs between compilers, and
 * an integer's width can differ between the machine that saved and the machine that reads. So every type the game saves has
 * its own pair of routines: one that writes it out field by field, one that reads it back the same way.
 *
 * The naming is mechanical and worth learning once. A name beginning @c sfo writes, and the matching @c sfi reads; the rest
 * of the name is the type. They come in pairs and must stay in step -- a field added to the writer and forgotten in the
 * reader produces a save file that is read as garbage from that point on, with no error.
 *
 * On top of each pair sits a capitalised name. Under a modern compiler that name is a single type-directed selection: write
 * this thing, whatever it is, and the compiler picks the right routine. Under an older one, where that facility does not
 * exist, the same name is defined as a direct call. The point of the two spellings is that calling code is identical either
 * way.
 *
 * Every routine also takes a name for the field being written. It is not used to store anything -- it exists so that a save
 * file can be traced when the format is being changed, and so that a mismatch can say which field it was reading.
 *
 * @note Adding a saved type means adding both routines, both capitalised forms in the older-compiler block, and an entry in
 *       each type-directed selection. Missing the last is a compile error; missing one of the first two is not.
 * @warning A few types are deliberately outside the type-directed selection, listed at the foot of the file. They are types
 *          the selection cannot distinguish -- a plain character and a signed character among them -- so they are always
 *          called by name.
 */

/**
 * @file savefile.h
 * @brief 게임이 저장해야 하는 모든 종류의 것을 쓰고 읽기.
 *
 * 저장 게임은 메모리 이미지가 아니다. 포인터는 기록되면 아무 의미가 없고, 구조체 여백은 컴파일러마다 다르며, 정수의 폭은 저장한 기계와 읽는 기계 사이에서 다를 수 있다. 그래서 게임이 저장하는
 * 모든 타입에는 자기 짝의 루틴이 있다. 하나는 필드 하나하나를 써 내고, 하나는 같은 방식으로 되읽는다.
 *
 * 이름 규칙은 기계적이며 한 번 익혀 둘 만하다. @c sfo 로 시작하는 이름은 쓰고, 짝이 되는 @c sfi 는 읽는다. 이름의 나머지가 타입이다. 이들은 짝으로 오며 보조를 맞춰야 한다. 쓰는 쪽에는 더하고
 * 읽는 쪽에서는 잊은 필드는 그 지점부터 쓰레기로 읽히는 저장 파일을 만들며, 오류는 나지 않는다.
 *
 * 각 짝 위에는 첫 글자가 대문자인 이름이 놓인다. 현대적인 컴파일러에서 그 이름은 하나의 타입 기반 선택이다. 이것이 무엇이든 쓰라고 하면 컴파일러가 알맞은 루틴을 고른다. 그 기능이 없는 예전
 * 컴파일러에서는 같은 이름이 직접 호출로 정의된다. 두 표기가 있는 요점은 호출하는 코드가 어느 쪽이든 동일하다는 것이다.
 *
 * 모든 루틴은 기록되는 필드의 이름도 함께 받는다. 무엇을 저장하는 데 쓰이는 것이 아니다. 형식을 바꾸는 중일 때 저장 파일을 추적할 수 있도록, 그리고 불일치가 어느 필드를 읽던 중이었는지 말할 수
 * 있도록 존재한다.
 *
 * @note 저장되는 타입을 더하는 것은 두 루틴, 예전 컴파일러 블록의 두 대문자 형태, 그리고 각 타입 기반 선택의 항목을 더하는 일이다. 마지막을 빠뜨리면 컴파일 오류다. 앞의 둘 중 하나를 빠뜨리면
 *       그렇지 않다.
 * @warning 몇 타입은 의도적으로 타입 기반 선택 밖에 있으며 파일 끝에 나열되어 있다. 선택이 구별할 수 없는 타입들이다. 평범한 문자와 부호 있는 문자가 그 중에 있다. 그래서 항상 이름으로 호출된다.
 */

#ifndef SAVEFILE_H
#define SAVEFILE_H

/* #define SAVEFILE_DEBUGGING */

extern void sf_init(void);
/**
 * @name Writing
 * @brief One routine per saved type, writing it field by field.
 * @note Each takes the file, a pointer to the thing, and the field's name for tracing. The name is not written into the
 *       file.
 * @warning Every one of these has a reading counterpart that must read exactly what it wrote, in the same order. Nothing
 *          checks this.
 * @{
 */
/**
 * @name 쓰기
 * @brief 저장되는 타입마다 하나의 루틴. 필드 하나하나를 써 낸다.
 * @note 각각은 파일, 그것을 가리키는 포인터, 그리고 추적을 위한 필드 이름을 받는다. 그 이름은 파일에 기록되지 않는다.
 * @warning 이들 모두에는 자신이 쓴 것을 정확히 같은 순서로 읽어야 하는 읽기 짝이 있다. 그것을 검사하는 것은 없다.
 * @{
 */
/* sfbase.c output functions */
extern void sfo_aligntyp(NHFILE *, aligntyp *, const char *);
extern void sfo_any(NHFILE *, anything *, const char *);
extern void sfo_boolean(NHFILE *, boolean *, const char *);
extern void sfo_char(NHFILE *, char *, const char *, int);
extern void sfo_genericptr(NHFILE *, genericptr_t *, const char *);
extern void sfo_int16(NHFILE *, int16 *, const char *);
extern void sfo_int32(NHFILE *, int32 *, const char *);
extern void sfo_int64(NHFILE *, int64 *, const char *);
extern void sfo_uchar(NHFILE *, uchar *, const char *);
extern void sfo_uint16(NHFILE *, uint16 *, const char *);
extern void sfo_uint32(NHFILE *, uint32 *, const char *);
extern void sfo_uint64(NHFILE *, uint64 *, const char *);
extern void sfo_size_t(NHFILE *, size_t *, const char *);
extern void sfo_time_t(NHFILE *, time_t *, const char *);
//extern void sfo_str(NHFILE *, char *, const char *, int);
extern void sfo_arti_info(NHFILE *nhfp,
                                  struct arti_info *d_arti_info,
                                  const char *myname);
extern void sfo_dgn_topology(NHFILE *nhfp,
                                  struct dgn_topology *d_dgn_topology,
                                  const char *myname);
extern void sfo_dungeon(NHFILE *nhfp, struct dungeon *d_dungeon,
                             const char *myname);
extern void sfo_branch(NHFILE *nhfp, struct branch *d_branch,
                            const char *myname);
extern void sfo_linfo(NHFILE *nhfp, struct linfo *d_linfo,
                           const char *myname);
extern void sfo_nhcoord(NHFILE *nhfp, struct nhcoord *d_nhcoord,
                             const char *myname);
extern void sfo_d_level(NHFILE *nhfp, struct d_level *d_d_level,
                           const char *myname);
extern void sfo_mapseen_feat(NHFILE *nhfp,
                                  struct mapseen_feat *d_mapseen_feat,
                                  const char *myname);
extern void sfo_mapseen_flags(NHFILE *nhfp,
                                   struct mapseen_flags *d_mapseen_flags,
                                   const char *myname);
extern void sfo_mapseen_rooms(NHFILE *nhfp,
                                   struct mapseen_rooms *d_mapseen_rooms,
                                   const char *myname);
extern void sfo_kinfo(NHFILE *nhfp,
                           struct kinfo *d_kinfo,
                           const char *myname);
extern void sfo_engr(NHFILE *, struct engr *, const char *);
extern void sfo_ls_t(NHFILE *, struct ls_t *, const char *);
extern void sfo_bubble(NHFILE *, struct bubble *, const char *);
extern void sfo_mkroom(NHFILE *, struct mkroom *, const char *);
extern void sfo_objclass(NHFILE *, struct objclass *, const char *);
extern void sfo_nhrect(NHFILE *, struct nhrect *, const char *);
extern void sfo_fe(NHFILE *, struct fe *, const char *);
extern void sfo_version_info(NHFILE *, struct version_info *,
                                  const char *);
extern void sfo_context_info(NHFILE *, struct context_info *,
                                  const char *);
extern void sfo_flag(NHFILE *, struct flag *, const char *);
extern void sfo_you(NHFILE *, struct you *, const char *);
extern void sfo_mvitals(NHFILE *, struct mvitals *, const char *);
extern void sfo_q_score(NHFILE *, struct q_score *, const char *);
extern void sfo_spell(NHFILE *, struct spell *, const char *);
extern void sfo_dest_area(NHFILE *, struct dest_area *, const char *);
extern void sfo_levelflags(NHFILE *, struct levelflags *, const char *);
extern void sfo_rm(NHFILE *, struct rm *, const char *);
extern void sfo_cemetery(NHFILE *, struct cemetery *, const char *);
extern void sfo_damage(NHFILE *, struct damage *, const char *);
extern void sfo_stairway(NHFILE *, struct stairway *, const char *);
extern void sfo_obj(NHFILE *, struct obj *, const char *);
extern void sfo_monst(NHFILE *, struct monst *, const char *);
extern void sfo_ebones(NHFILE *, struct ebones *, const char *);
extern void sfo_edog(NHFILE *, struct edog *, const char *);
extern void sfo_egd(NHFILE *, struct egd *, const char *);
extern void sfo_emin(NHFILE *, struct emin *, const char *);
extern void sfo_engr(NHFILE *, struct engr *, const char *);
extern void sfo_epri(NHFILE *, struct epri *, const char *);
extern void sfo_eshk(NHFILE *, struct eshk *, const char *);
extern void sfo_trap(NHFILE *, struct trap *, const char *);
extern void sfo_gamelog_line(NHFILE *, struct gamelog_line *, const char *);
extern void sfo_fruit(NHFILE *, struct fruit *, const char *);
extern void sfo_s_level(NHFILE *, struct s_level *, const char *);
extern void sfo_xint8(NHFILE *, xint8 *, const char *);
extern void sfo_xint16(NHFILE *, xint16 *, const char *);
extern void sfo_schar(NHFILE *, schar *, const char *);
extern void sfo_short(NHFILE *, short *, const char *);
extern void sfo_ushort(NHFILE *, ushort *, const char *);
extern void sfo_int(NHFILE *, int *, const char *);
extern void sfo_unsigned(NHFILE *, unsigned *, const char *);
extern void sfo_long(NHFILE *, long *, const char *);
extern void sfo_ulong(NHFILE *, ulong *, const char *);
/** @} */

/**
 * @name Reading
 * @brief The counterpart of each writing routine, reading back what it wrote.
 * @note @c sfi_addinfo is the exception in the group: it does not read a value but records descriptive information about
 *       what is being read, for tracing.
 * @warning Reading fewer or more fields than the writer wrote does not fail. It leaves the file position wrong and
 *          everything after it is read as something else.
 * @{
 */
/**
 * @name 읽기
 * @brief 각 쓰기 루틴의 짝. 그것이 쓴 것을 되읽는다.
 * @note @c sfi_addinfo 는 이 묶음의 예외다. 값을 읽는 것이 아니라 무엇을 읽고 있는지에 대한 설명 정보를 추적용으로 기록한다.
 * @warning 쓰는 쪽이 쓴 것보다 적게나 많게 읽는 것은 실패하지 않는다. 파일 위치를 틀리게 남기고, 그 뒤의 모든 것이 다른 것으로 읽힌다.
 * @{
 */
/* sfbase.c input functions */
extern void sfi_addinfo(NHFILE *, const char *, const char *);
extern void sfi_aligntyp(NHFILE *, aligntyp *, const char *);
extern void sfi_any(NHFILE *, anything *, const char *);
extern void sfi_boolean(NHFILE *, boolean *, const char *);
extern void sfi_genericptr(NHFILE *, genericptr_t *, const char *);
extern void sfi_char(NHFILE *, char *, const char *, int);
extern void sfi_int16(NHFILE *, int16 *, const char *);
extern void sfi_int32(NHFILE *, int32 *, const char *);
extern void sfi_int64(NHFILE *, int64 *, const char *);
extern void sfi_uchar(NHFILE *, uchar *, const char *);
extern void sfi_uint16(NHFILE *, uint16 *, const char *);
extern void sfi_uint32(NHFILE *, uint32 *, const char *);
extern void sfi_uint64(NHFILE *, uint64 *, const char *);
extern void sfi_size_t(NHFILE *, size_t *, const char *);
extern void sfi_time_t(NHFILE *, time_t *, const char *);
extern void sfi_arti_info(NHFILE *nhfp,
                                  struct arti_info *d_arti_info,
                                  const char *myname);
extern void sfi_dungeon(NHFILE *nhfp, struct dungeon *d_dungeon,
                             const char *myname);
extern void sfi_dgn_topology(NHFILE *nhfp,
                                  struct dgn_topology *d_dgn_topology,
                                  const char *myname);
extern void sfi_branch(NHFILE *nhfp, struct branch *d_branch,
                            const char *myname);
extern void sfi_linfo(NHFILE *nhfp, struct linfo *d_linfo,
                         const char *myname);
extern void sfi_nhcoord(NHFILE *nhfp, struct nhcoord *d_nhcoord,
                           const char *myname);
extern void sfi_d_level(NHFILE *nhfp, struct d_level *d_d_level,
                             const char *myname);
extern void sfi_mapseen_feat(NHFILE *nhfp,
                                  struct mapseen_feat *d_mapseen_feat,
                                  const char *myname);
extern void sfi_mapseen_flags(NHFILE *nhfp,
                                   struct mapseen_flags *d_mapseen_flags,
                                   const char *myname);
extern void sfi_mapseen_rooms(NHFILE *nhfp,
                                   struct mapseen_rooms *d_mapseen_rooms,
                                   const char *myname);
extern void sfi_kinfo(NHFILE *nhfp,
                           struct kinfo *d_kinfo,
                           const char *myname);
extern void sfi_engr(NHFILE *, struct engr *, const char *);
extern void sfi_ls_t(NHFILE *, struct ls_t *, const char *);
extern void sfi_bubble(NHFILE *, struct bubble *, const char *);
extern void sfi_mkroom(NHFILE *, struct mkroom *, const char *);
extern void sfi_objclass(NHFILE *, struct objclass *, const char *);
extern void sfi_nhrect(NHFILE *, struct nhrect *, const char *);
extern void sfi_fe(NHFILE *, struct fe *, const char *);
extern void sfi_version_info(NHFILE *, struct version_info *,
                                  const char *);
extern void sfi_context_info(NHFILE *, struct context_info *,
                                  const char *);
extern void sfi_flag(NHFILE *, struct flag *, const char *);
extern void sfi_you(NHFILE *, struct you *, const char *);
extern void sfi_mvitals(NHFILE *, struct mvitals *, const char *);
extern void sfi_q_score(NHFILE *, struct q_score *, const char *);
extern void sfi_spell(NHFILE *, struct spell *, const char *);
extern void sfi_dest_area(NHFILE *, struct dest_area *, const char *);
extern void sfi_levelflags(NHFILE *, struct levelflags *, const char *);
extern void sfi_rm(NHFILE *, struct rm *, const char *);
extern void sfi_cemetery(NHFILE *, struct cemetery *, const char *);
extern void sfi_damage(NHFILE *, struct damage *, const char *);
extern void sfi_stairway(NHFILE *, struct stairway *, const char *);
extern void sfi_obj(NHFILE *, struct obj *, const char *);
extern void sfi_monst(NHFILE *, struct monst *, const char *);
extern void sfi_ebones(NHFILE *, struct ebones *, const char *);
extern void sfi_edog(NHFILE *, struct edog *, const char *);
extern void sfi_egd(NHFILE *, struct egd *, const char *);
extern void sfi_emin(NHFILE *, struct emin *, const char *);
extern void sfi_engr(NHFILE *, struct engr *, const char *);
extern void sfi_epri(NHFILE *, struct epri *, const char *);
extern void sfi_eshk(NHFILE *, struct eshk *, const char *);
extern void sfi_trap(NHFILE *, struct trap *, const char *);
extern void sfi_fruit(NHFILE *, struct fruit *, const char *);
extern void sfi_gamelog_line(NHFILE *, struct gamelog_line *, const char *);
extern void sfi_s_level(NHFILE *, struct s_level *, const char *);
extern void sfi_xint8(NHFILE *, xint8 *, const char *);
extern void sfi_xint16(NHFILE *, xint16 *, const char *);
extern void sfi_schar(NHFILE *, schar *, const char *);
extern void sfi_short(NHFILE *, short *, const char *);
extern void sfi_ushort(NHFILE *, ushort *, const char *);
extern void sfi_int(NHFILE *, int *, const char *);
extern void sfi_unsigned(NHFILE *, unsigned *, const char *);
extern void sfi_long(NHFILE *, long *, const char *);
extern void sfi_ulong(NHFILE *, ulong *, const char *);
#ifdef DEMO_UPLIFTS
extern void sfi_mystruct(NHFILE *, struct mystruct *, const char *);
extern void sfi_mystruct_rev0(NHFILE *, struct mystruct_rev0 *, const char *);
#endif
/** @} */

/**
 * @name Named forms for older compilers
 * @brief The capitalised names, spelled out one per type.
 *
 * Where the compiler cannot select a routine by argument type, each capitalised name is simply defined as the call it would
 * have selected. This block and the type-directed selection below it are two implementations of the same interface, and
 * calling code cannot tell which it is using.
 *
 * @warning The two must be kept in agreement. A type added to only one of them compiles under one compiler and fails under
 *          the other, so it will not necessarily be noticed locally.
 * @{
 */
/**
 * @name 예전 컴파일러를 위한 이름 형태
 * @brief 대문자로 시작하는 이름들. 타입마다 하나씩 적어 낸 것.
 *
 * 컴파일러가 인자 타입으로 루틴을 고를 수 없는 곳에서는, 각 대문자 이름이 그것이 골랐을 호출로 그냥 정의된다. 이 블록과 그 아래의 타입 기반 선택은 같은 인터페이스의 두 구현이며, 호출하는 코드는
 * 자신이 어느 것을 쓰고 있는지 알 수 없다.
 *
 * @warning 둘은 일치하도록 유지되어야 한다. 한쪽에만 더한 타입은 한 컴파일러에서는 컴파일되고 다른 컴파일러에서는 실패하므로, 로컬에서 반드시 발견되지는 않는다.
 * @{
 */
#if NH_C < 202300L
#define Sfo_aligntyp(a,b,c) sfo_aligntyp(a, b, c)
#define Sfo_any(a,b,c) sfo_any(a, b, c)
#define Sfo_genericptr(a,b,c) sfo_genericptr(a, b, c)
#define Sfo_coordxy(a,b,c) sfo_int16(a, b, c)
#define Sfo_char(a,b,c,d) sfo_char(a, b, c, d)
#define Sfo_int16(a,b,c) sfo_int16(a, b, c)
#define Sfo_int32(a,b,c) sfo_int32(a, b, c)
#define Sfo_int64(a,b,c) sfo_int64(a, b, c)
#define Sfo_uchar(a,b,c) sfo_uchar(a, b, c)
#define Sfo_uint16(a,b,c) sfo_uint16(a, b, c)
#define Sfo_uint32(a,b,c) sfo_uint32(a, b, c)
#define Sfo_uint64(a,b,c) sfo_uint64(a, b, c)
#define Sfo_size_t(a,b,c) sfo_size_t(a, b, c)
#define Sfo_time_t(a,b,c) sfo_time_t(a, b, c)
#define Sfo_str(a,b,c) sfo_str(a, b, c)
#define Sfo_arti_info(a,b,c) sfo_arti_info(a, b, c)
#define Sfo_dgn_topology(a,b,c) sfo_dgn_topology(a, b, c)
#define Sfo_dungeon(a,b,c) sfo_dungeon(a, b, c)
#define Sfo_branch(a,b,c) sfo_branch(a, b, c)
#define Sfo_linfo(a,b,c) sfo_linfo(a, b, c)
#define Sfo_nhcoord(a,b,c) sfo_nhcoord(a, b, c)
#define Sfo_d_level(a,b,c) sfo_d_level(a, b, c)
#define Sfo_mapseen_feat(a,b,c) sfo_mapseen_feat(a, b, c)
#define Sfo_mapseen_flags(a,b,c) sfo_mapseen_flags(a, b, c)
#define Sfo_mapseen_rooms(a,b,c) sfo_mapseen_rooms(a, b, c)
#define Sfo_kinfo(a,b,c) sfo_kinfo(a, b, c)
#define Sfo_engr(a,b,c) sfo_engr(a, b, c)
#define Sfo_ls_t(a,b,c) sfo_ls_t(a, b, c)
#define Sfo_bubble(a,b,c) sfo_bubble(a, b, c)
#define Sfo_mkroom(a,b,c) sfo_mkroom(a, b, c)
#define Sfo_objclass(a,b,c) sfo_objclass(a, b, c)
#define Sfo_nhrect(a,b,c) sfo_nhrect(a, b, c)
#define Sfo_fe(a,b,c) sfo_fe(a, b, c)
#define Sfo_version_info(a,b,c) sfo_version_info(a, b, c)
#define Sfo_context_info(a,b,c) sfo_context_info(a, b, c)
#define Sfo_flag(a,b,c) sfo_flag(a, b, c)
#define Sfo_you(a,b,c) sfo_you(a, b, c)
#define Sfo_mvitals(a,b,c) sfo_mvitals(a, b, c)
#define Sfo_q_score(a,b,c) sfo_q_score(a, b, c)
#define Sfo_spell(a,b,c) sfo_spell(a, b, c)
#define Sfo_dest_area(a,b,c) sfo_dest_area(a, b, c)
#define Sfo_levelflags(a,b,c) sfo_levelflags(a, b, c)
#define Sfo_rm(a,b,c) sfo_rm(a, b, c)
#define Sfo_cemetery(a,b,c) sfo_cemetery(a, b, c)
#define Sfo_damage(a,b,c) sfo_damage(a, b, c)
#define Sfo_stairway(a,b,c) sfo_stairway(a, b, c)
#define Sfo_obj(a,b,c) sfo_obj(a, b, c)
#define Sfo_monst(a,b,c) sfo_monst(a, b, c)
#define Sfo_ebones(a,b,c) sfo_ebones(a, b, c)
#define Sfo_edog(a,b,c) sfo_edog(a, b, c)
#define Sfo_egd(a,b,c) sfo_egd(a, b, c)
#define Sfo_emin(a,b,c) sfo_emin(a, b, c)
#define Sfo_engr(a,b,c) sfo_engr(a, b, c)
#define Sfo_epri(a,b,c) sfo_epri(a, b, c)
#define Sfo_eshk(a,b,c) sfo_eshk(a, b, c)
#define Sfo_trap(a,b,c) sfo_trap(a, b, c)
#define Sfo_gamelog_line(a,b,c) sfo_gamelog_line(a, b, c)
#define Sfo_fruit(a,b,c) sfo_fruit(a, b, c)
#define Sfo_s_level(a,b,c) sfo_s_level(a, b, c)
#define Sfo_short(a, b, c) sfo_short(a, b, c)
#define Sfo_ushort(a, b, c) sfo_ushort(a, b, c)
#define Sfo_int(a, b, c) sfo_int(a, b, c)
#define Sfo_unsigned(a, b, c) sfo_unsigned(a, b, c)
#define Sfo_xint8(a, b, c) sfo_xint8(a, b, c);
#define Sfo_xint16(a, b, c) sfo_xint16(a, b, c)
/* sfbase.c input functions */
#define Sfi_addinfo(a,b,c) sfi_addinfo(a, b, c)
#define Sfi_aligntyp(a,b,c) sfi_aligntyp(a, b, c)
#define Sfi_any(a,b,c) sfi_any(a, b, c)
#define Sfi_genericptr(a,b,c) sfi_genericptr(a, b, c)
#define Sfi_coordxy(a,b,c) sfi_int16(a, b, c)
#define Sfi_int16(a,b,c) sfi_int16(a, b, c)
#define Sfi_int32(a,b,c) sfi_int32(a, b, c)
#define Sfi_int64(a,b,c) sfi_int64(a, b, c)
#define Sfi_uchar(a,b,c) sfi_uchar(a, b, c)
#define Sfi_uint16(a,b,c) sfi_uint16(a, b, c)
#define Sfi_uint32(a,b,c) sfi_uint32(a, b, c)
#define Sfi_uint64(a,b,c) sfi_uint64(a, b, c)
#define Sfi_size_t(a,b,c) sfi_size_t(a, b, c)
#define Sfi_time_t(a,b,c) sfi_time_t(a, b, c)
#define Sfi_arti_info(a,b,c) sfi_arti_info(a, b, c)
#define Sfi_dungeon(a,b,c) sfi_dungeon(a, b, c)
#define Sfi_dgn_topology(a,b,c) sfi_dgn_topology(a, b, c)
#define Sfi_branch(a,b,c) sfi_branch(a, b, c)
#define Sfi_linfo(a,b,c) sfi_linfo(a, b, c)
#define Sfi_nhcoord(a,b,c) sfi_nhcoord(a, b, c)
#define Sfi_d_level(a,b,c) sfi_d_level(a, b, c)
#define Sfi_mapseen_feat(a,b,c) sfi_mapseen_feat(a, b, c)
#define Sfi_mapseen_flags(a,b,c) sfi_mapseen_flags(a, b, c)
#define Sfi_mapseen_rooms(a,b,c) sfi_mapseen_rooms(a, b, c)
#define Sfi_kinfo(a,b,c) sfi_kinfo(a, b, c)
#define Sfi_engr(a,b,c) sfi_engr(a, b, c)
#define Sfi_ls_t(a,b,c) sfi_ls_t(a, b, c)
#define Sfi_bubble(a,b,c) sfi_bubble(a, b, c)
#define Sfi_mkroom(a,b,c) sfi_mkroom(a, b, c)
#define Sfi_objclass(a,b,c) sfi_objclass(a, b, c)
#define Sfi_nhrect(a,b,c) sfi_nhrect(a, b, c)
#define Sfi_fe(a,b,c) sfi_fe(a, b, c)
#define Sfi_version_info(a,b,c) sfi_version_info(a, b, c)
#define Sfi_context_info(a,b,c) sfi_context_info(a, b, c)
#define Sfi_flag(a,b,c) sfi_flag(a, b, c)
#define Sfi_you(a,b,c) sfi_you(a, b, c)
#define Sfi_mvitals(a,b,c) sfi_mvitals(a, b, c)
#define Sfi_q_score(a,b,c) sfi_q_score(a, b, c)
#define Sfi_spell(a,b,c) sfi_spell(a, b, c)
#define Sfi_dest_area(a,b,c) sfi_dest_area(a, b, c)
#define Sfi_levelflags(a,b,c) sfi_levelflags(a, b, c)
#define Sfi_rm(a,b,c) sfi_rm(a, b, c)
#define Sfi_cemetery(a,b,c) sfi_cemetery(a, b, c)
#define Sfi_damage(a,b,c) sfi_damage(a, b, c)
#define Sfi_stairway(a,b,c) sfi_stairway(a, b, c)
#define Sfi_obj(a,b,c) sfi_obj(a, b, c)
#define Sfi_monst(a,b,c) sfi_monst(a, b, c)
#define Sfi_ebones(a,b,c) sfi_ebones(a, b, c)
#define Sfi_edog(a,b,c) sfi_edog(a, b, c)
#define Sfi_egd(a,b,c) sfi_egd(a, b, c)
#define Sfi_emin(a,b,c) sfi_emin(a, b, c)
#define Sfi_engr(a,b,c) sfi_engr(a, b, c)
#define Sfi_epri(a,b,c) sfi_epri(a, b, c)
#define Sfi_eshk(a,b,c) sfi_eshk(a, b, c)
#define Sfi_trap(a,b,c) sfi_trap(a, b, c)
#define Sfi_fruit(a,b,c) sfi_fruit(a, b, c)
#define Sfi_gamelog_line(a,b,c) sfi_gamelog_line(a, b, c)
#define Sfi_s_level(a,b,c) sfi_s_level(a, b, c)
#define Sfi_short(a, b, c) sfi_short(a, b, c)
#define Sfi_ushort(a, b, c) sfi_ushort(a, b, c)
#define Sfi_int(a, b, c) sfi_int(a, b, c);
#define Sfi_unsigned(a, b, c) sfi_unsigned(a, b, c);
#define Sfi_xint8(a, b, c) sfi_xint8(a, b, c);
#define Sfi_xint16(a, b, c) sfi_xint16(a, b, c);
#ifdef DEMO_UPLIFTS
#define Sfi_mystruct(a, b, c) sfi_mystruct(a, b, c)
#define Sfi_mystruct_rev0(a, b, c) sfi_mystruct_rev0(a, b, c)
#endif
/** @} */
#else

/**
 * @def sfo
 * @brief Write anything, letting the compiler choose the routine from the argument's type.
 * @param nhfp the file being written
 * @param dt a pointer to the thing to write; its type selects the routine
 * @param tag the field's name, for tracing
 * @note This is what makes the capitalised names above a single mechanism rather than a list. Each capitalised name expands
 *       to this, so a call site names no type at all.
 * @warning A type absent from the list is a compile error rather than a wrong call, which is the one place in this file
 *          where an omission is caught for you.
 */
/**
 * @def sfo
 * @brief 무엇이든 쓴다. 컴파일러가 인자의 타입에서 루틴을 고르게 한다.
 * @param nhfp 기록 중인 파일
 * @param dt 쓸 것을 가리키는 포인터. 그 타입이 루틴을 고른다
 * @param tag 필드의 이름. 추적용
 * @note 이것이 위의 대문자 이름들을 목록이 아니라 하나의 기제로 만드는 것이다. 각 대문자 이름이 이것으로 펼쳐지므로, 호출 지점은 어떤 타입도 지칭하지 않는다.
 * @warning 목록에 없는 타입은 잘못된 호출이 아니라 컴파일 오류다. 이 파일에서 빠뜨림이 대신 잡히는 유일한 곳이다.
 */
#define sfo(nhfp, dt, tag)                     \
  _Generic( (dt),                              \
    anything *            : sfo_any,           \
    int16_t *             : sfo_int16,         \
    int32_t *             : sfo_int32,         \
    int64_t *             : sfo_int64,         \
    uchar *               : sfo_uchar,         \
    uint16_t *            : sfo_uint16,        \
    uint32_t *            : sfo_uint32,        \
    uint64_t *            : sfo_uint64,        \
    xint8 *               : sfo_xint8,         \
    struct arti_info *    : sfo_arti_info,     \
    struct nhrect *       : sfo_nhrect,        \
    struct branch *       : sfo_branch,        \
    struct bubble *       : sfo_bubble,        \
    struct cemetery *     : sfo_cemetery,      \
    struct context_info * : sfo_context_info,  \
    coord *               : sfo_nhcoord,       \
    struct damage *       : sfo_damage,        \
    struct dgn_topology * : sfo_dgn_topology,  \
    dungeon *             : sfo_dungeon,       \
    d_level *             : sfo_d_level,       \
    struct levelflags *   : sfo_levelflags,    \
    light_source *        : sfo_ls_t,          \
    struct dest_area *    : sfo_dest_area,     \
    struct ebones *       : sfo_ebones,        \
    struct edog *         : sfo_edog,          \
    struct egd *          : sfo_egd,           \
    struct emin *         : sfo_emin,          \
    struct engr *         : sfo_engr,          \
    struct epri *         : sfo_epri,          \
    struct eshk *         : sfo_eshk,          \
    struct fe *           : sfo_fe,            \
    struct flag *         : sfo_flag,          \
    struct fruit *        : sfo_fruit,         \
    struct gamelog_line * : sfo_gamelog_line,  \
    struct kinfo *        : sfo_kinfo,         \
    struct linfo *        : sfo_linfo,         \
    struct mapseen_feat * : sfo_mapseen_feat,  \
    struct mapseen_flags *: sfo_mapseen_flags, \
    struct mapseen_rooms *: sfo_mapseen_rooms, \
    struct mkroom *       : sfo_mkroom,        \
    struct monst *        : sfo_monst,         \
    struct mvitals *      : sfo_mvitals,       \
    struct obj *          : sfo_obj,           \
    struct objclass *     : sfo_objclass,      \
    struct q_score *      : sfo_q_score,       \
    struct rm *           : sfo_rm,            \
    struct spell *        : sfo_spell,         \
    struct stairway *     : sfo_stairway,      \
    struct s_level *      : sfo_s_level,       \
    struct trap *         : sfo_trap,          \
    struct version_info * : sfo_version_info,  \
    struct you *          : sfo_you            \
  ) (nhfp, dt, tag)

/*    struct container * : sfo_container, */
/*    struct mapseen *   : sfo_mapseen,   */
/*    struct mextra *    : sfo_mextra,    */
/*    struct oextra *    : sfo_oextra,    */
/*    struct permonst *  : sfo_permonst,  */

/**
 * @def sfi
 * @brief Read anything, letting the compiler choose the routine from the argument's type.
 * @param nhfp the file being read
 * @param dt a pointer to the thing to fill in; its type selects the routine
 * @param tag the field's name, for tracing
 * @warning Must list exactly the same types in the same manner as the writing counterpart. A type present in one selection
 *          and absent from the other means saving works and restoring does not compile, or the reverse.
 */
/**
 * @def sfi
 * @brief 무엇이든 읽는다. 컴파일러가 인자의 타입에서 루틴을 고르게 한다.
 * @param nhfp 읽고 있는 파일
 * @param dt 채울 것을 가리키는 포인터. 그 타입이 루틴을 고른다
 * @param tag 필드의 이름. 추적용
 * @warning 쓰기 짝과 정확히 같은 타입을 같은 방식으로 나열해야 한다. 한 선택에는 있고 다른 선택에는 없는 타입은 저장은 되고 복원은 컴파일되지 않거나, 그 반대를 뜻한다.
 */
#define sfi(nhfp, dt, tag)                     \
  _Generic( (dt),                              \
    anything *            : sfi_any,           \
    int16_t *             : sfi_int16,         \
    int32_t *             : sfi_int32,         \
    int64_t *             : sfi_int64,         \
    uchar *               : sfi_uchar,         \
    uint16_t *            : sfi_uint16,        \
    uint32_t *            : sfi_uint32,        \
    uint64_t *            : sfi_uint64,        \
    xint8 *               : sfi_xint8,         \
    struct arti_info *    : sfi_arti_info,     \
    struct nhrect *       : sfi_nhrect,        \
    struct branch *       : sfi_branch,        \
    struct bubble *       : sfi_bubble,        \
    struct cemetery *     : sfi_cemetery,      \
    struct context_info * : sfi_context_info,  \
    coord *               : sfi_nhcoord,       \
    struct damage *       : sfi_damage,        \
    struct dgn_topology * : sfi_dgn_topology,  \
    dungeon *             : sfi_dungeon,       \
    d_level *             : sfi_d_level,       \
    struct levelflags *   : sfi_levelflags,    \
    light_source *        : sfi_ls_t,          \
    struct dest_area *    : sfi_dest_area,     \
    struct ebones *       : sfi_ebones,        \
    struct edog *         : sfi_edog,          \
    struct egd *          : sfi_egd,           \
    struct emin *         : sfi_emin,          \
    struct engr *         : sfi_engr,          \
    struct epri *         : sfi_epri,          \
    struct eshk *         : sfi_eshk,          \
    struct fe *           : sfi_fe,            \
    struct flag *         : sfi_flag,          \
    struct fruit *        : sfi_fruit,         \
    struct gamelog_line * : sfi_gamelog_line,  \
    struct kinfo *        : sfi_kinfo,         \
    struct linfo *        : sfi_linfo,         \
    struct mapseen_feat * : sfi_mapseen_feat,  \
    struct mapseen_flags *: sfi_mapseen_flags, \
    struct mapseen_rooms *: sfi_mapseen_rooms, \
    struct mkroom *       : sfi_mkroom,        \
    struct monst *        : sfi_monst,         \
    struct mvitals *      : sfi_mvitals,       \
    struct obj *          : sfi_obj,           \
    struct objclass *     : sfi_objclass,      \
    struct q_score *      : sfi_q_score,       \
    struct rm *           : sfi_rm,            \
    struct spell *        : sfi_spell,         \
    struct stairway *     : sfi_stairway,      \
    struct s_level *      : sfi_s_level,       \
    struct trap *         : sfi_trap,          \
    struct version_info * : sfi_version_info,  \
    struct you *          : sfi_you            \
  ) (nhfp, dt, tag)

/*    char *                : sfo_char,   */
/*    char *                : sfi_char,   */
/*    struct container * : sfi_container, */
/*    struct mapseen *   : sfi_mapseen,   */
/*    struct mextra *    : sfi_mextra,    */
/*    struct oextra *    : sfi_oextra,    */
/*    struct permonst *  : sfi_permonst,  */

#define Sfo_any(a,b,c) sfo(a, b, c)
#define Sfo_aligntyp(a,b,c) sfo(a, b, c)
#define Sfo_genericptr(a,b,c) sfo(a, b, c)
#define Sfo_coordxy(a,b,c) sfo(a, b, c)
#define Sfo_int16(a,b,c) sfo(a, b, c)
#define Sfo_int32(a,b,c) sfo(a, b, c)
#define Sfo_int64(a,b,c) sfo(a, b, c)
#define Sfo_uchar(a,b,c) sfo(a, b, c)
#define Sfo_unsigned(a,b,c) sfo(a, b, c)
#define Sfo_uchar(a,b,c) sfo(a, b, c)
#define Sfo_uint16(a,b,c) sfo(a, b, c)
#define Sfo_uint32(a,b,c) sfo(a, b, c)
#define Sfo_uint64(a,b,c) sfo(a, b, c)
#define Sfo_size_t(a,b,c) sfo(a, b, c)
#define Sfo_time_t(a,b,c) sfo(a, b, c)
#define Sfo_str(a,b,c) sfo(a, b, c)
#define Sfo_arti_info(a,b,c) sfo(a, b, c)
#define Sfo_dgn_topology(a,b,c) sfo(a, b, c)
#define Sfo_dungeon(a,b,c) sfo(a, b, c)
#define Sfo_branch(a,b,c) sfo(a, b, c)
#define Sfo_linfo(a,b,c) sfo(a, b, c)
#define Sfo_nhcoord(a,b,c) sfo(a, b, c)
#define Sfo_d_level(a,b,c) sfo(a, b, c)
#define Sfo_mapseen_feat(a,b,c) sfo(a, b, c)
#define Sfo_mapseen_flags(a,b,c) sfo(a, b, c)
#define Sfo_mapseen_rooms(a,b,c) sfo(a, b, c)
#define Sfo_kinfo(a,b,c) sfo(a, b, c)
#define Sfo_engr(a,b,c) sfo(a, b, c)
#define Sfo_ls_t(a,b,c) sfo(a, b, c)
#define Sfo_bubble(a,b,c) sfo(a, b, c)
#define Sfo_mkroom(a,b,c) sfo(a, b, c)
#define Sfo_objclass(a,b,c) sfo(a, b, c)
#define Sfo_nhrect(a,b,c) sfo(a, b, c)
#define Sfo_fe(a,b,c) sfo(a, b, c)
#define Sfo_version_info(a,b,c) sfo(a, b, c)
#define Sfo_context_info(a,b,c) sfo(a, b, c)
#define Sfo_flag(a,b,c) sfo(a, b, c)
#define Sfo_you(a,b,c) sfo(a, b, c)
#define Sfo_mvitals(a,b,c) sfo(a, b, c)
#define Sfo_q_score(a,b,c) sfo(a, b, c)
#define Sfo_spell(a,b,c) sfo(a, b, c)
#define Sfo_dest_area(a,b,c) sfo(a, b, c)
#define Sfo_levelflags(a,b,c) sfo(a, b, c)
#define Sfo_rm(a,b,c) sfo(a, b, c)
#define Sfo_cemetery(a,b,c) sfo(a, b, c)
#define Sfo_damage(a,b,c) sfo(a, b, c)
#define Sfo_stairway(a,b,c) sfo(a, b, c)
#define Sfo_obj(a,b,c) sfo(a, b, c)
#define Sfo_monst(a,b,c) sfo(a, b, c)
#define Sfo_ebones(a,b,c) sfo(a, b, c)
#define Sfo_edog(a,b,c) sfo(a, b, c)
#define Sfo_egd(a,b,c) sfo(a, b, c)
#define Sfo_emin(a,b,c) sfo(a, b, c)
#define Sfo_engr(a,b,c) sfo(a, b, c)
#define Sfo_epri(a,b,c) sfo(a, b, c)
#define Sfo_eshk(a,b,c) sfo(a, b, c)
#define Sfo_trap(a,b,c) sfo(a, b, c)
#define Sfo_gamelog_line(a,b,c) sfo(a, b, c)
#define Sfo_fruit(a,b,c) sfo(a, b, c)
#define Sfo_s_level(a,b,c) sfo(a, b, c)
#define Sfo_short(a, b, c) sfo(a, b, c)
#define Sfo_ushort(a, b, c) sfo(a, b, c)
#define Sfo_int(a, b, c) sfo(a, b, c)
#define Sfo_unsigned(a, b, c) sfo(a, b, c)
#define Sfo_xint8(a, b, c) sfo(a, b, c)
#define Sfo_xint16(a, b, c) sfo(a, b, c)

/* sfbase.c input functions */
#define Sfi_addinfo(a,b,c) sfi(a, b, c)
#define Sfi_aligntyp(a,b,c) sfi(a, b, c)
#define Sfi_any(a,b,c) sfi(a, b, c)
#define Sfi_genericptr(a,b,c) sfi(a, b, c)
#define Sfi_coordxy(a,b,c) sfi(a, b, c)
#define Sfi_int16(a,b,c) sfi(a, b, c)
#define Sfi_int32(a,b,c) sfi(a, b, c)
#define Sfi_int64(a,b,c) sfi(a, b, c)
#define Sfi_uchar(a,b,c) sfi(a, b, c)
#define Sfi_uint16(a,b,c) sfi(a, b, c)
#define Sfi_uint32(a,b,c) sfi(a, b, c)
#define Sfi_uint64(a,b,c) sfi(a, b, c)
#define Sfi_size_t(a,b,c) sfi(a, b, c)
#define Sfi_time_t(a,b,c) sfi(a, b, c)
#define Sfi_arti_info(a,b,c) sfi(a, b, c)
#define Sfi_dungeon(a,b,c) sfi(a, b, c)
#define Sfi_dgn_topology(a,b,c) sfi(a, b, c)
#define Sfi_branch(a,b,c) sfi(a, b, c)
#define Sfi_linfo(a,b,c) sfi(a, b, c)
#define Sfi_nhcoord(a,b,c) sfi(a, b, c)
#define Sfi_d_level(a,b,c) sfi(a, b, c)
#define Sfi_mapseen_feat(a,b,c) sfi(a, b, c)
#define Sfi_mapseen_flags(a,b,c) sfi(a, b, c)
#define Sfi_mapseen_rooms(a,b,c) sfi(a, b, c)
#define Sfi_kinfo(a,b,c) sfi(a, b, c)
#define Sfi_engr(a,b,c) sfi(a, b, c)
#define Sfi_ls_t(a,b,c) sfi(a, b, c)
#define Sfi_bubble(a,b,c) sfi(a, b, c)
#define Sfi_mkroom(a,b,c) sfi(a, b, c)
#define Sfi_objclass(a,b,c) sfi(a, b, c)
#define Sfi_nhrect(a,b,c) sfi(a, b, c)
#define Sfi_fe(a,b,c) sfi(a, b, c)
#define Sfi_version_info(a,b,c) sfi(a, b, c)
#define Sfi_context_info(a,b,c) sfi(a, b, c)
#define Sfi_flag(a,b,c) sfi(a, b, c)
#define Sfi_you(a,b,c) sfi(a, b, c)
#define Sfi_mvitals(a,b,c) sfi(a, b, c)
#define Sfi_q_score(a,b,c) sfi(a, b, c)
#define Sfi_spell(a,b,c) sfi(a, b, c)
#define Sfi_dest_area(a,b,c) sfi(a, b, c)
#define Sfi_levelflags(a,b,c) sfi(a, b, c)
#define Sfi_rm(a,b,c) sfi(a, b, c)
#define Sfi_cemetery(a,b,c) sfi_cemetery(a, b, c)
#define Sfi_damage(a,b,c) sfi(a, b, c)
#define Sfi_stairway(a,b,c) sfi(a, b, c)
#define Sfi_obj(a,b,c) sfi(a, b, c)
#define Sfi_monst(a,b,c) sfi(a, b, c)
#define Sfi_ebones(a,b,c) sfi(a, b, c)
#define Sfi_edog(a,b,c) sfi(a, b, c)
#define Sfi_egd(a,b,c) sfi(a, b, c)
#define Sfi_emin(a,b,c) sfi(a, b, c)
#define Sfi_engr(a,b,c) sfi(a, b, c)
#define Sfi_epri(a,b,c) sfi(a, b, c)
#define Sfi_eshk(a,b,c) sfi(a, b, c)
#define Sfi_trap(a,b,c) sfi(a, b, c)
#define Sfi_fruit(a,b,c) sfi(a, b, c)
#define Sfi_gamelog_line(a,b,c) sfi(a, b, c)
#define Sfi_s_level(a,b,c) sfi(a, b, c)
#define Sfi_short(a, b, c) sfi(a, b, c)
#define Sfi_ushort(a, b, c) sfi(a, b, c)
#define Sfi_int(a,b,c) sfi(a, b, c)
#define Sfi_unsigned(a, b, c) sfi(a, b, c)
#define Sfi_xint8(a, b, c) sfi(a, b, c)
#define Sfi_xint16(a, b, c) sfi(a, b, c)
#ifdef DEMO_UPLIFTS
#define Sfi_mystruct(a, b, c) sfi(a, b, c)
#define Sfi_mystruct_rev0(a, b, c) sfi(a, b, c)
#endif
#endif

/**
 * @name Types called by name always
 * @brief Types the type-directed selection cannot handle, so both compilers call these directly.
 *
 * Some of these the selection genuinely cannot distinguish: a plain character, a signed character and a boolean may all be
 * the same underlying type, so no selection could tell them apart. The character routines also take an extra length
 * argument, which does not fit the shape of the others.
 *
 * @note Defined once, outside both conditional blocks, precisely because there is nothing to choose between.
 * @{
 */
/**
 * @name 항상 이름으로 호출되는 타입
 * @brief 타입 기반 선택이 다룰 수 없는 타입들. 그래서 두 컴파일러 모두 이것을 직접 호출한다.
 *
 * 이 중 일부는 선택이 정말로 구별할 수 없다. 평범한 문자, 부호 있는 문자, 논리값이 모두 같은 바탕 타입일 수 있으므로 어떤 선택도 그것들을 가려낼 수 없다. 문자 루틴들은 길이 인자를 하나 더
 * 받는데, 그것은 나머지의 모양에 맞지 않는다.
 *
 * @note 두 조건 블록 밖에 한 번만 정의된다. 바로 고를 것이 없기 때문이다.
 * @{
 */
/* not in _Generic */
#define Sfo_long(a,b,c) sfo_long(a, b, c);
#define Sfo_ulong(a,b,c) sfo_ulong(a, b, c);
#define Sfo_char(a,b,c,d) sfo_char(a, b, c, d)
#define Sfo_boolean(a,b,c) sfo_boolean(a, b, c)
#define Sfo_schar(a,b,c) sfo_schar(a, b, c)

#define Sfi_long(a,b,c) sfi_long(a, b, c);
#define Sfi_ulong(a,b,c) sfi_ulong(a, b, c);
#define Sfi_char(a,b,c,d) sfi_char(a, b, c, d)
#define Sfi_boolean(a,b,c) sfi_boolean(a, b, c)
#define Sfi_schar(a,b,c) sfi_schar(a, b, c)
/** @} */

#endif /* SAVEFILE_H */

