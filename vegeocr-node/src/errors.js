export class HttpError extends Error {
  constructor(status, code, message, headers) {
    super(message);
    this.status = status;
    this.code = code;
    this.headers = headers;
  }
}
/** Failures inside OCR that are the caller's fault (retrying won't help). */
export class OcrInputError extends Error {
  constructor(code, message) {
    super(message);
    this.code = code;
    this.name = "OcrInputError";
  }
}
