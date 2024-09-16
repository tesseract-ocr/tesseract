export declare enum FileType {
    File = "file",
    Directory = "directory"
}
export declare function fileExists(fileName: string, type?: FileType): Promise<boolean>;
