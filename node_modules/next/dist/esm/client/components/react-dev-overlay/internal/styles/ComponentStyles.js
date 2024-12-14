import { _ as _tagged_template_literal_loose } from "@swc/helpers/_/_tagged_template_literal_loose";
function _templateObject() {
    const data = _tagged_template_literal_loose([
        "\n        ",
        "\n        ",
        "\n        ",
        "\n        ",
        "\n        ",
        "\n        ",
        "\n        ",
        "\n        ",
        "\n        ",
        "\n        ",
        "\n      "
    ]);
    _templateObject = function() {
        return data;
    };
    return data;
}
import { jsx as _jsx } from "react/jsx-runtime";
import { styles as codeFrame } from '../components/CodeFrame/styles';
import { styles as dialog } from '../components/Dialog';
import { styles as leftRightDialogHeader } from '../components/LeftRightDialogHeader/styles';
import { styles as overlay } from '../components/Overlay/styles';
import { styles as terminal } from '../components/Terminal/styles';
import { styles as toast } from '../components/Toast';
import { styles as versionStaleness } from '../components/VersionStalenessInfo';
import { styles as buildErrorStyles } from '../container/BuildError';
import { styles as containerErrorStyles } from '../container/Errors';
import { styles as containerRuntimeErrorStyles } from '../container/RuntimeError';
import { noop as css } from '../helpers/noop-template';
export function ComponentStyles() {
    return /*#__PURE__*/ _jsx("style", {
        children: css(_templateObject(), overlay, toast, dialog, leftRightDialogHeader, codeFrame, terminal, buildErrorStyles, containerErrorStyles, containerRuntimeErrorStyles, versionStaleness)
    });
}

//# sourceMappingURL=ComponentStyles.js.map