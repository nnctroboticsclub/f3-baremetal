{
  inputs.nixpkgs.url = "github:nixos/nixpkgs/release-25.11";

  inputs.roboenv.url = "git+ssh://git@github.com/nnctroboticsclub/roboenv-nix.git";

  outputs =
    { nixpkgs, roboenv, ... }:
    let
      system = "x86_64-linux";
      pkgs = import nixpkgs {
        inherit system;
      };
      rpkgs = roboenv.legacyPackages.${system};
    in
    {
      packages.x86_64-linux.default = rpkgs.rlib.buildCMakeProject {
        pname = "f3-baremetal";
        version = "v0.2.0";
        src = ./.;

        cmakeBuildInputs = [ ];
        nativeBuildInputs = [
          pkgs.gtest
        ];
      };
    };
}
