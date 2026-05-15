load("@rules_cc//cc:defs.bzl", "cc_library")
load("@rules_license//rules:license.bzl", "license")

# License
package(default_applicable_licenses = [ ":license" ])
exports_files([ "LICENSE" ])
license(name = "license", license_kinds = [ "@rules_license//licenses/spdx:0BSD" ], license_text = "LICENSE")

# Common
cc_library(
  name = "atom-common",
  srcs = [ "common/src/panic.cc" ],
  hdrs = [
    "common/include/atom/arena.hh",
    "common/include/atom/arguments.hh",
    "common/include/atom/bit.hh",
    "common/include/atom/const_char_array.hh",
    "common/include/atom/detail/parse_utils.hh",
    "common/include/atom/float.hh",
    "common/include/atom/hash.hh",
    "common/include/atom/integer.hh",
    "common/include/atom/literal.hh",
    "common/include/atom/meta.hh",
    "common/include/atom/non_copyable.hh",
    "common/include/atom/non_moveable.hh",
    "common/include/atom/panic.hh",
    "common/include/atom/punning.hh",
    "common/include/atom/result.hh",
    "common/include/atom/vector_n.hh",
  ],
  includes = [ "common/include" ],
  deps = [ "@fmt" ],
  visibility = [ "//visibility:public" ]
)

# Logger
cc_library(
  name = "atom-logger",
  srcs = [
    "logger/src/logger.cc",
    "logger/src/sink/console.cc",
    "logger/src/sink/file.cc",
  ],
  hdrs = [
    "logger/include/atom/logger/logger.hh",
    "logger/include/atom/logger/sink/console.hh",
    "logger/include/atom/logger/sink/file.hh",
  ],
  includes = [ "logger/include" ],
  deps = [ ":atom-common" ],
  visibility = [ "//visibility:public" ]
)

# Mathematics
cc_library(
  name = "atom-math",
  hdrs = [
    "math/include/atom/math/box3.hh",
    "math/include/atom/math/frustum.hh",
    "math/include/atom/math/matrix4.hh",
    "math/include/atom/math/plane.hh",
    "math/include/atom/math/quaternion.hh",
    "math/include/atom/math/traits.hh",
    "math/include/atom/math/vector.hh",
  ],
  includes = [ "math/include" ],
  deps = [ ":atom-common" ],
  visibility = [ "//visibility:public" ]
)

# Monolith
cc_library(
  name = "atom",
  deps = [
    ":atom-common",
    ":atom-logger",
    ":atom-math",
  ],
  visibility = [ "//visibility:public" ]
)
