# RouteFix PSP plugin

This is a small PSP PRX plugin scaffold for testing whether your inverted speaker or headphone behavior can be corrected in software.

## What it does

- Forces the audio routing mode to `1`, which is the mode commonly described as "speaker on".
- Polls the PSP headphone detect state through `sceHprmIsHeadphoneExist()`.
- Logs both values to `ms0:/seplugins/routefix.log`.

## Why this is useful

There are two different failure shapes:

1. The routing state is wrong, but the hardware detect signal is fine.
2. The headphone detect switch in the jack is physically wrong or stuck.

This plugin helps separate those.

If forcing routing mode makes audio behave normally while the plugin is loaded, the problem is at least partly software-routable.

If the detect state stays inverted or the route keeps snapping back, the jack detect hardware is more likely the root cause.

## Build

You need PSPSDK installed so `psp-config` is available.

```sh
make
```

Expected output is a PRX named `routefix.prx`.

## Install

Copy `routefix.prx` to `ms0:/seplugins/`.

Add this line to `vsh.txt` and `game.txt`:

```txt
ms0:/seplugins/routefix.prx 1
```

Then enable it in recovery and reboot.

## Test

1. Boot with nothing plugged into the headphone jack.
2. Check speaker behavior.
3. Plug headphones in.
4. Check headphone behavior.
5. Read `ms0:/seplugins/routefix.log`.

## Reading the log

Example:

```txt
start hprm_hp=0 route_before=0 route_after=1 forced=1
tick hprm_hp=1 route_before=1 route_after=1 forced=1
```

- `hprm_hp=0` means the PSP thinks no headphones are present.
- `hprm_hp=1` means the PSP thinks headphones are present.
- `route_after=1` means the plugin successfully requested speaker-on routing.

## If you want the opposite experiment

Change this line in `main.c`:

```c
#define ENFORCED_ROUTE_MODE ROUTE_MODE_SPEAKER_ON
```

to:

```c
#define ENFORCED_ROUTE_MODE ROUTE_MODE_HEADPHONE_ONLY
```

Then rebuild and retest.
