

/**
 * @file mcastu.h
 * @brief The list of monster spells, written once and read several ways.
 *
 * Every entry appears exactly once here, and the file is included repeatedly with
 * @c MONSPELL defined differently each time -- so the same list becomes an
 * enumeration, an initializer table, and the names used when dumping internals.
 * Adding a spell in one place therefore updates all of them.
 *
 * @note Deliberately has no include guard, because being included more than once
 *       is how it works.
 * @warning Do not reorder the entries. The enumeration's values come from their
 *          position, so moving one silently renumbers every spell after it and
 *          desynchronizes it from the table built from the same list.
 */

/**
 * @file mcastu.h
 * @brief 몬스터 주문 목록. 한 번 쓰고 여러 방식으로 읽는다.
 *
 * 모든 항목이 여기 정확히 한 번만 나타나며, 이 파일은 @c MONSPELL 을 매번 다르게
 * 정의한 채 반복 포함된다. 그래서 같은 목록이 열거가 되고, 초기화 표가 되고, 내부를
 * 덤프할 때 쓰는 이름이 된다. 한 곳에 주문을 추가하면 그 전부가 함께 갱신된다.
 *
 * @note include guard 가 의도적으로 없다. 두 번 이상 포함되는 것이 이 파일의 동작
 *       방식이기 때문이다.
 * @warning 항목 순서를 바꾸지 말 것. 열거값이 위치에서 나오므로, 하나를 옮기면 그
 *          뒤의 모든 주문이 조용히 다시 번호 매겨지고 같은 목록으로 만든 표와
 *          어긋난다.
 */

/**
 * @brief Conditions a monster spell places on its own use.
 * @note @c MCF_INDIRECT marks a spell with no target, which is what lets a
 *       monster cast it when the hero is not visible.
 */
/**
 * @brief 몬스터 주문이 자신의 사용에 부과하는 조건.
 * @note @c MCF_INDIRECT 는 대상이 없는 주문을 표시한다. 덕분에 영웅이 보이지 않을
 *       때도 몬스터가 그것을 시전할 수 있다.
 */
#define MCF_NONE     0x0000
#define MCF_INDIRECT 0x0001 /* untargeted/indirect spell */
#define MCF_SIGHT    0x0002 /* monster needs to see hero */
#define MCF_HOSTILE  0x0004 /* cast by hostile monsters only */

#if defined(MCASTU_ENUM)
#define MONSPELL(def, lvl, flags) MCAST_##def
#elif defined(MCASTU_INIT)
#define MONSPELL(def, lvl, flags) { lvl, flags }
#elif defined(DUMP_MCASTU_ENUM1)
#define MONSPELL(def, lvl, flags) MCAST_DUMPENUM_##def
#elif defined(DUMP_MCASTU_ENUM2)
#define MONSPELL(def, lvl, flags) { MCAST_DUMPENUM_##def, #def }
#endif

MONSPELL(PSI_BOLT,     0, MCF_HOSTILE|MCF_SIGHT),
MONSPELL(OPEN_WOUNDS,  0, MCF_HOSTILE|MCF_SIGHT),
MONSPELL(CURE_SELF,    1, MCF_INDIRECT),
MONSPELL(HASTE_SELF,   2, MCF_INDIRECT),
MONSPELL(CONFUSE_YOU,  2, MCF_HOSTILE|MCF_SIGHT),
MONSPELL(STUN_YOU,     3, MCF_HOSTILE|MCF_SIGHT),
MONSPELL(DISAPPEAR,    4, MCF_INDIRECT),
MONSPELL(PARALYZE,     4, MCF_HOSTILE|MCF_SIGHT),
MONSPELL(BLIND_YOU,    6, MCF_HOSTILE|MCF_SIGHT),
MONSPELL(WEAKEN_YOU,   6, MCF_HOSTILE|MCF_SIGHT),
MONSPELL(DESTRY_ARMR,  8, MCF_HOSTILE|MCF_SIGHT),
MONSPELL(INSECTS,      8, MCF_HOSTILE|MCF_INDIRECT|MCF_SIGHT),
MONSPELL(CURSE_ITEMS, 10, MCF_HOSTILE|MCF_SIGHT),
MONSPELL(LIGHTNING,   11, MCF_HOSTILE|MCF_SIGHT),
MONSPELL(FIRE_PILLAR, 12, MCF_HOSTILE|MCF_SIGHT),
MONSPELL(GEYSER,      13, MCF_HOSTILE|MCF_SIGHT),
MONSPELL(AGGRAVATION, 13, MCF_INDIRECT|MCF_HOSTILE|MCF_SIGHT),
MONSPELL(SUMMON_MONS, 15, MCF_HOSTILE|MCF_INDIRECT|MCF_SIGHT),
MONSPELL(CLONE_WIZ,   18, MCF_HOSTILE|MCF_INDIRECT|MCF_SIGHT),
MONSPELL(DEATH_TOUCH, 20, MCF_HOSTILE|MCF_SIGHT),

#undef MONSPELL
