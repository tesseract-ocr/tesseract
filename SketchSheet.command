#!/bin/zsh
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
cd "$SCRIPT_DIR"

if [[ ! -d ".venv" ]]; then
  osascript -e 'display dialog "Missing .venv in project folder. Open Terminal and run setup first." buttons {"OK"} default button "OK"'
  exit 1
fi

source ".venv/bin/activate"
python3 -m tools.sketch_sheet.quick_app

