from celery import Celery

from ..config import settings

celery = Celery("vegeocr", broker=settings.redis_url, backend=settings.redis_url, include=["app.worker.tasks"])
celery.conf.update(
    task_routes={"app.worker.tasks.train_model": {"queue": "training"},
                 "app.worker.tasks.check_auto_retrain": {"queue": "training"},
                 "app.worker.tasks.run_ocr_job": {"queue": "ocr"},
                 "app.worker.tasks.run_lab_run": {"queue": "ocr"},
                 "app.worker.tasks.train_dataset_model": {"queue": "training"}},
    task_acks_late=True, worker_prefetch_multiplier=1, task_track_started=True,
    beat_schedule={"auto-retrain-check": {"task": "app.worker.tasks.check_auto_retrain", "schedule": 600.0}},
    timezone="UTC",
)
