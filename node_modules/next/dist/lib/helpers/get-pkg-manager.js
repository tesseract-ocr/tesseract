"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "getPkgManager", {
    enumerable: true,
    get: function() {
        return getPkgManager;
    }
});
const _fs = /*#__PURE__*/ _interop_require_default(require("fs"));
const _path = /*#__PURE__*/ _interop_require_default(require("path"));
const _child_process = require("child_process");
function _interop_require_default(obj) {
    return obj && obj.__esModule ? obj : {
        default: obj
    };
}
function getPkgManager(baseDir) {
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
            if (_fs.default.existsSync(_path.default.join(baseDir, lockFile))) {
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
            (0, _child_process.execSync)('yarn --version', {
                stdio: 'ignore'
            });
            return 'yarn';
        } catch  {
            (0, _child_process.execSync)('pnpm --version', {
                stdio: 'ignore'
            });
            return 'pnpm';
        }
    } catch  {
        return 'npm';
    }
}

//# sourceMappingURL=get-pkg-manager.js.map