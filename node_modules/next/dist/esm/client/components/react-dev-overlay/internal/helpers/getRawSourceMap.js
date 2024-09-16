import dataUriToBuffer from "next/dist/compiled/data-uri-to-buffer";
import { getSourceMapUrl } from "./getSourceMapUrl";
export function getRawSourceMap(fileContents) {
    const sourceUrl = getSourceMapUrl(fileContents);
    if (!(sourceUrl == null ? void 0 : sourceUrl.startsWith("data:"))) {
        return null;
    }
    let buffer;
    try {
        buffer = dataUriToBuffer(sourceUrl);
    } catch (err) {
        console.error("Failed to parse source map URL:", err);
        return null;
    }
    if (buffer.type !== "application/json") {
        console.error("Unknown source map type: " + buffer.typeFull + ".");
        return null;
    }
    try {
        return JSON.parse(buffer.toString());
    } catch (e) {
        console.error("Failed to parse source map.");
        return null;
    }
}

//# sourceMappingURL=getRawSourceMap.js.map