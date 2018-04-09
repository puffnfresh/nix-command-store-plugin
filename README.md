# nix-command-store-plugin

This currently relies on https://github.com/NixOS/nix/pull/1713

Nix 2.0 plugin to create a `command://` store. The store specifies a command to
execute to build derivations. It can be used as a remote builder:

    nix-build example.nix \
      --option plugin-files libnix-command-store.dylib \
      --builders 'command:///etc/nix/build-via-docker.sh x86_64-linux'

For example, a command using Docker on a remote host:

    #!/bin/sh
    
    if [ "$1" == "master" ]; then
      echo "started"
    else
      ssh build-server docker run -i nix-2.0 nix-daemon --stdio
    fi
