import { HttpError } from "../errors.js";

const MULTER = {
  LIMIT_FILE_SIZE: [413, "file_too_large", "The file exceeds the upload size limit"],
  LIMIT_UNEXPECTED_FILE: [400, "unexpected_field", 'Send the file in a multipart field named "file"'],
  LIMIT_FILE_COUNT: [400, "too_many_files", "Upload one file per request"],
  LIMIT_PART_COUNT: [400, "bad_upload", "Too many form parts"],
  LIMIT_FIELD_COUNT: [400, "bad_upload", "Too many form fields"],
  LIMIT_FIELD_KEY: [400, "bad_upload", "Form field name too long"],
  LIMIT_FIELD_VALUE: [400, "bad_upload", "Form field value too long"],
};

export function notFound(req, res) {
  res.status(404).json({ error: { code: "not_found", message: "Not found" } });
}

export function errorHandler(logger) {
  // eslint-disable-next-line no-unused-vars
  return (err, req, res, next) => {
    if (res.headersSent) return res.destroy();
    let status = 500, code = "internal_error", message = "Something went wrong";
    if (err instanceof HttpError) {
      ({ status, code, message } = err);
      if (err.headers) res.set(err.headers);
    } else if (err?.name === "MulterError") {
      [status, code, message] = MULTER[err.code] ?? [400, "bad_upload", "The upload could not be processed"];
    } else if (/Unexpected end of form|Malformed|Boundary not found|Multipart/i.test(err?.message ?? "")) {
      [status, code, message] = [400, "malformed_upload", "The multipart body is malformed"];
    } else if (/Task queue is at limit/i.test(err?.message ?? "")) {
      [status, code, message] = [503, "busy", "All OCR workers are busy. Retry shortly or use mode=async."];
      res.set("Retry-After", "5");
    } else if (err?.name === "AbortError" || err?.name === "TimeoutError") {
      [status, code, message] = [504, "ocr_timeout", "OCR took too long and was cancelled"];
    } else if (err?.input) {
      [status, code, message] = [422, err.code, err.message];
    } else {
      req.log?.error({ err }, "unhandled error");
      logger.debug?.({ err: err?.message }, "unhandled");
    }
    res.status(status).json({ error: { code, message } });
  };
}
