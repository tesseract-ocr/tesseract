import fs from 'fs';
export const nodeFs = {
    existsSync: fs.existsSync,
    readFile: fs.promises.readFile,
    readFileSync: fs.readFileSync,
    writeFile: (f, d)=>fs.promises.writeFile(f, d),
    mkdir: (dir)=>fs.promises.mkdir(dir, {
            recursive: true
        }),
    stat: (f)=>fs.promises.stat(f)
};

//# sourceMappingURL=node-fs-methods.js.map