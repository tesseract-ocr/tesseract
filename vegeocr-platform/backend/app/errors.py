from fastapi import HTTPException, Request
from fastapi.responses import JSONResponse


class ApiError(HTTPException):
    def __init__(self, status: int, code: str, message: str, headers=None):
        super().__init__(status_code=status, detail={"code": code, "message": message}, headers=headers)


def _body(status: int, detail):
    if isinstance(detail, dict) and "code" in detail:
        return {"error": detail}
    return {"error": {"code": {400: "bad_request", 401: "unauthorized", 403: "forbidden", 404: "not_found",
                               413: "too_large", 422: "invalid_request", 429: "rate_limited"}.get(status, "error"),
                      "message": detail if isinstance(detail, str) else "Request failed"}}


async def http_error_handler(request: Request, exc: HTTPException):
    return JSONResponse(_body(exc.status_code, exc.detail), status_code=exc.status_code,
                        headers=getattr(exc, "headers", None))


async def validation_error_handler(request: Request, exc):
    msgs = "; ".join(f"{'.'.join(str(p) for p in e['loc'] if p != 'body')}: {e['msg']}" for e in exc.errors())
    return JSONResponse({"error": {"code": "invalid_request", "message": msgs}}, status_code=422)
