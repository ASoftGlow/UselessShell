with import <nixpkgs> {};
fastStdenv.mkDerivation {
  name = "env";
  nativeBuildInputs = [ cmake clang-tools ];
  buildInputs = [ gcc glibc pkg-config ];
  shellHook = "code . && mkdir -p build/linux && cd build/linux";
}
