from ..db import SessionLocal
from ..models import OcrJob
from ..services import ocr, training
from .celery_app import celery
from .dispatch import dispatch


@celery.task(name="app.worker.tasks.run_ocr_job")
def run_ocr_job(job_id: str):
    with SessionLocal() as db:
        job = db.get(OcrJob, job_id)
        if job and job.status in ("queued", "processing"):
            ocr.run_job(db, job)


@celery.task(name="app.worker.tasks.train_model")
def train_model(run_id: int):
    with SessionLocal() as db:
        training.execute_run(db, run_id)


@celery.task(name="app.worker.tasks.check_auto_retrain")
def check_auto_retrain():
    """Runs on a timer and after each approval: start a training run for every language that is due."""
    started = []
    with SessionLocal() as db:
        for lang, trigger in training.due_runs(db):
            run = training.enqueue_run(db, lang, trigger)
            if run.status == "queued":
                started.append(run.id)
    for rid in started:
        dispatch("train_model", rid)
    return started


@celery.task(name="app.worker.tasks.train_dataset_model")
def train_dataset_model(model_id: int):
    from ..ocrlab import train
    with SessionLocal() as db:
        train.execute(db, model_id)


@celery.task(name="app.worker.tasks.run_lab_run")
def run_lab_run(run_id: str):
    from ..ocrlab import service
    with SessionLocal() as db:
        service.execute_run(db, run_id)
