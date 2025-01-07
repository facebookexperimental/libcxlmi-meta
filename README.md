# libcxlmi-meta

This is developed on top of CXL Management Interface library (libcxlmi).

Link: https://github.com/computexpresslink/libcxlmi/tree/main

For more details on the libcxlmi refer libcxlmi/README.md

Source code
===========
Following are the main folders in this repo
- `libcxmli`: git submodule to https://github.com/computexpresslink/libcxlmi/tree/main
If we want to get latest changes from upstream, just update the submodule
pointer

- `cxl`: source code for actual cxl cli based tool, any command string and
its helper function must be add here. However actual implementation must be
moved to `vendor_meta`

- `vendor_meta`: source code for all custom vendor command implementation
along with command opcode and data strctures

- `vendor_util`: place holder for all helper functions or utlities which are
not only specific to vendor command. This contains the parser and other
supporting utility implementations

- Also, note each of this folder contains meson.build file, inc and src
folders. `meson.build` must be updated for any new file addition/deletion.
`src` and `inc` files should be used appropriately

NOTE
====
- Please run the clang-format only on these three folders while submitting the
changes to the repo
  - cxl
  - vendor_meta
  - vendor_util
- Do not run clang-format for libcxlmi

Clone
=====
To clone this repo using https
```
git clone https://github.com/facebookexperimental/libcxlmi-meta.git

cd libcxlmi-meta;
git checkout <branch_name>
```

Initialize the git submodule for the first time.
Note this step is mandatory to fetch the submodule code locally
```
cd libcxlmi-meta;
git submodule init;
git submodule update;
```

Everytime the branch is changed or updated, it is recommended to update the
submodule as well
```
git checkout <branch_name>;

git submodule update;
```

Build
=====
To `configure` the project as a shared library (default):
Note, here build is name of build directory);

```
meson setup build;
```
Alternatively, to configure for static libraries (note required in our case):
```
meson setup --default-library=static build
```
Also, to configure with dbus support  to enable MCTP scanning:
```
meson setup -Dlibdbus=enabled build
```

Then compile it:
```
meson compile -C build;
```
Optionally, to install:
```
meson install -C build
```
To clean:
```
meson install --clean -C build
```

Execution
=========
Executable name: cxl
Executable path: build/cxl/cxl

To list the available commands
```
./build/cxl/cxl --list-cmds
```

To list the args of each command
```
./build/cxl/cxl <cmd>
```

To execute command for one CXL device
```
./build/cxl/cxl <cmd> <cmd_args> mem0
```

To execute command for more than one CXL device, give a list of devices
```
./build/cxl/cxl <cmd> <cmd_args> mem0 mem1 <list_of_devices>
```

To execute command for all CXL device, use all
```
./build/cxl/cxl <cmd> <cmd_args> all
```

Examples
========
```
# ./build/cxl/cxl get-fw-info

 usage: cxl get-fw-info <mem0> [<mem1>..<memN>] [<options>]

    -z, --osimage         select OS(a.k.a boot1) image
```
```
# ./build/cxl/cxl get-fw-info mem0
================================= mem0 : get fw info ==================================
FW Slots Supported: 2
Active FW Slot: 1
Staged FW Slot: 2
FW Activation Capabilities: 1
Slot 1 FW Revision: 2.1.6-ef93b2c00
Slot 2 FW Revision: 
Slot 3 FW Revision: 
Slot 4 FW Revision: 
```
```
# ./build/cxl/cxl get-fw-info mem0 mem1
================================= mem0 : get fw info ==================================
FW Slots Supported: 2
Active FW Slot: 1
Staged FW Slot: 2
FW Activation Capabilities: 1
Slot 1 FW Revision: 2.1.6-ef93b2c00
Slot 2 FW Revision: 
Slot 3 FW Revision: 
Slot 4 FW Revision: 
================================= mem1 : get fw info ==================================
FW Slots Supported: 2
Active FW Slot: 1
Staged FW Slot: 2
FW Activation Capabilities: 1
Slot 1 FW Revision: 2.1.6-ef93b2c00
Slot 2 FW Revision: 
Slot 3 FW Revision: 
Slot 4 FW Revision: 
```
```
# ./build/cxl/cxl get-fw-info all
================================= mem0 : get fw info ==================================
FW Slots Supported: 2
Active FW Slot: 1
Staged FW Slot: 2
FW Activation Capabilities: 1
Slot 1 FW Revision: 2.1.6-ef93b2c00
Slot 2 FW Revision: 
Slot 3 FW Revision: 
Slot 4 FW Revision: 
================================= mem1 : get fw info ==================================
FW Slots Supported: 2
Active FW Slot: 1
Staged FW Slot: 2
FW Activation Capabilities: 1
Slot 1 FW Revision: 2.1.6-ef93b2c00
Slot 2 FW Revision: 
Slot 3 FW Revision: 
Slot 4 FW Revision: 
```
```
# ./build/cxl/cxl get-fw-info -z all
================================= mem0 : get fw info ==================================
OS Slots Supported: 2
Active OS Slot: 1
Staged OS Slot: 2
OS Activation Capabilities: 1
Slot 1 OS Revision: 2.1.6-ef93b2c00
Slot 2 OS Revision: 
Slot 3 OS Revision: 
Slot 4 OS Revision: 
================================= mem1 : get fw info ==================================
OS Slots Supported: 2
Active OS Slot: 1
Staged OS Slot: 2
OS Activation Capabilities: 1
Slot 1 OS Revision: 2.1.6-ef93b2c00
Slot 2 OS Revision: 
Slot 3 OS Revision: 
Slot 4 OS Revision: 
```

