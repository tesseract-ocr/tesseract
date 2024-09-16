import * as React from 'react';
import type { VersionInfo } from '../../../../../server/dev/parse-version-info';
export type BuildErrorProps = {
    message: string;
    versionInfo?: VersionInfo;
};
export declare const BuildError: React.FC<BuildErrorProps>;
export declare const styles: string;
