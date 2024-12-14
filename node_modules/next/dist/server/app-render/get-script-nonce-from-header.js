"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "getScriptNonceFromHeader", {
    enumerable: true,
    get: function() {
        return getScriptNonceFromHeader;
    }
});
const _htmlescape = require("../htmlescape");
function getScriptNonceFromHeader(cspHeaderValue) {
    var _directive_split_slice_map_find;
    const directives = cspHeaderValue// Directives are split by ';'.
    .split(';').map((directive)=>directive.trim());
    // First try to find the directive for the 'script-src', otherwise try to
    // fallback to the 'default-src'.
    const directive = directives.find((dir)=>dir.startsWith('script-src')) || directives.find((dir)=>dir.startsWith('default-src'));
    // If no directive could be found, then we're done.
    if (!directive) {
        return;
    }
    // Extract the nonce from the directive
    const nonce = (_directive_split_slice_map_find = directive.split(' ')// Remove the 'strict-src'/'default-src' string, this can't be the nonce.
    .slice(1).map((source)=>source.trim())// Find the first source with the 'nonce-' prefix.
    .find((source)=>source.startsWith("'nonce-") && source.length > 8 && source.endsWith("'"))) == null ? void 0 : _directive_split_slice_map_find.slice(7, -1);
    // If we could't find the nonce, then we're done.
    if (!nonce) {
        return;
    }
    // Don't accept the nonce value if it contains HTML escape characters.
    // Technically, the spec requires a base64'd value, but this is just an
    // extra layer.
    if (_htmlescape.ESCAPE_REGEX.test(nonce)) {
        throw new Error('Nonce value from Content-Security-Policy contained HTML escape characters.\nLearn more: https://nextjs.org/docs/messages/nonce-contained-invalid-characters');
    }
    return nonce;
}

//# sourceMappingURL=get-script-nonce-from-header.js.map