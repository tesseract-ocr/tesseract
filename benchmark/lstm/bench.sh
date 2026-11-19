#!/bin/bash
# Times lstmtraining for a fixed number of training lines.
#
# Usage: bench.sh BUILDDIR [COMMAND PREFIX ...]
#
#   BUILDDIR  build directory with bin/lstmtraining and bin/combine_lang_model,
#             absolute or relative to the Tesseract source directory.
#   PREFIX    optional command which runs lstmtraining, for example
#             "env OMP_THREAD_LIMIT=1" or "taskset -c 5-9,15-19".
#
# Environment variables:
#   ITER      number of training iterations (default 1000)
#   NAME      name of the run for the log and the output files
#             (default: name of the build directory)
#   NET_SPEC  network specification (default: the one of tesstrain)
#   WORK      directory for the model, checkpoints and logs
#             (default: $TMPDIR/tess-train-bench)
#   REPO      Tesseract source directory (default: the one of this script)
#
# The training data is test/testdata/eng.Arial.exp0.lstmf (1036 lines), the
# starter traineddata is made from test/testdata/eng/eng.unicharset and the
# sibling directory ../langdata_lstm. The training starts from scratch, so
# the times of different builds are comparable, and the last log line shows
# whether the results are the same.
#
# Run only one benchmark at a time and look at the load of the machine
# (uptime) before trusting a result.

set -e

if [ $# -lt 1 ]; then
  sed -n '2,25p' "$0" | sed 's/^# \{0,1\}//'
  exit 1
fi

REPO=${REPO:-$(cd "$(dirname "$0")/../.." && pwd)}
ITER=${ITER:-1000}
NET_SPEC=${NET_SPEC:-[1,36,0,1 Ct3,3,16 Mp3,3 Lfys48 Lfx96 Lrx96 Lfx192 O1c111]}
WORK=${WORK:-${TMPDIR:-/tmp}/tess-train-bench}

builddir=$1
shift
case $builddir in
  /*) ;;
  *) builddir=$REPO/$builddir ;;
esac
NAME=${NAME:-$(basename "$builddir")}
if [ $# -eq 0 ]; then
  set -- env
fi

lstmf=$REPO/test/testdata/eng.Arial.exp0.lstmf
for f in "$builddir/bin/lstmtraining" "$builddir/bin/combine_lang_model" "$lstmf" \
         "$REPO/test/testdata/eng/eng.unicharset" "$REPO/../langdata_lstm/eng"; do
  if [ ! -e "$f" ]; then
    echo "Missing $f" >&2
    exit 1
  fi
done

mkdir -p "$WORK/out" "$WORK/lang"
if [ ! -f "$WORK/lang/eng/eng.traineddata" ]; then
  "$builddir/bin/combine_lang_model" \
    --input_unicharset "$REPO/test/testdata/eng/eng.unicharset" \
    --script_dir "$REPO/../langdata_lstm" --output_dir "$WORK/lang" --lang eng \
    > "$WORK/combine_lang_model.log" 2>&1 || {
    tail -5 "$WORK/combine_lang_model.log" >&2
    exit 1
  }
fi
echo "$lstmf" > "$WORK/train.list"

log=$WORK/$NAME.log
rm -f "$WORK/out/$NAME"_* "$WORK/out/$NAME".*
LC_ALL=C /usr/bin/time -p "$@" "$builddir/bin/lstmtraining" \
  --traineddata "$WORK/lang/eng/eng.traineddata" --net_spec "$NET_SPEC" \
  --model_output "$WORK/out/$NAME" --train_listfile "$WORK/train.list" \
  --max_iterations "$ITER" > "$log" 2>&1 || {
  tail -5 "$log" >&2
  exit 1
}

times=$(grep -E '^(real|user|sys) ' "$log" | tr '\n' ' ')
last=$(grep 'At iteration' "$log" | tail -1 | sed -e 's/^At iteration //' | cut -c1-76)
printf '%-18s %s| %s\n' "$NAME:" "$times" "$last"
