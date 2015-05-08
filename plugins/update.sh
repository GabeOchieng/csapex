#!/bin/bash
bld=$(tput bold)       # Bold
blu=$(tput setaf 4)    # Blue
rst=$(tput sgr0)       # Text reset

for f in *; do
  if [[ -d $f ]]; then
    cd $f
    if [[ -d .git ]]; then
      echo "${bld}${blu}pulling $f${rst}"
      git pull
    fi
    cd - 1> /dev/null
  fi
done
