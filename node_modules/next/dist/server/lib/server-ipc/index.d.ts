import type NextServer from '../../next-server';
export declare function createIpcServer(server: InstanceType<typeof NextServer>): Promise<{
    ipcPort: number;
    ipcServer: import('http').Server;
    ipcValidationKey: string;
}>;
