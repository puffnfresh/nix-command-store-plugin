{ stdenv, nixUnstable, pkgconfig, cmake }:

stdenv.mkDerivation {
  name = "nix-command-store-plugin";
  buildInputs = [ nixUnstable pkgconfig cmake ];
  src = ./.;
}
