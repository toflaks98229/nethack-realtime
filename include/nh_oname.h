/* NetHack 5.0	nh_oname.h	$NHDT-Date: 0 $ */
/* NetHack may be freely redistributed.  See license for details. */
/* Extracted 2026-07 from hack.h for the real-time fork (see MODIFICATIONS.md).
   Content is unchanged; hack.h includes this header at the point the
   definitions used to occupy, so preprocessing order is identical. */

/**
 * @file nh_oname.h
 * @brief How an artifact came to exist, recorded when it is named.
 *
 * Naming an object can turn it into an artifact, and the game cares about the
 * provenance: an artifact wished for, granted as a divine gift, dipped for in a
 * fountain, or placed by a level's definition are not equivalent, and some are
 * announced to other players in the live log. These flags carry that origin
 * through @c oname() and are consulted by @c artifact_origin().
 *
 * @note @c ONAME_BONES means the artifact arrived from a bones file, where its
 *       original provenance is no longer known and does not matter.
 * @warning Bit 0x0001U is reserved for the 'exists' query and is deliberately
 *          absent from this list.
 */

/**
 * @file nh_oname.h
 * @brief 아티팩트가 어떻게 생겨났는지를 명명 시점에 기록하는 플래그.
 *
 * 물건에 이름을 붙이면 아티팩트가 될 수 있으며, 게임은 그 유래를 중요하게
 * 여긴다. 소원으로 얻은 것, 신의 선물로 받은 것, 분수에 담가 만든 것, 특수
 * 레벨 정의가 배치한 것은 서로 같지 않고, 일부는 라이브 로그로 다른
 * 플레이어에게 알려진다. 이 플래그들이 그 유래를 @c oname() 을 통해 전달하며
 * @c artifact_origin() 이 참조한다.
 *
 * @note @c ONAME_BONES 는 본즈 파일에서 넘어온 아티팩트를 뜻한다. 이 경우 원래
 *       유래는 더 이상 알 수 없고 중요하지도 않다.
 * @warning 0x0001U 비트는 'exists' 질의용으로 예약되어 있으며, 이 목록에서
 *          의도적으로 빠져 있다.
 */

#ifndef NH_ONAME_H
#define NH_ONAME_H

/* Flags for oname(), artifact_exists(), artifact_origin() */
#define ONAME_NO_FLAGS   0U /* none of the below; they apply to artifacts */
/*                       0x0001U is reserved for 'exists' */
/* flags indicating where an artifact came from */
#define ONAME_VIA_NAMING 0x0002U /* oname() is being called by do_oname();
                                  * only matters if creating Sting|Orcrist */
#define ONAME_WISH       0x0004U /* created via wish */
#define ONAME_GIFT       0x0008U /* created as a divine reward after #offer or
                                  * special #pray result of being crowned */
#define ONAME_VIA_DIP    0x0010U /* created Excalibur in a fountain */
#define ONAME_LEVEL_DEF  0x0020U /* placed by a special level's definition */
#define ONAME_BONES      0x0040U /* object came from bones; in its original
                                  * game it had one of the other bits but we
                                  * don't care which one */
#define ONAME_RANDOM     0x0080U /* something created an artifact randomly
                                  * with mk_artifact() (mksboj or mk_player)
                                  * or m_initweap() (lawful Angel) */
/* flag controlling potential livelog event of finding an artifact */
#define ONAME_KNOW_ARTI  0x0100U /* hero is already aware of this artifact */
/* flag for suppressing perm_invent update when name gets assigned */
#define ONAME_SKIP_INVUPD 0x0200U /* don't call update_inventory() */

#endif /* NH_ONAME_H */
