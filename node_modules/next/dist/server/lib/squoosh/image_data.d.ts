/// <reference types="node" />
export default class ImageData {
    static from(input: ImageData): ImageData;
    private _data;
    width: number;
    height: number;
    get data(): Buffer;
    constructor(data: Buffer | Uint8Array | Uint8ClampedArray, width: number, height: number);
}
