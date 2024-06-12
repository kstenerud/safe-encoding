project(
  'safeenc',
  'c',
  version : '1.0.0',
  license : 'MIT',
  default_options : ['warning_level=3']
)

project_source_files = [
  'src/main.c'
]

project_dependencies = [
  dependency('safe16', fallback : ['safe16', 'safe16_dep']),
  dependency('safe32', fallback : ['safe32', 'safe32_dep']),
  dependency('safe64', fallback : ['safe64', 'safe64_dep']),
  dependency('safe80', fallback : ['safe80', 'safe80_dep']),
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
  'safeenc',
  project_source_files,
  dependencies: project_dependencies,
  install : true,
  c_args : build_args,
)

test('basic', project_target)
