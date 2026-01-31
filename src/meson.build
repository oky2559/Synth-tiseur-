cc = meson.get_compiler('c')
libm = cc.find_library('m', required: true)

lib_sources = files('custom_math.c')

# On crée la librairie partagée
math_lib = shared_library('baseproject',
  lib_sources,
  include_directories: inc,
  dependencies: [libm],
  install: true,
)

# On déclare la dépendance pour que d'autres cibles puissent l'utiliser
libbase_project_dep = declare_dependency(
  link_with: math_lib,
  include_directories: inc,
  dependencies: [libm]
)