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
      packages.${system} = rec {
        f3-baremetal = rpkgs.rlib.buildCMakeProject {
          pname = "f3-baremetal";
          version = "v1.0.0";
          src = ./.;

          cmakeBuildInputs = [
            rpkgs.cmsis5-device-f3
            rpkgs.clang-arm-toolchain
            rpkgs.roboenv-loader
            nano.packages.${system}.default
          ];
        };
        default = f3-baremetal;
        f3-can-monitor = rpkgs.rlib.buildCMakeProject {
          pname = "f3-can-monitor";
          version = "v1.0.0";
          src = ./CANMonitor;

          cmakeBuildInputs = [
            rpkgs.clang-arm-toolchain
            rpkgs.roboenv-loader
            nano.packages.${system}.default
            f3-baremetal
          ];
        };
      };
    };
}
