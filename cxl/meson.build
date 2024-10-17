# SPDX-License-Identifier: LGPL-2.1-or-later
#
# This file is part of libcxlmi.
#
sources = [
    'src/cxl_main.c',
    'src/cxl_cmd.c',
    'src/cmd_parser.c',
]

includes = [
    'inc',
]

deps = [
    libcxlmi_dep,
    libvendor_meta_dep,
    libvendor_util_dep,
]

executable(
    'cxl',
    sources,
    dependencies: deps,
    include_directories: [includes, inc]
)
