"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
if (!window._nextSetupHydrationWarning) {
    const origConsoleError = window.console.error;
    window.console.error = function() {
        for(var _len = arguments.length, args = new Array(_len), _key = 0; _key < _len; _key++){
            args[_key] = arguments[_key];
        }
        const isHydrateError = args.some((arg)=>typeof arg === "string" && arg.match(/(hydration|content does not match|did not match)/i));
        if (isHydrateError) {
            args = [
                ...args,
                "\nSee more info here: https://nextjs.org/docs/messages/react-hydration-error"
            ];
        }
        origConsoleError.apply(window.console, args);
    };
    window._nextSetupHydrationWarning = true;
}

if ((typeof exports.default === 'function' || (typeof exports.default === 'object' && exports.default !== null)) && typeof exports.default.__esModule === 'undefined') {
  Object.defineProperty(exports.default, '__esModule', { value: true });
  Object.assign(exports.default, exports);
  module.exports = exports.default;
}

//# sourceMappingURL=setup-hydration-warning.js.map