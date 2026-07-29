/* NetHack 5.0	nh_trapflags.h	$NHDT-Date: 0 $ */
/* NetHack may be freely redistributed.  See license for details. */
/* Extracted 2026-07 from hack.h for the real-time fork (see MODIFICATIONS.md).
   Content is unchanged; hack.h includes this header at the point the
   definitions used to occupy, so preprocessing order is identical. */

/**
 * @file nh_trapflags.h
 * @brief Circumstances under which a trap is being triggered.
 *
 * Whether a trap fires, how much it hurts, and what the player is told all
 * depend on *how* the victim arrived. Deliberately climbing into a pit, being
 * flung into one, blundering while fumbling, or setting a trap off by failing
 * to disarm it are all the same trap and different events. These flags tell
 * @c dotrap() and @c mintrap() which case they are handling.
 *
 * @note @c RECURSIVETRAP marks a trap that changed into another type during the
 *       same turn, which the code must not treat as a fresh trigger.
 */

/**
 * @file nh_trapflags.h
 * @brief 함정이 발동되는 상황을 나타내는 플래그.
 *
 * 함정이 실제로 작동하는지, 얼마나 아픈지, 플레이어에게 무엇을 알리는지는 모두
 * 피해자가 *어떻게* 도달했는지에 달려 있다. 스스로 구덩이로 내려간 경우, 던져져
 * 빠진 경우, 발을 헛디딘 경우, 해제에 실패해 발동시킨 경우는 모두 같은
 * 함정이지만 서로 다른 사건이다. 이 플래그들이 @c dotrap() 과 @c mintrap() 에
 * 어떤 경우를 처리하는 중인지 알려 준다.
 *
 * @note @c RECURSIVETRAP 은 같은 턴에 다른 종류로 바뀐 함정을 표시하며, 코드는
 *       이를 새로운 발동으로 취급해서는 안 된다.
 */

#ifndef NH_TRAPFLAGS_H
#define NH_TRAPFLAGS_H

/* Flags to control dotrap() and mintrap() in trap.c */
#define NO_TRAP_FLAGS 0x00U
#define FORCETRAP     0x01U /* triggering not left to chance */
#define NOWEBMSG      0x02U /* suppress stumble into web message */
#define FORCEBUNGLE   0x04U /* adjustments appropriate for bungling */
#define RECURSIVETRAP 0x08U /* trap changed into another type this same turn */
#define TOOKPLUNGE    0x10U /* used '>' to enter pit below you */
#define VIASITTING    0x20U /* #sit while at trap location (affects message) */
#define FAILEDUNTRAP  0x40U /* trap activated by failed untrap attempt */
#define HURTLING      0x80U /* monster is hurtling through air */

#endif /* NH_TRAPFLAGS_H */
