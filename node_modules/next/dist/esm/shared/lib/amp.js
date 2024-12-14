import React from 'react';
import { AmpStateContext } from './amp-context.shared-runtime';
import { isInAmpMode } from './amp-mode';
export function useAmp() {
    // Don't assign the context value to a variable to save bytes
    return isInAmpMode(React.useContext(AmpStateContext));
}

//# sourceMappingURL=amp.js.map