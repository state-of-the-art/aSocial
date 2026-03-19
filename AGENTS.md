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
  Three UI plugins exist: `ui-gui` (Qt Quick rich GUI), `ui-console` (interactive REPL),
  `ui-cmd` (non-interactive CLI for scripting).
* Storage - enables user to keep files. Regular ones are just storing on disk, advanced ones could
 utilize cloud, encrypt the files on disk or use other ways to keep the files available. In case
 file can't be stored on the system itself - there should be a preview available: small thumbnail
 for photos and a few thumbnail frames of the video.
* DBKV - key-value database with dual-mode support: filesystem mode for unencrypted background
 services (like relay) and QIODevice mode for encrypted profile storage via VFS.
 All CRUD operations use QProtobufMessage for serialisation/deserialisation.
* TODO - I think we need more types of plugins.

Plugins could contain their specific libs (which will be built by the build system) and if they are
using some common dependencies - those need to be put in `./libs` directory in the repo root to be
build.

## Storage Pipeline

Profile data is stored through the VFS → DBKV-rocksdb plugin chain:

1. **VFS-cake** manages an encrypted container file (`data.vfs`). Each passphrase
   derives a unique encryption key that reveals a separate set of virtual files
   (plausible deniability).
2. **DBKV-rocksdb** opens a virtual file (e.g. `dbkv-rocksdb.dat`) from the VFS
   container.  All key-value pairs are serialised to / from protobuf wire format
   and stored in a temporary RocksDB instance.  `flush()` serialises the snapshot
   back through the encrypted VFS layer.
3. **DBKV-json** remains available for unencrypted background/relay operations
   and does **not** store profile data.  It stores protobuf wire format in
   individual `.pb` files on the filesystem.

### DBKV Interface

The `DBKVPluginInterface` stores and retrieves `QProtobufMessage` objects:

* `openDatabase(const QString& path)` – directory-based, for background/relay use.
* `openDatabase(QIODevice* device)` – VFS-compatible, data packed as a single
  stream for encrypted containers.
* `storeObject(key, QProtobufMessage&)` / `retrieveObject(key, QProtobufMessage&)`
  – protobuf-native CRUD.
* `listObjects(prefix)` – prefix-based key scanning.
* `flushDatabase()` / `closeDatabase()` manage persistence and cleanup.

### Key Design

Keys follow a hierarchical prefix scheme for efficient range scans:

```
p                                       -> Profile
a/<persona_uid>                         -> Persona
c/<persona_uid>/<contact_uid>           -> Contact
g/<persona_uid>/<group_uid>             -> Group
gm/<group_uid>/<contact_uid>            -> GroupMember
m/<persona_uid>/<created_ms>/<msg_uid>  -> Message (time-ordered)
e/<persona_uid>/<event_uid>             -> Event
pp/<key>                                -> ProfileParam
```

## Data Schema

Protobuf specification files live in `libs/asocial_proto/proto/asocial/v1/`:

* `profile.proto`       – Profile and Persona types
* `contact.proto`       – Contact (known peer) with avatar, birthday,
                          deathday, relationship_type, parent_contact_uid
* `group.proto`         – Group (with display colour) and GroupMember
* `message.proto`       – Message with Markdown body, content_type,
                          MessageAttachment (filename, mime, data, thumbnail), is_read
* `event.proto`         – Timeline events with event_type (point/range), colour, category
* `profile_param.proto` – Per-profile key-value settings

C++ types are generated at build time by `qt_add_protobuf` into
`${CMAKE_BINARY_DIR}/proto/asocial/v1/*.qpb.h` via the shared
`asocial_proto` library.  Both Core and plugins use these protobuf
types directly through CoreInterface — no QVariantMap conversion.
Factory methods (`create*`) return populated but **unsaved** objects;
callers must call the corresponding `store*` method to persist.

## GUI Interface (ui-gui)

The Qt Quick GUI plugin (`plugins/ui-gui/`) provides a rich graphical interface
designed for both desktop (mouse) and mobile (touch) use.

### Architecture

* **GuiBackend** (`src/GuiBackend.h/cpp`) – QML singleton that bridges all
  CoreInterface operations to QML via `Q_INVOKABLE` methods returning
  `QVariantMap` / `QVariantList`. Temporary `QByteArray` buffers holding
  sensitive data are securely wiped with random bytes before deallocation.
* **Plugin** (`src/plugin.h/cpp`) – Implements `UiPluginInterface`.
  `startUI()` creates a `QQmlApplicationEngine` and loads `Main.qml`.
  The `GuiBackend` singleton is injected via `rootContext()->setContextProperty`.
* **QML files** live in `qml/ui-gui/` and are embedded into the plugin and copied
  to the build directory

### Interaction Model

* **Desktop**: mouse wheel / Ctrl+scroll to zoom the persona graph and timeline.
  Right-click shows the radial context menu; left-click executes the default action.
* **Android**: pinch to zoom, long-press shows the radial menu, short-tap
  executes the default action.
* **Radial menu (context-aware)**: Every visual element in the scene may declare a
  `property var contextActions: [...]` containing its own action descriptors.
  When the user right-clicks (desktop) or long-presses (Android), the
  RadialMenu hit-tests the item tree at that point via recursive
  `childAt()` + parent-chain walk, collecting `contextActions` from every
  ancestor layer. The result is a merged, de-duplicated list ordered
  most-specific-first (topmost hit child) to least-specific (background).
  Each action object carries `{label, icon, action, section, data, handler}`.
  `section` visually groups and colour-codes the wedges; `handler(data)` is
  called when the user picks a wedge. Long-press on a wedge stores that
  action as the default left-click action for the section in profile params.
* **Day/night background**: animated gradient shifts with wall-clock time,
  showing sun (day), moon + stars (night), with smooth transitions.

### Additional Qt Dependencies

`Qt::Quick`, `Qt::QuickControls2`, `Qt::Qml`

## CLI Interface (ui-cmd)

The CLI provides the following command hierarchy:

```
main>
  settings>  list | get <key> | set <key> <value>
  init       <size_mb>
  init_auto  (shows proposed size)
  profile>   open <pw> | create <pw> <name> | current | update <field> <val>
             close | delete | export | import <b64> <vfs_pw>
  persona>   list | create <name> | select <id> | info <id> | update | delete
  contact>   list | add <name> | info <id> | update <id> <f> <v> | delete | search
  group>     list | create <name> | info <id> | update | delete | add_member | remove_member
  message>   list | send <to> <body> | read <id> | delete <id>
  event>     list | create <title> <date> | info <id> | update | delete
  qrcode     <text>
  color | nocolor
```

All CLI data access goes through CoreInterface CRUD methods (no direct
database access from the UI layer).

## Integration Tests

CLI integration tests live in `tests/cli_integration_tests.cpp` and exercise
the full aSocial AppImage through its CLI interface.

The **Kotik** helper class (`tests/kotik/Kotik.h`) manages the complete
lifecycle of a test instance:

* Creates an isolated `QTemporaryDir` per test
* Launches the AppImage with `-w <tmpdir>` so settings and data stay sandboxed
* Streams stdout/stderr in real time and provides assertion helpers:
  `waitForLog()`, `contains()`, `noErrorLogs()`
* `write(command)` sends a CLI command (appends newline automatically)
* `exitApp()` sends `exit` and waits for clean shutdown
* Destructor kills the process if it is still running

Typical test pattern:

```cpp
Kotik* k = new Kotik(this);
QVERIFY(k->waitForLog("main>", 5000));
k->write("help");
QVERIFY(k->waitForLog("Commands available:", 500));
QVERIFY(k->exitApp());
QCOMPARE(k->exitCode(), 0);
```

## Build & validation

IMPORTANT: use only `./build.sh` to check that complete build is working and `./test.sh` to ensure
unit & integration tests are running fine and plugins could be built standalone. No other tool can
help with this task.
