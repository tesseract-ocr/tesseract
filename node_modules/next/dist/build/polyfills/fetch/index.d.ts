declare const fetchModule: ((input: RequestInfo | URL, init?: RequestInit) => Promise<Response>) & typeof fetch;
