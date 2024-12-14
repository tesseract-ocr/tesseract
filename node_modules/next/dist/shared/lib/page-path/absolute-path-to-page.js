"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "absolutePathToPage", {
    enumerable: true,
    get: function() {
        return absolutePathToPage;
    }
});
const _interop_require_default = require("@swc/helpers/_/_interop_require_default");
const _ensureleadingslash = require("./ensure-leading-slash");
const _normalizepathsep = require("./normalize-path-sep");
const _path = /*#__PURE__*/ _interop_require_default._(require("../isomorphic/path"));
const _removepagepathtail = require("./remove-page-path-tail");
const _getmetadataroute = require("../../../lib/metadata/get-metadata-route");
function absolutePathToPage(pagePath, options) {
    const isAppDir = options.pagesType === 'app';
    const page = (0, _removepagepathtail.removePagePathTail)((0, _normalizepathsep.normalizePathSep)((0, _ensureleadingslash.ensureLeadingSlash)(_path.default.relative(options.dir, pagePath))), {
        extensions: options.extensions,
        keepIndex: options.keepIndex
    });
    return isAppDir ? (0, _getmetadataroute.normalizeMetadataRoute)(page) : page;
}

//# sourceMappingURL=absolute-path-to-page.js.map