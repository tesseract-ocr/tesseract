// TODO: Remove use of `any` type.
import { initialize, version, router, emitter } from './';
import initHMR from './dev/hot-middleware-client';
import { pageBootstrap } from './page-bootstrap';
//@ts-expect-error requires "moduleResolution": "node16" in tsconfig.json and not .ts extension
import { connect } from '@vercel/turbopack-ecmascript-runtime/browser/dev/hmr-client/hmr-client.ts';
window.next = {
    version: "" + version + "-turbo",
    // router is initialized later so it has to be live-binded
    get router () {
        return router;
    },
    emitter
};
self.__next_set_public_path__ = ()=>{};
self.__webpack_hash__ = '';
const devClient = initHMR('turbopack');
initialize({
    devClient
}).then((param)=>{
    let { assetPrefix } = param;
    // for the page loader
    ;
    self.__turbopack_load_page_chunks__ = (page, chunksData)=>{
        const chunkPromises = chunksData.map(__turbopack_load__);
        Promise.all(chunkPromises).catch((err)=>console.error('failed to load chunks for page ' + page, err));
    };
    connect({
        addMessageListener (cb) {
            devClient.addTurbopackMessageListener(cb);
        },
        sendMessage: devClient.sendTurbopackMessage,
        onUpdateError: devClient.handleUpdateError
    });
    return pageBootstrap(assetPrefix);
}).catch((err)=>{
    console.error('Error was not caught', err);
});

//# sourceMappingURL=next-dev-turbopack.js.map