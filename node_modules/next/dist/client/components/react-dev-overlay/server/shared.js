"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
0 && (module.exports = {
    badRequest: null,
    findSourcePackage: null,
    getOriginalCodeFrame: null,
    internalServerError: null,
    json: null,
    noContent: null
});
function _export(target, all) {
    for(var name in all)Object.defineProperty(target, name, {
        enumerable: true,
        get: all[name]
    });
}
_export(exports, {
    badRequest: function() {
        return badRequest;
    },
    findSourcePackage: function() {
        return findSourcePackage;
    },
    getOriginalCodeFrame: function() {
        return getOriginalCodeFrame;
    },
    internalServerError: function() {
        return internalServerError;
    },
    json: function() {
        return json;
    },
    noContent: function() {
        return noContent;
    }
});
const _codeframe = require("next/dist/compiled/babel/code-frame");
/** React that's compiled with `next`. Used by App Router. */ const reactVendoredRe = /[\\/]next[\\/]dist[\\/]compiled[\\/](react|react-dom|react-server-dom-(webpack|turbopack)|scheduler)[\\/]/;
/** React the user installed. Used by Pages Router, or user imports in App Router. */ const reactNodeModulesRe = /node_modules[\\/](react|react-dom|scheduler)[\\/]/;
const nextInternalsRe = /(node_modules[\\/]next[\\/]|[\\/].next[\\/]static[\\/]chunks[\\/]webpack\.js$|(edge-runtime-webpack|webpack-runtime)\.js$)/;
const nextMethodRe = /(^__webpack_.*|node_modules[\\/]next[\\/])/;
function isInternal(file) {
    if (!file) return false;
    return nextInternalsRe.test(file) || reactVendoredRe.test(file) || reactNodeModulesRe.test(file);
}
function findSourcePackage(param) {
    let { file, methodName } = param;
    if (file) {
        // matching React first since vendored would match under `next` too
        if (reactVendoredRe.test(file) || reactNodeModulesRe.test(file)) {
            return "react";
        } else if (nextInternalsRe.test(file)) {
            return "next";
        }
    }
    if (methodName) {
        if (nextMethodRe.test(methodName)) {
            return "next";
        }
    }
}
function getOriginalCodeFrame(frame, source) {
    var _frame_file;
    if (!source || ((_frame_file = frame.file) == null ? void 0 : _frame_file.includes("node_modules")) || isInternal(frame.file)) {
        return null;
    }
    var _frame_lineNumber, _frame_column;
    return (0, _codeframe.codeFrameColumns)(source, {
        start: {
            // 1-based, but -1 means start line without highlighting
            line: (_frame_lineNumber = frame.lineNumber) != null ? _frame_lineNumber : -1,
            // 1-based, but 0 means whole line without column highlighting
            column: (_frame_column = frame.column) != null ? _frame_column : 0
        }
    }, {
        forceColor: true
    });
}
function noContent(res) {
    res.statusCode = 204;
    res.end("No Content");
}
function badRequest(res) {
    res.statusCode = 400;
    res.end("Bad Request");
}
function internalServerError(res, e) {
    res.statusCode = 500;
    res.end(e != null ? e : "Internal Server Error");
}
function json(res, data) {
    res.setHeader("Content-Type", "application/json").end(Buffer.from(JSON.stringify(data)));
}

if ((typeof exports.default === 'function' || (typeof exports.default === 'object' && exports.default !== null)) && typeof exports.default.__esModule === 'undefined') {
  Object.defineProperty(exports.default, '__esModule', { value: true });
  Object.assign(exports.default, exports);
  module.exports = exports.default;
}

//# sourceMappingURL=shared.js.map