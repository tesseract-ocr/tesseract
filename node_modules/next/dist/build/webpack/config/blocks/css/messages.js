"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
0 && (module.exports = {
    getCustomDocumentError: null,
    getGlobalImportError: null,
    getGlobalModuleImportError: null,
    getLocalModuleImportError: null
});
function _export(target, all) {
    for(var name in all)Object.defineProperty(target, name, {
        enumerable: true,
        get: all[name]
    });
}
_export(exports, {
    getCustomDocumentError: function() {
        return getCustomDocumentError;
    },
    getGlobalImportError: function() {
        return getGlobalImportError;
    },
    getGlobalModuleImportError: function() {
        return getGlobalModuleImportError;
    },
    getLocalModuleImportError: function() {
        return getLocalModuleImportError;
    }
});
const _picocolors = require("../../../../../lib/picocolors");
function getGlobalImportError() {
    return `Global CSS ${(0, _picocolors.bold)("cannot")} be imported from files other than your ${(0, _picocolors.bold)("Custom <App>")}. Due to the Global nature of stylesheets, and to avoid conflicts, Please move all first-party global CSS imports to ${(0, _picocolors.cyan)("pages/_app.js")}. Or convert the import to Component-Level CSS (CSS Modules).\nRead more: https://nextjs.org/docs/messages/css-global`;
}
function getGlobalModuleImportError() {
    return `Global CSS ${(0, _picocolors.bold)("cannot")} be imported from within ${(0, _picocolors.bold)("node_modules")}.\nRead more: https://nextjs.org/docs/messages/css-npm`;
}
function getLocalModuleImportError() {
    return `CSS Modules ${(0, _picocolors.bold)("cannot")} be imported from within ${(0, _picocolors.bold)("node_modules")}.\nRead more: https://nextjs.org/docs/messages/css-modules-npm`;
}
function getCustomDocumentError() {
    return `CSS ${(0, _picocolors.bold)("cannot")} be imported within ${(0, _picocolors.cyan)("pages/_document.js")}. Please move global styles to ${(0, _picocolors.cyan)("pages/_app.js")}.`;
}

//# sourceMappingURL=messages.js.map