/* NetHack 5.0	weight.h	$NHDT-Date: 1781973091 2026/06/20 16:31:31 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.4 $ */
/* Copyright (c) Michael Allison, 2025. */
/* NetHack may be freely redistributed.  See license for details. */

/**
 * @file weight.h
 * @brief The weights and thresholds that encumbrance is measured against.
 *
 * Weight is not only a limit on what can be carried. It decides whether the
 * hero can squeeze diagonally between boulders, whether they splash walking into
 * water, whether a fumbled step is heard, and how much a falling object hurts --
 * so these numbers appear in places that have nothing to do with inventory.
 *
 * Body weights are here as well, since a corpse weighs what the creature did.
 *
 * @note Gathered into one enumeration rather than scattered as macros so that
 *       the relationships between them -- which threshold is stricter than
 *       which -- are visible at a glance.
 * @warning @c MAX_CARR_CAP caps carrying capacity but is not the heaviest thing
 *          in the game; boulders are deliberately heavier so that they cannot
 *          be picked up.
 */

/**
 * @file weight.h
 * @brief 하중을 재는 기준이 되는 무게와 임계값들.
 *
 * 무게는 들 수 있는 양의 한계만이 아니다. 바위 사이를 대각선으로 비집고 지날 수 있는지,
 * 물에 들어갈 때 물을 튀기는지, 발을 헛디딘 소리가 들리는지, 떨어지는 물건이 얼마나
 * 아픈지를 정한다. 그래서 이 숫자들이 소지품과 무관한 곳에도 나타난다.
 *
 * 몸 무게도 여기 있다. 시체는 그 생물이 지녔던 무게를 지니기 때문이다.
 *
 * @note 매크로로 흩어 두는 대신 하나의 열거로 모았다. 그래야 어느 임계값이 어느 것보다
 *       엄격한지 같은 관계가 한눈에 보인다.
 * @warning @c MAX_CARR_CAP 은 소지 한계를 제한하지만 게임에서 가장 무거운 것은 아니다.
 *          바위는 집을 수 없도록 의도적으로 더 무겁다.
 */

#ifndef WEIGHT_H
#define WEIGHT_H

/**
 * @brief Weight values and the thresholds compared against them.
 * @note Values are in the game's own unit, not a real-world one; only their
 *       relative sizes matter.
 */
/**
 * @brief 무게 값과 그것을 견주는 임계값들.
 * @note 값들은 현실 단위가 아니라 게임 자체의 단위다. 상대적 크기만이 의미를 가진다.
 */
/* weight-related constants and thresholds */
enum weight_constants {
    WT_ETHEREAL          =    0,
    WT_SPLASH_THRESHOLD  =    9,  /* weight needed to make splash in water */
    WT_WEIGHTCAP_STRCON  =   25,  /* str + con multiplied by this for conv to
                                   * carrying capacity in weight_cap() */
    WT_WEIGHTCAP_SPARE   =   50,  /* used in weight_cap calc */
    WT_JELLY             =   50,  /* weight of jelly body */
    WT_WOUNDEDLEG_REDUCT =  100,  /* wounded legs reduce carrcap by this */
    WT_TO_DMG            =  100,  /* divisor to convert weight to dmg amt */
    WT_IRON_BALL_INCR    =  160,  /* weight increment of heavy iron ball */
    WT_IRON_BALL_BASE    =  480,  /* base starting weight of iron ball */
    WT_NOISY_INV         =  500,  /* inv_weight() max for noisy fumbling */
    WT_NYMPH             =  600,  /* weight of nymph body */
    WT_TOOMUCH_DIAGONAL  =  600,  /* weight_cap threshold for diag squeeze */
    WT_ELF               =  800,  /* weight of elf body */
    WT_SQUEEZABLE_INV    =  850,  /* inv_weight() maximum for squeezing */
    MAX_CARR_CAP         = 1000,  /* max carrying capacity, so that
                                   * boulders can be heavier */
    WT_HUMAN             = 1450,  /* weight of human body */
    WT_BABY_DRAGON       = 1500,  /* weight of baby dragon body */
    WT_DRAGON            = 4500,  /* weight of dragon body */
};

#endif /* WEIGHT_H */


