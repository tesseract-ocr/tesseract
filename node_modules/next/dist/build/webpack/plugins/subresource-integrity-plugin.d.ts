import { webpack } from 'next/dist/compiled/webpack/webpack';
export type SubresourceIntegrityAlgorithm = 'sha256' | 'sha384' | 'sha512';
export declare class SubresourceIntegrityPlugin {
    private readonly algorithm;
    constructor(algorithm: SubresourceIntegrityAlgorithm);
    apply(compiler: webpack.Compiler): void;
}
