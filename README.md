SirenSetting Limit Adjuster v2.0.0
by cp702
==================================

This mod raises various siren limits in GTA V.

The per-vehicle siren limit is raised from 20 to 32. Vehicles can now have
siren1 through siren32, which are all fully functioning and independent. The
siren_glass limit is also raised to 32. Note that the limit of 128 total bones
on a vehicle still exists.

The maximum value of a siren setting ID in carcols is raised to 65535. In the
base game, siren settings can conflict if one of them has a value over 255.
Now, siren setting IDs can go up to 65535 without conflicting with each other.

This mod also expands the maximum number of unique siren settings you can have.
If you have 400 cars, and they all have unique light patterns, that'll work. 

### Installation ###

Copy the included ASI to your game folder.

### Usage ###

For additional lights, just use siren21 through siren32 and add extra entries
to the siren settings in carcols.meta. Again, note that there is still a limit
of 128 total bones on a single vehicle.

For the increased ID limit, just use a number bigger than "255" as the ID.
The values 0, 255, and 65535 are reserved, so don't use those specific values.

No additional configuration is required.

### RPH Users ###

SSLA does not require RAGEPluginHook, but it is compatible with it. However,
it is **ONLY** compatible with the latest version! RPH version 1.98 has a
compatibility patch that will play nicely with SSLA v2. The latest RPH version
can be found bundled with LSPDFR at https://www.lspdfr.com, or on the
RAGEPluginHook Discord server.

If you had RAGEPluginHook and SSLA v1, you may have had a compatibility DLL in
your Plugins folder. This DLL is no longer required and should be removed. RPH
now talks directly to SSLA.

RPH plugins that deal with emergency lighting will continue to function, but
may need to be updated to support lights 21 through 32.

### FiveM Users ###

SSLA v2 is compatible with FiveM. The server owner must enable ASIs in
`server.cfg` with `sv_scripthookallowed true`, and each user must copy the ASI
into the plugins folder in their FiveM Application Data.

### RPH Developers ###

In RPH version 1.98 and up, the `Lights` member of an `EmergencyLighting` will
be length `EmergencyLighting.MaxLights` (a static class property) instead of
necessarily being length 20. They will still all have the same length, it's
just that that length will be longer than 20 if SSLA is installed.

### Logging ###

This ASI produces a SirenSettings.log file in your game directory. It records:
* Every loaded SirenSetting, its name, its ID, and its internal index;
* Every loaded vehicle, its name, its siren ID, and the index it was matched to;
* Any conflicts, where two SirenSetting entries have the same ID.

### License ###

Please link to an official upload instead of redistributing this yourself.

### Changelog ###
2.0.0: Enabled the use of up to 32 sirens. Also reworked RPH compatibility and
       added FiveM support.
1.1.0: Removed compatibility with RPH build 1.78 (for GTA build 1868.1). Added
       compatibility with RPH build 1.81 (for GTA build 2060).

### Credits ###

cp702: Author
LMS: RPH integration help
LMS, alexguirre: Research assistance
Cartres, Jacobmaate, PNWParksFan: Screenshots
Officer Failz, Jacobmaate, NefariousBonne: Testing
PNWParksFan, SuperStumpje, Swed, 11john11: Testing
Cartres, NefariousBonne, Officer Failz, PNWParksFan: Test vehicle creation