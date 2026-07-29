/* NetHack 5.0	nh_monnam.h	$NHDT-Date: 0 $ */
/* NetHack may be freely redistributed.  See license for details. */
/* Extracted 2026-07 from hack.h for the real-time fork (see MODIFICATIONS.md).
   Content is unchanged; hack.h includes this header at the point the
   definitions used to occupy, so preprocessing order is identical. */

/**
 * @file nh_monnam.h
 * @brief Article selection and detail-suppression masks for monster names.
 *
 * Naming a monster is a judgement about what the hero may know. The same
 * creature is "it", "the gnome lord", or "your kitten" depending on whether it
 * is seen, remembered, or tame, and hallucination, invisibility, saddles, and
 * mimicry each hide or alter part of the description. The article constants
 * pick the leading word; the suppress masks tell @c x_monnam() which of those
 * details to leave out.
 *
 * @note @c EXACT_NAME is the union of the individual suppress bits, not a
 *       separate flag; it asks for the plain, unqualified name.
 * @note Extracted verbatim from @c hack.h; include @c hack.h rather than this
 *       header directly, which is what every existing source file does.
 */

/**
 * @file nh_monnam.h
 * @brief 몬스터 이름의 관사 선택과 세부 정보 억제 마스크.
 *
 * 몬스터의 이름을 붙이는 일은 영웅이 무엇을 알 수 있는지에 대한 판단이다. 같은
 * 생물이라도 보이는지, 기억하는지, 길들여졌는지에 따라 "그것", "노움 군주",
 * "당신의 새끼 고양이"가 되며, 환각·투명·안장·의태는 각각 설명의 일부를
 * 감추거나 바꾼다. 관사 상수는 앞에 붙일 단어를 고르고, 억제 마스크는
 * @c x_monnam() 에 어떤 세부 정보를 빼야 하는지 알려 준다.
 *
 * @note @c EXACT_NAME 은 개별 억제 비트들의 합집합이며 별도의 플래그가 아니다.
 *       수식 없는 순수한 이름을 요청하는 뜻이다.
 * @note @c hack.h 에서 그대로 추출했다. 기존 모든 소스 파일이 그렇듯 이 헤더를
 *       직접 포함하지 말고 @c hack.h 를 포함할 것.
 */

#ifndef NH_MONNAM_H
#define NH_MONNAM_H

/* Monster name articles */
#define ARTICLE_NONE 0
#define ARTICLE_THE 1
#define ARTICLE_A 2
#define ARTICLE_YOUR 3

/* x_monnam() monster name suppress masks */
#define SUPPRESS_IT            0x01
#define SUPPRESS_INVISIBLE     0x02
#define SUPPRESS_HALLUCINATION 0x04
#define SUPPRESS_SADDLE        0x08
#define SUPPRESS_MAPPEARANCE   0x10
#define EXACT_NAME             0x1F
#define SUPPRESS_NAME 0x20
#define AUGMENT_IT    0x40 /* use "someone" or "something" instead of "it" */

#endif /* NH_MONNAM_H */
