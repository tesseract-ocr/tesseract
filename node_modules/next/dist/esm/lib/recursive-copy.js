import path from 'path';
import { promises, constants } from 'fs';
import { Sema } from 'next/dist/compiled/async-sema';
import isError from './is-error';
const COPYFILE_EXCL = constants.COPYFILE_EXCL;
export async function recursiveCopy(source, dest, { concurrency = 32, overwrite = false, filter = ()=>true } = {}) {
    const cwdPath = process.cwd();
    const from = path.resolve(cwdPath, source);
    const to = path.resolve(cwdPath, dest);
    const sema = new Sema(concurrency);
    // deep copy the file/directory
    async function _copy(item, lstats) {
        const target = item.replace(from, to);
        await sema.acquire();
        if (!lstats) {
            // after lock on first run
            lstats = await promises.lstat(from);
        }
        // readdir & lstat do not follow symbolic links
        // if part is a symbolic link, follow it with stat
        let isFile = lstats.isFile();
        let isDirectory = lstats.isDirectory();
        if (lstats.isSymbolicLink()) {
            const stats = await promises.stat(item);
            isFile = stats.isFile();
            isDirectory = stats.isDirectory();
        }
        if (isDirectory) {
            try {
                await promises.mkdir(target, {
                    recursive: true
                });
            } catch (err) {
                // do not throw `folder already exists` errors
                if (isError(err) && err.code !== 'EEXIST') {
                    throw err;
                }
            }
            sema.release();
            const files = await promises.readdir(item, {
                withFileTypes: true
            });
            await Promise.all(files.map((file)=>_copy(path.join(item, file.name), file)));
        } else if (isFile && // before we send the path to filter
        // we remove the base path (from) and replace \ by / (windows)
        filter(item.replace(from, '').replace(/\\/g, '/'))) {
            await promises.copyFile(item, target, overwrite ? undefined : COPYFILE_EXCL).catch((err)=>{
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