{ stdenv, nixUnstable, pkgconfig, cmake, boost }:

stdenv.mkDerivation {
  name = "nix-command-store-plugin";
  nativeBuildInputs = [  pkgconfig cmake ];
  buildInputs = [ nixUnstable boost ];
  src = ./.;

  meta = with stdenv.lib; {
    description = "Nix 2.0 plugin to create a command:// store.";
    homepage = "https://github.com/puffnfresh/nix-command-store-plugin";
    license = licenses.lgpl;
  };
}
