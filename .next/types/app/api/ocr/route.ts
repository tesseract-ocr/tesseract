// File: /Users/thun/Desktop/Research Document/tesseract/app/api/ocr/route.ts
import * as entry from '../../../../../app/api/ocr/route.js'
import type { NextRequest } from 'next/server.js'

type TEntry = typeof import('../../../../../app/api/ocr/route.js')

// Check that the entry is a valid entry
checkFields<Diff<{
  GET?: Function
  HEAD?: Function
  OPTIONS?: Function
  POST?: Function
  PUT?: Function
  DELETE?: Function
  PATCH?: Function
  config?: {}
  generateStaticParams?: Function
  revalidate?: RevalidateRange<TEntry> | false
  dynamic?: 'auto' | 'force-dynamic' | 'error' | 'force-static'
  dynamicParams?: boolean
  fetchCache?: 'auto' | 'force-no-store' | 'only-no-store' | 'default-no-store' | 'default-cache' | 'only-cache' | 'force-cache'
  preferredRegion?: 'auto' | 'global' | 'home' | string | string[]
  runtime?: 'nodejs' | 'experimental-edge' | 'edge'
  maxDuration?: number
  
}, TEntry, ''>>()

// Check the prop type of the entry function
if ('GET' in entry) {
  checkFields<
    Diff<
      ParamCheck<Request | NextRequest>,
      {
        __tag__: 'GET'
        __param_position__: 'first'
        __param_type__: FirstArg<MaybeField<TEntry, 'GET'>>
      },
      'GET'
    >
  >()
  checkFields<
    Diff<
      ParamCheck<PageParams>,
      {
        __tag__: 'GET'
        __param_position__: 'second'
        __param_type__: SecondArg<MaybeField<TEntry, 'GET'>>
      },
      'GET'
    >
  >()
  
  checkFields<
    Diff<
      {
        __tag__: 'GET',
        __return_type__: Response | void | never | Promise<Response | void | never>
      },
      {
        __tag__: 'GET',
        __return_type__: ReturnType<MaybeField<TEntry, 'GET'>>
      },
      'GET'
    >
  >()
}
// Check the prop type of the entry function
if ('HEAD' in entry) {
  checkFields<
    Diff<
      ParamCheck<Request | NextRequest>,
      {
        __tag__: 'HEAD'
        __param_position__: 'first'
        __param_type__: FirstArg<MaybeField<TEntry, 'HEAD'>>
      },
      'HEAD'
    >
  >()
  checkFields<
    Diff<
      ParamCheck<PageParams>,
      {
        __tag__: 'HEAD'
        __param_position__: 'second'
        __param_type__: SecondArg<MaybeField<TEntry, 'HEAD'>>
      },
      'HEAD'
    >
  >()
  
  checkFields<
    Diff<
      {
        __tag__: 'HEAD',
        __return_type__: Response | void | never | Promise<Response | void | never>
      },
      {
        __tag__: 'HEAD',
        __return_type__: ReturnType<MaybeField<TEntry, 'HEAD'>>
      },
      'HEAD'
    >
  >()
}
// Check the prop type of the entry function
if ('OPTIONS' in entry) {
  checkFields<
    Diff<
      ParamCheck<Request | NextRequest>,
      {
        __tag__: 'OPTIONS'
        __param_position__: 'first'
        __param_type__: FirstArg<MaybeField<TEntry, 'OPTIONS'>>
      },
      'OPTIONS'
    >
  >()
  checkFields<
    Diff<
      ParamCheck<PageParams>,
      {
        __tag__: 'OPTIONS'
        __param_position__: 'second'
        __param_type__: SecondArg<MaybeField<TEntry, 'OPTIONS'>>
      },
      'OPTIONS'
    >
  >()
  
  checkFields<
    Diff<
      {
        __tag__: 'OPTIONS',
        __return_type__: Response | void | never | Promise<Response | void | never>
      },
      {
        __tag__: 'OPTIONS',
        __return_type__: ReturnType<MaybeField<TEntry, 'OPTIONS'>>
      },
      'OPTIONS'
    >
  >()
}
// Check the prop type of the entry function
if ('POST' in entry) {
  checkFields<
    Diff<
      ParamCheck<Request | NextRequest>,
      {
        __tag__: 'POST'
        __param_position__: 'first'
        __param_type__: FirstArg<MaybeField<TEntry, 'POST'>>
      },
      'POST'
    >
  >()
  checkFields<
    Diff<
      ParamCheck<PageParams>,
      {
        __tag__: 'POST'
        __param_position__: 'second'
        __param_type__: SecondArg<MaybeField<TEntry, 'POST'>>
      },
      'POST'
    >
  >()
  
  checkFields<
    Diff<
      {
        __tag__: 'POST',
        __return_type__: Response | void | never | Promise<Response | void | never>
      },
      {
        __tag__: 'POST',
        __return_type__: ReturnType<MaybeField<TEntry, 'POST'>>
      },
      'POST'
    >
  >()
}
// Check the prop type of the entry function
if ('PUT' in entry) {
  checkFields<
    Diff<
      ParamCheck<Request | NextRequest>,
      {
        __tag__: 'PUT'
        __param_position__: 'first'
        __param_type__: FirstArg<MaybeField<TEntry, 'PUT'>>
      },
      'PUT'
    >
  >()
  checkFields<
    Diff<
      ParamCheck<PageParams>,
      {
        __tag__: 'PUT'
        __param_position__: 'second'
        __param_type__: SecondArg<MaybeField<TEntry, 'PUT'>>
      },
      'PUT'
    >
  >()
  
  checkFields<
    Diff<
      {
        __tag__: 'PUT',
        __return_type__: Response | void | never | Promise<Response | void | never>
      },
      {
        __tag__: 'PUT',
        __return_type__: ReturnType<MaybeField<TEntry, 'PUT'>>
      },
      'PUT'
    >
  >()
}
// Check the prop type of the entry function
if ('DELETE' in entry) {
  checkFields<
    Diff<
      ParamCheck<Request | NextRequest>,
      {
        __tag__: 'DELETE'
        __param_position__: 'first'
        __param_type__: FirstArg<MaybeField<TEntry, 'DELETE'>>
      },
      'DELETE'
    >
  >()
  checkFields<
    Diff<
      ParamCheck<PageParams>,
      {
        __tag__: 'DELETE'
        __param_position__: 'second'
        __param_type__: SecondArg<MaybeField<TEntry, 'DELETE'>>
      },
      'DELETE'
    >
  >()
  
  checkFields<
    Diff<
      {
        __tag__: 'DELETE',
        __return_type__: Response | void | never | Promise<Response | void | never>
      },
      {
        __tag__: 'DELETE',
        __return_type__: ReturnType<MaybeField<TEntry, 'DELETE'>>
      },
      'DELETE'
    >
  >()
}
// Check the prop type of the entry function
if ('PATCH' in entry) {
  checkFields<
    Diff<
      ParamCheck<Request | NextRequest>,
      {
        __tag__: 'PATCH'
        __param_position__: 'first'
        __param_type__: FirstArg<MaybeField<TEntry, 'PATCH'>>
      },
      'PATCH'
    >
  >()
  checkFields<
    Diff<
      ParamCheck<PageParams>,
      {
        __tag__: 'PATCH'
        __param_position__: 'second'
        __param_type__: SecondArg<MaybeField<TEntry, 'PATCH'>>
      },
      'PATCH'
    >
  >()
  
  checkFields<
    Diff<
      {
        __tag__: 'PATCH',
        __return_type__: Response | void | never | Promise<Response | void | never>
      },
      {
        __tag__: 'PATCH',
        __return_type__: ReturnType<MaybeField<TEntry, 'PATCH'>>
      },
      'PATCH'
    >
  >()
}

// Check the arguments and return type of the generateStaticParams function
if ('generateStaticParams' in entry) {
  checkFields<Diff<{ params: PageParams }, FirstArg<MaybeField<TEntry, 'generateStaticParams'>>, 'generateStaticParams'>>()
  checkFields<Diff<{ __tag__: 'generateStaticParams', __return_type__: any[] | Promise<any[]> }, { __tag__: 'generateStaticParams', __return_type__: ReturnType<MaybeField<TEntry, 'generateStaticParams'>> }>>()
}

type PageParams = any
export interface PageProps {
  params?: any
  searchParams?: any
}
export interface LayoutProps {
  children?: React.ReactNode

  params?: any
}

// =============
// Utility types
type RevalidateRange<T> = T extends { revalidate: any } ? NonNegative<T['revalidate']> : never

// If T is unknown or any, it will be an empty {} type. Otherwise, it will be the same as Omit<T, keyof Base>.
type OmitWithTag<T, K extends keyof any, _M> = Omit<T, K>
type Diff<Base, T extends Base, Message extends string = ''> = 0 extends (1 & T) ? {} : OmitWithTag<T, keyof Base, Message>

type FirstArg<T extends Function> = T extends (...args: [infer T, any]) => any ? unknown extends T ? any : T : never
type SecondArg<T extends Function> = T extends (...args: [any, infer T]) => any ? unknown extends T ? any : T : never
type MaybeField<T, K extends string> = T extends { [k in K]: infer G } ? G extends Function ? G : never : never

type ParamCheck<T> = {
  __tag__: string
  __param_position__: string
  __param_type__: T
}

function checkFields<_ extends { [k in keyof any]: never }>() {}

// https://github.com/sindresorhus/type-fest
type Numeric = number | bigint
type Zero = 0 | 0n
type Negative<T extends Numeric> = T extends Zero ? never : `${T}` extends `-${string}` ? T : never
type NonNegative<T extends Numeric> = T extends Zero ? T : Negative<T> extends never ? T : '__invalid_negative_number__'
