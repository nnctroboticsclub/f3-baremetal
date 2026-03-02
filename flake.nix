{
  inputs.nixpkgs.url = "github:nixos/nixpkgs/release-25.11";

  inputs.roboenv.url = "github:nnctroboticsclub/roboenv-nix";
  inputs.roboenv.inputs.nixpkgs.follows = "nixpkgs";

  inputs.nano.url = "github:nnctroboticsclub/nano";
  inputs.nano.inputs.roboenv.follows = "roboenv";
  inputs.nano.inputs.nixpkgs.follows = "nixpkgs";

  outputs =
    {
      self,
      nixpkgs,
      roboenv,
      nano,
    }:
    let
      system = "x86_64-linux";
      rpkgs = roboenv.legacyPackages.${system};
    in
    {
      packages.x86_64-linux.default = rpkgs.rlib.buildCMakeProject {
        pname = "f3-baremetal";
        version = "v0.2.0";
        src = ./.;

        cmakeBuildInputs = [
          rpkgs.cmsis5-device-f3
          rpkgs.clang-arm-toolchain
          rpkgs.roboenv-loader
          nano.packages.${system}.default
        ];
      };
    };
}
