#!/usr/bin/env python3

import sys

if not (sys.version_info.major == 3 and sys.version_info.minor >= 6):
    print(sys.argv)
    print("You are using not supported Python {}.{}.".format(sys.version_info.major, sys.version_info.minor))
    sys.exit(1)

import os

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

l.append("func() {")
l.append("")
l.append("local setenv_prefix")
l.append("local v")
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
"""

l.append(recursive)

l.append("")
l.append("}")
l.append("")
l.append("func")

organize_env_path = f"""
if python-check-version.py >/dev/null 2>&1 && which organize-env-path.py >/dev/null 2>&1 ; then
    eval "$(organize-env-path.py)"
fi
"""

l.append(organize_env_path)

with open(setenv_fn, mode = "w") as f:
    f.write("\n".join(l))
