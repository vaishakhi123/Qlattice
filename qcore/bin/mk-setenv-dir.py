#!/usr/bin/env python3

import os
import sys

prefix = os.getenv("prefix")

assert prefix[:-1] != '/'

setenv_fn = os.path.join(prefix, "setenv.sh")

l_init = []
if "--keep" in sys.argv and os.path.isfile(setenv_fn):
    with open(setenv_fn, mode = "r") as f:
        l_init = f.readlines()
        l_init = [ v.rstrip() for v in l_init ]

l = []

l.append("#!/bin/bash")

l.append("")

l.append(f'setenv_prefix="{prefix}"')

if l_init:
    l.append("")
    l.append("# -------------------------------------------------------------------")
    l += l_init
    l.append("# -------------------------------------------------------------------")

recursive = f"""
for v in "$setenv_prefix"/*/setenv.sh ; do
    if [ -f "$v" ] ; then
        echo "Loading:" "$v"
        source "$v"
        echo "Loaded: " "$v"
    fi
done
unset v
"""

l.append(recursive)

l.append(f'unset setenv_prefix')

organize_env_path = f"""
if which organize-env-path.py >/dev/null 2>&1 ; then
    source <(organize-env-path.py)
fi
"""

l.append(organize_env_path)


with open(setenv_fn, mode = "w") as f:
    f.write("\n".join(l))