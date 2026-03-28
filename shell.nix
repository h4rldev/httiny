{pkgs ? import <nixpkgs> {}}:
with pkgs;
  mkShell {
    name = "httiny";
    description = "A tiny little HTTP/1.1 server for learning purposes.";

    buildInputs = [
      mold
    ];

    packages = [
      bear
      just
      clang-tools
      valgrind
      gf
    ];
  }
