"""OCR lab API (JWT): extraction runs with before/after previews, user datasets, corrections and on-demand training."""
from __future__ import annotations

from typing import List, Optional

from fastapi import APIRouter, Depends, File, Form, Query, UploadFile
from fastapi.responses import FileResponse, Response
from pydantic import BaseModel, Field
from sqlalchemy import func, select
from sqlalchemy.orm import Session

from ..config import settings
from ..db import get_db
from ..deps import current_user
from ..errors import ApiError
from ..models import Dataset, DatasetModel, DatasetSample, LabRun, User
from ..ocrlab import datasets as ds_svc, forms as form_svc, llm_extract, natori, service, train

router = APIRouter(prefix="/api/web/lab", tags=["web: OCR lab"])


@router.get("/capabilities")
def capabilities(_: User = Depends(current_user)):
    return service.capabilities()


# ---- extraction runs --------------------------------------------------------------------------------------------

@router.post("/runs", status_code=202)
async def create_run(file: UploadFile = File(...), mode: str = Form("printed"), lang: str = Form("eng"), enhance: str = Form("auto"),
                     engine: str = Form("auto"), dataset_id: Optional[int] = Form(None), db: Session = Depends(get_db), user: User = Depends(current_user)):
    data = await file.read(service.MAX_UPLOAD + 1)
    return service.out(service.create_run(db, user, file.filename or "upload", data, mode, lang, enhance, engine, dataset_id), with_result=False)


@router.get("/runs")
def list_runs(db: Session = Depends(get_db), user: User = Depends(current_user)):
    service.reap_stale(db, user.id)
    rows = db.scalars(select(LabRun).where(LabRun.user_id == user.id).order_by(LabRun.created_at.desc()).limit(30)).all()
    return [service.out(r, with_result=False) for r in rows]


@router.get("/runs/{run_id}")
def get_run(run_id: str, db: Session = Depends(get_db), user: User = Depends(current_user)):
    service.reap_stale(db, user.id)
    return service.out(service.own_run(db, run_id, user))


@router.delete("/runs/{run_id}", status_code=204)
def delete_run(run_id: str, db: Session = Depends(get_db), user: User = Depends(current_user)):
    import shutil
    r = service.own_run(db, run_id, user)
    shutil.rmtree(settings.storage_dir / "lab" / r.id, ignore_errors=True)
    db.delete(r)
    db.commit()
    return Response(status_code=204)


@router.get("/runs/{run_id}/pages/{page}/{which}")
def run_image(run_id: str, page: int, which: str, db: Session = Depends(get_db), user: User = Depends(current_user)):
    """`which` is `original` or `enhanced`: the two halves of the side-by-side preview."""
    return FileResponse(service.page_image(service.own_run(db, run_id, user), page, which), media_type="image/png", headers={"Cache-Control": "private, max-age=3600"})


class Correction(BaseModel):
    page: int = Field(ge=1)
    index: int = Field(ge=0)
    text: str = Field(min_length=1, max_length=300)
    dataset_id: int


@router.post("/runs/{run_id}/corrections", status_code=201)
def correct(run_id: str, body: Correction, db: Session = Depends(get_db), user: User = Depends(current_user)):
    """The feedback loop: a corrected line becomes a (line image, right text) sample in one of YOUR datasets."""
    r = service.own_run(db, run_id, user)
    ds = ds_svc.owned(db, body.dataset_id, user)
    return service.save_correction(db, user, r, body.page, body.index, body.text, ds)


# ---- datasets -----------------------------------------------------------------------------------------------------

class DatasetIn(BaseModel):
    name: str = Field(min_length=1, max_length=80)
    category: str = Field(default="general", max_length=40)
    lang: str = "eng"
    description: str = Field(default="", max_length=500)


class DatasetPatch(BaseModel):
    name: Optional[str] = Field(default=None, min_length=1, max_length=80)
    category: Optional[str] = Field(default=None, max_length=40)
    description: Optional[str] = Field(default=None, max_length=500)


@router.get("/datasets")
def list_datasets(category: str = "", db: Session = Depends(get_db), user: User = Depends(current_user)):
    stmt = select(Dataset).where(Dataset.user_id == user.id).order_by(Dataset.category, Dataset.name)
    if category:
        stmt = stmt.where(Dataset.category == category.lower())
    rows = db.scalars(stmt).all()
    counts = dict(db.execute(select(DatasetSample.dataset_id, func.count()).where(DatasetSample.dataset_id.in_([d.id for d in rows])).group_by(DatasetSample.dataset_id)).all()) if rows else {}
    cats = sorted({c for (c,) in db.execute(select(Dataset.category).where(Dataset.user_id == user.id).distinct())})
    return {"items": [ds_svc.out(d, counts) for d in rows], "categories": cats}


@router.post("/datasets", status_code=201)
def create_dataset(body: DatasetIn, db: Session = Depends(get_db), user: User = Depends(current_user)):
    return ds_svc.out(ds_svc.create(db, user, body.name, body.category, body.lang, body.description))


@router.post("/datasets/import", status_code=201)
async def import_new_dataset(file: UploadFile = File(...), name: str = Form(""), category: str = Form(""), lang: str = Form(""),
                             db: Session = Depends(get_db), user: User = Depends(current_user)):
    """Create a dataset from a zip (labels.tsv + images, optional dataset.json). Explicit form fields win over dataset.json."""
    data = await file.read(60 * 1024 * 1024)
    _, _, meta = ds_svc.read_zip(data)
    stem = (file.filename or "imported").rsplit(".", 1)[0]
    ds = ds_svc.create(db, user, name or meta.get("name") or stem, category or meta.get("category") or "general", lang or meta.get("lang") or "eng", meta.get("description", ""))
    try:
        report = ds_svc.import_zip(db, ds, data)
    except ApiError:
        ds_svc.delete(db, ds)
        raise
    return {"dataset": ds_svc.out(ds), "import": report}


@router.get("/datasets/{dataset_id}")
def get_dataset(dataset_id: int, db: Session = Depends(get_db), user: User = Depends(current_user)):
    ds = ds_svc.owned(db, dataset_id, user)
    return {**ds_svc.out(ds), "models": [ds_svc.model_out(m) for m in ds.models], "training": train.requirements(ds)}


@router.patch("/datasets/{dataset_id}")
def patch_dataset(dataset_id: int, body: DatasetPatch, db: Session = Depends(get_db), user: User = Depends(current_user)):
    ds = ds_svc.owned(db, dataset_id, user)
    if body.name is not None and body.name.strip() != ds.name:
        if db.scalar(select(Dataset).where(Dataset.user_id == user.id, Dataset.name == body.name.strip())):
            raise ApiError(409, "dataset_exists", f"You already have a dataset called '{body.name.strip()}'")
        ds.name = body.name.strip()
    if body.category is not None:
        ds.category = body.category.strip().lower()[:40] or "general"
    if body.description is not None:
        ds.description = body.description.strip()
    db.commit()
    return ds_svc.out(ds)


@router.delete("/datasets/{dataset_id}", status_code=204)
def delete_dataset(dataset_id: int, db: Session = Depends(get_db), user: User = Depends(current_user)):
    ds_svc.delete(db, ds_svc.owned(db, dataset_id, user))
    return Response(status_code=204)


@router.get("/datasets/{dataset_id}/export")
def export_dataset(dataset_id: int, db: Session = Depends(get_db), user: User = Depends(current_user)):
    ds = ds_svc.owned(db, dataset_id, user)
    from urllib.parse import quote
    return Response(ds_svc.export_zip(ds), media_type="application/zip", headers={"Content-Disposition": f"attachment; filename*=UTF-8''{quote(ds.name)}.zip"})


@router.post("/datasets/{dataset_id}/import")
async def import_into_dataset(dataset_id: int, file: UploadFile = File(...), db: Session = Depends(get_db), user: User = Depends(current_user)):
    ds = ds_svc.owned(db, dataset_id, user)
    return ds_svc.import_zip(db, ds, await file.read(60 * 1024 * 1024))


# ---- samples ----------------------------------------------------------------------------------------------------------

class SampleText(BaseModel):
    text: str = Field(min_length=1, max_length=300)


def own_sample(db: Session, dataset_id: int, sample_id: int, user: User) -> DatasetSample:
    ds = ds_svc.owned(db, dataset_id, user)
    s = db.get(DatasetSample, sample_id)
    if not s or s.dataset_id != ds.id:
        raise ApiError(404, "sample_not_found", "Sample not found")
    return s


@router.get("/datasets/{dataset_id}/samples")
def list_samples(dataset_id: int, q: str = "", page: int = Query(1, ge=1), per_page: int = Query(20, ge=1, le=100),
                 db: Session = Depends(get_db), user: User = Depends(current_user)):
    ds = ds_svc.owned(db, dataset_id, user)
    stmt = select(DatasetSample).where(DatasetSample.dataset_id == ds.id)
    if q:
        stmt = stmt.where(DatasetSample.text.ilike(f"%{q}%"))
    total = db.scalar(select(func.count()).select_from(stmt.subquery()))
    rows = db.scalars(stmt.order_by(DatasetSample.id.desc()).offset((page - 1) * per_page).limit(per_page)).all()
    return {"total": total, "page": page, "per_page": per_page, "items": [ds_svc.sample_out(s) for s in rows]}


@router.post("/datasets/{dataset_id}/samples", status_code=201)
async def add_sample(dataset_id: int, image: UploadFile = File(...), text: str = Form(...), db: Session = Depends(get_db), user: User = Depends(current_user)):
    ds = ds_svc.owned(db, dataset_id, user)
    return ds_svc.sample_out(ds_svc.add_sample(db, ds, await image.read(ds_svc.MAX_IMAGE_BYTES + 1), text, source="upload"))


@router.patch("/datasets/{dataset_id}/samples/{sample_id}")
def edit_sample(dataset_id: int, sample_id: int, body: SampleText, db: Session = Depends(get_db), user: User = Depends(current_user)):
    from ..services.samples import clean_label
    s = own_sample(db, dataset_id, sample_id, user)
    s.text = clean_label(body.text)
    db.commit()
    return ds_svc.sample_out(s)


@router.delete("/datasets/{dataset_id}/samples/{sample_id}", status_code=204)
def delete_sample(dataset_id: int, sample_id: int, db: Session = Depends(get_db), user: User = Depends(current_user)):
    ds_svc.remove_sample(db, own_sample(db, dataset_id, sample_id, user))
    return Response(status_code=204)


@router.get("/datasets/{dataset_id}/samples/{sample_id}/image")
def sample_image(dataset_id: int, sample_id: int, db: Session = Depends(get_db), user: User = Depends(current_user)):
    return FileResponse(own_sample(db, dataset_id, sample_id, user).image_path, media_type="image/png", headers={"Cache-Control": "private, max-age=600"})


# ---- training -----------------------------------------------------------------------------------------------------------

@router.post("/datasets/{dataset_id}/train", status_code=202)
def train_dataset(dataset_id: int, db: Session = Depends(get_db), user: User = Depends(current_user)):
    return ds_svc.model_out(train.start(db, ds_svc.owned(db, dataset_id, user)))


@router.get("/datasets/{dataset_id}/models")
def list_models(dataset_id: int, db: Session = Depends(get_db), user: User = Depends(current_user)):
    ds = ds_svc.owned(db, dataset_id, user)
    return {"models": [ds_svc.model_out(m) for m in ds.models], "training": train.requirements(ds)}


def own_model(db: Session, dataset_id: int, model_id: int, user: User):
    ds = ds_svc.owned(db, dataset_id, user)
    m = db.get(DatasetModel, model_id)
    if not m or m.dataset_id != ds.id:
        raise ApiError(404, "model_not_found", "Model not found")
    return ds, m


@router.post("/datasets/{dataset_id}/models/{model_id}/activate")
def activate_model(dataset_id: int, model_id: int, db: Session = Depends(get_db), user: User = Depends(current_user)):
    ds, m = own_model(db, dataset_id, model_id, user)
    train.activate(db, ds, m, True)
    return ds_svc.model_out(m)


@router.post("/datasets/{dataset_id}/models/{model_id}/deactivate")
def deactivate_model(dataset_id: int, model_id: int, db: Session = Depends(get_db), user: User = Depends(current_user)):
    ds, m = own_model(db, dataset_id, model_id, user)
    train.activate(db, ds, m, False)
    return ds_svc.model_out(m)


# ---- order forms (発注書) -----------------------------------------------------------------------------------------

def _catalogs():
    """Catalogs shipped with the app plus any built into storage (storage wins on a name clash)."""
    found = {}
    for root in (form_svc.ASSETS / "form_catalogs", settings.storage_dir / "form_catalogs"):
        if root.exists():
            found.update({d.name: d for d in sorted(root.iterdir()) if (d / "catalog.json").exists()})
    return found


@router.get("/forms/catalogs")
def form_catalogs(_: User = Depends(current_user)):
    import json
    out = []
    for name, d in _catalogs().items():
        meta = json.loads((d / "catalog.json").read_text(encoding="utf-8"))
        out.append({"id": name, "title": meta.get("title", name), "items": len(meta["items"])})
    out.append({"id": "natori", "title": "なとりデリカ 発注書", "items": len(natori.ITEMS)})
    return {"catalogs": out, "handwriting_model": form_svc.digits.available()}


@router.post("/forms/read")
async def read_form(file: UploadFile = File(...), catalog: str = Form(""), _: User = Depends(current_user)):
    """Read one photographed/scanned order form: printed rows are matched to the catalog, handwritten quantities are read, and
    anything doubtful comes back with review=true instead of a guess."""
    import cv2
    import numpy as np
    data = await file.read(service.MAX_UPLOAD + 1)
    if len(data) > service.MAX_UPLOAD:
        raise ApiError(413, "file_too_large", "That file is too large.")
    img = cv2.imdecode(np.frombuffer(data, np.uint8), cv2.IMREAD_COLOR)
    if img is None:
        raise ApiError(400, "bad_image", "That file is not a readable image (use JPG or PNG; export PDF pages as images first).")
    if max(img.shape[:2]) > 6000:
        raise ApiError(413, "image_too_large", "Image is too large; please use one under 6000 px on the long side.")
    cats = _catalogs()
    if catalog == "natori":
        from starlette.concurrency import run_in_threadpool
        res = await run_in_threadpool(natori.read, img)
        return {**res.as_dict(), "review_count": sum(r.review for r in res.rows), "catalog": "natori"}
    if catalog and catalog not in cats:
        raise ApiError(404, "unknown_catalog", "No such form catalog.")
    cat = form_svc.Catalog.load(cats[catalog or next(iter(cats))]) if cats else None
    from starlette.concurrency import run_in_threadpool
    res = await run_in_threadpool(form_svc.read_form, img, cat)
    out = res.as_dict()
    out["review_count"] = sum(r.review for r in res.rows)
    out["catalog"] = catalog or (next(iter(cats)) if cats else None)
    return out


@router.post("/forms/extract")
async def extract_form(
    file: UploadFile = File(...),
    catalog: str = Form(""),
    delivery_date: str = Form(""),
    use_llm: str = Form("auto"),
    _: User = Depends(current_user),
):
    """Structured order extraction (発注書 → JSON).

    ``use_llm``:
      * ``auto``  — use Claude vision when ``ANTHROPIC_API_KEY`` is set, else fall back to the
                    local cell-grid reader.
      * ``llm``   — require Claude vision; return 503 if the key is not configured.
      * ``local`` — always use the local cell-grid reader (no LLM call).

    Returns the extraction JSON schema (fields: supplier, customer_name, delivery_date, items …).
    """
    import cv2
    import numpy as np

    data = await file.read(service.MAX_UPLOAD + 1)
    if len(data) > service.MAX_UPLOAD:
        raise ApiError(413, "file_too_large", "That file is too large.")

    want_llm = use_llm in ("llm", "auto")
    have_llm = llm_extract.llm_available()

    if use_llm == "llm" and not have_llm:
        raise ApiError(503, "llm_unavailable",
                       "Claude vision extraction requires ANTHROPIC_API_KEY to be set.")

    if want_llm and have_llm:
        # Detect media type from magic bytes
        mt = "image/jpeg"
        if data[:4] == b"\x89PNG":
            mt = "image/png"
        elif data[:4] in (b"GIF8", b"GIF9"):
            mt = "image/gif"
        from starlette.concurrency import run_in_threadpool
        result = await run_in_threadpool(llm_extract.extract_with_llm, data, media_type=mt)
        result["_source"] = "llm"
        return result

    # --- local fallback ---
    img = cv2.imdecode(np.frombuffer(data, np.uint8), cv2.IMREAD_COLOR)
    if img is None:
        raise ApiError(400, "bad_image",
                       "That file is not a readable image (JPG or PNG; export PDF pages first).")
    if max(img.shape[:2]) > 6000:
        raise ApiError(413, "image_too_large", "Image too large; use one under 6000 px on the long side.")

    cats = _catalogs()
    dd = delivery_date.strip() or None

    from starlette.concurrency import run_in_threadpool
    if catalog == "natori":
        res = await run_in_threadpool(natori.read, img)
    elif catalog and catalog not in cats:
        raise ApiError(404, "unknown_catalog", "No such form catalog.")
    else:
        cat = form_svc.Catalog.load(cats[catalog or next(iter(cats))]) if cats else None
        res = await run_in_threadpool(form_svc.read_form, img, cat)

    return llm_extract.local_to_extract(res, catalog=catalog, delivery_date=dd)
