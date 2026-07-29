/* NetHack 5.0	nh_dgntopo.h	$NHDT-Date: 0 $ */
/* NetHack may be freely redistributed.  See license for details. */
/* Extracted 2026-07 from hack.h for the real-time fork (see MODIFICATIONS.md).
   Content is unchanged; hack.h includes this header at the point the
   definitions used to occupy, so preprocessing order is identical. */

/**
 * @file nh_dgntopo.h
 * @brief Cached locations of the dungeon's special levels and branches.
 *
 * The game repeatedly asks "is this the Oracle level?", "where does Sokoban
 * start?", and so on. Rather than search the dungeon description each time,
 * those answers are resolved once into @c struct dgn_topology and reached
 * through the short accessor macros here.
 *
 * @note The macros read @c svd.dungeon_topology, which is filled in during
 *       dungeon initialization; they are not meaningful before that.
 * @note Extracted verbatim from @c hack.h; include @c hack.h rather than this
 *       header directly, which is what every existing source file does.
 */

/**
 * @file nh_dgntopo.h
 * @brief 던전의 특수 레벨과 분기 위치를 캐시한 정의.
 *
 * 게임은 "여기가 신탁 레벨인가?", "소코반은 어디서 시작하는가?" 같은 질문을
 * 반복해서 던진다. 매번 던전 서술을 뒤지는 대신, 그 답을 한 번 계산해
 * @c struct dgn_topology 에 담아 두고 여기의 짧은 접근 매크로로 참조한다.
 *
 * @note 이 매크로들은 던전 초기화 과정에서 채워지는 @c svd.dungeon_topology 를
 *       읽으므로, 초기화 이전에는 의미가 없다.
 * @note @c hack.h 에서 그대로 추출했다. 기존 모든 소스 파일이 그렇듯 이 헤더를
 *       직접 포함하지 말고 @c hack.h 를 포함할 것.
 */

#ifndef NH_DGNTOPO_H
#define NH_DGNTOPO_H

struct dgn_topology { /* special dungeon levels for speed */
    d_level d_oracle_level;
    d_level d_bigroom_level; /* unused */
    d_level d_rogue_level;
    d_level d_medusa_level;
    d_level d_stronghold_level;
    d_level d_valley_level;
    d_level d_wiz1_level;
    d_level d_wiz2_level;
    d_level d_wiz3_level;
    d_level d_juiblex_level;
    d_level d_orcus_level;
    d_level d_baalzebub_level; /* unused */
    d_level d_asmodeus_level;  /* unused */
    d_level d_portal_level;    /* only in goto_level() [do.c] */
    d_level d_sanctum_level;
    d_level d_earth_level;
    d_level d_water_level;
    d_level d_fire_level;
    d_level d_air_level;
    d_level d_astral_level;
    xint16 d_tower_dnum;
    xint16 d_sokoban_dnum;
    xint16 d_mines_dnum, d_quest_dnum;
    xint16 d_tutorial_dnum;
    d_level d_qstart_level, d_qlocate_level, d_nemesis_level;
    d_level d_knox_level;
    d_level d_mineend_level;
    d_level d_sokoend_level;
};

/* macros for accessing the dungeon levels by their old names */
/* clang-format off */
#define oracle_level            (svd.dungeon_topology.d_oracle_level)
#define bigroom_level           (svd.dungeon_topology.d_bigroom_level)
#define rogue_level             (svd.dungeon_topology.d_rogue_level)
#define medusa_level            (svd.dungeon_topology.d_medusa_level)
#define stronghold_level        (svd.dungeon_topology.d_stronghold_level)
#define valley_level            (svd.dungeon_topology.d_valley_level)
#define wiz1_level              (svd.dungeon_topology.d_wiz1_level)
#define wiz2_level              (svd.dungeon_topology.d_wiz2_level)
#define wiz3_level              (svd.dungeon_topology.d_wiz3_level)
#define juiblex_level           (svd.dungeon_topology.d_juiblex_level)
#define orcus_level             (svd.dungeon_topology.d_orcus_level)
#define baalzebub_level         (svd.dungeon_topology.d_baalzebub_level)
#define asmodeus_level          (svd.dungeon_topology.d_asmodeus_level)
#define portal_level            (svd.dungeon_topology.d_portal_level)
#define sanctum_level           (svd.dungeon_topology.d_sanctum_level)
#define earth_level             (svd.dungeon_topology.d_earth_level)
#define water_level             (svd.dungeon_topology.d_water_level)
#define fire_level              (svd.dungeon_topology.d_fire_level)
#define air_level               (svd.dungeon_topology.d_air_level)
#define astral_level            (svd.dungeon_topology.d_astral_level)
#define tower_dnum              (svd.dungeon_topology.d_tower_dnum)
#define sokoban_dnum            (svd.dungeon_topology.d_sokoban_dnum)
#define mines_dnum              (svd.dungeon_topology.d_mines_dnum)
#define quest_dnum              (svd.dungeon_topology.d_quest_dnum)
#define tutorial_dnum           (svd.dungeon_topology.d_tutorial_dnum)
#define qstart_level            (svd.dungeon_topology.d_qstart_level)
#define qlocate_level           (svd.dungeon_topology.d_qlocate_level)
#define nemesis_level           (svd.dungeon_topology.d_nemesis_level)
#define knox_level              (svd.dungeon_topology.d_knox_level)
#define mineend_level           (svd.dungeon_topology.d_mineend_level)
#define sokoend_level           (svd.dungeon_topology.d_sokoend_level)
/* clang-format on */

#define dunlev_reached(x) (svd.dungeons[(x)->dnum].dunlev_ureached)
#define MAXLINFO (MAXDUNGEON * MAXLEVEL)

#endif /* NH_DGNTOPO_H */
