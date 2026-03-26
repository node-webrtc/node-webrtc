#!/usr/bin/env bash

NODE_ROOT=""
if command -v asdf; then
  NODE_ROOT="$(asdf where nodejs)"
fi

if [[ "x${NODE_ROOT}" = "x" ]]; then
  NODE_ROOT="$(realpath "$(dirname "$(which node)")/..")"
fi

lldb -- "$NODE_ROOT/bin/node" "$NODE_ROOT/lib/node_modules/npm/bin/npm-cli.js" test
