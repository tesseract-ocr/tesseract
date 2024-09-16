/// <reference types="node" />
type RotateOperation = {
    type: 'rotate';
    numRotations: number;
};
type ResizeOperation = {
    type: 'resize';
} & ({
    width: number;
    height?: never;
} | {
    height: number;
    width?: never;
} | {
    width: number;
    height: number;
});
export type Operation = RotateOperation | ResizeOperation;
export type Encoding = 'jpeg' | 'png' | 'webp' | 'avif';
export declare function getMetadata(buffer: Buffer): Promise<{
    width: number;
    height: number;
}>;
export declare function processBuffer(buffer: Buffer, operations: Operation[], encoding: Encoding, quality: number): Promise<Buffer>;
export declare function decodeBuffer(buffer: Buffer): Promise<import("./image_data").default>;
export {};
