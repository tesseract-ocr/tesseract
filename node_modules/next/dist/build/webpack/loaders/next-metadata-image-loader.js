/*
 * This loader is responsible for extracting the metadata image info for rendering in html
 */ "use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
0 && (module.exports = {
    default: null,
    raw: null
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
    raw: function() {
        return raw;
    }
});
const _fs = require("fs");
const _path = /*#__PURE__*/ _interop_require_default(require("path"));
const _loaderutils3 = /*#__PURE__*/ _interop_require_default(require("next/dist/compiled/loader-utils3"));
const _imageoptimizer = require("../../../server/image-optimizer");
const _mimetype = require("../../../lib/mime-type");
const _constants = require("../../../lib/constants");
const _normalizepathsep = require("../../../shared/lib/page-path/normalize-path-sep");
const _utils = require("./utils");
function _interop_require_default(obj) {
    return obj && obj.__esModule ? obj : {
        default: obj
    };
}
// [NOTE] For turbopack, refer to app_page_loader_tree's write_metadata_item for
// corresponding features.
async function nextMetadataImageLoader(content) {
    const options = this.getOptions();
    const { type, segment, pageExtensions, basePath } = options;
    const { resourcePath, rootContext: context } = this;
    const { name: fileNameBase, ext } = _path.default.parse(resourcePath);
    const useNumericSizes = type === 'twitter' || type === 'openGraph';
    let extension = ext.slice(1);
    if (extension === 'jpg') {
        extension = 'jpeg';
    }
    const opts = {
        context,
        content
    };
    // No hash query for favicon.ico
    const contentHash = type === 'favicon' ? '' : _loaderutils3.default.interpolateName(this, '[contenthash]', opts);
    const interpolatedName = _loaderutils3.default.interpolateName(this, '[name].[ext]', opts);
    const isDynamicResource = pageExtensions.includes(extension);
    const pageSegment = isDynamicResource ? fileNameBase : interpolatedName;
    const hashQuery = contentHash ? '?' + contentHash : '';
    const pathnamePrefix = (0, _normalizepathsep.normalizePathSep)(_path.default.join(basePath, segment));
    if (isDynamicResource) {
        const exportedFieldsExcludingDefault = (await (0, _utils.getLoaderModuleNamedExports)(resourcePath, this)).filter((name)=>name !== 'default');
        // re-export and spread as `exportedImageData` to avoid non-exported error
        return `\
    import {
      ${exportedFieldsExcludingDefault.map((field)=>`${field} as _${field}`).join(',')}
    } from ${JSON.stringify(// This is an arbitrary resource query to ensure it's a new request, instead
        // of sharing the same module with next-metadata-route-loader.
        // Since here we only need export fields such as `size`, `alt` and
        // `generateImageMetadata`, avoid sharing the same module can make this entry
        // smaller.
        resourcePath + '?' + _constants.WEBPACK_RESOURCE_QUERIES.metadataImageMeta)}
    import { fillMetadataSegment } from 'next/dist/lib/metadata/get-metadata-route'

    const imageModule = {
      ${exportedFieldsExcludingDefault.map((field)=>`${field}: _${field}`).join(',')}
    }

    export default async function (props) {
      const { __metadata_id__: _, ...params } = await props.params
      const imageUrl = fillMetadataSegment(${JSON.stringify(pathnamePrefix)}, params, ${JSON.stringify(pageSegment)})

      const { generateImageMetadata } = imageModule

      function getImageMetadata(imageMetadata, idParam) {
        const data = {
          alt: imageMetadata.alt,
          type: imageMetadata.contentType || 'image/png',
          url: imageUrl + (idParam ? ('/' + idParam) : '') + ${JSON.stringify(hashQuery)},
        }
        const { size } = imageMetadata
        if (size) {
          ${type === 'twitter' || type === 'openGraph' ? 'data.width = size.width; data.height = size.height;' : 'data.sizes = size.width + "x" + size.height;'}
        }
        return data
      }

      if (generateImageMetadata) {
        const imageMetadataArray = await generateImageMetadata({ params })
        return imageMetadataArray.map((imageMetadata, index) => {
          const idParam = (imageMetadata.id || index) + ''
          return getImageMetadata(imageMetadata, idParam)
        })
      } else {
        return [getImageMetadata(imageModule, '')]
      }
    }`;
    }
    const imageSize = await (0, _imageoptimizer.getImageSize)(content).catch((err)=>err);
    if (imageSize instanceof Error) {
        const err = imageSize;
        err.name = 'InvalidImageFormatError';
        throw err;
    }
    const imageData = {
        ...extension in _mimetype.imageExtMimeTypeMap && {
            type: _mimetype.imageExtMimeTypeMap[extension]
        },
        ...useNumericSizes && imageSize.width != null && imageSize.height != null ? imageSize : {
            sizes: // For SVGs, skip sizes and use "any" to let it scale automatically based on viewport,
            // For the images doesn't provide the size properly, use "any" as well.
            // If the size is presented, use the actual size for the image.
            extension !== 'svg' && imageSize.width != null && imageSize.height != null ? `${imageSize.width}x${imageSize.height}` : 'any'
        }
    };
    if (type === 'openGraph' || type === 'twitter') {
        const altPath = _path.default.join(_path.default.dirname(resourcePath), fileNameBase + '.alt.txt');
        if ((0, _fs.existsSync)(altPath)) {
            imageData.alt = await _fs.promises.readFile(altPath, 'utf8');
        }
    }
    return `\
  import { fillMetadataSegment } from 'next/dist/lib/metadata/get-metadata-route'

  export default async (props) => {
    const imageData = ${JSON.stringify(imageData)}
    const imageUrl = fillMetadataSegment(${JSON.stringify(pathnamePrefix)}, await props.params, ${JSON.stringify(pageSegment)})

    return [{
      ...imageData,
      url: imageUrl + ${JSON.stringify(type === 'favicon' ? '' : hashQuery)},
    }]
  }`;
}
const raw = true;
const _default = nextMetadataImageLoader;

//# sourceMappingURL=next-metadata-image-loader.js.map