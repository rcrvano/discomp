# Discomp : Distributed Computing System of Modular Programming

**Discomp** is a distributed computing platform: a central server hands out
computational jobs to a pool of client (worker) nodes, monitors their state,
and collects results. Programs are assembled from reusable **modules**, and the
job-control logic is expressed as [CLIPS](https://www.clipsrules.net/)
expert-system rules — so scheduling behaviour is driven by declarative
fact/rule bases rather than hard-coded C++.

> Copyright (C) 2004–2011 ISDCT — Institute of System Dynamics and Control
> Theory SB RAS.

## Components

The C++ sources live under [`src/`](src/):

| Directory              | Description                                                                 |
| ---------------------- | --------------------------------------------------------------------------- |
| `src/server`           | Central server: distributes jobs, tracks nodes, runs the CLIPS/JS engine.   |
| `src/client`           | Worker node agent: executes modules, monitors the host (via SIGAR).         |
| `src/rpc_client`       | RPC client used to talk to the server.                                      |
| `src/httpserver`       | Embedded HTTP server (status / control interface).                          |
| `src/communicator`     | Messaging / transport layer between nodes.                                  |
| `src/lib`              | Shared library code (sockets, XML, MD5, logging, module abstraction, CLIPS glue). |
| `src/lib/clips`        | Bundled CLIPS rule-engine sources.                                          |
| `src/doc`              | Doxygen configuration and generated docs.                                   |

## Requirements

The C++ code is built on the **Qt 4.7** framework (the `.pro` files are
generated for Qt 4.7.2 / qmake 2.01a). Each component links only the Qt
modules it needs:

| Qt module   | Used for                                        |
| ----------- | ----------------------------------------------- |
| `QtCore`    | core types, threading, containers               |
| `QtNetwork` | node-to-node and client/server communication    |
| `QtXml`     | module / scheme descriptors and message parsing |
| `QtSql`     | job and node state persistence                  |
| `QtScript`  | embedded JavaScript job logic (`js_interpretator`, `module_js`) |

Other dependencies: **CLIPS** (bundled under `src/lib/clips`), **glib-2.0**,
and the [SIGAR](https://github.com/hyperic/sigar) system-info library (client
only, for host monitoring).

> Note: this is **Qt 4**, not Qt 5/6. `QtScript` was removed in later Qt
> versions, so building against modern Qt requires porting.

## Building

Each component is a [qmake](https://doc.qt.io/qt-4.8/qmake-manual.html)
project (`*.pro`). Build everything from `src/`:

```sh
cd src
./make.sh          # builds client, server, rpc_client, httpserver
```

or build a single component:

```sh
cd src/server
qmake && make
```

Clean with `./make_clean.sh` and install with `./make_install.sh`.

## Notes

This repository contains **sources only**. Build artifacts (`obj/`, `*.o`,
`*.so`), core dumps, runtime files (`*.out-*`, `*.rep-*`, `machinefile-*`),
vendored third-party build trees (`src/client/tmp/`), prebuilt SIGAR binaries,
and generated Doxygen HTML are intentionally excluded — see
[`.gitignore`](.gitignore).
