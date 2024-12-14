"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "styles", {
    enumerable: true,
    get: function() {
        return styles;
    }
});
const _tagged_template_literal_loose = require("@swc/helpers/_/_tagged_template_literal_loose");
const _nooptemplate = require("../../helpers/noop-template");
function _templateObject() {
    const data = _tagged_template_literal_loose._([
        "\n  .nextjs-toast {\n    position: fixed;\n    bottom: var(--size-gap-double);\n    left: var(--size-gap-double);\n    max-width: 420px;\n    z-index: 9000;\n    box-shadow: 0px var(--size-gap-double) var(--size-gap-quad)\n      rgba(0, 0, 0, 0.25);\n  }\n\n  @media (max-width: 440px) {\n    .nextjs-toast {\n      max-width: 90vw;\n      left: 5vw;\n    }\n  }\n\n  .nextjs-toast-errors-parent {\n    padding: 16px;\n    border-radius: var(--size-gap-quad);\n    font-weight: 500;\n    color: var(--color-ansi-bright-white);\n    background-color: var(--color-ansi-red);\n  }\n\n  .nextjs-static-indicator-toast-wrapper {\n    width: 30px;\n    height: 30px;\n    overflow: hidden;\n    border: 0;\n    border-radius: var(--size-gap-triple);\n    background: var(--color-background);\n    color: var(--color-font);\n    transition: all 0.3s ease-in-out;\n    box-shadow:\n      inset 0 0 0 1px var(--color-border-shadow),\n      0 11px 40px 0 rgba(0, 0, 0, 0.25),\n      0 2px 10px 0 rgba(0, 0, 0, 0.12);\n  }\n\n  .nextjs-static-indicator-toast-wrapper:hover {\n    width: 140px;\n  }\n\n  .nextjs-static-indicator-toast-icon {\n    display: flex;\n    align-items: center;\n    justify-content: center;\n    width: 30px;\n    height: 30px;\n  }\n\n  .nextjs-static-indicator-toast-text {\n    font-size: 14px;\n    display: flex;\n    align-items: center;\n    justify-content: center;\n    opacity: 0;\n    white-space: nowrap;\n    transition: opacity 0.3s ease-in-out;\n    line-height: 30px;\n    position: absolute;\n    left: 30px;\n    top: 0;\n  }\n\n  .nextjs-static-indicator-toast-wrapper:hover\n    .nextjs-static-indicator-toast-text {\n    opacity: 1;\n  }\n\n  .nextjs-static-indicator-toast-wrapper button {\n    color: var(--color-font);\n    opacity: 0.8;\n    background: none;\n    border: none;\n    margin-left: 6px;\n    margin-top: -2px;\n    outline: 0;\n  }\n\n  .nextjs-static-indicator-toast-wrapper button:focus {\n    opacity: 1;\n  }\n\n  .nextjs-static-indicator-toast-wrapper button > svg {\n    width: 16px;\n    height: 16px;\n  }\n"
    ]);
    _templateObject = function() {
        return data;
    };
    return data;
}
const styles = (0, _nooptemplate.noop)(_templateObject());

if ((typeof exports.default === 'function' || (typeof exports.default === 'object' && exports.default !== null)) && typeof exports.default.__esModule === 'undefined') {
  Object.defineProperty(exports.default, '__esModule', { value: true });
  Object.assign(exports.default, exports);
  module.exports = exports.default;
}

//# sourceMappingURL=styles.js.map