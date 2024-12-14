import { denormalizePagePath } from '../shared/lib/page-path/denormalize-page-path';
import { normalizePagePath } from '../shared/lib/page-path/normalize-page-path';
export function getPageFiles(buildManifest, page) {
    const normalizedPage = denormalizePagePath(normalizePagePath(page));
    let files = buildManifest.pages[normalizedPage];
    if (!files) {
        console.warn(`Could not find files for ${normalizedPage} in .next/build-manifest.json`);
        return [];
    }
    return files;
}

//# sourceMappingURL=get-page-files.js.map