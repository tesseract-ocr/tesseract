"""Sample learning content, seeded once when the tables are empty. Edit this file (or insert rows) to change the curriculum.
`correct` is a list of option indexes. It never leaves the server before a quiz is submitted."""

MODULES = [
    {
        "slug": "ocr-basics", "title": "OCR basics", "topic": "Getting started", "difficulty": "beginner", "position": 1,
        "summary": "What OCR is, how to run your first extraction, and how to read the results.",
        "lessons": [
            {"title": "What is OCR?", "minutes": 4, "content": """# What is OCR?

**Optical Character Recognition** turns pictures of text (scans, photos, PDFs) into text a computer can search, copy and edit.

## How VegeOCR works
1. You upload an **image** or **PDF**.
2. The engine finds each **line** of text and reads it.
3. You get the text back, with a **confidence** score for every line.

## Good to know
- Clear, well-lit, upright pages give the best results.
- Digital PDFs that already contain text are read directly; scanned pages are recognised from the picture.
""", "quiz": {"title": "Check: what is OCR?", "questions": [
                {"prompt": "What does OCR do?", "kind": "single", "options": ["Compresses images", "Turns pictures of text into text", "Translates between languages", "Encrypts documents"], "correct": [1], "explanation": "OCR recognises characters in an image and outputs them as text."},
                {"prompt": "Which inputs can VegeOCR read?", "kind": "multiple", "options": ["Images (PNG, JPEG...)", "PDF files", "Audio recordings", "Scanned pages"], "correct": [0, 1, 3], "explanation": "Images, PDFs and scanned pages work; audio is not supported."}]}},
            {"title": "Running your first extraction", "minutes": 5, "content": """# Running your first extraction

1. Open **New OCR** in the dashboard.
2. Drop a file in the box and pick a **language**. Japanese + English is the default.
3. Press **Run OCR**.

## Reading the result
- Each line shows its **confidence**. Lines under 70% are outlined in red: check those first.
- You can **copy** the text or download it as **.txt**, **.json** or **.pdf**.
- Large documents run in the background; the page updates by itself when they finish.
"""},
            {"title": "Choosing the right language", "minutes": 4, "content": """# Choosing the right language

The language tells the engine which characters to expect.

| You have | Choose |
|---|---|
| Japanese with some English or numbers | Japanese + English |
| Simplified Chinese | Simplified Chinese |
| Traditional Chinese | Traditional Chinese |
| Only English | English |

**Tip:** mixing in the wrong language lowers accuracy. If a page is only English, choose English.
""", "quiz": {"title": "Check: languages", "questions": [
                {"prompt": "A receipt has Japanese text and prices in Arabic numerals. Which setting fits best?", "kind": "single", "options": ["English", "Japanese + English", "Traditional Chinese", "Any, they are equivalent"], "correct": [1], "explanation": "Japanese + English covers kanji, kana and Latin letters/digits."}]}},
        ],
    },
    {
        "slug": "better-accuracy", "title": "Getting more accurate results", "topic": "Quality", "difficulty": "intermediate", "position": 2,
        "summary": "Prepare your documents, read confidence scores and correct mistakes so the model improves.",
        "lessons": [
            {"title": "Preparing your documents", "minutes": 6, "content": """# Preparing your documents

Better input means better output.

- **Resolution:** aim for about **300 DPI**. Tiny or blurry text is the most common cause of errors.
- **Orientation:** photos are rotated automatically from their EXIF data, but flat scans work best.
- **Contrast:** dark text on a plain light background.
- **Cropping:** remove large borders and shadows.

## PDFs
Scanned PDFs are rendered at 300 DPI before recognition. Password-protected PDFs are not supported.
""", "quiz": {"title": "Check: preparation", "questions": [
                {"prompt": "Roughly what resolution gives good OCR results?", "kind": "single", "options": ["30 DPI", "72 DPI", "300 DPI", "Higher is always worse"], "correct": [2], "explanation": "About 300 DPI is the usual target for text recognition."},
                {"prompt": "Which of these usually HURT accuracy?", "kind": "multiple", "options": ["Blurry photos", "Dark text on a light background", "Heavy shadows across the page", "Tiny text"], "correct": [0, 2, 3], "explanation": "Blur, shadows and tiny text are common causes of errors."}]}},
            {"title": "Reading confidence scores", "minutes": 5, "content": """# Reading confidence scores

Every line has a confidence from 0 to 100.

- **90+** is usually right.
- **70-90** is probably right: skim it.
- **Under 70** deserves a look. These lines are outlined in red in the editor.

A high score is not a guarantee, and a low score is not always wrong: use it to decide **where to look first**.
"""},
            {"title": "Correcting mistakes", "minutes": 6, "content": """# Correcting mistakes

Open a result, edit any line in the **Line corrections** tab and press **Save corrections**.

## Why corrections matter
Each corrected line is saved with its **image** and the **right text**. After an administrator approves it, it becomes **training data**, and the model is retrained from it. A new model only goes live if it measures better than the current one.

## Tips
- Correct the line, not the whole page: line-level edits are what training uses.
- Only correct what you are sure of.
""", "quiz": {"title": "Check: corrections", "questions": [
                {"prompt": "What happens to a line you correct?", "kind": "single", "options": ["Nothing, it only changes your copy", "It is saved as training data once an administrator approves it", "It is published to other users immediately", "The whole document is deleted"], "correct": [1], "explanation": "Corrections are queued for review and used for training after approval."},
                {"prompt": "A new model is deployed only if...", "kind": "single", "options": ["it was trained today", "it has more training samples", "it measures better than the current model", "an API key requests it"], "correct": [2], "explanation": "Candidates are evaluated on held-out lines and rejected if they don't improve."}]}},
        ],
    },
    {
        "slug": "api-and-keys", "title": "Using the API and API keys", "topic": "Integration", "difficulty": "advanced", "position": 3,
        "summary": "Create scoped API keys, call the OCR and learning endpoints, and handle limits and errors.",
        "lessons": [
            {"title": "API keys and scopes", "minutes": 6, "content": """# API keys and scopes

Create keys under **API keys**. Each key has:

- a **name**, an **expiration** (never, 30/90/365 days or a custom date) and **scopes**.
- a secret that is shown **once**. We store only a hash, so a lost key cannot be recovered: **rotate** it instead.

## Scopes
| Scope | Allows |
|---|---|
| `read` | read OCR jobs and key usage |
| `write` | run OCR, submit training data |
| `learning:read` | read modules, lessons, progress |
| `learning:write` | update progress, submit quizzes |
| `admin` | everything (administrators only) |

Give each integration the **fewest scopes** it needs.
""", "quiz": {"title": "Check: keys and scopes", "questions": [
                {"prompt": "You lost a key's secret. What do you do?", "kind": "single", "options": ["Look it up in the dashboard", "Rotate the key to get a new secret", "Ask support to email it", "Nothing, it still works without it"], "correct": [1], "explanation": "Secrets are stored hashed, so they can't be shown again. Rotating issues a new one and revokes the old."},
                {"prompt": "A key with only learning:read tries POST /api/v1/progress. What status do you get?", "kind": "single", "options": ["200", "401", "403", "404"], "correct": [2], "explanation": "The key is valid (so not 401) but lacks learning:write, so the answer is 403."}]}},
            {"title": "Calling the API", "minutes": 7, "content": """# Calling the API

Send the key in the **X-API-Key** header.

```
curl -X POST $API/api/v1/ocr -H "X-API-Key: YOUR_KEY" -F file=@invoice.pdf
```

Small files answer immediately (**200**). Large files return **202** with a job id: poll `GET /api/v1/jobs/{id}`.

The **API docs** page lists every endpoint with its required scope, a curl example and a sample response.
"""},
            {"title": "Limits and errors", "minutes": 5, "content": """# Limits and errors

Every key has a **rate limit** (requests per minute) and a **monthly quota**.

| Status | Meaning |
|---|---|
| 401 | missing, invalid, disabled, revoked or expired key |
| 403 | the key lacks the scope this endpoint needs |
| 429 | rate limit or quota reached: wait for `Retry-After` |

Errors always look like `{"error": {"code": "...", "message": "..."}}`.
""", "quiz": {"title": "Check: errors", "questions": [
                {"prompt": "Which status means the key is valid but not allowed to do this?", "kind": "single", "options": ["401", "403", "404", "429"], "correct": [1], "explanation": "403 = authenticated but missing the required scope."},
                {"prompt": "What should a client do on a 429?", "kind": "single", "options": ["Retry immediately in a loop", "Wait for the Retry-After period", "Create a new key", "Switch to GET"], "correct": [1], "explanation": "Respect Retry-After; hammering the API keeps you rate limited."}]}},
        ],
    },
    {
        "slug": "ocr-lab", "title": "Handwriting, blurry images and your own training data", "topic": "Extraction lab", "difficulty": "intermediate", "position": 4,
        "summary": "Read handwriting, rescue poor-quality images, and teach the model with your own corrected samples.",
        "lessons": [
            {"title": "Reading handwriting", "minutes": 6, "content": """# Reading handwriting

Open **Learning → Extraction lab**, choose **Handwriting**, and drop in a photo or scan.

## How it works
- The image is split into **lines**, and each line is read on its own by a handwriting model (**TrOCR**).
- Every line gets a **confidence** score. Lines below the review threshold are **outlined in red**: check those first.
- You can edit any line, copy the text, or download it as `.txt`.

## What to know
- The handwriting engine reads **English** only. For other languages the lab falls back to Tesseract and tells you so; expect much lower accuracy on handwriting.
- It handles **print and cursive**. If the writing is neat block capitals, **Printed** mode is often just as good or better.
- Best results: dark ink on plain paper, even light, the whole line in frame, one column of text.
""", "quiz": {"title": "Check: handwriting", "questions": [
                {"prompt": "What do the red-outlined lines in a result mean?", "kind": "single", "options": ["They were already corrected", "Their confidence is below the review threshold, so they may be wrong", "They are in a different language", "They were skipped"], "correct": [1], "explanation": "Low-confidence lines are flagged so you review them first."},
                {"prompt": "Which of these help the handwriting engine?", "kind": "multiple", "options": ["Dark ink on plain paper", "Even lighting", "Several columns of text on one page", "The whole line inside the picture"], "correct": [0, 1, 3], "explanation": "Clear contrast, even light and complete lines help; multi-column layouts confuse the line finder."}]}},
            {"title": "Rescuing blurry and low-quality images", "minutes": 6, "content": """# Rescuing blurry and low-quality images

With **Image enhancement: Auto**, every upload is measured first:

| Measured | Flagged when |
|---|---|
| Edge sharpness | the page is blurry or slightly soft |
| Noise | grain or speckle is high |
| Lighting | shadows or a brightness gradient |
| Text size | characters are too small to read reliably |

## What enhancement does
Only the steps that are needed: **denoise**, **flatten shadows**, **straighten** a tilted page, **upscale** small text, **sharpen**, and **boost contrast** for very faint pages.

## You stay in control
- The lab shows the **original and enhanced images side by side**, what was wrong, and what was applied.
- For printed text both versions are read and the one the engine is **more confident** about is kept.
- Enhancement can't invent detail: a badly out-of-focus photo is best retaken.
""", "quiz": {"title": "Check: enhancement", "questions": [
                {"prompt": "In Auto mode, when does enhancement run?", "kind": "single", "options": ["Always", "Never", "Only when the image is measured as needing it", "Only for PDFs"], "correct": [2], "explanation": "Auto measures blur, noise, lighting and text size first."},
                {"prompt": "What is the best fix for a photo that is severely out of focus?", "kind": "single", "options": ["Enhance it twice", "Retake the photo", "Switch to Japanese", "Delete the dataset"], "correct": [1], "explanation": "Enhancement cannot recover detail that was never captured."}]}},
            {"title": "Training on your own data", "minutes": 8, "content": """# Training on your own data

Under **Learning → My training data**, a **dataset** holds samples: a picture of **one line** of text and **exactly what it says**.

## Getting samples in
1. **Add** a line image and its text by hand.
2. **Import a zip** with a `labels.tsv` (image path, a tab, the text) and the images.
3. **Correct lines** in the Extraction lab and save them: the line is cropped for you.

## Training
- You need at least **10 samples**. One in five is **held out** and never trained on.
- Press **Train**. When it finishes you see the **error rate before and after** on the held-out lines. **Lower is better.**
- If it improved, the model is switched on. If not, you are told, and it stays off: add more varied samples and retrain.
- Pick **My model** in the lab to use it.

## Keep your data portable
Every dataset can be **exported as a zip** in the same format the importer reads.
""", "quiz": {"title": "Check: training", "questions": [
                {"prompt": "Why are some samples held out during training?", "kind": "single", "options": ["To save disk space", "So accuracy is measured on lines the model has never seen", "Because they are wrong", "To make training faster"], "correct": [1], "explanation": "Scoring on training data would flatter the model."},
                {"prompt": "What should one training sample contain?", "kind": "single", "options": ["A whole page and its text", "One line image and its exact text", "Only text", "A PDF"], "correct": [1], "explanation": "Samples are single lines with the exact transcription."},
                {"prompt": "The new model's error rate was higher than before. What happens?", "kind": "single", "options": ["It replaces the old one anyway", "It is kept off, and you are told", "Your data is deleted", "The server restarts"], "correct": [1], "explanation": "A model that does not improve is never switched on automatically."}]}},
        ],
    },
]
