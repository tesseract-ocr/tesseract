import React from 'react';
export declare const HeadManagerContext: React.Context<{
    updateHead?: (state: any) => void;
    mountedInstances?: any;
    updateScripts?: (state: any) => void;
    scripts?: any;
    getIsSsr?: () => boolean;
    appDir?: boolean;
    nonce?: string;
}>;
