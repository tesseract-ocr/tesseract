import { webpack, sources } from 'next/dist/compiled/webpack/webpack';
import crypto from 'crypto';
import { SUBRESOURCE_INTEGRITY_MANIFEST } from '../../../shared/lib/constants';
const PLUGIN_NAME = 'SubresourceIntegrityPlugin';
export class SubresourceIntegrityPlugin {
    constructor(algorithm){
        this.algorithm = algorithm;
    }
    apply(compiler) {
        compiler.hooks.make.tap(PLUGIN_NAME, (compilation)=>{
            compilation.hooks.afterOptimizeAssets.tap({
                name: PLUGIN_NAME,
                stage: webpack.Compilation.PROCESS_ASSETS_STAGE_ADDITIONS
            }, (assets)=>{
                // Collect all the assets.
                let files = new Set();
                for (const asset of compilation.getAssets()){
                    files.add(asset.name);
                }
                // For each file, deduped, calculate the file hash.
                const hashes = {};
                for (const file of files.values()){
                    // Get the buffer for the asset.
                    const asset = assets[file];
                    if (!asset) {
                        throw new Error(`could not get asset: ${file}`);
                    }
                    // Get the buffer for the asset.
                    const buffer = asset.buffer();
                    // Create the hash for the content.
                    const hash = crypto.createHash(this.algorithm).update(buffer).digest().toString('base64');
                    hashes[file] = `${this.algorithm}-${hash}`;
                }
                const json = JSON.stringify(hashes, null, 2);
                const file = 'server/' + SUBRESOURCE_INTEGRITY_MANIFEST;
                assets[file + '.js'] = new sources.RawSource(`self.__SUBRESOURCE_INTEGRITY_MANIFEST=${JSON.stringify(json)}`);
                assets[file + '.json'] = new sources.RawSource(json);
            });
        });
    }
}

//# sourceMappingURL=subresource-integrity-plugin.js.map