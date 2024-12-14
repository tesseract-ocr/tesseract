import { LRUCache } from './lru-cache';
import { createRequestResponseMocks } from './mock-request';
import { HMR_ACTIONS_SENT_TO_BROWSER } from '../dev/hot-reloader-types';
/**
 * The DevBundlerService provides an interface to perform tasks with the
 * bundler while in development.
 */ export class DevBundlerService {
    constructor(bundler, handler){
        this.bundler = bundler;
        this.handler = handler;
        this.ensurePage = async (definition)=>{
            // TODO: remove after ensure is pulled out of server
            return await this.bundler.hotReloader.ensurePage(definition);
        };
        this.logErrorWithOriginalStack = this.bundler.logErrorWithOriginalStack.bind(this.bundler);
        this.appIsrManifestInner = new LRUCache(8000, function length() {
            return 16;
        });
    }
    async getFallbackErrorComponents(url) {
        await this.bundler.hotReloader.buildFallbackError();
        // Build the error page to ensure the fallback is built too.
        // TODO: See if this can be moved into hotReloader or removed.
        await this.bundler.hotReloader.ensurePage({
            page: '/_error',
            clientOnly: false,
            definition: undefined,
            url
        });
    }
    async getCompilationError(page) {
        const errors = await this.bundler.hotReloader.getCompilationErrors(page);
        if (!errors) return;
        // Return the very first error we found.
        return errors[0];
    }
    async revalidate({ urlPath, revalidateHeaders, opts: revalidateOpts }) {
        const mocked = createRequestResponseMocks({
            url: urlPath,
            headers: revalidateHeaders
        });
        await this.handler(mocked.req, mocked.res);
        await mocked.res.hasStreamed;
        if (mocked.res.getHeader('x-nextjs-cache') !== 'REVALIDATED' && !(mocked.res.statusCode === 404 && revalidateOpts.unstable_onlyGenerated)) {
            throw new Error(`Invalid response ${mocked.res.statusCode}`);
        }
        return {};
    }
    get appIsrManifest() {
        const serializableManifest = {};
        for (const key of this.appIsrManifestInner.keys()){
            serializableManifest[key] = this.appIsrManifestInner.get(key);
        }
        return serializableManifest;
    }
    setAppIsrStatus(key, value) {
        var _this_bundler_hotReloader, _this_bundler;
        if (value === null) {
            this.appIsrManifestInner.remove(key);
        } else {
            this.appIsrManifestInner.set(key, value);
        }
        (_this_bundler = this.bundler) == null ? void 0 : (_this_bundler_hotReloader = _this_bundler.hotReloader) == null ? void 0 : _this_bundler_hotReloader.send({
            action: HMR_ACTIONS_SENT_TO_BROWSER.APP_ISR_MANIFEST,
            data: this.appIsrManifest
        });
    }
}

//# sourceMappingURL=dev-bundler-service.js.map