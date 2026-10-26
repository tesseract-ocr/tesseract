"""Send work to Celery, or run it in a background thread when CELERY_EAGER=true (dev without Redis)."""
import threading

from ..config import settings


def dispatch(name: str, *args):
    from . import tasks
    task = getattr(tasks, name)
    if settings.celery_eager:
        threading.Thread(target=task.run, args=args, daemon=True).start()
    else:
        task.delay(*args)
