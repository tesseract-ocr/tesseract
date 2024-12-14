// escape delimiters used by path-to-regexp
"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "default", {
    enumerable: true,
    get: function() {
        return escapePathDelimiters;
    }
});
function escapePathDelimiters(segment, escapeEncoded) {
    return segment.replace(new RegExp("([/#?]" + (escapeEncoded ? '|%(2f|23|3f|5c)' : '') + ")", 'gi'), (char)=>encodeURIComponent(char));
}

//# sourceMappingURL=escape-path-delimiters.js.map