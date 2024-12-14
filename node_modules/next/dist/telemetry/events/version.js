"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "eventCliSession", {
    enumerable: true,
    get: function() {
        return eventCliSession;
    }
});
const _path = /*#__PURE__*/ _interop_require_default(require("path"));
function _interop_require_default(obj) {
    return obj && obj.__esModule ? obj : {
        default: obj
    };
}
const EVENT_VERSION = 'NEXT_CLI_SESSION_STARTED';
function hasBabelConfig(dir) {
    try {
        var _res_options_presets, _res_options, _res_options_plugins, _res_options1;
        const noopFile = _path.default.join(dir, 'noop.js');
        const res = require('next/dist/compiled/babel/core').loadPartialConfig({
            cwd: dir,
            filename: noopFile,
            sourceFileName: noopFile
        });
        const isForTooling = ((_res_options = res.options) == null ? void 0 : (_res_options_presets = _res_options.presets) == null ? void 0 : _res_options_presets.every((e)=>{
            var _e_file;
            return (e == null ? void 0 : (_e_file = e.file) == null ? void 0 : _e_file.request) === 'next/babel';
        })) && ((_res_options1 = res.options) == null ? void 0 : (_res_options_plugins = _res_options1.plugins) == null ? void 0 : _res_options_plugins.length) === 0;
        return res.hasFilesystemConfig() && !isForTooling;
    } catch  {
        return false;
    }
}
function eventCliSession(dir, nextConfig, event) {
    var _nextConfig_experimental_staleTimes, _nextConfig_experimental_staleTimes1, _nextConfig_experimental_reactCompiler, _nextConfig_experimental_reactCompiler1;
    // This should be an invariant, if it fails our build tooling is broken.
    if (typeof "15.1.0" !== 'string') {
        return [];
    }
    const { images, i18n } = nextConfig || {};
    const payload = {
        nextVersion: "15.1.0",
        nodeVersion: process.version,
        cliCommand: event.cliCommand,
        isSrcDir: event.isSrcDir,
        hasNowJson: event.hasNowJson,
        isCustomServer: event.isCustomServer,
        hasNextConfig: nextConfig.configOrigin !== 'default',
        buildTarget: 'default',
        hasWebpackConfig: typeof (nextConfig == null ? void 0 : nextConfig.webpack) === 'function',
        hasBabelConfig: hasBabelConfig(dir),
        imageEnabled: !!images,
        imageFutureEnabled: !!images,
        basePathEnabled: !!(nextConfig == null ? void 0 : nextConfig.basePath),
        i18nEnabled: !!i18n,
        locales: (i18n == null ? void 0 : i18n.locales) ? i18n.locales.join(',') : null,
        localeDomainsCount: (i18n == null ? void 0 : i18n.domains) ? i18n.domains.length : null,
        localeDetectionEnabled: !i18n ? null : i18n.localeDetection !== false,
        imageDomainsCount: (images == null ? void 0 : images.domains) ? images.domains.length : null,
        imageRemotePatternsCount: (images == null ? void 0 : images.remotePatterns) ? images.remotePatterns.length : null,
        imageLocalPatternsCount: (images == null ? void 0 : images.localPatterns) ? images.localPatterns.length : null,
        imageSizes: (images == null ? void 0 : images.imageSizes) ? images.imageSizes.join(',') : null,
        imageLoader: images == null ? void 0 : images.loader,
        imageFormats: (images == null ? void 0 : images.formats) ? images.formats.join(',') : null,
        nextConfigOutput: (nextConfig == null ? void 0 : nextConfig.output) || null,
        trailingSlashEnabled: !!(nextConfig == null ? void 0 : nextConfig.trailingSlash),
        reactStrictMode: !!(nextConfig == null ? void 0 : nextConfig.reactStrictMode),
        webpackVersion: event.webpackVersion || null,
        turboFlag: event.turboFlag || false,
        appDir: event.appDir,
        pagesDir: event.pagesDir,
        staticStaleTime: ((_nextConfig_experimental_staleTimes = nextConfig.experimental.staleTimes) == null ? void 0 : _nextConfig_experimental_staleTimes.static) ?? null,
        dynamicStaleTime: ((_nextConfig_experimental_staleTimes1 = nextConfig.experimental.staleTimes) == null ? void 0 : _nextConfig_experimental_staleTimes1.dynamic) ?? null,
        reactCompiler: Boolean(nextConfig.experimental.reactCompiler),
        reactCompilerCompilationMode: typeof nextConfig.experimental.reactCompiler !== 'boolean' ? ((_nextConfig_experimental_reactCompiler = nextConfig.experimental.reactCompiler) == null ? void 0 : _nextConfig_experimental_reactCompiler.compilationMode) ?? null : null,
        reactCompilerPanicThreshold: typeof nextConfig.experimental.reactCompiler !== 'boolean' ? ((_nextConfig_experimental_reactCompiler1 = nextConfig.experimental.reactCompiler) == null ? void 0 : _nextConfig_experimental_reactCompiler1.panicThreshold) ?? null : null
    };
    return [
        {
            eventName: EVENT_VERSION,
            payload
        }
    ];
}

//# sourceMappingURL=version.js.map