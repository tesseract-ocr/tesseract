import { codeFrameColumns } from 'next/dist/compiled/babel/code-frame';
import isInternal, { nextInternalsRe, reactNodeModulesRe, reactVendoredRe } from '../../../../shared/lib/is-internal';
const nextMethodRe = /(^__webpack_.*|node_modules[\\/]next[\\/])/;
/** Given a frame, it parses which package it belongs to. */ export function findSourcePackage(param) {
    let { file, methodName } = param;
    if (file) {
        // matching React first since vendored would match under `next` too
        if (reactVendoredRe.test(file) || reactNodeModulesRe.test(file)) {
            return 'react';
        } else if (nextInternalsRe.test(file)) {
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
/**
 * It looks up the code frame of the traced source.
 * @note It ignores Next.js/React internals, as these can often be huge bundled files.
 */ export function getOriginalCodeFrame(frame, source) {
    if (!source || isInternal(frame.file)) {
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
        forceColor: process.stdout.isTTY
    });
}
export function noContent(res) {
    res.statusCode = 204;
    res.end('No Content');
}
export function badRequest(res) {
    res.statusCode = 400;
    res.end('Bad Request');
}
export function internalServerError(res, e) {
    res.statusCode = 500;
    res.end(e != null ? e : 'Internal Server Error');
}
export function json(res, data) {
    res.setHeader('Content-Type', 'application/json').end(Buffer.from(JSON.stringify(data)));
}
export function jsonString(res, data) {
    res.setHeader('Content-Type', 'application/json').end(Buffer.from(data));
}

//# sourceMappingURL=shared.js.map