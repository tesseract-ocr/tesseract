// This class creates a simplified webpack error that formats nicely based on
// webpack's build in serializer.
// https://github.com/webpack/webpack/blob/c9d4ff7b054fc581c96ce0e53432d44f9dd8ca72/lib/Stats.js#L294-L356
export class SimpleWebpackError extends Error {
    constructor(file, message){
        super(message);
        this.file = file;
    }
}

//# sourceMappingURL=simpleWebpackError.js.map