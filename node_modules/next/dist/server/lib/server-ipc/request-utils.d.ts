export declare const deserializeErr: (serializedErr: any) => any;
export declare function invokeIpcMethod({ fetchHostname, method, args, ipcPort, ipcKey, }: {
    fetchHostname?: string;
    method: string;
    args: any[];
    ipcPort?: string;
    ipcKey?: string;
}): Promise<any>;
