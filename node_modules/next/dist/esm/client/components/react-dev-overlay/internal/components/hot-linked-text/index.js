import { jsx as _jsx, jsxs as _jsxs, Fragment as _Fragment } from "react/jsx-runtime";
import React from "react";
import { decodeMagicIdentifier, MAGIC_IDENTIFIER_REGEX } from "../../../../../../shared/lib/magic-identifier";
const linkRegex = /https?:\/\/[^\s/$.?#].[^\s)'"]*/i;
const splitRegexp = new RegExp("(" + MAGIC_IDENTIFIER_REGEX.source + "|\\s+)");
export const HotlinkedText = function HotlinkedText(props) {
    const { text, matcher } = props;
    const wordsAndWhitespaces = text.split(splitRegexp);
    return /*#__PURE__*/ _jsx(_Fragment, {
        children: wordsAndWhitespaces.map((word, index)=>{
            if (linkRegex.test(word)) {
                const link = linkRegex.exec(word);
                const href = link[0];
                // If link matcher is present but the link doesn't match, don't turn it into a link
                if (typeof matcher === "function" && !matcher(href)) {
                    return word;
                }
                return /*#__PURE__*/ _jsx(React.Fragment, {
                    children: /*#__PURE__*/ _jsx("a", {
                        href: href,
                        target: "_blank",
                        rel: "noreferrer noopener",
                        children: word
                    })
                }, "link-" + index);
            }
            try {
                const decodedWord = decodeMagicIdentifier(word);
                if (decodedWord !== word) {
                    return /*#__PURE__*/ _jsxs("i", {
                        children: [
                            "{",
                            decodedWord,
                            "}"
                        ]
                    }, "ident-" + index);
                }
            } catch (e) {
                return /*#__PURE__*/ _jsxs("i", {
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
            return /*#__PURE__*/ _jsx(React.Fragment, {
                children: word
            }, "text-" + index);
        })
    });
};

//# sourceMappingURL=index.js.map