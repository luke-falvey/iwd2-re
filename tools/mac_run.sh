#!/bin/zsh
# Start the Mac build in a play folder with the test driver on, in the
# background. Output goes to <play>/iwd2re-run.log; a crash also leaves
# <play>/iwd2re-stacks.txt.
#
# Usage: tools/mac_run.sh [play dir] [port] [exe]
PLAY=${1:-$HOME/iwd2-play-port}
PORT=${2:-47800}
EXE=${3:-${0:A:h:h}/build-mac/iwd2-re}
cd "$PLAY" || exit 1
rm -f iwd2re-stacks.txt
IWD2RE_TEST_DRIVER=$PORT nohup "$EXE" > iwd2re-run.log 2>&1 &
echo "pid $! port $PORT log $PLAY/iwd2re-run.log"
