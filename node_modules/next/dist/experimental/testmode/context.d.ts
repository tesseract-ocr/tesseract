export interface TestReqInfo {
    url: string;
    proxyPort: number;
    testData: string;
}
export interface TestRequestReader<R> {
    url(req: R): string;
    header(req: R, name: string): string | null;
}
export declare function withRequest<R, T>(req: R, reader: TestRequestReader<R>, fn: () => T): T;
export declare function getTestReqInfo<R>(req?: R, reader?: TestRequestReader<R>): TestReqInfo | undefined;
