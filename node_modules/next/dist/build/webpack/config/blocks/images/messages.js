"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "getCustomDocumentImageError", {
    enumerable: true,
    get: function() {
        return getCustomDocumentImageError;
    }
});
const _picocolors = require("../../../../../lib/picocolors");
function getCustomDocumentImageError() {
    return `Images ${(0, _picocolors.bold)("cannot")} be imported within ${(0, _picocolors.cyan)("pages/_document.js")}. Please move image imports that need to be displayed on every page into ${(0, _picocolors.cyan)("pages/_app.js")}.\nRead more: https://nextjs.org/docs/messages/custom-document-image-import`;
}

//# sourceMappingURL=messages.js.map