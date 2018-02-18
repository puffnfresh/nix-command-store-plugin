with import <nixpkgs> { };

stdenv.mkDerivation {
  name = "nix-script-store-plugin";
  buildInputs = [ nixUnstable pkgconfig ];
  src = ./.;
  buildPhase = ''
    $CXX \
      -std=c++14 -g -Wall -include config.h \
      $(pkg-config --cflags --libs nix-store) \
      -shared \
      -o script-store-plugin.so \
      script.cc script-store.cc
  '';
  installPhase = ''
    mkdir -p $out/lib
    mv script-store-plugin.so $out/lib/
  '';
}
