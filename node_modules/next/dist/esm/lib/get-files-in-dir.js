import { join } from 'path';
import fs from 'fs/promises';
export async function getFilesInDir(path) {
    const dir = await fs.opendir(path);
    const results = new Set();
    for await (const file of dir){
        let resolvedFile = file;
        if (file.isSymbolicLink()) {
            resolvedFile = await fs.stat(join(path, file.name));
        }
        if (resolvedFile.isFile()) {
            results.add(file.name);
        }
    }
    return results;
}

//# sourceMappingURL=get-files-in-dir.js.map