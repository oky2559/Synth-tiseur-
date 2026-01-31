app_sources = files('main.c', 'gui.c')

# Dépendance GTK requise pour l'interface graphique
gtk_dep = dependency('gtk+-3.0', required: true)

executable('base_project_app',
  app_sources,
  include_directories : inc,
  link_with : math_lib,      # On lie avec notre librairie math
  dependencies : [gtk_dep, libm], # On ajoute GTK et Math
  install : true,
)