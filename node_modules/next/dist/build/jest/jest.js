"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, /**
 * @example
 * ```ts
 * // Usage in jest.config.js
 * const nextJest = require('next/jest');
 *
 * // Optionally provide path to Next.js app which will enable loading next.config.js and .env files
 * const createJestConfig = nextJest({ dir })
 *
 * // Any custom config you want to pass to Jest
 * const customJestConfig = {
 *     setupFilesAfterEnv: ['<rootDir>/jest.setup.js'],
 * }
 *
 * // createJestConfig is exported in this way to ensure that next/jest can load the Next.js config which is async
 * module.exports = createJestConfig(customJestConfig)
 * ```
 *
 * Read more: [Next.js Docs: Setting up Jest with Next.js](https://nextjs.org/docs/app/building-your-application/testing/jest)
 */ "default", {
    enumerable: true,
    get: function() {
        return nextJest;
    }
});
const _env = require("@next/env");
const _path = require("path");
const _config = /*#__PURE__*/ _interop_require_default(require("../../server/config"));
const _constants = require("../../shared/lib/constants");
const _loadjsconfig = /*#__PURE__*/ _interop_require_default(require("../load-jsconfig"));
const _log = /*#__PURE__*/ _interop_require_wildcard(require("../output/log"));
const _findpagesdir = require("../../lib/find-pages-dir");
const _swc = require("../swc");
function _interop_require_default(obj) {
    return obj && obj.__esModule ? obj : {
        default: obj
    };
}
function _getRequireWildcardCache(nodeInterop) {
    if (typeof WeakMap !== "function") return null;
    var cacheBabelInterop = new WeakMap();
    var cacheNodeInterop = new WeakMap();
    return (_getRequireWildcardCache = function(nodeInterop) {
        return nodeInterop ? cacheNodeInterop : cacheBabelInterop;
    })(nodeInterop);
}
function _interop_require_wildcard(obj, nodeInterop) {
    if (!nodeInterop && obj && obj.__esModule) {
        return obj;
    }
    if (obj === null || typeof obj !== "object" && typeof obj !== "function") {
        return {
            default: obj
        };
    }
    var cache = _getRequireWildcardCache(nodeInterop);
    if (cache && cache.has(obj)) {
        return cache.get(obj);
    }
    var newObj = {
        __proto__: null
    };
    var hasPropertyDescriptor = Object.defineProperty && Object.getOwnPropertyDescriptor;
    for(var key in obj){
        if (key !== "default" && Object.prototype.hasOwnProperty.call(obj, key)) {
            var desc = hasPropertyDescriptor ? Object.getOwnPropertyDescriptor(obj, key) : null;
            if (desc && (desc.get || desc.set)) {
                Object.defineProperty(newObj, key, desc);
            } else {
                newObj[key] = obj[key];
            }
        }
    }
    newObj.default = obj;
    if (cache) {
        cache.set(obj, newObj);
    }
    return newObj;
}
const DEFAULT_TRANSPILED_PACKAGES = require('../../lib/default-transpiled-packages.json');
async function getConfig(dir) {
    const conf = await (0, _config.default)(_constants.PHASE_TEST, dir);
    return conf;
}
/**
 * Loads closest package.json in the directory hierarchy
 */ function loadClosestPackageJson(dir, attempts = 1) {
    if (attempts > 5) {
        throw new Error("Can't resolve main package.json file");
    }
    var mainPath = attempts === 1 ? './' : Array(attempts).join('../');
    try {
        return require((0, _path.join)(dir, mainPath + 'package.json'));
    } catch (e) {
        return loadClosestPackageJson(dir, attempts + 1);
    }
}
/** Loads dotenv files and sets environment variables based on next config. */ function setUpEnv(dir) {
    const dev = false;
    (0, _env.loadEnvConfig)(dir, dev, _log);
}
function nextJest(options = {}) {
    // createJestConfig
    return (customJestConfig)=>{
        // Function that is provided as the module.exports of jest.config.js
        // Will be called and awaited by Jest
        return async ()=>{
            var _nextConfig_experimental, _nextConfig_experimental1;
            let nextConfig;
            let jsConfig;
            let resolvedBaseUrl;
            let isEsmProject = false;
            let pagesDir;
            let serverComponents;
            if (options.dir) {
                const resolvedDir = (0, _path.resolve)(options.dir);
                const packageConfig = loadClosestPackageJson(resolvedDir);
                isEsmProject = packageConfig.type === 'module';
                nextConfig = await getConfig(resolvedDir);
                const findPagesDirResult = (0, _findpagesdir.findPagesDir)(resolvedDir);
                serverComponents = !!findPagesDirResult.appDir;
                pagesDir = findPagesDirResult.pagesDir;
                setUpEnv(resolvedDir);
                // TODO: revisit when bug in SWC is fixed that strips `.css`
                const result = await (0, _loadjsconfig.default)(resolvedDir, nextConfig);
                jsConfig = result.jsConfig;
                resolvedBaseUrl = result.resolvedBaseUrl;
            }
            // Ensure provided async config is supported
            const resolvedJestConfig = (typeof customJestConfig === 'function' ? await customJestConfig() : customJestConfig) ?? {};
            // eagerly load swc bindings instead of waiting for transform calls
            await (0, _swc.loadBindings)(nextConfig == null ? void 0 : (_nextConfig_experimental = nextConfig.experimental) == null ? void 0 : _nextConfig_experimental.useWasmBinary);
            if (_swc.lockfilePatchPromise.cur) {
                await _swc.lockfilePatchPromise.cur;
            }
            const transpiled = ((nextConfig == null ? void 0 : nextConfig.transpilePackages) ?? []).concat(DEFAULT_TRANSPILED_PACKAGES).join('|');
            const jestTransformerConfig = {
                modularizeImports: nextConfig == null ? void 0 : nextConfig.modularizeImports,
                swcPlugins: nextConfig == null ? void 0 : (_nextConfig_experimental1 = nextConfig.experimental) == null ? void 0 : _nextConfig_experimental1.swcPlugins,
                compilerOptions: nextConfig == null ? void 0 : nextConfig.compiler,
                jsConfig,
                resolvedBaseUrl,
                serverComponents,
                isEsmProject,
                pagesDir
            };
            return {
                ...resolvedJestConfig,
                moduleNameMapper: {
                    // Handle CSS imports (with CSS modules)
                    // https://jestjs.io/docs/webpack#mocking-css-modules
                    '^.+\\.module\\.(css|sass|scss)$': require.resolve('./object-proxy.js'),
                    // Handle CSS imports (without CSS modules)
                    '^.+\\.(css|sass|scss)$': require.resolve('./__mocks__/styleMock.js'),
                    // Handle image imports
                    '^.+\\.(png|jpg|jpeg|gif|webp|avif|ico|bmp)$': require.resolve(`./__mocks__/fileMock.js`),
                    // Keep .svg to it's own rule to make overriding easy
                    '^.+\\.(svg)$': require.resolve(`./__mocks__/fileMock.js`),
                    // Handle @next/font
                    '@next/font/(.*)': require.resolve('./__mocks__/nextFontMock.js'),
                    // Handle next/font
                    'next/font/(.*)': require.resolve('./__mocks__/nextFontMock.js'),
                    // Disable server-only
                    'server-only': require.resolve('./__mocks__/empty.js'),
                    // custom config comes last to ensure the above rules are matched,
                    // fixes the case where @pages/(.*) -> src/pages/$! doesn't break
                    // CSS/image mocks
                    ...resolvedJestConfig.moduleNameMapper || {}
                },
                testPathIgnorePatterns: [
                    // Don't look for tests in node_modules
                    '/node_modules/',
                    // Don't look for tests in the Next.js build output
                    '/.next/',
                    // Custom config can append to testPathIgnorePatterns but not modify it
                    // This is to ensure `.next` and `node_modules` are always excluded
                    ...resolvedJestConfig.testPathIgnorePatterns || []
                ],
                transform: {
                    // Use SWC to compile tests
                    '^.+\\.(js|jsx|ts|tsx|mjs)$': [
                        require.resolve('../swc/jest-transformer'),
                        jestTransformerConfig
                    ],
                    // Allow for appending/overriding the default transforms
                    ...resolvedJestConfig.transform || {}
                },
                transformIgnorePatterns: [
                    // To match Next.js behavior node_modules is not transformed, only `transpiledPackages`
                    ...transpiled ? [
                        `/node_modules/(?!.pnpm)(?!(${transpiled})/)`,
                        `/node_modules/.pnpm/(?!(${transpiled.replace(/\//g, '\\+')})@)`
                    ] : [
                        '/node_modules/'
                    ],
                    // CSS modules are mocked so they don't need to be transformed
                    '^.+\\.module\\.(css|sass|scss)$',
                    // Custom config can append to transformIgnorePatterns but not modify it
                    // This is to ensure `node_modules` and .module.css/sass/scss are always excluded
                    ...resolvedJestConfig.transformIgnorePatterns || []
                ],
                watchPathIgnorePatterns: [
                    // Don't re-run tests when the Next.js build output changes
                    '/.next/',
                    ...resolvedJestConfig.watchPathIgnorePatterns || []
                ]
            };
        };
    };
}

if ((typeof exports.default === 'function' || (typeof exports.default === 'object' && exports.default !== null)) && typeof exports.default.__esModule === 'undefined') {
  Object.defineProperty(exports.default, '__esModule', { value: true });
  Object.assign(exports.default, exports);
  module.exports = exports.default;
}

//# sourceMappingURL=jest.js.map