declare module 'styled-jsx' {
  export type StyledJsxStyleRegistry = {
    styles(options?: { nonce?: string }): JSX.Element[]
    flush(): void
    add(props: any): void
    remove(props: any): void
  }
  export function useStyleRegistry(): StyledJsxStyleRegistry
  export function StyleRegistry({
    children,
    registry
  }: {
    children: JSX.Element | import('react').ReactNode
    registry?: StyledJsxStyleRegistry
  }): JSX.Element
  export function createStyleRegistry(): StyledJsxStyleRegistry
}
