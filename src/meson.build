libbase_project = library('baseproject',
  sources: ['custom_math.c'],
  include_directories: inc,
  install: true
)

libbase_project_dep = declare_dependency(
  link_with: libbase_project,
  include_directories: inc
)
