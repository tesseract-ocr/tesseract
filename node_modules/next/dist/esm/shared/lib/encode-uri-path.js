export function encodeURIPath(file) {
    return file.split("/").map((p)=>encodeURIComponent(p)).join("/");
}

//# sourceMappingURL=encode-uri-path.js.map