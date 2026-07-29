# NetHack — real-time fork

A modified copy of **NetHack 5.0** that can run as a **constant-rate real-time**
game instead of the classic turn-based one: monsters, timeouts, and the whole
world keep advancing on a wall-clock whether or not you press a key.

> This is an unofficial, modified copy of NetHack. It is **not** endorsed by or
> affiliated with the NetHack DevTeam. What you have here is not what they
> distributed. See [`MODIFICATIONS.md`](MODIFICATIONS.md).

## Turn real-time on/off

Real-time play is controlled by one switch in [`include/config.h`](include/config.h):

```c
#define REALTIME_PROTO          /* comment out for stock turn-based NetHack */
#ifdef REALTIME_PROTO
#define RT_TURN_MS 150          /* real ms per game turn; smaller = faster */
#define RT_POLL_MS 10
#endif
```

Rebuild after changing it. Full design notes are in
[`MODIFICATIONS.md`](MODIFICATIONS.md).

## Building (Windows, Visual Studio / Build Tools)

```
sys\windows\fetch.cmd lua            REM fetch the required Lua source
sys\windows\fetch.cmd pdcursesmod    REM (optional) curses interface
```

Then build the solution `sys\windows\vs\NetHack.sln` (x64), or from a
Developer Command Prompt:

```
msbuild sys\windows\vs\NetHack.sln /p:Configuration=Debug /p:Platform=x64 /m
```

Outputs: `NetHackW.exe` (tile/GUI, "image" mode) and `NetHack.exe` (console).
Both honor the real-time switch.

## Trying it

Stand next to a monster and press nothing: in stock NetHack the world freezes;
here the monster keeps coming and attacks. Speed differences now matter — you
can kite slower monsters.

## License

NetHack, and this modified copy, are distributed under the **NetHack General
Public License (NGPL)** — see [`dat/license`](dat/license). The complete source
is this repository. You may share and modify it, provided you keep it freely
available under the same license.

Upstream project: <https://github.com/NetHack/NetHack>
