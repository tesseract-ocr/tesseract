/**
 * Recursively delete directory contents
 */
export declare function recursiveDelete(
/** Directory to delete the contents of */
dir: string, 
/** Exclude based on relative file path */
exclude?: RegExp, 
/** Ensures that parameter dir exists, this is not passed recursively */
previousPath?: string): Promise<void>;
