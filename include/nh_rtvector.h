/* NetHack 5.0	nh_rtvector.h	$NHDT-Date: 0 $ */
/* NetHack may be freely redistributed.  See license for details. */
/* Added 2026-07 for the real-time fork (see MODIFICATIONS.md). */

/**
 * @file nh_rtvector.h
 * @brief Continuous positions for entities whose game position is a grid
 *        square.
 *
 * NetHack places everything on whole squares, and its rules depend on that:
 * adjacency decides melee, line of sight is traced square to square, and
 * pathfinding steps between them. Those rules stay authoritative here. What
 * this layer adds is a second, continuous position per entity -- a real-valued
 * point that chases the entity's square over time -- so that motion has a
 * position *between* squares even though the game does not.
 *
 * The grid remains the truth about where something *is*; these vectors are the
 * truth about where it should be *drawn*. Nothing in this layer feeds back into
 * gameplay, which is what makes it safe to add to a game whose rules assume
 * discrete simultaneity.
 *
 * Positions are kept in grid units rather than pixels, so a port converts to
 * its own scale; nothing here knows about tiles or screens.
 *
 * @note Entities are tracked in a side table keyed by @c m_id rather than by
 *       fields on @c struct monst, so nothing about the saved game changes.
 * @warning A vector is a rendering hint and may be absent or stale: an entity
 *          that just teleported, arrived on the level, or was never tracked has
 *          no meaningful interpolated position. Callers must handle "no vector
 *          available" and fall back to the grid square.
 */

/**
 * @file nh_rtvector.h
 * @brief 게임상 위치가 격자 칸인 엔티티들을 위한 연속 위치.
 *
 * NetHack 은 모든 것을 온전한 칸 위에 놓으며, 규칙이 그것에 의존한다. 인접
 * 여부가 근접 공격을 결정하고, 시야는 칸에서 칸으로 추적되며, 경로 탐색도 칸
 * 단위로 나아간다. 그 규칙들은 여기서도 권위를 유지한다. 이 계층이 더하는 것은
 * 엔티티마다 하나씩 갖는 두 번째 연속 위치다. 시간에 따라 해당 엔티티의 칸을
 * 뒤쫓는 실수 좌표이며, 덕분에 게임에는 없는 *칸과 칸 사이*의 위치가 움직임에는
 * 존재하게 된다.
 *
 * 무언가가 실제로 어디에 *있는지* 에 대한 진실은 여전히 격자이고, 이 벡터들은
 * 그것을 어디에 *그려야 하는지* 에 대한 진실이다. 이 계층의 어떤 것도 게임
 * 진행으로 되먹임되지 않으며, 이산 동시성을 전제하는 규칙을 가진 게임에 이것을
 * 안전하게 더할 수 있는 이유가 바로 그것이다.
 *
 * 위치는 픽셀이 아니라 격자 단위로 보관하므로, 각 포팅이 자기 축척으로 변환한다.
 * 여기의 어떤 것도 타일이나 화면을 알지 못한다.
 *
 * @note 엔티티는 @c struct monst 의 필드가 아니라 @c m_id 를 키로 하는 사이드
 *       테이블에 추적되므로, 저장된 게임에 관해서는 아무것도 달라지지 않는다.
 * @warning 벡터는 렌더링을 위한 힌트이며 없거나 낡았을 수 있다. 방금 순간이동을
 *          했거나, 레벨에 막 도착했거나, 애초에 추적되지 않은 엔티티는 의미 있는
 *          보간 위치를 갖지 않는다. 호출자는 "벡터 없음"을 처리하고 격자 칸으로
 *          물러날 수 있어야 한다.
 */

#ifndef NH_RTVECTOR_H
#define NH_RTVECTOR_H

#ifdef REALTIME_PROTO

/**
 * @brief Identifier reserved for the hero, who has no @c m_id of their own.
 * @note Chosen above any value @c m_id takes so it cannot collide with a
 *       monster.
 */
/**
 * @brief 자신의 @c m_id 가 없는 영웅을 위해 예약된 식별자.
 * @note @c m_id 가 취하는 어떤 값보다 크게 잡아 몬스터와 충돌하지 않게 했다.
 */
#define RTV_HERO_ID 0xFFFFFFFFU

/**
 * @brief Tell the layer that an entity now occupies a given square.
 *
 * Call this wherever an entity's square changes. If the entity was already
 * tracked and the new square is adjacent, its continuous position is left
 * where it was and will travel to the new square over the following frames;
 * any larger jump snaps, because a teleport is not motion.
 *
 * @param[in] id Entity identifier: a monster's @c m_id, or @c RTV_HERO_ID.
 * @param[in] x  Column the entity now occupies.
 * @param[in] y  Row the entity now occupies.
 * @note Safe to call for entities that are never drawn; untracked entities
 *       simply occupy a slot until it is reclaimed.
 */
/**
 * @brief 어떤 엔티티가 이제 지정한 칸을 차지한다고 이 계층에 알린다.
 *
 * 엔티티의 칸이 바뀌는 곳이면 어디서든 호출한다. 이미 추적 중이고 새 칸이
 * 인접하다면 연속 위치는 있던 자리에 남아 이후 프레임에 걸쳐 새 칸으로
 * 이동한다. 그보다 큰 도약은 즉시 맞춘다. 순간이동은 움직임이 아니기 때문이다.
 *
 * @param[in] id 엔티티 식별자. 몬스터의 @c m_id 또는 @c RTV_HERO_ID.
 * @param[in] x  엔티티가 이제 차지하는 열.
 * @param[in] y  엔티티가 이제 차지하는 행.
 * @note 그려지지 않는 엔티티에 대해 호출해도 안전하다. 추적되지 않는 엔티티는
 *       슬롯이 회수될 때까지 자리를 차지할 뿐이다.
 */
extern void rtv_set_square(unsigned id, coordxy x, coordxy y);

/**
 * @brief Advance every tracked position toward the square it belongs to.
 *
 * Call once per rendered frame. Elapsed real time decides how far each
 * position travels, so motion runs at the same rate whatever the frame rate,
 * and a position that has arrived stays put.
 *
 * @note Also reclaims slots that have not been touched for a long time, which
 *       is how entities that died or left the level are forgotten.
 */
/**
 * @brief 추적 중인 모든 위치를 각자 속한 칸 쪽으로 전진시킨다.
 *
 * 렌더링 프레임마다 한 번 호출한다. 각 위치가 얼마나 이동할지는 실제 경과
 * 시간이 정하므로, 프레임률과 무관하게 같은 속도로 움직이며, 이미 도착한
 * 위치는 그대로 머문다.
 *
 * @note 오랫동안 갱신되지 않은 슬롯도 함께 회수한다. 죽었거나 레벨을 떠난
 *       엔티티를 잊는 방식이 바로 이것이다.
 */
extern void rtv_advance(void);

/**
 * @brief Ask where the occupant of a square should currently be drawn.
 *
 * Answers in grid units relative to the square itself, so a result of (0,0)
 * means "draw it exactly on its square" and (-0.4, 0) means "four tenths of a
 * square to the left of it".
 *
 * @param[in]  x   Column of the square whose occupant is being drawn.
 * @param[in]  y   Row of that square.
 * @param[out] ox  Receives the horizontal offset, in grid units.
 * @param[out] oy  Receives the vertical offset, in grid units.
 * @retval TRUE  The occupant is in motion; @p ox and @p oy hold its offset.
 * @retval FALSE There is nothing to interpolate; draw on the square itself.
 * @note Takes a square rather than an identifier so that a renderer working
 *       from a grid of glyphs can use it without identifying anyone.
 * @warning Reports the position of whichever monster the core has on that
 *          square. A caller that draws from remembered or filtered map data
 *          must confirm it is actually showing a monster there before using
 *          the result, or it would animate something the player cannot see.
 */
/**
 * @brief 어떤 칸의 점유자를 지금 어디에 그려야 하는지 묻는다.
 *
 * 답은 그 칸 자신을 기준으로 한 격자 단위다. 따라서 (0,0)은 "칸 위에 정확히
 * 그리라"는 뜻이고, (-0.4, 0)은 "칸에서 왼쪽으로 10분의 4만큼"이라는 뜻이다.
 *
 * @param[in]  x   점유자를 그리려는 칸의 열.
 * @param[in]  y   그 칸의 행.
 * @param[out] ox  수평 오프셋을 격자 단위로 받는다.
 * @param[out] oy  수직 오프셋을 격자 단위로 받는다.
 * @retval TRUE  점유자가 이동 중이며 @p ox, @p oy 에 오프셋이 담긴다.
 * @retval FALSE 보간할 것이 없다. 칸 위에 그대로 그리면 된다.
 * @note 식별자가 아니라 칸을 받는다. 그래야 glyph 격자로 작업하는 렌더러가
 *       누구인지 알아내지 않고도 사용할 수 있다.
 * @warning 코어가 그 칸에 두고 있는 몬스터의 위치를 보고한다. 기억되거나
 *          걸러진 지도 데이터로 그리는 호출자는 결과를 쓰기 전에 그 자리에
 *          실제로 몬스터를 표시하고 있는지 확인해야 한다. 그러지 않으면
 *          플레이어가 볼 수 없는 것을 움직이게 된다.
 */
extern boolean rtv_offset_at(coordxy x, coordxy y, double *ox, double *oy);

/**
 * @brief Ask where the hero should currently be drawn.
 * @param[out] ox Receives the horizontal offset, in grid units.
 * @param[out] oy Receives the vertical offset, in grid units.
 * @retval TRUE  The hero is in motion; @p ox and @p oy hold the offset.
 * @retval FALSE The hero is at rest; draw on @c u.ux, @c u.uy.
 */
/**
 * @brief 영웅을 지금 어디에 그려야 하는지 묻는다.
 * @param[out] ox 수평 오프셋을 격자 단위로 받는다.
 * @param[out] oy 수직 오프셋을 격자 단위로 받는다.
 * @retval TRUE  영웅이 이동 중이며 @p ox, @p oy 에 오프셋이 담긴다.
 * @retval FALSE 영웅이 정지해 있다. @c u.ux, @c u.uy 위에 그리면 된다.
 */
extern boolean rtv_hero_offset(double *ox, double *oy);

/**
 * @brief Move the hero freely, letting the square it occupies follow.
 *
 * This is the full inversion: the hero has a real position that input moves in
 * any direction, and the square the game knows about is simply that position
 * rounded. Crossing into a different square is what produces a step, so the
 * hero is no longer confined to moving a square at a time.
 *
 * Because the game executes that step asynchronously and may refuse it -- a
 * wall, a closed door, something in the way -- the position is *predicted* and
 * then reconciled: a step that is accepted leaves the position where the player
 * put it, and one that is refused pulls it back into the square the hero is
 * really in. That is what stops the drawn hero from walking through walls.
 *
 * @param[in]  dx Horizontal input, -1 to 1; need not be normalized.
 * @param[in]  dy Vertical input, -1 to 1; need not be normalized.
 * @param[out] sx Receives the horizontal component of the step to take.
 * @param[out] sy Receives the vertical component of the step to take.
 * @retval TRUE  A square boundary was crossed; deliver this step to the game.
 * @retval FALSE Nothing to do this frame.
 * @note The step is reported rather than submitted, so the port chooses how to
 *       deliver it. That matters: a step placed on the command queue is taken
 *       without the game waiting, which loses the pacing, whereas one delivered
 *       as input is consumed by the wait already in progress.
 * @note Diagonals are scaled so that moving corner-wise is no faster than
 *       moving straight.
 * @note Call every frame, including with (0,0) when nothing is held, so that
 *       elapsed time is accounted for and a stall cannot bank up into a lurch.
 * @warning Only one step may be outstanding at a time; while the game has yet
 *          to act on one, the position is held at the boundary rather than
 *          continuing into a square the hero may not be allowed to enter.
 */
/**
 * @brief 영웅을 자유롭게 이동시키고, 차지하는 칸이 그것을 따라오게 한다.
 *
 * 완전한 역전이다. 영웅은 실제 위치를 가지며 입력이 그것을 임의 방향으로
 * 움직이고, 게임이 아는 칸은 그 위치를 반올림한 것일 뿐이다. 다른 칸으로
 * 넘어가는 사건이 걸음을 만들어 내므로, 영웅은 더 이상 한 번에 한 칸씩만
 * 움직이도록 갇혀 있지 않다.
 *
 * 게임은 그 걸음을 비동기로 실행하며 거부할 수도 있으므로 -- 벽, 닫힌 문,
 * 가로막은 무언가 -- 위치는 *예측*한 뒤 조정된다. 받아들여진 걸음은 위치를
 * 플레이어가 둔 자리에 남기고, 거부된 걸음은 위치를 영웅이 실제로 있는 칸으로
 * 되돌린다. 그려지는 영웅이 벽을 통과하지 않는 이유가 이것이다.
 *
 * @param[in]  dx 수평 입력. -1에서 1 사이이며 정규화되어 있지 않아도 된다.
 * @param[in]  dy 수직 입력. -1에서 1 사이이며 정규화되어 있지 않아도 된다.
 * @param[out] sx 내디딜 걸음의 수평 성분을 받는다.
 * @param[out] sy 내디딜 걸음의 수직 성분을 받는다.
 * @retval TRUE  칸 경계를 넘었다. 이 걸음을 게임에 전달할 것.
 * @retval FALSE 이번 프레임에 할 일이 없다.
 * @note 걸음을 직접 제출하지 않고 보고만 하므로, 전달 방식은 포팅이 고른다.
 *       이는 중요하다. 명령 큐에 놓인 걸음은 게임이 기다리지 않고 곧바로
 *       처리해 페이싱을 잃지만, 입력으로 전달된 걸음은 이미 진행 중인 대기가
 *       소비하기 때문이다.
 * @note 대각선은 모서리 방향 이동이 직선 이동보다 빠르지 않도록 보정된다.
 * @note 아무것도 눌리지 않았을 때 (0,0) 으로도 매 프레임 호출할 것. 그래야 경과
 *       시간이 반영되고, 멈춰 있던 시간이 한꺼번에 튀지 않는다.
 * @warning 한 번에 하나의 걸음만 미결 상태일 수 있다. 게임이 아직 처리하지
 *          않은 동안에는, 들어가지 못할 수도 있는 칸으로 계속 나아가는 대신
 *          위치를 경계에 붙잡아 둔다.
 */
extern boolean rtv_hero_free_move(double dx, double dy,
                                  coordxy *sx, coordxy *sy);

/**
 * @brief Forget every tracked position.
 * @note Call when the map the positions referred to is no longer the map being
 *       drawn -- changing level, restoring a game -- so that nothing glides
 *       from a square it occupied somewhere else.
 */
/**
 * @brief 추적 중인 모든 위치를 잊는다.
 * @note 위치들이 가리키던 지도가 더 이상 그려지는 지도가 아닐 때 -- 레벨 이동,
 *       게임 복원 -- 호출한다. 그래야 다른 곳에서 차지하던 칸으로부터 미끄러져
 *       오는 일이 없다.
 */
extern void rtv_reset(void);

#endif /* REALTIME_PROTO */

#endif /* NH_RTVECTOR_H */
