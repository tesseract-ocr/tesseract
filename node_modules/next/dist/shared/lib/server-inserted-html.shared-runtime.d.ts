import React from 'react';
export type ServerInsertedHTMLHook = (callbacks: () => React.ReactNode) => void;
export declare const ServerInsertedHTMLContext: React.Context<ServerInsertedHTMLHook | null>;
export declare function useServerInsertedHTML(callback: () => React.ReactNode): void;
