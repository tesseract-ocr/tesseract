export declare class SerializableError extends Error {
    constructor(page: string, method: string, path: string, message: string);
}
export declare function isSerializableProps(page: string, method: string, input: any): true;
