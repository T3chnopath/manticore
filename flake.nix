{
  description = "Manticore Flake";

  inputs = {
    nixpkgs.url = "nixpkgs/nixos-22.11";
    flake-utils.url = "github:numtide/flake-utils";
    threadx.url = "github:azure-rtos/threadx";
    threadx.flake = false;
  };

  outputs = { self, nixpkgs, flake-utils, threadx }:
    flake-utils.lib.eachDefaultSystem (system: let
      pkgs = import nixpkgs { inherit system; };
    in {
      packages = rec {
        default = pkgs.stdenv.mkDerivation {
          name = "manticore";
          src = ./.;

          # Assuming you want to build ThreadX too, you can include it as a source
          nativeBuildInputs = [ pkgs.makeWrapper ]; # or other build tools you might need

          # You can specify the source path to the ThreadX repository
          buildInputs = with pkgs; [
            python39
          ];

          buildPhase = ''
            mkdir -p $out
            cp -r $src $out/src  # Copy ThreadX source to the output
            # Your build commands here
          '';

          installPhase = ''
            # Your install commands here
          '';

        };
      };
    });
}
