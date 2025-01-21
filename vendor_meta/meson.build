# SPDX-License-Identifier: LGPL-2.1-or-later
#
# This file is part of libcxlmi.
#
deps = [
  libdbus_dep,
  libcxlmi_dep,
]

includes = [
  '.'
]

sources = [
	'src/vendor_commands.c',
]

vendor_meta = library('vendor_meta', # defaults to shared lib
		sources,
		include_directories : [inc],
		dependencies : deps,
		install : true)

libvendor_meta_dep = declare_dependency(
    include_directories: [includes],
    link_with: vendor_meta,
)
