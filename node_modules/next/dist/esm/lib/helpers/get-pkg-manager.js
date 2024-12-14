import fs from 'fs';
import path from 'path';
import { execSync } from 'child_process';
export function getPkgManager(baseDir) {
    try {
        for (const { lockFile, packageManager } of [
            {
                lockFile: 'yarn.lock',
                packageManager: 'yarn'
            },
            {
                lockFile: 'pnpm-lock.yaml',
                packageManager: 'pnpm'
            },
            {
                lockFile: 'package-lock.json',
                packageManager: 'npm'
            }
        ]){
            if (fs.existsSync(path.join(baseDir, lockFile))) {
                return packageManager;
            }
        }
        const userAgent = process.env.npm_config_user_agent;
        if (userAgent) {
            if (userAgent.startsWith('yarn')) {
                return 'yarn';
            } else if (userAgent.startsWith('pnpm')) {
                return 'pnpm';
            }
        }
        try {
            execSync('yarn --version', {
                stdio: 'ignore'
            });
            return 'yarn';
        } catch  {
            execSync('pnpm --version', {
                stdio: 'ignore'
            });
            return 'pnpm';
        }
    } catch  {
        return 'npm';
    }
}

//# sourceMappingURL=get-pkg-manager.js.map