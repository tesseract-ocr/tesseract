"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "recursiveReadDir", {
    enumerable: true,
    get: function() {
        return recursiveReadDir;
    }
});
const _promises = /*#__PURE__*/ _interop_require_default(require("fs/promises"));
const _path = /*#__PURE__*/ _interop_require_default(require("path"));
function _interop_require_default(obj) {
    return obj && obj.__esModule ? obj : {
        default: obj
    };
}
async function recursiveReadDir(rootDirectory, options = {}) {
    // Grab our options.
    const { pathnameFilter, ignoreFilter, ignorePartFilter, sortPathnames = true, relativePathnames = true } = options;
    // The list of pathnames to return.
    const pathnames = [];
    /**
   * Coerces the pathname to be relative if requested.
   */ const coerce = relativePathnames ? (pathname)=>pathname.replace(rootDirectory, '') : (pathname)=>pathname;
    // The queue of directories to scan.
    let directories = [
        rootDirectory
    ];
    while(directories.length > 0){
        // Load all the files in each directory at the same time.
        const results = await Promise.all(directories.map(async (directory)=>{
            const result = {
                directories: [],
                pathnames: [],
                links: []
            };
            try {
                const dir = await _promises.default.readdir(directory, {
                    withFileTypes: true
                });
                for (const file of dir){
                    // If enabled, ignore the file if it matches the ignore filter.
                    if (ignorePartFilter && ignorePartFilter(file.name)) {
                        continue;
                    }
                    // Handle each file.
                    const absolutePathname = _path.default.join(directory, file.name);
                    // If enabled, ignore the file if it matches the ignore filter.
                    if (ignoreFilter && ignoreFilter(absolutePathname)) {
                        continue;
                    }
                    // If the file is a directory, then add it to the list of directories,
                    // they'll be scanned on a later pass.
                    if (file.isDirectory()) {
                        result.directories.push(absolutePathname);
                    } else if (file.isSymbolicLink()) {
                        result.links.push(absolutePathname);
                    } else if (!pathnameFilter || pathnameFilter(absolutePathname)) {
                        result.pathnames.push(coerce(absolutePathname));
                    }
                }
            } catch (err) {
                // This can only happen when the underlying directory was removed. If
                // anything other than this error occurs, re-throw it.
                // if (err.code !== 'ENOENT') throw err
                if (err.code !== 'ENOENT' || directory === rootDirectory) throw err;
                // The error occurred, so abandon reading this directory.
                return null;
            }
            return result;
        }));
        // Empty the directories, we'll fill it later if some of the files are
        // directories.
        directories = [];
        // Keep track of any symbolic links we find, we'll resolve them later.
        const links = [];
        // For each result of directory scans...
        for (const result of results){
            // If the directory was removed, then skip it.
            if (!result) continue;
            // Add any directories to the list of directories to scan.
            directories.push(...result.directories);
            // Add any symbolic links to the list of symbolic links to resolve.
            links.push(...result.links);
            // Add any file pathnames to the list of pathnames.
            pathnames.push(...result.pathnames);
        }
        // Resolve all the symbolic links we found if any.
        if (links.length > 0) {
            const resolved = await Promise.all(links.map(async (absolutePathname)=>{
                try {
                    return await _promises.default.stat(absolutePathname);
                } catch (err) {
                    // This can only happen when the underlying link was removed. If
                    // anything other than this error occurs, re-throw it.
                    if (err.code !== 'ENOENT') throw err;
                    // The error occurred, so abandon reading this directory.
                    return null;
                }
            }));
            for(let i = 0; i < links.length; i++){
                const stats = resolved[i];
                // If the link was removed, then skip it.
                if (!stats) continue;
                // We would have already ignored the file if it matched the ignore
                // filter, so we don't need to check it again.
                const absolutePathname = links[i];
                if (stats.isDirectory()) {
                    directories.push(absolutePathname);
                } else if (!pathnameFilter || pathnameFilter(absolutePathname)) {
                    pathnames.push(coerce(absolutePathname));
                }
            }
        }
    }
    // Sort the pathnames in place if requested.
    if (sortPathnames) {
        pathnames.sort();
    }
    return pathnames;
}

//# sourceMappingURL=recursive-readdir.js.map