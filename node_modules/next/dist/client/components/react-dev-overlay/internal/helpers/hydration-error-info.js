"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
0 && (module.exports = {
    getHydrationWarningType: null,
    getReactHydrationDiffSegments: null,
    hydrationErrorState: null,
    storeHydrationErrorStateFromConsoleArgs: null
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
    getReactHydrationDiffSegments: function() {
        return getReactHydrationDiffSegments;
    },
    hydrationErrorState: function() {
        return hydrationErrorState;
    },
    storeHydrationErrorStateFromConsoleArgs: function() {
        return storeHydrationErrorStateFromConsoleArgs;
    }
});
const _ishydrationerror = require("../../../is-hydration-error");
const hydrationErrorState = {};
// https://github.com/facebook/react/blob/main/packages/react-dom/src/__tests__/ReactDOMHydrationDiff-test.js used as a reference
const htmlTagsWarnings = new Set([
    'Warning: In HTML, %s cannot be a child of <%s>.%s\nThis will cause a hydration error.%s',
    'Warning: In HTML, %s cannot be a descendant of <%s>.\nThis will cause a hydration error.%s',
    'Warning: In HTML, text nodes cannot be a child of <%s>.\nThis will cause a hydration error.',
    "Warning: In HTML, whitespace text nodes cannot be a child of <%s>. Make sure you don't have any extra whitespace between tags on each line of your source code.\nThis will cause a hydration error.",
    'Warning: Expected server HTML to contain a matching <%s> in <%s>.%s',
    'Warning: Did not expect server HTML to contain a <%s> in <%s>.%s'
]);
const textAndTagsMismatchWarnings = new Set([
    'Warning: Expected server HTML to contain a matching text node for "%s" in <%s>.%s',
    'Warning: Did not expect server HTML to contain the text node "%s" in <%s>.%s'
]);
const textMismatchWarning = 'Warning: Text content did not match. Server: "%s" Client: "%s"%s';
const getHydrationWarningType = (message)=>{
    if (typeof message !== 'string') {
        // TODO: Doesn't make sense to treat no message as a hydration error message.
        // We should bail out somewhere earlier.
        return 'text';
    }
    const normalizedMessage = message.startsWith('Warning: ') ? message : "Warning: " + message;
    if (isHtmlTagsWarning(normalizedMessage)) return 'tag';
    if (isTextInTagsMismatchWarning(normalizedMessage)) return 'text-in-tag';
    return 'text';
};
const isHtmlTagsWarning = (message)=>htmlTagsWarnings.has(message);
const isTextMismatchWarning = (message)=>textMismatchWarning === message;
const isTextInTagsMismatchWarning = (msg)=>textAndTagsMismatchWarnings.has(msg);
const isKnownHydrationWarning = (message)=>{
    if (typeof message !== 'string') {
        return false;
    }
    // React 18 has the `Warning: ` prefix.
    // React 19 does not.
    const normalizedMessage = message.startsWith('Warning: ') ? message : "Warning: " + message;
    return isHtmlTagsWarning(normalizedMessage) || isTextInTagsMismatchWarning(normalizedMessage) || isTextMismatchWarning(normalizedMessage);
};
const getReactHydrationDiffSegments = (msg)=>{
    if (msg) {
        const { message, diff } = (0, _ishydrationerror.getHydrationErrorStackInfo)(msg);
        if (message) return [
            message,
            diff
        ];
    }
    return undefined;
};
function storeHydrationErrorStateFromConsoleArgs() {
    for(var _len = arguments.length, args = new Array(_len), _key = 0; _key < _len; _key++){
        args[_key] = arguments[_key];
    }
    const [msg, serverContent, clientContent, componentStack] = args;
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
}

if ((typeof exports.default === 'function' || (typeof exports.default === 'object' && exports.default !== null)) && typeof exports.default.__esModule === 'undefined') {
  Object.defineProperty(exports.default, '__esModule', { value: true });
  Object.assign(exports.default, exports);
  module.exports = exports.default;
}

//# sourceMappingURL=hydration-error-info.js.map