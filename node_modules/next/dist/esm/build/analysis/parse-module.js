import LRUCache from "next/dist/compiled/lru-cache";
import { withPromiseCache } from "../../lib/with-promise-cache";
import { createHash } from "crypto";
import { parse } from "../swc";
/**
 * Parses a module with SWC using an LRU cache where the parsed module will
 * be indexed by a sha of its content holding up to 500 entries.
 */ export const parseModule = withPromiseCache(new LRUCache({
    max: 500
}), async (filename, content)=>parse(content, {
        isModule: "unknown",
        filename
    }).catch(()=>null), (_, content)=>createHash("sha1").update(content).digest("hex"));

//# sourceMappingURL=parse-module.js.map