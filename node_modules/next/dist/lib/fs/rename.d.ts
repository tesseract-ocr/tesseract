/**
 * A drop-in replacement for `fs.rename` that:
 * - allows to move across multiple disks
 * - attempts to retry the operation for certain error codes on Windows
 */
export declare function rename(source: string, target: string, windowsRetryTimeout?: number | false): Promise<void>;
