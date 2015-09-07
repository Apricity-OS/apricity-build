Project [Calamares][] includes a fork of the KDE Partition Manager code to take
advantage of the abstraction it implements on top of libparted.

Calarames uses this code to build an internal library named `libcalapm`. To
build it, one must switch to the `calamares` branch and build from the
`calamares` subdirectory.

Building from the `calamares` subdirectory is important: building from the top
directory still builds KDE Partition Manager!

The library contains only a subset of KDE Partition Manager code. Some files
are not built, some have parts removed out with `#ifndef CALAMARES... #endif`
blocks.

The library is installed as `libcalapm.so`, but does not install any headers.
Code using this library is expected to use KDE Partition Manager source code.

[Calamares]: http://github.com/calamares
