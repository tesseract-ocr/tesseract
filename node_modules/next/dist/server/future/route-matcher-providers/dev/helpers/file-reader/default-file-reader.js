"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "DefaultFileReader", {
    enumerable: true,
    get: function() {
        return DefaultFileReader;
    }
});
const _recursivereaddir = require("../../../../../../lib/recursive-readdir");
class DefaultFileReader {
    /**
   * Creates a new file reader.
   *
   * @param pathnameFilter filter to ignore files with absolute pathnames, false to ignore
   * @param ignoreFilter filter to ignore files and directories with absolute pathnames, false to ignore
   * @param ignorePartFilter filter to ignore files and directories with the pathname part, false to ignore
   */ constructor(options){
        this.options = options;
    }
    /**
   * Reads all the files in the directory and its subdirectories following any
   * symbolic links.
   *
   * @param dir the directory to read
   * @returns a promise that resolves to the list of files
   */ async read(dir) {
        return (0, _recursivereaddir.recursiveReadDir)(dir, {
            pathnameFilter: this.options.pathnameFilter,
            ignoreFilter: this.options.ignoreFilter,
            ignorePartFilter: this.options.ignorePartFilter,
            // We don't need to sort the results because we're not depending on the
            // order of the results.
            sortPathnames: false,
            // We want absolute pathnames because we're going to be comparing them
            // with other absolute pathnames.
            relativePathnames: false
        });
    }
}

//# sourceMappingURL=default-file-reader.js.map