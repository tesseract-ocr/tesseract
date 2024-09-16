"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
0 && (module.exports = {
    getHydrationWarningType: null,
    hydrationErrorState: null,
    patchConsoleError: null
});
function _export(target, all) {
    for(var name in all)Object.defineProperty(target, name, {
        enumerable: true,
        get: all[name]
    });
}
_export(exports, {
    getHydrationWarningType: function() {
        return getHydrationWarningType;
    },
    hydrationErrorState: function() {
        return hydrationErrorState;
    },
    patchConsoleError: function() {
        return patchConsoleError;
    }
});
const getHydrationWarningType = (msg)=>{
    if (isHtmlTagsWarning(msg)) return "tag";
    if (isTextInTagsMismatchWarning(msg)) return "text-in-tag";
    return "text";
};
const isHtmlTagsWarning = (msg)=>Boolean(msg && htmlTagsWarnings.has(msg));
const isTextMismatchWarning = (msg)=>textMismatchWarning === msg;
const isTextInTagsMismatchWarning = (msg)=>Boolean(msg && textAndTagsMismatchWarnings.has(msg));
const isKnownHydrationWarning = (msg)=>isHtmlTagsWarning(msg) || isTextInTagsMismatchWarning(msg) || isTextMismatchWarning(msg);
const hydrationErrorState = {};
// https://github.com/facebook/react/blob/main/packages/react-dom/src/__tests__/ReactDOMHydrationDiff-test.js used as a reference
const htmlTagsWarnings = new Set([
    'Warning: Cannot render a sync or defer <script> outside the main document without knowing its order. Try adding async="" or moving it into the root <head> tag.%s',
    "Warning: In HTML, %s cannot be a child of <%s>.%s\nThis will cause a hydration error.%s",
    "Warning: In HTML, %s cannot be a descendant of <%s>.\nThis will cause a hydration error.%s",
    "Warning: In HTML, text nodes cannot be a child of <%s>.\nThis will cause a hydration error.",
    "Warning: In HTML, whitespace text nodes cannot be a child of <%s>. Make sure you don't have any extra whitespace between tags on each line of your source code.\nThis will cause a hydration error.",
    "Warning: Expected server HTML to contain a matching <%s> in <%s>.%s",
    "Warning: Did not expect server HTML to contain a <%s> in <%s>.%s"
]);
const textAndTagsMismatchWarnings = new Set([
    'Warning: Expected server HTML to contain a matching text node for "%s" in <%s>.%s',
    'Warning: Did not expect server HTML to contain the text node "%s" in <%s>.%s'
]);
const textMismatchWarning = 'Warning: Text content did not match. Server: "%s" Client: "%s"%s';
function patchConsoleError() {
    const prev = console.error;
    console.error = function(msg, serverContent, clientContent, componentStack) {
        if (isKnownHydrationWarning(msg)) {
            hydrationErrorState.warning = [
                // remove the last %s from the message
                msg,
                serverContent,
                clientContent
            ];
            hydrationErrorState.componentStack = componentStack;
            hydrationErrorState.serverContent = serverContent;
            hydrationErrorState.clientContent = clientContent;
        }
        // @ts-expect-error argument is defined
        prev.apply(console, arguments);
    };
}

if ((typeof exports.default === 'function' || (typeof exports.default === 'object' && exports.default !== null)) && typeof exports.default.__esModule === 'undefined') {
  Object.defineProperty(exports.default, '__esModule', { value: true });
  Object.assign(exports.default, exports);
  module.exports = exports.default;
}

//# sourceMappingURL=hydration-error-info.js.map