# aSocial agent helper file

aSocial is a p2p distributed private social network.

## Dev Stack

* Qt 6.10
* Android / Desktop
* C++
* CMake

## Changes

Please make sure that if any of the structure was changed - it need to be reflected in AGENTS.md,
as well as in README.md to ensure no documentation drift will happen.

## Platform

In it's core aSocial is a basic console application with core functionality that makes it work - it
organizes the framework of how modules are connected and defines the interfaces for them, as well
as dispatching control messages between the plugins.

Plugins consist of 3 parts:
* Implementation, which is a static library that contains logic and used in plugin and unit tests
* Plugin which is a shared library with Qt plugin interface to dynamically connect to the aSocial
* Tests that are built if `-DBUILD_TESTS=ON` and could be started by `ctest`.

Plugins are:
* Comm - allows communication between aSocial nodes. It's not just networking, but also using
  the other mediums for message transferring.
* VFS - Encrypted Virtual filesystem for storage and communication between aSocial nodes. Need to
  be secure and follow plausable deniability.
* UI - shows the UI for the user. Separated plugin to allow to run aSocial as a headless service.
* Storage - enables user to keep files. Regular ones are just storing on disk, advanced ones could
  utilize cloud, encrypt the files on disk or use other ways to keep the files available. In case
  file can't be stored on the system itself - there should be a preview available: small thumbnail
  for photos and a few thumbnail frames of the video.
* DBKV - unencrypted key-value database for background services (like relay) day-to-day operations.
* DBSQL - encrypted SQL database to store account information.
* TODO - I think we need more types of plugins.

Plugins could contain their specific libs (which will be built by the build system) and if they are
using some common dependencies - those need to be put in `./libs` directory in the repo root to be
build.

## Build & validation

IMPORTANT: use only `./build.sh` to check that complete build is working and `./test.sh` to ensure
unit & integration tests are running fine and plugins could be built standalone. No other tool can
help with this task.
