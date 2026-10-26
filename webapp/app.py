import os, subprocess, tempfile
from extract import extract, SUPPORTED
import psycopg
from urllib.parse import quote
from flask import Response, Flask, render_template, request, redirect, url_for, jsonify

TESSERACT = os.environ.get("TESSERACT_BIN", "tesseract")
TESSDATA = os.environ.get("TESSDATA_PREFIX")
DB = dict(
    host=os.environ.get("PGHOST", "localhost"),
    port=os.environ.get("PGPORT", "5432"),
    dbname=os.environ.get("PGDATABASE", "ocrvege"),
    user=os.environ.get("PGUSER", "postgres"),
    password=os.environ.get("PGPASSWORD", ""),
)

LANGS = [
    ("jpn+eng", "日本語 Japanese + English"),
    ("jpn", "日本語 Japanese"),
    ("jpn_vert", "日本語 Japanese (vertical)"),
    ("chi_sim", "简体中文 Chinese (Simplified)"),
    ("chi_tra", "繁體中文 Chinese (Traditional)"),
    ("chi_sim+eng", "简体中文 Chinese (Simplified) + English"),
    ("chi_tra+eng", "繁體中文 Chinese (Traditional) + English"),
    ("eng", "English"),
]
DEFAULT_LANG = LANGS[0][0]

app = Flask(__name__)
app.config["MAX_CONTENT_LENGTH"] = 20 * 1024 * 1024


def db():
    return psycopg.connect(**DB)


def init_db():
    with db() as c:
        c.execute(
            """create table if not exists ocr_results(
                 id serial primary key,
                 filename text not null,
                 lang text not null,
                 text text not null,
                 created_at timestamptz not null default now())"""
        )


@app.get("/")
def index():
    with db() as c:
        rows = c.execute(
            "select id, filename, lang, text, created_at from ocr_results order by id desc limit 50"
        ).fetchall()
    return render_template("index.html", langs=LANGS, exts=",".join(sorted(SUPPORTED)), rows=rows, total_chars=sum(len(r[3]) for r in rows), error=request.args.get("error"))


@app.post("/upload")
def upload():
    f = request.files.get("image")
    lang = request.form.get("lang", DEFAULT_LANG)
    if not f or not f.filename:
        return redirect(url_for("index", error="Choose an image first"))
    if lang not in dict(LANGS):
        return redirect(url_for("index", error="Bad language code"))
    with tempfile.NamedTemporaryFile(suffix=os.path.splitext(f.filename)[1]) as tmp:
        f.save(tmp.name)
        try:
            text = extract(tmp.name, f.filename, lang)
        except ValueError as e:
            return redirect(url_for("index", error=str(e)))
        except Exception as e:
            return redirect(url_for("index", error=f"OCR failed: {e}"))
    with db() as c:
        c.execute(
            "insert into ocr_results(filename, lang, text) values (%s,%s,%s)",
            (f.filename, lang, text),
        )
    return redirect(url_for("index"))


@app.get("/download/<int:rid>.txt")
def download(rid):
    with db() as c:
        row = c.execute("select filename, text from ocr_results where id=%s", (rid,)).fetchone()
    if not row:
        return ("Not found", 404)
    name = os.path.splitext(row[0])[0] + ".txt"
    return Response(row[1], mimetype="text/plain; charset=utf-8",
                    headers={"Content-Disposition": f"attachment; filename*=UTF-8''{quote(name)}"})


@app.post("/delete/<int:rid>")
def delete(rid):
    with db() as c:
        c.execute("delete from ocr_results where id=%s", (rid,))
    return redirect(url_for("index"))


@app.get("/health")
def health():
    with db() as c:
        c.execute("select 1")
    return jsonify(status="ok", tesseract=subprocess.run([TESSERACT, "--version"], capture_output=True, text=True).stderr.split("\n")[0] or "ok")


if __name__ == "__main__":
    init_db()
    app.run(host="127.0.0.1", port=int(os.environ.get("PORT", 5000)))
