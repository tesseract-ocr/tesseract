"TURBOPACK { transition: next-shared }";
import { requestAsyncStorage } from "./request-async-storage-instance";
export { requestAsyncStorage };
export function getExpectedRequestStore(callingExpression) {
    const store = requestAsyncStorage.getStore();
    if (store) return store;
    throw new Error("`" + callingExpression + "` was called outside a request scope. Read more: https://nextjs.org/docs/messages/next-dynamic-api-wrong-context");
}

//# sourceMappingURL=request-async-storage.external.js.map