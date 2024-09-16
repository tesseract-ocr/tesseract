// Buffer.byteLength polyfill in the Edge runtime, with only utf8 strings
// supported at the moment.
export function byteLength(payload) {
    return new TextEncoder().encode(payload).buffer.byteLength;
}

//# sourceMappingURL=web.js.map