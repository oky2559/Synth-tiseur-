{ pkgs, lib, config, inputs, ... }:

{
  packages = [ pkgs.git pkgs.meson pkgs.clang_20 pkgs.python314Packages.ninja ];
  languages.c.enable = true;
  languages.cplusplus.enable = true;

  # See full reference at https://devenv.sh/reference/options/
}
