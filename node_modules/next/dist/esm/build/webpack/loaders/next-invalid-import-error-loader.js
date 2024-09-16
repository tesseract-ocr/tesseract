export default function nextInvalidImportErrorLoader() {
    const { message } = this.getOptions();
    throw new Error(message);
}

//# sourceMappingURL=next-invalid-import-error-loader.js.map