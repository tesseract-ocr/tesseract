/**
 * In edge runtime, these props directly accessed from environment variables.
 *   - local: env vars will be injected through edge-runtime as runtime env vars
 *   - deployment: env vars will be replaced by edge build pipeline
 */
export declare function getEdgePreviewProps(): {
    previewModeId: string;
    previewModeSigningKey: string;
    previewModeEncryptionKey: string;
};
