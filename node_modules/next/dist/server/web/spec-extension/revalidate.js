"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
0 && (module.exports = {
    revalidatePath: null,
    revalidateTag: null
});
function _export(target, all) {
    for(var name in all)Object.defineProperty(target, name, {
        enumerable: true,
        get: all[name]
    });
}
_export(exports, {
    revalidatePath: function() {
        return revalidatePath;
    },
    revalidateTag: function() {
        return revalidateTag;
    }
});
const _dynamicrendering = require("../../app-render/dynamic-rendering");
const _utils = require("../../../shared/lib/router/utils");
const _constants = require("../../../lib/constants");
const _url = require("../../../lib/url");
const _staticgenerationasyncstorageexternal = require("../../../client/components/static-generation-async-storage.external");
function revalidateTag(tag) {
    return revalidate(tag, `revalidateTag ${tag}`);
}
function revalidatePath(originalPath, type) {
    if (originalPath.length > _constants.NEXT_CACHE_SOFT_TAG_MAX_LENGTH) {
        console.warn(`Warning: revalidatePath received "${originalPath}" which exceeded max length of ${_constants.NEXT_CACHE_SOFT_TAG_MAX_LENGTH}. See more info here https://nextjs.org/docs/app/api-reference/functions/revalidatePath`);
        return;
    }
    let normalizedPath = `${_constants.NEXT_CACHE_IMPLICIT_TAG_ID}${originalPath}`;
    if (type) {
        normalizedPath += `${normalizedPath.endsWith("/") ? "" : "/"}${type}`;
    } else if ((0, _utils.isDynamicRoute)(originalPath)) {
        console.warn(`Warning: a dynamic page path "${originalPath}" was passed to "revalidatePath", but the "type" parameter is missing. This has no effect by default, see more info here https://nextjs.org/docs/app/api-reference/functions/revalidatePath`);
    }
    return revalidate(normalizedPath, `revalidatePath ${originalPath}`);
}
function revalidate(tag, expression) {
    const store = _staticgenerationasyncstorageexternal.staticGenerationAsyncStorage.getStore();
    if (!store || !store.incrementalCache) {
        throw new Error(`Invariant: static generation store missing in ${expression}`);
    }
    if (store.isUnstableCacheCallback) {
        throw new Error(`Route ${(0, _url.getPathname)(store.urlPathname)} used "${expression}" inside a function cached with "unstable_cache(...)" which is unsupported. To ensure revalidation is performed consistently it must always happen outside of renders and cached functions. See more info here: https://nextjs.org/docs/app/building-your-application/rendering/static-and-dynamic#dynamic-rendering`);
    }
    // a route that makes use of revalidation APIs should be considered dynamic
    // as otherwise it would be impossible to revalidate
    (0, _dynamicrendering.trackDynamicDataAccessed)(store, expression);
    if (!store.revalidatedTags) {
        store.revalidatedTags = [];
    }
    if (!store.revalidatedTags.includes(tag)) {
        store.revalidatedTags.push(tag);
    }
    // TODO: only revalidate if the path matches
    store.pathWasRevalidated = true;
}

//# sourceMappingURL=revalidate.js.map