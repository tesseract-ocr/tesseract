"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "recursiveCopy", {
    enumerable: true,
    get: function() {
        return recursiveCopy;
    }
});
const _path = /*#__PURE__*/ _interop_require_default(require("path"));
const _fs = require("fs");
const _asyncsema = require("next/dist/compiled/async-sema");
const _iserror = /*#__PURE__*/ _interop_require_default(require("./is-error"));
function _interop_require_default(obj) {
    return obj && obj.__esModule ? obj : {
        default: obj
    };
}
const COPYFILE_EXCL = _fs.constants.COPYFILE_EXCL;
async function recursiveCopy(source, dest, { concurrency = 32, overwrite = false, filter = ()=>true } = {}) {
    const cwdPath = process.cwd();
    const from = _path.default.resolve(cwdPath, source);
    const to = _path.default.resolve(cwdPath, dest);
    const sema = new _asyncsema.Sema(concurrency);
    // deep copy the file/directory
    async function _copy(item, lstats) {
        const target = item.replace(from, to);
        await sema.acquire();
        if (!lstats) {
            // after lock on first run
            lstats = await _fs.promises.lstat(from);
        }
        // readdir & lstat do not follow symbolic links
        // if part is a symbolic link, follow it with stat
        let isFile = lstats.isFile();
        let isDirectory = lstats.isDirectory();
        if (lstats.isSymbolicLink()) {
            const stats = await _fs.promises.stat(item);
            isFile = stats.isFile();
            isDirectory = stats.isDirectory();
        }
        if (isDirectory) {
            try {
                await _fs.promises.mkdir(target, {
                    recursive: true
                });
            } catch (err) {
                // do not throw `folder already exists` errors
                if ((0, _iserror.default)(err) && err.code !== 'EEXIST') {
                    throw err;
                }
            }
            sema.release();
            const files = await _fs.promises.readdir(item, {
                withFileTypes: true
            });
            await Promise.all(files.map((file)=>_copy(_path.default.join(item, file.name), file)));
        } else if (isFile && // before we send the path to filter
        // we remove the base path (from) and replace \ by / (windows)
        filter(item.replace(from, '').replace(/\\/g, '/'))) {
            await _fs.promises.copyFile(item, target, overwrite ? undefined : COPYFILE_EXCL).catch((err)=>{
                // if overwrite is false we shouldn't fail on EEXIST
                if (err.code !== 'EEXIST') {
                    throw err;
                }
            });
            sema.release();
        } else {
            sema.release();
        }
    }
    await _copy(from);
}

//# sourceMappingURL=recursive-copy.js.map