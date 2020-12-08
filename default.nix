with import <nixpkgs> {};
stdenv.mkDerivation {
  name = "gamagora-realtime";

  hardeningDisable = [ "fortify" ];

  buildInputs = [cmake glfw glm cimg x11];

  NIX_CFLAGS_COMPILE="-I../external/glad/include -I../external/tinyply/include";
}
