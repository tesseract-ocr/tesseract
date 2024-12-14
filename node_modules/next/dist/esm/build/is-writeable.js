import fs from 'fs';
export async function isWriteable(directory) {
    try {
        await fs.promises.access(directory, (fs.constants || fs).W_OK);
        return true;
    } catch (err) {
        return false;
    }
}

//# sourceMappingURL=is-writeable.js.map