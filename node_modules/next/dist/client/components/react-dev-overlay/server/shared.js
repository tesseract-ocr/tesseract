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
    jsonString: null,
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
    jsonString: function() {
        return jsonString;
    },
    noContent: function() {
        return noContent;
    }
});
const _interop_require_wildcard = require("@swc/helpers/_/_interop_require_wildcard");
const _codeframe = require("next/dist/compiled/babel/code-frame");
const _isinternal = /*#__PURE__*/ _interop_require_wildcard._(require("../../../../shared/lib/is-internal"));
const nextMethodRe = /(^__webpack_.*|node_modules[\\/]next[\\/])/;
function findSourcePackage(param) {
    let { file, methodName } = param;
    if (file) {
        // matching React first since vendored would match under `next` too
        if (_isinternal.reactVendoredRe.test(file) || _isinternal.reactNodeModulesRe.test(file)) {
            return 'react';
        } else if (_isinternal.nextInternalsRe.test(file)) {
            return 'next';
        } else if (file.startsWith('[turbopack]/')) {
            return 'next';
        }
    }
    if (methodName) {
        if (nextMethodRe.test(methodName)) {
            return 'next';
        }
    }
}
function getOriginalCodeFrame(frame, source) {
    if (!source || (0, _isinternal.default)(frame.file)) {
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
        forceColor: process.stdout.isTTY
    });
}
function noContent(res) {
    res.statusCode = 204;
    res.end('No Content');
}
function badRequest(res) {
    res.statusCode = 400;
    res.end('Bad Request');
}
function internalServerError(res, e) {
    res.statusCode = 500;
    res.end(e != null ? e : 'Internal Server Error');
}
function json(res, data) {
    res.setHeader('Content-Type', 'application/json').end(Buffer.from(JSON.stringify(data)));
}
function jsonString(res, data) {
    res.setHeader('Content-Type', 'application/json').end(Buffer.from(data));
}

if ((typeof exports.default === 'function' || (typeof exports.default === 'object' && exports.default !== null)) && typeof exports.default.__esModule === 'undefined') {
  Object.defineProperty(exports.default, '__esModule', { value: true });
  Object.assign(exports.default, exports);
  module.exports = exports.default;
}

//# sourceMappingURL=shared.js.map