"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
0 && (module.exports = {
    getDefaultHydrationErrorMessage: null,
    getHydrationErrorStackInfo: null,
    isHydrationError: null,
    isReactHydrationErrorMessage: null
});
function _export(target, all) {
    for(var name in all)Object.defineProperty(target, name, {
        enumerable: true,
        get: all[name]
    });
}
_export(exports, {
    getDefaultHydrationErrorMessage: function() {
        return getDefaultHydrationErrorMessage;
    },
    getHydrationErrorStackInfo: function() {
        return getHydrationErrorStackInfo;
    },
    isHydrationError: function() {
        return isHydrationError;
    },
    isReactHydrationErrorMessage: function() {
        return isReactHydrationErrorMessage;
    }
});
const _interop_require_default = require("@swc/helpers/_/_interop_require_default");
const _iserror = /*#__PURE__*/ _interop_require_default._(require("../../lib/is-error"));
const hydrationErrorRegex = /hydration failed|while hydrating|content does not match|did not match|HTML didn't match/i;
const reactUnifiedMismatchWarning = "Hydration failed because the server rendered HTML didn't match the client. As a result this tree will be regenerated on the client. This can happen if a SSR-ed Client Component used";
const reactHydrationStartMessages = [
    reactUnifiedMismatchWarning,
    "A tree hydrated but some attributes of the server rendered HTML didn't match the client properties. This won't be patched up. This can happen if a SSR-ed Client Component used:"
];
const reactHydrationErrorDocLink = 'https://react.dev/link/hydration-mismatch';
const getDefaultHydrationErrorMessage = ()=>{
    return reactUnifiedMismatchWarning;
};
function isHydrationError(error) {
    return (0, _iserror.default)(error) && hydrationErrorRegex.test(error.message);
}
function isReactHydrationErrorMessage(msg) {
    return reactHydrationStartMessages.some((prefix)=>msg.startsWith(prefix));
}
function getHydrationErrorStackInfo(rawMessage) {
    rawMessage = rawMessage.replace(/^Error: /, '');
    if (!isReactHydrationErrorMessage(rawMessage)) {
        return {
            message: null
        };
    }
    const firstLineBreak = rawMessage.indexOf('\n');
    rawMessage = rawMessage.slice(firstLineBreak + 1).trim();
    const [message, trailing] = rawMessage.split("" + reactHydrationErrorDocLink);
    const trimmedMessage = message.trim();
    // React built-in hydration diff starts with a newline, checking if length is > 1
    if (trailing && trailing.length > 1) {
        const stacks = [];
        const diffs = [];
        trailing.split('\n').forEach((line)=>{
            if (line.trim() === '') return;
            if (line.trim().startsWith('at ')) {
                stacks.push(line);
            } else {
                diffs.push(line);
            }
        });
        return {
            message: trimmedMessage,
            link: reactHydrationErrorDocLink,
            diff: diffs.join('\n'),
            stack: stacks.join('\n')
        };
    } else {
        return {
            message: trimmedMessage,
            link: reactHydrationErrorDocLink,
            stack: trailing
        };
    }
}

if ((typeof exports.default === 'function' || (typeof exports.default === 'object' && exports.default !== null)) && typeof exports.default.__esModule === 'undefined') {
  Object.defineProperty(exports.default, '__esModule', { value: true });
  Object.assign(exports.default, exports);
  module.exports = exports.default;
}

//# sourceMappingURL=is-hydration-error.js.map