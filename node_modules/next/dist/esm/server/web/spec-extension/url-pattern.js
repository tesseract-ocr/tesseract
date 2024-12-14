const GlobalURLPattern = // @ts-expect-error: URLPattern is not available in Node.js
typeof URLPattern === 'undefined' ? undefined : URLPattern;
export { GlobalURLPattern as URLPattern };

//# sourceMappingURL=url-pattern.js.map