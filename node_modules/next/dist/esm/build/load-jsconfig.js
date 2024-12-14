import path from 'path';
import fs from 'fs';
import * as Log from './output/log';
import { getTypeScriptConfiguration } from '../lib/typescript/getTypeScriptConfiguration';
import { readFileSync } from 'fs';
import isError from '../lib/is-error';
import { hasNecessaryDependencies } from '../lib/has-necessary-dependencies';
let TSCONFIG_WARNED = false;
export function parseJsonFile(filePath) {
    const JSON5 = require('next/dist/compiled/json5');
    const contents = readFileSync(filePath, 'utf8');
    // Special case an empty file
    if (contents.trim() === '') {
        return {};
    }
    try {
        return JSON5.parse(contents);
    } catch (err) {
        if (!isError(err)) throw err;
        const { codeFrameColumns } = require('next/dist/compiled/babel/code-frame');
        const codeFrame = codeFrameColumns(String(contents), {
            start: {
                line: err.lineNumber || 0,
                column: err.columnNumber || 0
            }
        }, {
            message: err.message,
            highlightCode: true
        });
        throw new Error(`Failed to parse "${filePath}":\n${codeFrame}`);
    }
}
export default async function loadJsConfig(dir, config) {
    var _jsConfig_compilerOptions;
    let typeScriptPath;
    try {
        const deps = await hasNecessaryDependencies(dir, [
            {
                pkg: 'typescript',
                file: 'typescript/lib/typescript.js',
                exportsRestrict: true
            }
        ]);
        typeScriptPath = deps.resolved.get('typescript');
    } catch  {}
    const tsConfigPath = path.join(dir, config.typescript.tsconfigPath);
    const useTypeScript = Boolean(typeScriptPath && fs.existsSync(tsConfigPath));
    let implicitBaseurl;
    let jsConfig;
    // jsconfig is a subset of tsconfig
    if (useTypeScript) {
        if (config.typescript.tsconfigPath !== 'tsconfig.json' && TSCONFIG_WARNED === false) {
            TSCONFIG_WARNED = true;
            Log.info(`Using tsconfig file: ${config.typescript.tsconfigPath}`);
        }
        const ts = await Promise.resolve(require(typeScriptPath));
        const tsConfig = await getTypeScriptConfiguration(ts, tsConfigPath, true);
        jsConfig = {
            compilerOptions: tsConfig.options
        };
        implicitBaseurl = path.dirname(tsConfigPath);
    }
    const jsConfigPath = path.join(dir, 'jsconfig.json');
    if (!useTypeScript && fs.existsSync(jsConfigPath)) {
        jsConfig = parseJsonFile(jsConfigPath);
        implicitBaseurl = path.dirname(jsConfigPath);
    }
    let resolvedBaseUrl;
    if (jsConfig == null ? void 0 : (_jsConfig_compilerOptions = jsConfig.compilerOptions) == null ? void 0 : _jsConfig_compilerOptions.baseUrl) {
        resolvedBaseUrl = {
            baseUrl: path.resolve(dir, jsConfig.compilerOptions.baseUrl),
            isImplicit: false
        };
    } else {
        if (implicitBaseurl) {
            resolvedBaseUrl = {
                baseUrl: implicitBaseurl,
                isImplicit: true
            };
        }
    }
    return {
        useTypeScript,
        jsConfig,
        resolvedBaseUrl
    };
}

//# sourceMappingURL=load-jsconfig.js.map