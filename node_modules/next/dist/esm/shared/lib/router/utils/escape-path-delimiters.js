// escape delimiters used by path-to-regexp
export default function escapePathDelimiters(segment, escapeEncoded) {
    return segment.replace(new RegExp("([/#?]" + (escapeEncoded ? "|%(2f|23|3f)" : "") + ")", "gi"), (char)=>encodeURIComponent(char));
}

//# sourceMappingURL=escape-path-delimiters.js.map