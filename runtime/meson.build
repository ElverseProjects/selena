
project(
  'Selena C function dispatching. Runtime library.',      
  'c',
  version: '0.2.0',
  license: 'GPL 3.0',
  default_options: [
    'c_std=c11',
    'warning_level=3',
    'werror=true',
    'buildtype=release'
  ],
  meson_version: '>= 0.60.0'
)

cc = meson.get_compiler('c')

inc = include_directories('include')

# config.h
conf_data = configuration_data()
conf_data.set('VERSION', meson.project_version())
conf_data.set('PROJECT_NAME', meson.project_name())

configure_file(
  output: 'config.h',
  configuration: conf_data
)

sources = files(
  'src/selena-runtime.c',
)

# 7. Сборка исполняемого файла
static_library(
  'task2',
  sources,
  include_directories: inc,
  # dependencies: [ llibs_dep ],
  install: true
)
