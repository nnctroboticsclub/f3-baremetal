{
  inputs.roboenv.url = "git+ssh://git@github.com/nnctroboticsclub/roboenv-nix.git";
  inputs.nano.url = "git+ssh://git@github.com/nnctroboticsclub/nano.git";

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
