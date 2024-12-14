declare function resolveArray<T>(value: T | T[]): T[];
declare function resolveAsArrayOrUndefined<T>(value: T | T[] | undefined | null): T extends undefined | null ? undefined : T[];
declare function getOrigin(url: string | URL): string | undefined;
export { resolveAsArrayOrUndefined, resolveArray, getOrigin };
