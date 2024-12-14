"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "default", {
    enumerable: true,
    get: function() {
        return _default;
    }
});
const _getmodulebuildinfo = require("./get-module-build-info");
const _stringifyrequest = require("../stringify-request");
const nextEdgeFunctionLoader = function nextEdgeFunctionLoader() {
    const { absolutePagePath, page, rootDir, preferredRegion, middlewareConfig: middlewareConfigBase64 } = this.getOptions();
    const stringifiedPagePath = (0, _stringifyrequest.stringifyRequest)(this, absolutePagePath);
    const buildInfo = (0, _getmodulebuildinfo.getModuleBuildInfo)(this._module);
    const middlewareConfig = JSON.parse(Buffer.from(middlewareConfigBase64, 'base64').toString());
    buildInfo.route = {
        page: page || '/',
        absolutePagePath,
        preferredRegion,
        middlewareConfig
    };
    buildInfo.nextEdgeApiFunction = {
        page: page || '/'
    };
    buildInfo.rootDir = rootDir;
    return `
        import 'next/dist/esm/server/web/globals'
        import { adapter } from 'next/dist/esm/server/web/adapter'
        import { IncrementalCache } from 'next/dist/esm/server/lib/incremental-cache'
        import { wrapApiHandler } from 'next/dist/esm/server/api-utils'

        import handler from ${stringifiedPagePath}

        if (typeof handler !== 'function') {
          throw new Error('The Edge Function "pages${page}" must export a \`default\` function');
        }

        export default function nHandler (opts) {
          return adapter({
              ...opts,
              IncrementalCache,
              page: ${JSON.stringify(page)},
              handler: wrapApiHandler(${JSON.stringify(page)}, handler),
          })
        }
    `;
};
const _default = nextEdgeFunctionLoader;

//# sourceMappingURL=next-edge-function-loader.js.map