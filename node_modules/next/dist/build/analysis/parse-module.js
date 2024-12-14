"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "parseModule", {
    enumerable: true,
    get: function() {
        return parseModule;
    }
});
const _lrucache = require("../../server/lib/lru-cache");
const _withpromisecache = require("../../lib/with-promise-cache");
const _crypto = require("crypto");
const _swc = require("../swc");
const parseModule = (0, _withpromisecache.withPromiseCache)(new _lrucache.LRUCache(500), async (filename, content)=>(0, _swc.parse)(content, {
        isModule: 'unknown',
        filename
    }).catch(()=>null), (_, content)=>(0, _crypto.createHash)('sha1').update(content).digest('hex'));

//# sourceMappingURL=parse-module.js.map