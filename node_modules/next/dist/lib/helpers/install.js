"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "install", {
    enumerable: true,
    get: function() {
        return install;
    }
});
const _picocolors = require("../picocolors");
const _crossspawn = /*#__PURE__*/ _interop_require_default(require("next/dist/compiled/cross-spawn"));
function _interop_require_default(obj) {
    return obj && obj.__esModule ? obj : {
        default: obj
    };
}
function install(root, dependencies, { packageManager, isOnline, devDependencies }) {
    let args = [];
    if (dependencies.length > 0) {
        if (packageManager === 'yarn') {
            args = [
                'add',
                '--exact'
            ];
            if (devDependencies) args.push('--dev');
        } else if (packageManager === 'pnpm') {
            args = [
                'add',
                '--save-exact'
            ];
            args.push(devDependencies ? '--save-dev' : '--save-prod');
        } else {
            // npm
            args = [
                'install',
                '--save-exact'
            ];
            args.push(devDependencies ? '--save-dev' : '--save');
        }
        args.push(...dependencies);
    } else {
        args = [
            'install'
        ] // npm, pnpm, and yarn all support `install`
        ;
        if (!isOnline) {
            args.push('--offline');
            console.log((0, _picocolors.yellow)('You appear to be offline.'));
            if (packageManager !== 'npm') {
                console.log((0, _picocolors.yellow)(`Falling back to the local ${packageManager} cache.`));
            }
            console.log();
        }
    }
    return new Promise((resolve, reject)=>{
        /**
     * Spawn the installation process.
     */ const child = (0, _crossspawn.default)(packageManager, args, {
            cwd: root,
            stdio: 'inherit',
            env: {
                ...process.env,
                ADBLOCK: '1',
                // we set NODE_ENV to development as pnpm skips dev
                // dependencies when production
                NODE_ENV: 'development',
                DISABLE_OPENCOLLECTIVE: '1'
            }
        });
        child.on('close', (code)=>{
            if (code !== 0) {
                reject({
                    command: `${packageManager} ${args.join(' ')}`
                });
                return;
            }
            resolve();
        });
    });
}

//# sourceMappingURL=install.js.map