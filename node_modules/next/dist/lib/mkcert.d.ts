export interface SelfSignedCertificate {
    key: string;
    cert: string;
    rootCA?: string;
}
export declare function createSelfSignedCertificate(host?: string, certDir?: string): Promise<SelfSignedCertificate | undefined>;
