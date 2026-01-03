# aSocial agent helper file

aSocial is a p2p distributed private social network.

## Dev Stack

* Qt 6.10
* Android / Desktop
* C++
* CMake

## Platform

In it's core aSocial is a basic console application with core functionality that makes it work - it
organizes the framework of how modules are connected and defines the interfaces for them, as well
as dispatching control messages between the plugins.

Plugins are:
* Comm - allows communication between aSocial nodes. It's not just networking, but also using
  the other mediums for message transferring.
* Protection - communication between aSocial nodes need to be secure and potentially follow
  plausable deniability, so those plugins should help to transform message before it's sent and
  after it's received.
* UI - shows the UI for the user. Separated plugin to allow to run aSocial as a headless service.
* Storage - enables user to keep files. Regular ones are just storing on disk, advanced ones could
  utilize cloud, encrypt the files on disk or use other ways to keep the files available. In case
  file can't be stored on the system itself - there should be a preview available: small thumbnail
  for photos and a few thumbnail frames of the video.
* TODO - I think we need more types of plugins.

## Build & validation

IMPORTANT: use only `./build.sh` to verify the changes and make sure they are working fine. No
other tool can help with this task.
