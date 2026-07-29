/* NetHack 5.0	nh_corpstat.h	$NHDT-Date: 0 $ */
/* NetHack may be freely redistributed.  See license for details. */
/* Extracted 2026-07 from hack.h for the real-time fork (see MODIFICATIONS.md).
   Content is unchanged; hack.h includes this header at the point the
   definitions used to occupy, so preprocessing order is identical. */

/**
 * @file nh_corpstat.h
 * @brief Flags describing a corpse, statue, or figurine as it is created.
 *
 * A corpse or statue must remember what it came from, since the game can later
 * revive it, name it, or describe its gender. The low bits of these flags are
 * stored directly in the object's @c spe field, so they are part of the saved
 * object rather than a transient argument.
 *
 * @warning Bits 0..7 are recorded in @c obj->spe; they share that field with
 *          the other meanings @c spe carries, and their values must stay within
 *          @c CORPSTAT_SPE_VAL.
 * @note The gender encoding deliberately differs from the one used for live
 *       monsters so that zero can mean "unspecified/random" rather than "male".
 * @note Extracted verbatim from @c hack.h; include @c hack.h rather than this
 *       header directly, which is what every existing source file does.
 */

/**
 * @file nh_corpstat.h
 * @brief 시체·조각상·인형을 생성할 때의 속성을 나타내는 플래그.
 *
 * 시체나 조각상은 자신이 무엇에서 비롯되었는지 기억해야 한다. 게임이 나중에
 * 되살리거나, 이름을 붙이거나, 성별을 서술할 수 있기 때문이다. 이 플래그의
 * 하위 비트는 객체의 @c spe 필드에 그대로 저장되므로, 일시적인 인자가 아니라
 * 저장되는 객체의 일부다.
 *
 * @warning 0..7 비트는 @c obj->spe 에 기록된다. @c spe 가 지니는 다른 의미들과
 *          같은 필드를 공유하므로, 값은 @c CORPSTAT_SPE_VAL 범위를 벗어나면
 *          안 된다.
 * @note 성별 인코딩은 살아 있는 몬스터에 쓰이는 것과 의도적으로 다르다. 0이
 *       "남성"이 아니라 "미지정/무작위"를 뜻하게 하기 위함이다.
 * @note @c hack.h 에서 그대로 추출했다. 기존 모든 소스 파일이 그렇듯 이 헤더를
 *       직접 포함하지 말고 @c hack.h 를 포함할 것.
 */

#ifndef NH_CORPSTAT_H
#define NH_CORPSTAT_H

/**
 * @brief What a corpse or statue should remember about its origin.
 *
 * A corpse can later be revived, eaten, or named, and a statue can be broken
 * open or identified as historic, so the remains must carry enough of the
 * original creature to answer those questions.
 *
 * @warning Bits 0..7 are stored in @c obj->spe, a field that carries different
 *          meanings for other object types. Values must stay within
 *          @c CORPSTAT_SPE_VAL.
 * @note @c CORPSTAT_INIT and @c CORPSTAT_BURIED are instructions to the
 *       creating routine, not properties recorded on the object.
 */
/**
 * @brief 시체나 조각상이 자신의 유래에 관해 기억해야 할 것.
 *
 * 시체는 나중에 되살아나거나, 먹히거나, 이름이 붙을 수 있고, 조각상은 깨지거나
 * 역사적인 것으로 식별될 수 있다. 따라서 그 잔해는 원래 생물에 대한 정보를 그런
 * 질문에 답할 만큼 지니고 있어야 한다.
 *
 * @warning 0..7 비트는 @c obj->spe 에 저장된다. 이 필드는 다른 물건 종류에서는
 *          다른 의미를 지니므로, 값은 @c CORPSTAT_SPE_VAL 범위를 벗어나면 안 된다.
 * @note @c CORPSTAT_INIT 과 @c CORPSTAT_BURIED 는 생성 루틴에 대한 지시이며,
 *       물건에 기록되는 속성이 아니다.
 */
/* flags for make_corpse() and mkcorpstat(); 0..7 are recorded in obj->spe */
#define CORPSTAT_NONE     0x00
#define CORPSTAT_GENDER   0x03 /* 0x01 | 0x02 */
#define CORPSTAT_HISTORIC 0x04 /* historic statue; not used for corpse */
#define CORPSTAT_SPE_VAL  0x07 /* 0x03 | 0x04 */
#define CORPSTAT_INIT     0x08 /* pass init flag to mkcorpstat */
#define CORPSTAT_BURIED   0x10 /* bury the corpse or statue */
/* note: gender flags have different values from those used for monsters
   so that 0 can be unspecified/random instead of male */
#define CORPSTAT_RANDOM 0
#define CORPSTAT_FEMALE 1
#define CORPSTAT_MALE   2
#define CORPSTAT_NEUTER 3

#endif /* NH_CORPSTAT_H */
