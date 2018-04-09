{ stdenv, nixUnstable, pkgconfig, cmake }:

stdenv.mkDerivation {
  name = "nix-script-store-plugin";
  buildInputs = [ nixUnstable pkgconfig cmake ];
  src = ./.;
}
