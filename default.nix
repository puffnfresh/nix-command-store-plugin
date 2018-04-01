{ nix }:

with import <nixpkgs> { };

stdenv.mkDerivation {
  name = "nix-script-store-plugin";
  buildInputs = [ nix pkgconfig cmake ];
  src = ./.;
}
