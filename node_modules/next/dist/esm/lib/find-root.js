import { dirname } from 'path';
import findUp from 'next/dist/compiled/find-up';
export function findRootLockFile(cwd) {
    return findUp.sync([
        'pnpm-lock.yaml',
        'package-lock.json',
        'yarn.lock',
        'bun.lockb'
    ], {
        cwd
    });
}
export function findRootDir(cwd) {
    const lockFile = findRootLockFile(cwd);
    return lockFile ? dirname(lockFile) : undefined;
}

//# sourceMappingURL=find-root.js.map