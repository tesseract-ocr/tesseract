/// <reference types="./css" />
/// <reference types="./index" />
/// <reference types="./macro" />
/// <reference types="./style" />

import React from 'react'

declare module 'react' {
  interface StyleHTMLAttributes<T> extends HTMLAttributes<T> {
    jsx?: boolean
    global?: boolean
  }
}
