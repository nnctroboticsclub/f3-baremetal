{
  inputs.roboenv.url = "github:nnctroboticsclub/roboenv-nix";
  inputs.nano.url = "github:nnctroboticsclub/nano";

  outputs =
    {
      self,
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

          F3BARE_USE_STUB_BOOTLOADER = "1";
          F3BARE_EMULATION = "0";

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
            rpkgs.segger-rtt
            nano.packages.${system}.default
            f3-baremetal
          ];
        };
      };
    };
}
