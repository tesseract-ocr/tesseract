export declare function recursiveCopy(source: string, dest: string, { concurrency, overwrite, filter, }?: {
    concurrency?: number;
    overwrite?: boolean;
    filter?(filePath: string): boolean;
}): Promise<void>;
