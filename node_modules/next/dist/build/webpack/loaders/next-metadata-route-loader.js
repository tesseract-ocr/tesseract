"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
0 && (module.exports = {
    default: null,
    getFilenameAndExtension: null
});
function _export(target, all) {
    for(var name in all)Object.defineProperty(target, name, {
        enumerable: true,
        get: all[name]
    });
}
_export(exports, {
    default: function() {
        return _default;
    },
    getFilenameAndExtension: function() {
        return getFilenameAndExtension;
    }
});
const _fs = /*#__PURE__*/ _interop_require_default(require("fs"));
const _path = /*#__PURE__*/ _interop_require_default(require("path"));
const _mimetype = require("../../../lib/mime-type");
const _utils = require("./utils");
function _interop_require_default(obj) {
    return obj && obj.__esModule ? obj : {
        default: obj
    };
}
function errorOnBadHandler(resourcePath) {
    return `
  if (typeof handler !== 'function') {
    throw new Error('Default export is missing in ${JSON.stringify(resourcePath)}')
  }
  `;
}
const cacheHeader = {
    none: "no-cache, no-store",
    longCache: "public, immutable, no-transform, max-age=31536000",
    revalidate: "public, max-age=0, must-revalidate"
};
function getFilenameAndExtension(resourcePath) {
    const filename = _path.default.basename(resourcePath);
    const [name, ext] = filename.split(".", 2);
    return {
        name,
        ext
    };
}
function getContentType(resourcePath) {
    let { name, ext } = getFilenameAndExtension(resourcePath);
    if (ext === "jpg") ext = "jpeg";
    if (name === "favicon" && ext === "ico") return "image/x-icon";
    if (name === "sitemap") return "application/xml";
    if (name === "robots") return "text/plain";
    if (name === "manifest") return "application/manifest+json";
    if (ext === "png" || ext === "jpeg" || ext === "ico" || ext === "svg") {
        return _mimetype.imageExtMimeTypeMap[ext];
    }
    return "text/plain";
}
async function getStaticAssetRouteCode(resourcePath, fileBaseName) {
    const cache = fileBaseName === "favicon" ? "public, max-age=0, must-revalidate" : process.env.NODE_ENV !== "production" ? cacheHeader.none : cacheHeader.longCache;
    const code = `\
/* static asset route */
import { NextResponse } from 'next/server'

const contentType = ${JSON.stringify(getContentType(resourcePath))}
const buffer = Buffer.from(${JSON.stringify((await _fs.default.promises.readFile(resourcePath)).toString("base64"))}, 'base64'
  )

export function GET() {
  return new NextResponse(buffer, {
    headers: {
      'Content-Type': contentType,
      'Cache-Control': ${JSON.stringify(cache)},
    },
  })
}

export const dynamic = 'force-static'
`;
    return code;
}
function getDynamicTextRouteCode(resourcePath) {
    return `\
/* dynamic asset route */
import { NextResponse } from 'next/server'
import handler from ${JSON.stringify(resourcePath)}
import { resolveRouteData } from 'next/dist/build/webpack/loaders/metadata/resolve-route-data'

const contentType = ${JSON.stringify(getContentType(resourcePath))}
const fileType = ${JSON.stringify(getFilenameAndExtension(resourcePath).name)}

${errorOnBadHandler(resourcePath)}

export async function GET() {
  const data = await handler()
  const content = resolveRouteData(data, fileType)

  return new NextResponse(content, {
    headers: {
      'Content-Type': contentType,
      'Cache-Control': ${JSON.stringify(cacheHeader.revalidate)},
    },
  })
}
`;
}
// <metadata-image>/[id]/route.js
function getDynamicImageRouteCode(resourcePath) {
    return `\
/* dynamic image route */
import { NextResponse } from 'next/server'
import * as userland from ${JSON.stringify(resourcePath)}

const imageModule = { ...userland }

const handler = imageModule.default
const generateImageMetadata = imageModule.generateImageMetadata

${errorOnBadHandler(resourcePath)}

export async function GET(_, ctx) {
  const { __metadata_id__, ...params } = ctx.params || {}
  const targetId = __metadata_id__?.[0]
  let id = undefined
  const imageMetadata = generateImageMetadata ? await generateImageMetadata({ params }) : null

  if (imageMetadata) {
    id = imageMetadata.find((item) => {
      if (process.env.NODE_ENV !== 'production') {
        if (item?.id == null) {
          throw new Error('id property is required for every item returned from generateImageMetadata')
        }
      }
      return item.id.toString() === targetId
    })?.id
    if (id == null) {
      return new NextResponse('Not Found', {
        status: 404,
      })
    }
  }
  return handler({ params: ctx.params ? params : undefined, id })
}
`;
}
async function getDynamicSiteMapRouteCode(resourcePath, page, loaderContext) {
    let staticGenerationCode = "";
    const exportNames = await (0, _utils.getLoaderModuleNamedExports)(resourcePath, loaderContext);
    // Re-export configs but avoid conflicted exports
    const reExportNames = exportNames.filter((name)=>name !== "default" && name !== "generateSitemaps");
    const hasGenerateSiteMaps = exportNames.includes("generateSitemaps");
    if (process.env.NODE_ENV === "production" && hasGenerateSiteMaps && page.includes("[__metadata_id__]")) {
        staticGenerationCode = `\
/* dynamic sitemap route */ 
export async function generateStaticParams() {
  const sitemaps = generateSitemaps ? await generateSitemaps() : []
  const params = []

  for (const item of sitemaps) {
    params.push({ __metadata_id__: item.id.toString() + '.xml' })
  }
  return params
}
    `;
    }
    const code = `\
import { NextResponse } from 'next/server'
import * as userland from ${JSON.stringify(resourcePath)}
import { resolveRouteData } from 'next/dist/build/webpack/loaders/metadata/resolve-route-data'

const sitemapModule = { ...userland }
const handler = sitemapModule.default
const generateSitemaps = sitemapModule.generateSitemaps
const contentType = ${JSON.stringify(getContentType(resourcePath))}
const fileType = ${JSON.stringify(getFilenameAndExtension(resourcePath).name)}

${errorOnBadHandler(resourcePath)}

${"" /* re-export the userland route configs */ }
${reExportNames.length > 0 ? `export { ${reExportNames.join(", ")} } from ${JSON.stringify(resourcePath)}\n` : ""}

export async function GET(_, ctx) {
  const { __metadata_id__, ...params } = ctx.params || {}
  ${"" /* sitemap will be optimized to [__metadata_id__] from [[..._metadata_id__]] in production */ }
  const targetId = process.env.NODE_ENV !== 'production'
    ? __metadata_id__?.[0]
    : __metadata_id__

  let id = undefined
  const sitemaps = generateSitemaps ? await generateSitemaps() : null

  if (sitemaps) {
    id = sitemaps.find((item) => {
      if (process.env.NODE_ENV !== 'production') {
        if (item?.id == null) {
          throw new Error('id property is required for every item returned from generateSitemaps')
        }
      }
      let itemID = item.id.toString()
      if(process.env.NODE_ENV === 'production') {
        itemID += '.xml'
      }
      return itemID === targetId
    })?.id
    if (id == null) {
      return new NextResponse('Not Found', {
        status: 404,
      })
    }
  }

  const data = await handler({ id })
  const content = resolveRouteData(data, fileType)

  return new NextResponse(content, {
    headers: {
      'Content-Type': contentType,
      'Cache-Control': ${JSON.stringify(cacheHeader.revalidate)},
    },
  })
}

${staticGenerationCode}
`;
    return code;
}
// When it's static route, it could be favicon.ico, sitemap.xml, robots.txt etc.
// TODO-METADATA: improve the cache control strategy
const nextMetadataRouterLoader = async function() {
    const { page, isDynamic, filePath } = this.getOptions();
    const { name: fileBaseName } = getFilenameAndExtension(filePath);
    this.addDependency(filePath);
    let code = "";
    if (isDynamic === "1") {
        if (fileBaseName === "robots" || fileBaseName === "manifest") {
            code = getDynamicTextRouteCode(filePath);
        } else if (fileBaseName === "sitemap") {
            code = await getDynamicSiteMapRouteCode(filePath, page, this);
        } else {
            code = getDynamicImageRouteCode(filePath);
        }
    } else {
        code = await getStaticAssetRouteCode(filePath, fileBaseName);
    }
    return code;
};
const _default = nextMetadataRouterLoader;

//# sourceMappingURL=next-metadata-route-loader.js.map