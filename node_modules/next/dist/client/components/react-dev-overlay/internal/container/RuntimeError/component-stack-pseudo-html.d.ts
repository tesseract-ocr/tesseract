import type { ComponentStackFrame } from '../../helpers/parse-component-stack';
/**
 *
 * Format component stack into pseudo HTML
 * component stack is an array of strings, e.g.: ['p', 'p', 'Page', ...]
 *
 * For html tags mismatch, it will render it for the code block
 *
 * ```
 * <pre>
 *  <code>{`
 *    <Page>
 *       <p red>
 *         <p red>
 *  `}</code>
 * </pre>
 * ```
 *
 * For text mismatch, it will render it for the code block
 *
 * ```
 * <pre>
 * <code>{`
 *   <Page>
 *     <p>
 *       "Server Text" (green)
 *       "Client Text" (red)
 *     </p>
 *   </Page>
 * `}</code>
 * ```
 *
 * For bad text under a tag it will render it for the code block,
 * e.g. "Mismatched Text" under <p>
 *
 * ```
 * <pre>
 * <code>{`
 *   <Page>
 *     <div>
 *       <p>
 *         "Mismatched Text" (red)
 *      </p>
 *     </div>
 *   </Page>
 * `}</code>
 * ```
 *
 */
export declare function PseudoHtmlDiff({ componentStackFrames, firstContent, secondContent, hydrationMismatchType, ...props }: {
    componentStackFrames: ComponentStackFrame[];
    firstContent: string;
    secondContent: string;
    hydrationMismatchType: 'tag' | 'text' | 'text-in-tag';
} & React.HTMLAttributes<HTMLPreElement>): import("react/jsx-runtime").JSX.Element;
