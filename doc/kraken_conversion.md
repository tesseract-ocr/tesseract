# Kraken ↔ Tesseract model conversion (best-effort)

## Script

- `src/training/kraken_to_tesseract.py`

## Prerequisites

- Python 3.9+
- `torch`
- `numpy`

Example installation:

```bash
python3 -m pip install torch numpy
```

## Usage

Convert a kraken `.mlmodel` into a conversion bundle:

```bash
python3 src/training/kraken_to_tesseract.py \
  --input /path/to/model.mlmodel \
  --output_prefix /path/to/output/eng
```

If the kraken VGSL contains unsupported layers, the script exits with details.
To still emit a partial bundle:

```bash
python3 src/training/kraken_to_tesseract.py \
  --input /path/to/model.mlmodel \
  --output_prefix /path/to/output/eng \
  --allow_unsupported
```

## Generated files

For `--output_prefix /path/to/output/eng`:

- `/path/to/output/eng.network_spec`: mapped Tesseract VGSL spec
- `/path/to/output/eng.weights.npz`: extracted kraken tensor weights (NumPy)
- `/path/to/output/eng.conversion.json`: mapping summary and unsupported layer report

## VGSL feature mapping

Mapped directly where possible (depends on input model):

- Input / shape (`1...`)
- Convolution (`C...`, including `Cr/Ct/Cf` forms accepted by Tesseract)
- Maxpool (`M...`, including `Mp...`)
- LSTM family (`L...` supported by Tesseract parser)
- Fully connected / output (`F...`, `O...`)
- Other core Tesseract parser families (`S`, `P`, `A1`, `R`, `T`)

Kraken extensions that have no direct Tesseract equivalent are reported as unsupported:

- `Do` (Dropout) — dropped by default, or treated as unsupported with `--keep_dropout`
- `Bn` (BatchNorm)
- `Gr` (GroupNorm)
- `A<act>` standalone activation layers (except `A1` reduction)
- `Lpa` attention-based LSTM variants

## Format differences and limitation

Kraken stores models as PyTorch metadata + state dict, while Tesseract runtime
models use Tesseract's C++ binary network serialization inside the `.lstm`
component in `.traineddata`.

This script currently exports a conversion bundle (`.network_spec` + `.weights.npz`
+ `.conversion.json`) and reports unsupported VGSL features. It does **not** yet
emit a final Tesseract `.lstm`/`.traineddata` binary directly.
