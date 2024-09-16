"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "HotlinkedText", {
    enumerable: true,
    get: function() {
        return HotlinkedText;
    }
});
const _interop_require_default = require("@swc/helpers/_/_interop_require_default");
const _jsxruntime = require("react/jsx-runtime");
const _react = /*#__PURE__*/ _interop_require_default._(require("react"));
const _magicidentifier = require("../../../../../../shared/lib/magic-identifier");
const linkRegex = /https?:\/\/[^\s/$.?#].[^\s)'"]*/i;
const splitRegexp = new RegExp("(" + _magicidentifier.MAGIC_IDENTIFIER_REGEX.source + "|\\s+)");
const HotlinkedText = function HotlinkedText(props) {
    const { text, matcher } = props;
    const wordsAndWhitespaces = text.split(splitRegexp);
    return /*#__PURE__*/ (0, _jsxruntime.jsx)(_jsxruntime.Fragment, {
        children: wordsAndWhitespaces.map((word, index)=>{
            if (linkRegex.test(word)) {
                const link = linkRegex.exec(word);
                const href = link[0];
                // If link matcher is present but the link doesn't match, don't turn it into a link
                if (typeof matcher === "function" && !matcher(href)) {
                    return word;
                }
                return /*#__PURE__*/ (0, _jsxruntime.jsx)(_react.default.Fragment, {
                    children: /*#__PURE__*/ (0, _jsxruntime.jsx)("a", {
                        href: href,
                        target: "_blank",
                        rel: "noreferrer noopener",
                        children: word
                    })
                }, "link-" + index);
            }
            try {
                const decodedWord = (0, _magicidentifier.decodeMagicIdentifier)(word);
                if (decodedWord !== word) {
                    return /*#__PURE__*/ (0, _jsxruntime.jsxs)("i", {
                        children: [
                            "{",
                            decodedWord,
                            "}"
                        ]
                    }, "ident-" + index);
                }
            } catch (e) {
                return /*#__PURE__*/ (0, _jsxruntime.jsxs)("i", {
                    children: [
                        "{",
                        word,
                        " (decoding failed: ",
                        "" + e,
                        ")",
                        "}"
                    ]
                }, "ident-" + index);
            }
            return /*#__PURE__*/ (0, _jsxruntime.jsx)(_react.default.Fragment, {
                children: word
            }, "text-" + index);
        })
    });
};

if ((typeof exports.default === 'function' || (typeof exports.default === 'object' && exports.default !== null)) && typeof exports.default.__esModule === 'undefined') {
  Object.defineProperty(exports.default, '__esModule', { value: true });
  Object.assign(exports.default, exports);
  module.exports = exports.default;
}

//# sourceMappingURL=index.js.map