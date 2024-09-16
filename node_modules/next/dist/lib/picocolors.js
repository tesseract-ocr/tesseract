// ISC License
// Copyright (c) 2021 Alexey Raspopov, Kostiantyn Denysov, Anton Verinov
// Permission to use, copy, modify, and/or distribute this software for any
// purpose with or without fee is hereby granted, provided that the above
// copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
// WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
// ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
// WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
// ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
// OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
//
// https://github.com/alexeyraspopov/picocolors/blob/b6261487e7b81aaab2440e397a356732cad9e342/picocolors.js#L1
"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
0 && (module.exports = {
    bgBlack: null,
    bgBlue: null,
    bgCyan: null,
    bgGreen: null,
    bgMagenta: null,
    bgRed: null,
    bgWhite: null,
    bgYellow: null,
    black: null,
    blue: null,
    bold: null,
    cyan: null,
    dim: null,
    gray: null,
    green: null,
    hidden: null,
    inverse: null,
    italic: null,
    magenta: null,
    purple: null,
    red: null,
    reset: null,
    strikethrough: null,
    underline: null,
    white: null,
    yellow: null
});
function _export(target, all) {
    for(var name in all)Object.defineProperty(target, name, {
        enumerable: true,
        get: all[name]
    });
}
_export(exports, {
    bgBlack: function() {
        return bgBlack;
    },
    bgBlue: function() {
        return bgBlue;
    },
    bgCyan: function() {
        return bgCyan;
    },
    bgGreen: function() {
        return bgGreen;
    },
    bgMagenta: function() {
        return bgMagenta;
    },
    bgRed: function() {
        return bgRed;
    },
    bgWhite: function() {
        return bgWhite;
    },
    bgYellow: function() {
        return bgYellow;
    },
    black: function() {
        return black;
    },
    blue: function() {
        return blue;
    },
    bold: function() {
        return bold;
    },
    cyan: function() {
        return cyan;
    },
    dim: function() {
        return dim;
    },
    gray: function() {
        return gray;
    },
    green: function() {
        return green;
    },
    hidden: function() {
        return hidden;
    },
    inverse: function() {
        return inverse;
    },
    italic: function() {
        return italic;
    },
    magenta: function() {
        return magenta;
    },
    purple: function() {
        return purple;
    },
    red: function() {
        return red;
    },
    reset: function() {
        return reset;
    },
    strikethrough: function() {
        return strikethrough;
    },
    underline: function() {
        return underline;
    },
    white: function() {
        return white;
    },
    yellow: function() {
        return yellow;
    }
});
var _globalThis;
const { env, stdout } = ((_globalThis = globalThis) == null ? void 0 : _globalThis.process) ?? {};
const enabled = env && !env.NO_COLOR && (env.FORCE_COLOR || (stdout == null ? void 0 : stdout.isTTY) && !env.CI && env.TERM !== "dumb");
const replaceClose = (str, close, replace, index)=>{
    const start = str.substring(0, index) + replace;
    const end = str.substring(index + close.length);
    const nextIndex = end.indexOf(close);
    return ~nextIndex ? start + replaceClose(end, close, replace, nextIndex) : start + end;
};
const formatter = (open, close, replace = open)=>{
    if (!enabled) return String;
    return (input)=>{
        const string = "" + input;
        const index = string.indexOf(close, open.length);
        return ~index ? open + replaceClose(string, close, replace, index) + close : open + string + close;
    };
};
const reset = enabled ? (s)=>`\x1b[0m${s}\x1b[0m` : String;
const bold = formatter("\x1b[1m", "\x1b[22m", "\x1b[22m\x1b[1m");
const dim = formatter("\x1b[2m", "\x1b[22m", "\x1b[22m\x1b[2m");
const italic = formatter("\x1b[3m", "\x1b[23m");
const underline = formatter("\x1b[4m", "\x1b[24m");
const inverse = formatter("\x1b[7m", "\x1b[27m");
const hidden = formatter("\x1b[8m", "\x1b[28m");
const strikethrough = formatter("\x1b[9m", "\x1b[29m");
const black = formatter("\x1b[30m", "\x1b[39m");
const red = formatter("\x1b[31m", "\x1b[39m");
const green = formatter("\x1b[32m", "\x1b[39m");
const yellow = formatter("\x1b[33m", "\x1b[39m");
const blue = formatter("\x1b[34m", "\x1b[39m");
const magenta = formatter("\x1b[35m", "\x1b[39m");
const purple = formatter("\x1b[38;2;173;127;168m", "\x1b[39m");
const cyan = formatter("\x1b[36m", "\x1b[39m");
const white = formatter("\x1b[37m", "\x1b[39m");
const gray = formatter("\x1b[90m", "\x1b[39m");
const bgBlack = formatter("\x1b[40m", "\x1b[49m");
const bgRed = formatter("\x1b[41m", "\x1b[49m");
const bgGreen = formatter("\x1b[42m", "\x1b[49m");
const bgYellow = formatter("\x1b[43m", "\x1b[49m");
const bgBlue = formatter("\x1b[44m", "\x1b[49m");
const bgMagenta = formatter("\x1b[45m", "\x1b[49m");
const bgCyan = formatter("\x1b[46m", "\x1b[49m");
const bgWhite = formatter("\x1b[47m", "\x1b[49m");

//# sourceMappingURL=picocolors.js.map