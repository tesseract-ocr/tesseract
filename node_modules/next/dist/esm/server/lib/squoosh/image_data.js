export default class ImageData {
    static from(input) {
        return new ImageData(input.data || input._data, input.width, input.height);
    }
    get data() {
        if (Object.prototype.toString.call(this._data) === "[object Object]") {
            return Buffer.from(Object.values(this._data));
        }
        if (this._data instanceof Buffer || this._data instanceof Uint8Array || this._data instanceof Uint8ClampedArray) {
            return Buffer.from(this._data);
        }
        throw new Error("invariant");
    }
    constructor(data, width, height){
        this._data = data;
        this.width = width;
        this.height = height;
    }
}

//# sourceMappingURL=image_data.js.map