import type { types as BabelTypes } from 'next/dist/compiled/babel/core';
import type { PluginObj } from 'next/dist/compiled/babel/core';
export default function ({ types: t, }: {
    types: typeof BabelTypes;
}): PluginObj<any>;
