project(
  'selena',
  'c',
  version: '0.0.1',
  default_options: [
    'c_std=c99',
    'warning_level=3'
  ]
)

selena_inc = include_directories('include')

selena_lib = static_library(
  'selena_runtime',
  'src/selena-runtime.c',
  include_directories: selena_inc,
  install: true
)

selena_dep = declare_dependency(
  include_directories: selena_inc,
  link_with: selena_lib
)

executable(
  'selena_hello',
  'examples/hello.c',
  dependencies: selena_dep,
  install: false
)

install_headers('include/selena.h')
