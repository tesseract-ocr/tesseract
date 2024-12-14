"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.nextFontError = nextFontError;
/**
 * Throw NextFontError error. Used by the WellKnownErrorsPlugin to format errors thrown by next/font.
 */
function nextFontError(message) {
    const err = new Error(message);
    err.name = 'NextFontError';
    throw err;
}
