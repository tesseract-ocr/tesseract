/*
Copyright (c) 2021 The swc Project Developers

Permission is hereby granted, free of charge, to any
person obtaining a copy of this software and associated
documentation files (the "Software"), to deal in the
Software without restriction, including without
limitation the rights to use, copy, modify, merge,
publish, distribute, sublicense, and/or sell copies of
the Software, and to permit persons to whom the Software
is furnished to do so, subject to the following
conditions:

The above copyright notice and this permission notice
shall be included in all copies or substantial portions
of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF
ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT
SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR
IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.
*/ import vm from 'vm';
import { transformSync } from './index';
import { getJestSWCOptions } from './options';
// Jest use the `vm` [Module API](https://nodejs.org/api/vm.html#vm_class_vm_module) for ESM.
// see https://github.com/facebook/jest/issues/9430
const isSupportEsm = 'Module' in vm;
function getJestConfig(jestConfig) {
    return 'config' in jestConfig ? jestConfig.config : jestConfig;
}
function isEsm(isEsmProject, filename, jestConfig) {
    var _jestConfig_extensionsToTreatAsEsm;
    return /\.jsx?$/.test(filename) && isEsmProject || ((_jestConfig_extensionsToTreatAsEsm = jestConfig.extensionsToTreatAsEsm) == null ? void 0 : _jestConfig_extensionsToTreatAsEsm.some((ext)=>filename.endsWith(ext)));
}
const createTransformer = (inputOptions)=>({
        process (src, filename, jestOptions) {
            const jestConfig = getJestConfig(jestOptions);
            const swcTransformOpts = getJestSWCOptions({
                isServer: jestConfig.testEnvironment === 'node' || jestConfig.testEnvironment.includes('jest-environment-node'),
                filename,
                jsConfig: inputOptions == null ? void 0 : inputOptions.jsConfig,
                resolvedBaseUrl: inputOptions == null ? void 0 : inputOptions.resolvedBaseUrl,
                pagesDir: inputOptions == null ? void 0 : inputOptions.pagesDir,
                serverComponents: inputOptions == null ? void 0 : inputOptions.serverComponents,
                modularizeImports: inputOptions == null ? void 0 : inputOptions.modularizeImports,
                swcPlugins: inputOptions == null ? void 0 : inputOptions.swcPlugins,
                compilerOptions: inputOptions == null ? void 0 : inputOptions.compilerOptions,
                serverReferenceHashSalt: '',
                esm: isSupportEsm && isEsm(Boolean(inputOptions == null ? void 0 : inputOptions.isEsmProject), filename, jestConfig)
            });
            return transformSync(src, {
                ...swcTransformOpts,
                filename
            });
        }
    });
module.exports = {
    createTransformer
};

//# sourceMappingURL=jest-transformer.js.map