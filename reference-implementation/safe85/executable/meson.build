project(
  'safe85_exe',
  'c',
  version : '1.0.0',
  license : 'MIT',
  default_options : ['warning_level=3']
)

project_source_files = [
  'src/main.c'
]

project_dependencies = [
  dependency('safe85', fallback : ['safe85', 'safe85_dep']),
]

build_args = [
]


# ===================================================================

# ======
# Target
# ======

build_args += [
  '-DPROJECT_NAME=' + meson.project_name(),
  '-DPROJECT_VERSION=' + meson.project_version(),
]

project_target = executable(
  'safe85',
  project_source_files,
  dependencies: project_dependencies,
  install : true,
  c_args : build_args,
)

test('basic', project_target)
