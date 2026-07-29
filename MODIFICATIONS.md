# Modifications from upstream NetHack

This repository is a **modified copy of NetHack 5.0**. It is **not** the
official NetHack distribution, and what you have here is not what the NetHack
DevTeam distributed.

NetHack is distributed under the **NetHack General Public License (NGPL)**.
This modified copy remains licensed under the same NGPL, at no charge, on terms
identical to the original — see [`dat/license`](dat/license). The complete,
machine-readable source is this repository.

Per NGPL §2(a), the notices below (and the per-file `MODIFIED` comments in the
changed source files) state that the files were changed and when.

---

## Real-time conversion (`REALTIME_PROTO`) — 2026-07

An experimental conversion of NetHack's input-gated, turn-based loop into a
**constant-rate real-time** loop. The world (monsters, timeouts, regeneration)
advances on a wall-clock, not on player input. It is gated by the build switch
`REALTIME_PROTO`; **comment that switch out in `include/config.h` to get stock
turn-based NetHack back.**

### Design
- **Shared world clock.** `rt_world_tick_ready()` returns true at most once
  every `RT_TURN_MS` of real time. Both the console and the win32 tile/GUI
  build pace to the same clock, so play advances at a constant rate no matter
  how fast keys are pressed (mashing buffers input instead of fast-forwarding).
- **Input never freezes the world.** When no command is buffered on a tick, the
  hero simply waits that turn while monsters and timeouts keep running.
- **Kiting allowed.** The turn-rounding in `mcalcmove()` that existed to prevent
  "melee kiting" is disabled, so monster/hero speed differences translate into
  real positional advantage.
- **Timeouts need no rewrite.** Because one game turn now equals a fixed slice
  of real time, the existing per-turn machinery (`svm.moves`-based timeouts,
  regen, hunger, …) already tracks real time.

### Tunables (`include/config.h`)
- `RT_TURN_MS` — real milliseconds per game turn (default 150; smaller = faster).
- `RT_POLL_MS` — input-poll / CPU-yield granularity while waiting.

### Files changed for this feature
| File | Change |
|------|--------|
| `include/config.h` | `REALTIME_PROTO`, `RT_TURN_MS`, `RT_POLL_MS` switches |
| `include/extern.h` | declarations for `nt_ticks()`, `rt_world_tick_ready()` |
| `src/allmain.c` | shared world clock + real-time console command pacing |
| `src/mon.c` | `mcalcmove()` keeps exact speeds (kiting) under real-time |
| `sys/windows/windsys.c` | `nt_ticks()` millisecond wall-clock |
| `win/win32/mswproc.c` | tile/GUI command input paces to the world clock |
| `win/win32/mhmap.c` | smooth-camera pan interpolation (see below) |

### Smooth motion (win32 tile/GUI)
Two complementary interpolations, both contained in the map window
(`win/win32/mhmap.c`), driven by one WM_TIMER and applied at blit time; when
nothing is moving the render is pixel-identical to stock:

- **Camera pan.** When the map is larger than the window it re-centers on the
  hero, jumping the scroll origin a whole tile per step. We instead render at a
  fractional pixel offset that decays to zero over ~`RT_TURN_MS`, so the view
  pans smoothly.
- **Hero glide.** When the map fits in the window (no scroll), the camera can't
  pan, so the hero *tile itself* glides between cells: onPaint repaints the
  terrain over the hero's cell and draws the hero transparently at the
  interpolated position. Keyed on a single-step move that the camera isn't
  already handling. The back buffer is never modified, so when the glide ends
  the static hero simply reappears with no gap.

- **Monster glide.** The renderer draws from a grid of glyphs and has no entity
  identity, so it cannot tell that the monster now in one square is the one that
  was next door a moment ago. `place_monster()` does know — the monster's
  previous coordinates are still in `mtmp->mx/my` when it is called — so it
  reports single-step moves to a set of motion records keyed by *destination
  square*. The renderer then asks each visible square whether its occupant just
  arrived and from where, a question it can pose without identifying anyone, and
  draws the sprite partway back toward its origin.

  Only squares where the map is actually showing a monster are animated, so this
  cannot reveal a monster the player is not entitled to see. Teleports and level
  placement are ignored, because only adjacent steps are recorded. The core side
  is recording only and affects no gameplay logic.

### Known limitations (prototype)
- Commands that need follow-up input (e.g. `z` then a direction) still block on
  the follow-up keystroke.
- During occupations/running, pacing falls back toward stock behavior.
- Smoothing is a rendering overlay, not real sub-tile position: entities occupy
  whole squares as far as the game is concerned, and only their drawn position
  is interpolated. A monster that is not being shown (unseen, or on terrain with
  no remembered background to erase it with) is drawn stepping, not gliding.
- **Input during animation is dropped, not queued.** Keystrokes made while a
  glide/pan plays used to accumulate and then replay in a burst ("pre-input").
  Each tick now keeps only the most recent buffered keystroke and discards the
  rest, on both the console and win32 paths, so the hero acts on live intent.
  The trade-off is deliberate: rapid multi-key sequences entered mid-animation
  are not all honored.

### Future direction: vector positions (recorded)
The buffered-input problem above and the grid-stepped monster motion both stem
from the same root: positions are **tile/grid quantized** and time is sliced
into discrete turns. The intended long-term fix is to move entities to
**continuous vector positions** (real-valued x/y with velocity), sampling input
continuously and integrating motion per frame, with the grid retained only for
collision/pathfinding/game rules. That removes per-turn input buffering and
makes true sub-tile motion (all entities) natural rather than a rendering
overlay. This is a large engine change and is noted here as the target
architecture, not yet implemented.

---

## Build and structure changes — 2026-07

Work aimed at the `hack.h` "god header", which every source file includes and
which pulls in ~35 sub-headers.

### What was measured first
The original plan was to decompose `hack.h` and hide `struct obj` / `struct
monst` behind accessors. Measurement redirected it:

- The header graph is a **star centered on `hack.h`, with no cycles** — the
  sub-headers are each included by only that one header.
- Struct fields are read **directly in 7,000+ places** (`obj->…` ~3,081,
  `mtmp->…` ~4,187), so wholesale struct hiding is not safely mechanizable.
- The build cost came from **no precompiled header being configured**, not from
  header coupling.

### What was done
- **`include/nhfwd.h`** — canonical forward declarations for the core
  aggregates. A foundation for decomposition; additive and currently unused.
- **Precompiled headers** (`NetHack.vcxproj`, `NetHackW.vcxproj`) — `hack.h` is
  the through-header, `allmain.c` creates the PCH, and files that don't include
  `hack.h` first are excluded after a full conformance scan of every
  `ClCompile` entry. **No source changes.**
- **`include/nhaccess.h`** — opt-in accessors naming the concepts the raw field
  reads keep repeating (the object `where` predicates alone appear 130+ times).
  Additive; no existing call site was migrated.
- **`hack.h` decomposition into subsystem headers.** `hack.h` was never merely
  an include aggregator: ~1,540 of its 1,581 lines were its own definitions for
  a dozen unrelated subsystems. Cohesive blocks now live in their own headers,
  which `hack.h` includes **at the exact position the definitions occupied**:

  | header | contents |
  |---|---|
  | `nh_cmd.h` | command queue, special keys, command dispatch table |
  | `nh_dgntopo.h` | special-level topology and its accessor macros |
  | `nh_savefile.h` | `NHFILE` handle and serializer mode bits |
  | `nh_progstate.h` | `program_state`/`level_status` phases, `InputState` |
  | `nh_makemon.h` | `makemon()`/`goodpos()` flags sharing one bit space |
  | `nh_monnam.h` | articles and suppress masks for naming a monster |
  | `nh_fileprefix.h` | directory classes locating data and state files |
  | `nh_corpstat.h` | corpse/statue creation flags stored in `obj->spe` |
  | `nh_shop.h` | shopkeeper billing: selling states, damage prices, devaluation, repossession |
  | `nh_bubble.h` | drifting bubbles and their contents on the water/air levels |
  | `nh_msg.h` | message classification, `pline` wrappers, yes/no queries, `custompline()` flags |
  | `nh_objsel.h` | object-selection menus: `query_objlist`, `query_category`, `sortloot` |
  | `nh_move.h` | directions, movement styles, `test_move` and `m_move` outcomes |
  | `nh_oname.h` | artifact provenance recorded when an object is named |
  | `nh_trapflags.h` | circumstances under which a trap is triggered |

  `hack.h` remains a facade, so every existing source file and the PCH keep
  working unchanged. It is now 1,018 lines, down from 1,581 — a 36% reduction,
  with each remaining cohesive group given a name and a home.

  Where a group was contiguous it is extracted in place, included at the exact
  position it occupied. Where it was scattered — `nh_shop.h` (four places),
  `nh_msg.h` (four), `nh_objsel.h`, `nh_move.h`, `nh_bubble.h` (two each) — the
  pieces are gathered at the position of the **earliest** of them, so every
  definition becomes available no later than it was before. The savefile
  compatibility flags joined `nh_savefile.h`, whose include moved up to their
  position for the same reason.

  **Verification.** Each round is checked against the pre-decomposition
  baseline by two preprocessor comparisons, both of which must show zero
  differences:

  1. Preprocess a `hack.h`-including translation unit both ways and diff the
     output with `#line` directives stripped — **15,523 code lines, identical**.
     This covers structs, enums, and typedefs.
  2. Preprocess a probe that forces every extracted macro to expand into a
     value, and diff that — this covers the `#define`s, which check 1 cannot
     see, because stripping lines that begin with `#` also strips them.

  The refactor is therefore behavior-preserving at the preprocessor level, not
  merely "it still compiles".

### What was deliberately not done
**The core structs are not hidden**, and no source file was converted from
`hack.h` to minimal includes. Measured per-file compile cost settled the latter:

| configuration | ms/file |
|---|---|
| `hack.h` + PCH (current) | 56 |
| minimal include, no PCH | 96 |
| `hack.h`, no PCH (before) | 138 |

Converting a file off `hack.h` forces it out of the PCH, making it ~40 ms
*slower*; across the 138 PCH-using files that would add ~5.5 s to the build.
That is why decomposition kept `hack.h` as a facade rather than trimming each
file's includes — the structure is split, the build cost is not reintroduced.

Hiding `struct obj` / `struct monst` behind accessors remains out of scope:
their fields are read directly in 7,000+ places, which no build check can
validate mechanically. `nhfwd.h` and `nhaccess.h` are therefore **foundations
with no adopters yet** — intended for code that is newly written or revised,
not for a sweeping migration.

---

## Other local modifications

Beyond the real-time feature above, this tree also carries additional local
changes to various source files that differ from the upstream NetHack 5.0
release. Consult the git history of this repository for the authoritative,
dated record of every change.
