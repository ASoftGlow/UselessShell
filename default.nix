with import <nixpkgs> {};
fastStdenv.mkDerivation {
  name = "env";
  nativeBuildInputs = [ cmake ];
  buildInputs = [ gcc glibc pkg-config ];
  shellHook = "code . && cd build";
}
