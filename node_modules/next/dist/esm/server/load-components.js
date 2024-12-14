import { BUILD_MANIFEST, REACT_LOADABLE_MANIFEST, CLIENT_REFERENCE_MANIFEST, SERVER_REFERENCE_MANIFEST, DYNAMIC_CSS_MANIFEST } from '../shared/lib/constants';
import { join } from 'path';
import { requirePage } from './require';
import { interopDefault } from '../lib/interop-default';
import { getTracer } from './lib/trace/tracer';
import { LoadComponentsSpan } from './lib/trace/constants';
import { evalManifest, loadManifest } from './load-manifest';
import { wait } from '../lib/wait';
import { setReferenceManifestsSingleton } from './app-render/encryption-utils';
import { createServerModuleMap } from './app-render/action-utils';
import { isMetadataRoute } from '../lib/metadata/is-metadata-route';
/**
 * Load manifest file with retries, defaults to 3 attempts.
 */ export async function loadManifestWithRetries(manifestPath, attempts = 3) {
    while(true){
        try {
            return loadManifest(manifestPath);
        } catch (err) {
            attempts--;
            if (attempts <= 0) throw err;
            await wait(100);
        }
    }
}
/**
 * Load manifest file with retries, defaults to 3 attempts.
 */ export async function evalManifestWithRetries(manifestPath, attempts = 3) {
    while(true){
        try {
            return evalManifest(manifestPath);
        } catch (err) {
            attempts--;
            if (attempts <= 0) throw err;
            await wait(100);
        }
    }
}
async function loadClientReferenceManifest(manifestPath, entryName) {
    try {
        const context = await evalManifestWithRetries(manifestPath);
        return context.__RSC_MANIFEST[entryName];
    } catch (err) {
        return undefined;
    }
}
async function loadComponentsImpl({ distDir, page, isAppPath }) {
    let DocumentMod = {};
    let AppMod = {};
    if (!isAppPath) {
        ;
        [DocumentMod, AppMod] = await Promise.all([
            requirePage('/_document', distDir, false),
            requirePage('/_app', distDir, false)
        ]);
    }
    // Make sure to avoid loading the manifest for metadata route handlers.
    const hasClientManifest = isAppPath && !isMetadataRoute(page);
    // Load the manifest files first
    const [buildManifest, reactLoadableManifest, dynamicCssManifest, clientReferenceManifest, serverActionsManifest] = await Promise.all([
        loadManifestWithRetries(join(distDir, BUILD_MANIFEST)),
        loadManifestWithRetries(join(distDir, REACT_LOADABLE_MANIFEST)),
        // This manifest will only exist in Pages dir && Production && Webpack.
        isAppPath || process.env.TURBOPACK ? undefined : loadManifestWithRetries(join(distDir, `${DYNAMIC_CSS_MANIFEST}.json`)).catch(()=>undefined),
        hasClientManifest ? loadClientReferenceManifest(join(distDir, 'server', 'app', page.replace(/%5F/g, '_') + '_' + CLIENT_REFERENCE_MANIFEST + '.js'), page.replace(/%5F/g, '_')) : undefined,
        isAppPath ? loadManifestWithRetries(join(distDir, 'server', SERVER_REFERENCE_MANIFEST + '.json')).catch(()=>null) : null
    ]);
    // Before requiring the actual page module, we have to set the reference
    // manifests to our global store so Server Action's encryption util can access
    // to them at the top level of the page module.
    if (serverActionsManifest && clientReferenceManifest) {
        setReferenceManifestsSingleton({
            page,
            clientReferenceManifest,
            serverActionsManifest,
            serverModuleMap: createServerModuleMap({
                serverActionsManifest
            })
        });
    }
    const ComponentMod = await requirePage(page, distDir, isAppPath);
    const Component = interopDefault(ComponentMod);
    const Document = interopDefault(DocumentMod);
    const App = interopDefault(AppMod);
    const { getServerSideProps, getStaticProps, getStaticPaths, routeModule } = ComponentMod;
    return {
        App,
        Document,
        Component,
        buildManifest,
        reactLoadableManifest,
        dynamicCssManifest,
        pageConfig: ComponentMod.config || {},
        ComponentMod,
        getServerSideProps,
        getStaticProps,
        getStaticPaths,
        clientReferenceManifest,
        serverActionsManifest,
        isAppPath,
        page,
        routeModule
    };
}
export const loadComponents = getTracer().wrap(LoadComponentsSpan.loadComponents, loadComponentsImpl);

//# sourceMappingURL=load-components.js.map