/* NetHack 5.0	sfmacros.h $NHDT-Date$  $NHDT-Branch$:$NHDT-Revision$ */
/* Copyright (c) Michael Allison, 2025. */
/* NetHack may be freely redistributed.  See license for details. */

/* This file is included by sfbase.c, sfstruct.c */

/**
 * @file sfmacros.h
 * @brief The list of types the save code can write, named once.
 *
 * Every structure and field type that gets serialized is listed here exactly
 * once, and the file is included with @c SF_C and @c SF_A defined differently by
 * each consumer -- so one list generates the declarations, the definitions and
 * the dispatch tables for both save formats.
 *
 * That is the point: a type added here appears in every place that needs it,
 * rather than in three lists that can drift apart.
 *
 * @note Guarded on the macros being defined rather than on an include guard,
 *       since being included more than once is how it works.
 * @warning Removing or renaming an entry changes what the save code can write. It
 *          is not a list of conveniences -- it is the set of types a save file can
 *          contain.
 */

/**
 * @file sfmacros.h
 * @brief 저장 코드가 쓸 수 있는 타입들의 목록. 한 번만 이름을 적는다.
 *
 * 직렬화되는 모든 구조체와 필드 타입이 여기 정확히 한 번씩 나열되며, 이 파일은 소비하는
 * 쪽마다 @c SF_C 와 @c SF_A 를 다르게 정의한 채 포함된다. 그래서 하나의 목록이 두 저장
 * 형식 모두를 위한 선언과 정의, 디스패치 표를 생성한다.
 *
 * 그것이 핵심이다. 여기 추가한 타입은 서로 어긋날 수 있는 세 개의 목록이 아니라, 그것을
 * 필요로 하는 모든 곳에 한꺼번에 나타난다.
 *
 * @note include guard 대신 매크로가 정의되었는지로 감싸여 있다. 두 번 이상 포함되는 것이
 *       이 파일의 동작 방식이기 때문이다.
 * @warning 항목을 없애거나 이름을 바꾸면 저장 코드가 쓸 수 있는 것이 달라진다. 편의를 모아
 *          둔 목록이 아니라, 저장 파일이 담을 수 있는 타입의 집합이다.
 */

#if defined(SF_C) && defined(SF_A)

SF_C(struct, arti_info)
SF_C(struct, nhrect)
SF_C(struct, branch)
SF_C(struct, bubble)
SF_C(struct, cemetery)
SF_C(struct, context_info)
SF_C(struct, nhcoord)
SF_C(struct, damage)
SF_C(struct, dest_area)
SF_C(struct, dgn_topology)
SF_C(struct, dungeon)
SF_C(struct, d_level)
SF_C(struct, ebones)
SF_C(struct, edog)
SF_C(struct, egd)
SF_C(struct, emin)
SF_C(struct, engr)
SF_C(struct, epri)
SF_C(struct, eshk)
SF_C(struct, fe)
SF_C(struct, flag)
SF_C(struct, fruit)
SF_C(struct, gamelog_line)
SF_C(struct, kinfo)
SF_C(struct, levelflags)
SF_C(struct, ls_t)
SF_C(struct, linfo)
SF_C(struct, mapseen_feat)
SF_C(struct, mapseen_flags)
SF_C(struct, mapseen_rooms)
SF_C(struct, mkroom)
SF_C(struct, monst)
SF_C(struct, mvitals)
SF_C(struct, obj)
SF_C(struct, objclass)
SF_C(struct, q_score)
SF_C(struct, rm)
SF_C(struct, spell)
SF_C(struct, stairway)
SF_C(struct, s_level)
SF_C(struct, trap)
SF_C(struct, you)
SF_C(union, any)
#ifdef DEMO_UPLIFTS
SF_C(struct, mystruct)
SF_C(struct, mystruct_rev0)
#endif

SF_A(aligntyp)
SF_A(boolean)
SF_A(coordxy)
//SF_A(genericptr)
SF_A(int)
SF_A(int16)
SF_A(int32)
SF_A(int64)
SF_A(long)
SF_A(schar)
SF_A(short)
SF_A(size_t)
SF_A(time_t)
SF_A(uchar)
SF_A(uint16)
SF_A(uint32)
SF_A(uint64)
SF_A(ulong)
SF_A(unsigned)
SF_A(ushort)
SF_A(xint16)
SF_A(xint8)

#else

#error Non-productive inclusion of sfmacros.h

#endif  /* SF_C && SF_A */

