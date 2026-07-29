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

/**
 * @brief Resolved locations of every level and branch the code names directly.
 *
 * The dungeon is generated, so the depth of the Oracle or the Castle is not
 * known until play begins. Once resolved, those answers are cached here, since
 * the game asks "is this level X?" constantly and searching the dungeon
 * description each time would be wasteful.
 *
 * @note Some members are retained but unused (@c d_bigroom_level,
 *       @c d_baalzebub_level, @c d_asmodeus_level); they are kept so the
 *       structure's layout does not shift.
 * @warning Filled in during dungeon initialization. Reading these before that
 *          yields zeroes, which name a valid-looking but wrong level.
 */
/**
 * @brief 코드가 이름으로 직접 참조하는 모든 레벨과 분기의 해석된 위치.
 *
 * 던전은 생성되는 것이므로 신탁이나 성이 몇 층인지는 플레이가 시작되어야 알 수
 * 있다. 한 번 확정된 뒤에는 그 답을 여기에 캐시한다. 게임이 "이 레벨이 X인가?"를
 * 끊임없이 묻는데, 매번 던전 서술을 뒤지는 것은 낭비이기 때문이다.
 *
 * @note 일부 멤버는 남아 있으나 사용되지 않는다(@c d_bigroom_level,
 *       @c d_baalzebub_level, @c d_asmodeus_level). 구조체 배치가 어긋나지
 *       않도록 유지한다.
 * @warning 던전 초기화 중에 채워진다. 그 전에 읽으면 0이 나오는데, 이는 유효해
 *          보이지만 잘못된 레벨을 지칭한다.
 */
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

/**
 * @brief Short names for the cached levels and branch numbers.
 *
 * These read like plain globals at the call site, which is why the surrounding
 * code can say @c Is_astralevel(&u.uz) or compare against @c medusa_level
 * without mentioning where the value is stored.
 *
 * @note Each expands to an lvalue inside @c svd.dungeon_topology, so they are
 *       assignable; dungeon initialization sets them through these names.
 */
/**
 * @brief 캐시된 레벨과 분기 번호에 대한 짧은 이름들.
 *
 * 호출 지점에서는 평범한 전역처럼 읽힌다. 그래서 주변 코드가 값이 어디에
 * 저장되는지 언급하지 않고도 @c Is_astralevel(&u.uz) 라고 쓰거나
 * @c medusa_level 과 비교할 수 있다.
 *
 * @note 각각 @c svd.dungeon_topology 내부의 좌변값으로 확장되므로 대입이
 *       가능하다. 던전 초기화도 이 이름들을 통해 값을 설정한다.
 */
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

/**
 * @brief Report the deepest level the hero has reached in a dungeon branch.
 * @param x Pointer to a @c d_level naming the branch of interest.
 * @return The deepest level number reached within that branch.
 * @note An lvalue, so it is also how that high-water mark gets updated.
 */
/**
 * @brief 영웅이 해당 던전 분기에서 도달한 가장 깊은 레벨을 반환한다.
 * @param x 대상 분기를 지칭하는 @c d_level 포인터.
 * @return 그 분기 안에서 도달한 가장 깊은 레벨 번호.
 * @note 좌변값이므로, 이 최고 기록을 갱신하는 수단이기도 하다.
 */
#define dunlev_reached(x) (svd.dungeons[(x)->dnum].dunlev_ureached)

/** @brief Upper bound on level-information entries across the whole dungeon. */
/** @brief 던전 전체에 걸친 레벨 정보 항목 수의 상한. */
#define MAXLINFO (MAXDUNGEON * MAXLEVEL)

#endif /* NH_DGNTOPO_H */
