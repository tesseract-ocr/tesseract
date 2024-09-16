import { codeFrameColumns } from "next/dist/compiled/babel/code-frame";
/** React that's compiled with `next`. Used by App Router. */ const reactVendoredRe = /[\\/]next[\\/]dist[\\/]compiled[\\/](react|react-dom|react-server-dom-(webpack|turbopack)|scheduler)[\\/]/;
/** React the user installed. Used by Pages Router, or user imports in App Router. */ const reactNodeModulesRe = /node_modules[\\/](react|react-dom|scheduler)[\\/]/;
const nextInternalsRe = /(node_modules[\\/]next[\\/]|[\\/].next[\\/]static[\\/]chunks[\\/]webpack\.js$|(edge-runtime-webpack|webpack-runtime)\.js$)/;
const nextMethodRe = /(^__webpack_.*|node_modules[\\/]next[\\/])/;
function isInternal(file) {
    if (!file) return false;
    return nextInternalsRe.test(file) || reactVendoredRe.test(file) || reactNodeModulesRe.test(file);
}
/** Given a frame, it parses which package it belongs to. */ export function findSourcePackage(param) {
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
/**
 * It looks up the code frame of the traced source.
 * @note It ignores node_modules or Next.js/React internals, as these can often be huge budnled files.
 */ export function getOriginalCodeFrame(frame, source) {
    var _frame_file;
    if (!source || ((_frame_file = frame.file) == null ? void 0 : _frame_file.includes("node_modules")) || isInternal(frame.file)) {
        return null;
    }
    var _frame_lineNumber, _frame_column;
    return codeFrameColumns(source, {
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
export function noContent(res) {
    res.statusCode = 204;
    res.end("No Content");
}
export function badRequest(res) {
    res.statusCode = 400;
    res.end("Bad Request");
}
export function internalServerError(res, e) {
    res.statusCode = 500;
    res.end(e != null ? e : "Internal Server Error");
}
export function json(res, data) {
    res.setHeader("Content-Type", "application/json").end(Buffer.from(JSON.stringify(data)));
}

//# sourceMappingURL=shared.js.map