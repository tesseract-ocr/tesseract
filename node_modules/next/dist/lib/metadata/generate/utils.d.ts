declare function resolveArray<T>(value: T | T[]): T[];
declare function resolveAsArrayOrUndefined<T>(value: T | T[] | undefined | null): T extends undefined | null ? undefined : T[];
export { resolveAsArrayOrUndefined, resolveArray };
