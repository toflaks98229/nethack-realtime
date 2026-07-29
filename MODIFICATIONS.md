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

Independent monster motion is not yet interpolated (monsters still step
cell-to-cell); per-entity monster sprite interpolation, which needs a
core-side motion source, is possible future work.

### Known limitations (prototype)
- Commands that need follow-up input (e.g. `z` then a direction) still block on
  the follow-up keystroke.
- During occupations/running, pacing falls back toward stock behavior.
- Smoothing covers the camera pan and the hero's own movement; other monster
  sprites still step grid-to-grid rather than sliding.

---

## Other local modifications

Beyond the real-time feature above, this tree also carries additional local
changes to various source files that differ from the upstream NetHack 5.0
release. Consult the git history of this repository for the authoritative,
dated record of every change.
