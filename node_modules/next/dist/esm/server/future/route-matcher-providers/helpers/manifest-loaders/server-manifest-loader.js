export class ServerManifestLoader {
    constructor(getter){
        this.getter = getter;
    }
    load(name) {
        return this.getter(name);
    }
}

//# sourceMappingURL=server-manifest-loader.js.map