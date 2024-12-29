# CS2 External Kernel Driver Hack

## Dependencies

- [ImGui](https://github.com/ocornut/imgui) (already included)
- [ImGui Freetype](https://github.com/ocornut/imgui/tree/master/misc/freetype) (easiest way through vcpkg)
- [reversing-commons header-only lib](https://github.com/stdNullPtr/reversing-commons)

## Project Overview

This solution contains 2 projects:

- km-xd
    - The IOCTL Kernel Mode driver project. It builds a simple IOCTL driver that is used to proxy read/write mem calls
    - Produces **km-xd.sys**
- um-xd
    - The Client Mode application that interacts with the driver and has the actual cheat implementation.
    - /sdk folder contains dumps from [this awesome dumper](https://github.com/a2x/cs2-dumper)
    - Produces **um-xd.exe**

**COMPILE IN RELEASE MODE FOR SIGNIFICANT PERFORMANCE DIFFERENCE**

<h2>I will not show you how to build/run, this is for educational purposes. It's pretty simple if you are not a cheating
paster.</h2>
<h3><i>I don't support cheating, I support educational curiosity.</i></h3>

## Concept

The client mode application will read/write memory from a foreign process (cs2.exe) through an IOCTL driver
using control codes.

```cpp
DeviceIoControl(handle_, control_codes::read, &driver_request, sizeof(driver_request), &driver_request, sizeof(driver_request), nullptr, nullptr);
```

This approach makes the client app _**invisible**_ to the target of your memory reads, since the target
does not know that it's memory has been compromised. The target app has no way to find out since the manipulation is
done from Kernel space to Usermode.

## Examples

![kdmapper](/github/img/kd-mapper.png)
<br>
*Loading with [kdmapper](https://github.com/TheCruZ/kdmapper)*

![wait-join-game.png](/github/img/wait-join-game.png)
<br>
*The client will always maintain a valid state depending on where you are (in-game/minimized/loading)*

![menu.png](/github/img/menu.png)
<br>
*Cheat menu in-game (press INSERT)*

![menu-toggled.png](/github/img/menu-toggled.png)
<br>
*box ESP toggled*

![toggled.png](/github/img/toggled.png)
<br>
*box ESP is blue when the enemy is radar-visible to you (safe to aim lock)*