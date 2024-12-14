"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "patchIncorrectLockfile", {
    enumerable: true,
    get: function() {
        return patchIncorrectLockfile;
    }
});
const _fs = require("fs");
const _log = /*#__PURE__*/ _interop_require_wildcard(require("../build/output/log"));
const _findup = /*#__PURE__*/ _interop_require_default(require("next/dist/compiled/find-up"));
const _packagejson = /*#__PURE__*/ _interop_require_default(require("next/package.json"));
const _ciinfo = require("../server/ci-info");
const _getregistry = require("./helpers/get-registry");
function _interop_require_default(obj) {
    return obj && obj.__esModule ? obj : {
        default: obj
    };
}
function _getRequireWildcardCache(nodeInterop) {
    if (typeof WeakMap !== "function") return null;
    var cacheBabelInterop = new WeakMap();
    var cacheNodeInterop = new WeakMap();
    return (_getRequireWildcardCache = function(nodeInterop) {
        return nodeInterop ? cacheNodeInterop : cacheBabelInterop;
    })(nodeInterop);
}
function _interop_require_wildcard(obj, nodeInterop) {
    if (!nodeInterop && obj && obj.__esModule) {
        return obj;
    }
    if (obj === null || typeof obj !== "object" && typeof obj !== "function") {
        return {
            default: obj
        };
    }
    var cache = _getRequireWildcardCache(nodeInterop);
    if (cache && cache.has(obj)) {
        return cache.get(obj);
    }
    var newObj = {
        __proto__: null
    };
    var hasPropertyDescriptor = Object.defineProperty && Object.getOwnPropertyDescriptor;
    for(var key in obj){
        if (key !== "default" && Object.prototype.hasOwnProperty.call(obj, key)) {
            var desc = hasPropertyDescriptor ? Object.getOwnPropertyDescriptor(obj, key) : null;
            if (desc && (desc.get || desc.set)) {
                Object.defineProperty(newObj, key, desc);
            } else {
                newObj[key] = obj[key];
            }
        }
    }
    newObj.default = obj;
    if (cache) {
        cache.set(obj, newObj);
    }
    return newObj;
}
let registry;
async function fetchPkgInfo(pkg) {
    if (!registry) registry = (0, _getregistry.getRegistry)();
    const res = await fetch(`${registry}${pkg}`);
    if (!res.ok) {
        throw new Error(`Failed to fetch registry info for ${pkg}, got status ${res.status}`);
    }
    const data = await res.json();
    const versionData = data.versions[_packagejson.default.version];
    return {
        os: versionData.os,
        cpu: versionData.cpu,
        engines: versionData.engines,
        tarball: versionData.dist.tarball,
        integrity: versionData.dist.integrity
    };
}
async function patchIncorrectLockfile(dir) {
    if (process.env.NEXT_IGNORE_INCORRECT_LOCKFILE) {
        return;
    }
    const lockfilePath = await (0, _findup.default)('package-lock.json', {
        cwd: dir
    });
    if (!lockfilePath) {
        // if no lockfile present there is no action to take
        return;
    }
    const content = await _fs.promises.readFile(lockfilePath, 'utf8');
    // maintain current line ending
    const endingNewline = content.endsWith('\r\n') ? '\r\n' : content.endsWith('\n') ? '\n' : '';
    const lockfileParsed = JSON.parse(content);
    const lockfileVersion = parseInt(lockfileParsed == null ? void 0 : lockfileParsed.lockfileVersion, 10);
    const expectedSwcPkgs = Object.keys(_packagejson.default['optionalDependencies'] || {}).filter((pkg)=>pkg.startsWith('@next/swc-'));
    const patchDependency = (pkg, pkgData)=>{
        lockfileParsed.dependencies[pkg] = {
            version: _packagejson.default.version,
            resolved: pkgData.tarball,
            integrity: pkgData.integrity,
            optional: true
        };
    };
    const patchPackage = (pkg, pkgData)=>{
        lockfileParsed.packages[pkg] = {
            version: _packagejson.default.version,
            resolved: pkgData.tarball,
            integrity: pkgData.integrity,
            cpu: pkgData.cpu,
            optional: true,
            os: pkgData.os,
            engines: pkgData.engines
        };
    };
    try {
        const supportedVersions = [
            1,
            2,
            3
        ];
        if (!supportedVersions.includes(lockfileVersion)) {
            // bail on unsupported version
            return;
        }
        // v1 only uses dependencies
        // v2 uses dependencies and packages
        // v3 only uses packages
        const shouldPatchDependencies = lockfileVersion === 1 || lockfileVersion === 2;
        const shouldPatchPackages = lockfileVersion === 2 || lockfileVersion === 3;
        if (shouldPatchDependencies && !lockfileParsed.dependencies || shouldPatchPackages && !lockfileParsed.packages) {
            // invalid lockfile so bail
            return;
        }
        const missingSwcPkgs = [];
        let pkgPrefix;
        if (shouldPatchPackages) {
            pkgPrefix = '';
            for (const pkg of Object.keys(lockfileParsed.packages)){
                if (pkg.endsWith('node_modules/next')) {
                    pkgPrefix = pkg.substring(0, pkg.length - 4);
                }
            }
            if (!pkgPrefix) {
                // unable to locate the next package so bail
                return;
            }
        }
        for (const pkg of expectedSwcPkgs){
            if (shouldPatchDependencies && !lockfileParsed.dependencies[pkg] || shouldPatchPackages && !lockfileParsed.packages[`${pkgPrefix}${pkg}`]) {
                missingSwcPkgs.push(pkg);
            }
        }
        if (missingSwcPkgs.length === 0) {
            return;
        }
        _log.warn(`Found lockfile missing swc dependencies,`, _ciinfo.isCI ? 'run next locally to automatically patch' : 'patching...');
        if (_ciinfo.isCI) {
            // no point in updating in CI as the user can't save the patch
            return;
        }
        const pkgsData = await Promise.all(missingSwcPkgs.map((pkg)=>fetchPkgInfo(pkg)));
        for(let i = 0; i < pkgsData.length; i++){
            const pkg = missingSwcPkgs[i];
            const pkgData = pkgsData[i];
            if (shouldPatchDependencies) {
                patchDependency(pkg, pkgData);
            }
            if (shouldPatchPackages) {
                patchPackage(`${pkgPrefix}${pkg}`, pkgData);
            }
        }
        await _fs.promises.writeFile(lockfilePath, JSON.stringify(lockfileParsed, null, 2) + endingNewline);
        _log.warn('Lockfile was successfully patched, please run "npm install" to ensure @next/swc dependencies are downloaded');
    } catch (err) {
        _log.error(`Failed to patch lockfile, please try uninstalling and reinstalling next in this workspace`);
        console.error(err);
    }
}

//# sourceMappingURL=patch-incorrect-lockfile.js.map