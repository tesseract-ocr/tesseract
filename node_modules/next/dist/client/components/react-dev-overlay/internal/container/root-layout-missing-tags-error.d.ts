import * as React from 'react';
import type { VersionInfo } from '../../../../../server/dev/parse-version-info';
type RootLayoutMissingTagsErrorProps = {
    missingTags: string[];
    versionInfo?: VersionInfo;
};
export declare const RootLayoutMissingTagsError: React.FC<RootLayoutMissingTagsErrorProps>;
export {};
