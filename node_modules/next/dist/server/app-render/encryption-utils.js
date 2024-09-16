"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
0 && (module.exports = {
    arrayBufferToString: null,
    decrypt: null,
    encrypt: null,
    generateEncryptionKeyBase64: null,
    getActionEncryptionKey: null,
    getClientReferenceManifestSingleton: null,
    getServerModuleMap: null,
    setReferenceManifestsSingleton: null,
    stringToUint8Array: null
});
function _export(target, all) {
    for(var name in all)Object.defineProperty(target, name, {
        enumerable: true,
        get: all[name]
    });
}
_export(exports, {
    arrayBufferToString: function() {
        return arrayBufferToString;
    },
    decrypt: function() {
        return decrypt;
    },
    encrypt: function() {
        return encrypt;
    },
    generateEncryptionKeyBase64: function() {
        return generateEncryptionKeyBase64;
    },
    getActionEncryptionKey: function() {
        return getActionEncryptionKey;
    },
    getClientReferenceManifestSingleton: function() {
        return getClientReferenceManifestSingleton;
    },
    getServerModuleMap: function() {
        return getServerModuleMap;
    },
    setReferenceManifestsSingleton: function() {
        return setReferenceManifestsSingleton;
    },
    stringToUint8Array: function() {
        return stringToUint8Array;
    }
});
// Keep the key in memory as it should never change during the lifetime of the server in
// both development and production.
let __next_encryption_key_generation_promise = null;
let __next_loaded_action_key;
let __next_internal_development_raw_action_key;
function arrayBufferToString(buffer) {
    const bytes = new Uint8Array(buffer);
    const len = bytes.byteLength;
    // @anonrig: V8 has a limit of 65535 arguments in a function.
    // For len < 65535, this is faster.
    // https://github.com/vercel/next.js/pull/56377#pullrequestreview-1656181623
    if (len < 65535) {
        return String.fromCharCode.apply(null, bytes);
    }
    let binary = "";
    for(let i = 0; i < len; i++){
        binary += String.fromCharCode(bytes[i]);
    }
    return binary;
}
function stringToUint8Array(binary) {
    const len = binary.length;
    const arr = new Uint8Array(len);
    for(let i = 0; i < len; i++){
        arr[i] = binary.charCodeAt(i);
    }
    return arr;
}
function encrypt(key, iv, data) {
    return crypto.subtle.encrypt({
        name: "AES-GCM",
        iv
    }, key, data);
}
function decrypt(key, iv, data) {
    return crypto.subtle.decrypt({
        name: "AES-GCM",
        iv
    }, key, data);
}
async function generateEncryptionKeyBase64(dev) {
    // For development, we just keep one key in memory for all actions.
    // This makes things faster.
    if (dev) {
        if (typeof __next_internal_development_raw_action_key !== "undefined") {
            return __next_internal_development_raw_action_key;
        }
    }
    // This avoids it being generated multiple times in parallel.
    if (!__next_encryption_key_generation_promise) {
        __next_encryption_key_generation_promise = new Promise(async (resolve, reject)=>{
            try {
                const key = await crypto.subtle.generateKey({
                    name: "AES-GCM",
                    length: 256
                }, true, [
                    "encrypt",
                    "decrypt"
                ]);
                const exported = await crypto.subtle.exportKey("raw", key);
                const b64 = btoa(arrayBufferToString(exported));
                resolve([
                    key,
                    b64
                ]);
            } catch (error) {
                reject(error);
            }
        });
    }
    const [key, b64] = await __next_encryption_key_generation_promise;
    __next_loaded_action_key = key;
    if (dev) {
        __next_internal_development_raw_action_key = b64;
    }
    return b64;
}
// This is a global singleton that is used to encode/decode the action bound args from
// the closure. This can't be using a AsyncLocalStorage as it might happen on the module
// level. Since the client reference manifest won't be mutated, let's use a global singleton
// to keep it.
const SERVER_ACTION_MANIFESTS_SINGLETON = Symbol.for("next.server.action-manifests");
function setReferenceManifestsSingleton({ clientReferenceManifest, serverActionsManifest, serverModuleMap }) {
    // @ts-ignore
    globalThis[SERVER_ACTION_MANIFESTS_SINGLETON] = {
        clientReferenceManifest,
        serverActionsManifest,
        serverModuleMap
    };
}
function getServerModuleMap() {
    const serverActionsManifestSingleton = globalThis[SERVER_ACTION_MANIFESTS_SINGLETON];
    if (!serverActionsManifestSingleton) {
        throw new Error("Missing manifest for Server Actions. This is a bug in Next.js");
    }
    return serverActionsManifestSingleton.serverModuleMap;
}
function getClientReferenceManifestSingleton() {
    const serverActionsManifestSingleton = globalThis[SERVER_ACTION_MANIFESTS_SINGLETON];
    if (!serverActionsManifestSingleton) {
        throw new Error("Missing manifest for Server Actions. This is a bug in Next.js");
    }
    return serverActionsManifestSingleton.clientReferenceManifest;
}
async function getActionEncryptionKey() {
    if (__next_loaded_action_key) {
        return __next_loaded_action_key;
    }
    const serverActionsManifestSingleton = globalThis[SERVER_ACTION_MANIFESTS_SINGLETON];
    if (!serverActionsManifestSingleton) {
        throw new Error("Missing manifest for Server Actions. This is a bug in Next.js");
    }
    const rawKey = process.env.NEXT_SERVER_ACTIONS_ENCRYPTION_KEY || serverActionsManifestSingleton.serverActionsManifest.encryptionKey;
    if (rawKey === undefined) {
        throw new Error("Missing encryption key for Server Actions");
    }
    __next_loaded_action_key = await crypto.subtle.importKey("raw", stringToUint8Array(atob(rawKey)), "AES-GCM", true, [
        "encrypt",
        "decrypt"
    ]);
    return __next_loaded_action_key;
}

//# sourceMappingURL=encryption-utils.js.map