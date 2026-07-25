FROM nixos/nix:latest

# Enable flakes; disable sandbox (required inside containers).
RUN echo "experimental-features = nix-command flakes" >> /etc/nix/nix.conf \
    && echo "sandbox = false" >> /etc/nix/nix.conf

# DO NOT add packages here. Declare all project tools in flake.nix instead.
# The nix store is mounted from the host — packages install on first run
# and are cached across invocations. See docs/adr/0008 and docs/adr/0011.

WORKDIR /workspace
CMD ["/bin/bash"]
