#!/usr/bin/env bash
set -e
set -x

showHelp() {
# `cat << EOF` This means that cat should stop reading when EOF is detected
cat << EOF
Usage: $0 [-t=<tag>]
Build and push docker images

-h,     --help,        Display help

-t,     --tag=<tag>,         Set tag for images to be built
EOF
# EOF is found above and hence cat command stops reading. This is equivalent to echo but much neater when printing out.
}


# $@ is all command line parameters passed to the script.
# -o is for short options like -v
# -l is for long options with double dash like --version
# the comma separates different long options
# -a is for long options with single dash like -version
options=$(getopt -l "help,tag::" -o "ht::" -a -- "$@")

# set --:
# If no arguments follow this option, then the positional parameters are unset. Otherwise, the positional parameters
# are set to the arguments, even if some of them begin with a ‘-’.
eval set -- "$options"

export tag=latest

while true
do
case "$1" in
-h|--help)
    showHelp
    exit 0
    ;;
-t|--tag)
    shift
    export tag="$1"
    ;;
--)
    shift
    break;;
esac
shift
done


if [[ -n $tag ]]; then
  SCRIPT_ROOT="$( cd "$( dirname "${BASH_SOURCE[0]}" )" > /dev/null && pwd )"
  PROJECT_ROOT=${SCRIPT_ROOT}/..

  docker buildx build --push -t adaflow/adaflow-devel-cpu:$(arch)-$tag -f $PROJECT_ROOT/docker/adaflow-devel-cpu.dockerfile $PROJECT_ROOT

  docker buildx build  --build-arg DEVEL_TAG=$(arch)-$tag --pull --push  -t adaflow/adaflow-runtime-cpu:$(arch)-$tag -f $PROJECT_ROOT/docker/adaflow-runtime-cpu.dockerfile $PROJECT_ROOT

  docker buildx build --push -t adaflow/adaflow-devel-cuda:$tag -f $PROJECT_ROOT/docker/adaflow-devel-cuda.dockerfile $PROJECT_ROOT

  docker buildx build --pull --push -t adaflow/adaflow-devel-cpu:$tag -f $PROJECT_ROOT/docker/adaflow-runtime-cuda.dockerfile $PROJECT_ROOT
fi