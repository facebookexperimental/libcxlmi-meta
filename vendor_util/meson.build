# SPDX-License-Identifier: LGPL-2.1-or-later
#

deps = [
]

includes = [
  '.'
]

sources = [
	'src/util_main.c',
	'src/parse_option.c',
	'src/usage.c',
	'src/strbuf.c',
	'src/abspath.c'
]

vendor_util = library('vendor_util', # defaults to shared lib
		sources,
		include_directories : [inc],
		dependencies : deps,
		install : true)

libvendor_util_dep = declare_dependency(
    include_directories: [includes],
    link_with: vendor_util,
)
