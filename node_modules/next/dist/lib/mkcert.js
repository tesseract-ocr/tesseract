"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "createSelfSignedCertificate", {
    enumerable: true,
    get: function() {
        return createSelfSignedCertificate;
    }
});
const _fs = /*#__PURE__*/ _interop_require_default(require("fs"));
const _path = /*#__PURE__*/ _interop_require_default(require("path"));
const _getcachedirectory = require("./helpers/get-cache-directory");
const _log = /*#__PURE__*/ _interop_require_wildcard(require("../build/output/log"));
const _child_process = require("child_process");
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
const { WritableStream } = require("node:stream/web");
const MKCERT_VERSION = "v1.4.4";
function getBinaryName() {
    const platform = process.platform;
    const arch = process.arch === "x64" ? "amd64" : process.arch;
    if (platform === "win32") {
        return `mkcert-${MKCERT_VERSION}-windows-${arch}.exe`;
    }
    if (platform === "darwin") {
        return `mkcert-${MKCERT_VERSION}-darwin-${arch}`;
    }
    if (platform === "linux") {
        return `mkcert-${MKCERT_VERSION}-linux-${arch}`;
    }
    throw new Error(`Unsupported platform: ${platform}`);
}
async function downloadBinary() {
    try {
        const binaryName = getBinaryName();
        const cacheDirectory = (0, _getcachedirectory.getCacheDirectory)("mkcert");
        const binaryPath = _path.default.join(cacheDirectory, binaryName);
        if (_fs.default.existsSync(binaryPath)) {
            return binaryPath;
        }
        const downloadUrl = `https://github.com/FiloSottile/mkcert/releases/download/${MKCERT_VERSION}/${binaryName}`;
        await _fs.default.promises.mkdir(cacheDirectory, {
            recursive: true
        });
        _log.info(`Downloading mkcert package...`);
        const response = await fetch(downloadUrl);
        if (!response.ok || !response.body) {
            throw new Error(`request failed with status ${response.status}`);
        }
        _log.info(`Download response was successful, writing to disk`);
        const binaryWriteStream = _fs.default.createWriteStream(binaryPath);
        await response.body.pipeTo(new WritableStream({
            write (chunk) {
                return new Promise((resolve, reject)=>{
                    binaryWriteStream.write(chunk, (error)=>{
                        if (error) {
                            reject(error);
                            return;
                        }
                        resolve();
                    });
                });
            },
            close () {
                return new Promise((resolve, reject)=>{
                    binaryWriteStream.close((error)=>{
                        if (error) {
                            reject(error);
                            return;
                        }
                        resolve();
                    });
                });
            }
        }));
        await _fs.default.promises.chmod(binaryPath, 493);
        return binaryPath;
    } catch (err) {
        _log.error("Error downloading mkcert:", err);
    }
}
async function createSelfSignedCertificate(host, certDir = "certificates") {
    try {
        const binaryPath = await downloadBinary();
        if (!binaryPath) throw new Error("missing mkcert binary");
        const resolvedCertDir = _path.default.resolve(process.cwd(), `./${certDir}`);
        await _fs.default.promises.mkdir(resolvedCertDir, {
            recursive: true
        });
        const keyPath = _path.default.resolve(resolvedCertDir, "localhost-key.pem");
        const certPath = _path.default.resolve(resolvedCertDir, "localhost.pem");
        _log.info("Attempting to generate self signed certificate. This may prompt for your password");
        const defaultHosts = [
            "localhost",
            "127.0.0.1",
            "::1"
        ];
        const hosts = host && !defaultHosts.includes(host) ? [
            ...defaultHosts,
            host
        ] : defaultHosts;
        (0, _child_process.execSync)(`"${binaryPath}" -install -key-file "${keyPath}" -cert-file "${certPath}" ${hosts.join(" ")}`, {
            stdio: "ignore"
        });
        const caLocation = (0, _child_process.execSync)(`"${binaryPath}" -CAROOT`).toString().trim();
        if (!_fs.default.existsSync(keyPath) || !_fs.default.existsSync(certPath)) {
            throw new Error("Certificate files not found");
        }
        _log.info(`CA Root certificate created in ${caLocation}`);
        _log.info(`Certificates created in ${resolvedCertDir}`);
        const gitignorePath = _path.default.resolve(process.cwd(), "./.gitignore");
        if (_fs.default.existsSync(gitignorePath)) {
            const gitignore = await _fs.default.promises.readFile(gitignorePath, "utf8");
            if (!gitignore.includes(certDir)) {
                _log.info("Adding certificates to .gitignore");
                await _fs.default.promises.appendFile(gitignorePath, `\n${certDir}`);
            }
        }
        return {
            key: keyPath,
            cert: certPath,
            rootCA: `${caLocation}/rootCA.pem`
        };
    } catch (err) {
        _log.error("Failed to generate self-signed certificate. Falling back to http.", err);
    }
}

//# sourceMappingURL=mkcert.js.map