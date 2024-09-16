var require$$1 = require('path');
var require$$0 = require('fs');
var require$$0$1 = require('buffer');

function _interopDefaultLegacy (e) { return e && typeof e === 'object' && 'default' in e ? e : { 'default': e }; }

var require$$1__default = /*#__PURE__*/_interopDefaultLegacy(require$$1);
var require$$0__default = /*#__PURE__*/_interopDefaultLegacy(require$$0);
var require$$0__default$1 = /*#__PURE__*/_interopDefaultLegacy(require$$0$1);

var lib$3 = {};

var lib$2 = {};

function _createForOfIteratorHelperLoose$k(o, allowArrayLike) {
    var it = typeof Symbol !== "undefined" && o[Symbol.iterator] || o["@@iterator"];
    if (it) return (it = it.call(o)).next.bind(it);
    if (Array.isArray(o) || (it = unsupportedIterableToArray(o)) || allowArrayLike && o && typeof o.length === "number") {
        if (it) o = it;
        var i = 0;
        return function() {
            if (i >= o.length) return {
                done: true
            };
            return {
                done: false,
                value: o[i++]
            };
        };
    }
    throw new TypeError("Invalid attempt to iterate non-iterable instance.\\nIn order to be iterable, non-array objects must have a [Symbol.iterator]() method.");
}
Object.defineProperty(lib$2, "__esModule", {
    value: true
});
lib$2.declare = declare;
lib$2.declarePreset = void 0;
function declare(builder) {
    return function(api, options, dirname) {
        var _clonedApi2;
        var clonedApi;
        for(var _iterator = _createForOfIteratorHelperLoose$k(Object.keys(apiPolyfills)), _step; !(_step = _iterator()).done;){
            var name = _step.value;
            var _clonedApi;
            if (api[name]) continue;
            clonedApi = (_clonedApi = clonedApi) != null ? _clonedApi : copyApiObject(api);
            clonedApi[name] = apiPolyfills[name](clonedApi);
        }
        return builder((_clonedApi2 = clonedApi) != null ? _clonedApi2 : api, options || {}, dirname);
    };
}
var declarePreset = declare;
lib$2.declarePreset = declarePreset;
var apiPolyfills = {
    assertVersion: function(api) {
        return function(range) {
            throwVersionError(range, api.version);
        };
    },
    targets: function() {
        return function() {
            return {};
        };
    },
    assumption: function() {
        return function() {
            return undefined;
        };
    }
};
function copyApiObject(api) {
    var proto = null;
    if (typeof api.version === "string" && /^7\./.test(api.version)) {
        proto = Object.getPrototypeOf(api);
        if (proto && (!has$1(proto, "version") || !has$1(proto, "transform") || !has$1(proto, "template") || !has$1(proto, "types"))) {
            proto = null;
        }
    }
    return Object.assign({}, proto, api);
}
function has$1(obj, key) {
    return Object.prototype.hasOwnProperty.call(obj, key);
}
function throwVersionError(range, version) {
    if (typeof range === "number") {
        if (!Number.isInteger(range)) {
            throw new Error("Expected string or integer value.");
        }
        range = "^" + range + ".0.0-0";
    }
    if (typeof range !== "string") {
        throw new Error("Expected string or integer value.");
    }
    var limit = Error.stackTraceLimit;
    if (typeof limit === "number" && limit < 25) {
        Error.stackTraceLimit = 25;
    }
    var err;
    if (version.slice(0, 2) === "7.") {
        err = new Error('Requires Babel "^7.0.0-beta.41", but was loaded with "' + version + '". ' + "You'll need to update your @babel/core version.");
    } else {
        err = new Error('Requires Babel "' + range + '", but was loaded with "' + version + '". ' + "If you are sure you have a compatible version of @babel/core, " + "it is likely that something in your build process is loading the " + "wrong version. Inspect the stack trace of this error to look for " + 'the first entry that doesn\'t mention "@babel/core" or "babel-core" ' + "to see what is calling Babel.");
    }
    if (typeof limit === "number") {
        Error.stackTraceLimit = limit;
    }
    throw Object.assign(err, {
        code: "BABEL_VERSION_UNSUPPORTED",
        version: version,
        range: range
    });
}

Object.defineProperty(lib$3, "__esModule", {
    value: true
});
var default_1 = lib$3.default = void 0;
var _helperPluginUtils = lib$2;
var _default$5 = (0, _helperPluginUtils.declare)(function(api) {
    api.assertVersion(7);
    return {
        name: "syntax-jsx",
        manipulateOptions: function manipulateOptions(opts, parserOpts) {
            if (parserOpts.plugins.some(function(p) {
                return (Array.isArray(p) ? p[0] : p) === "typescript";
            })) {
                return;
            }
            parserOpts.plugins.push("jsx");
        }
    };
});
default_1 = lib$3.default = _default$5;

var lib$1 = {};

var isReactComponent$1 = {};

var buildMatchMemberExpression$1 = {};

var matchesPattern$1 = {};

var generated$4 = {};

var shallowEqual$1 = {};

function _createForOfIteratorHelperLoose$j(o, allowArrayLike) {
    var it = typeof Symbol !== "undefined" && o[Symbol.iterator] || o["@@iterator"];
    if (it) return (it = it.call(o)).next.bind(it);
    if (Array.isArray(o) || (it = unsupportedIterableToArray(o)) || allowArrayLike && o && typeof o.length === "number") {
        if (it) o = it;
        var i = 0;
        return function() {
            if (i >= o.length) return {
                done: true
            };
            return {
                done: false,
                value: o[i++]
            };
        };
    }
    throw new TypeError("Invalid attempt to iterate non-iterable instance.\\nIn order to be iterable, non-array objects must have a [Symbol.iterator]() method.");
}
Object.defineProperty(shallowEqual$1, "__esModule", {
    value: true
});
shallowEqual$1.default = shallowEqual;
function shallowEqual(actual, expected) {
    var keys = Object.keys(expected);
    for(var _iterator = _createForOfIteratorHelperLoose$j(keys), _step; !(_step = _iterator()).done;){
        var key = _step.value;
        if (actual[key] !== expected[key]) {
            return false;
        }
    }
    return true;
}

Object.defineProperty(generated$4, "__esModule", {
    value: true
});
generated$4.isArrayExpression = isArrayExpression;
generated$4.isAssignmentExpression = isAssignmentExpression;
generated$4.isBinaryExpression = isBinaryExpression;
generated$4.isInterpreterDirective = isInterpreterDirective;
generated$4.isDirective = isDirective;
generated$4.isDirectiveLiteral = isDirectiveLiteral;
generated$4.isBlockStatement = isBlockStatement;
generated$4.isBreakStatement = isBreakStatement;
generated$4.isCallExpression = isCallExpression;
generated$4.isCatchClause = isCatchClause;
generated$4.isConditionalExpression = isConditionalExpression;
generated$4.isContinueStatement = isContinueStatement;
generated$4.isDebuggerStatement = isDebuggerStatement;
generated$4.isDoWhileStatement = isDoWhileStatement;
generated$4.isEmptyStatement = isEmptyStatement;
generated$4.isExpressionStatement = isExpressionStatement;
generated$4.isFile = isFile;
generated$4.isForInStatement = isForInStatement;
generated$4.isForStatement = isForStatement;
generated$4.isFunctionDeclaration = isFunctionDeclaration;
generated$4.isFunctionExpression = isFunctionExpression;
generated$4.isIdentifier = isIdentifier;
generated$4.isIfStatement = isIfStatement;
generated$4.isLabeledStatement = isLabeledStatement;
generated$4.isStringLiteral = isStringLiteral;
generated$4.isNumericLiteral = isNumericLiteral;
generated$4.isNullLiteral = isNullLiteral;
generated$4.isBooleanLiteral = isBooleanLiteral;
generated$4.isRegExpLiteral = isRegExpLiteral;
generated$4.isLogicalExpression = isLogicalExpression;
generated$4.isMemberExpression = isMemberExpression;
generated$4.isNewExpression = isNewExpression;
generated$4.isProgram = isProgram;
generated$4.isObjectExpression = isObjectExpression;
generated$4.isObjectMethod = isObjectMethod;
generated$4.isObjectProperty = isObjectProperty;
generated$4.isRestElement = isRestElement;
generated$4.isReturnStatement = isReturnStatement;
generated$4.isSequenceExpression = isSequenceExpression;
generated$4.isParenthesizedExpression = isParenthesizedExpression;
generated$4.isSwitchCase = isSwitchCase;
generated$4.isSwitchStatement = isSwitchStatement;
generated$4.isThisExpression = isThisExpression;
generated$4.isThrowStatement = isThrowStatement;
generated$4.isTryStatement = isTryStatement;
generated$4.isUnaryExpression = isUnaryExpression;
generated$4.isUpdateExpression = isUpdateExpression;
generated$4.isVariableDeclaration = isVariableDeclaration;
generated$4.isVariableDeclarator = isVariableDeclarator;
generated$4.isWhileStatement = isWhileStatement;
generated$4.isWithStatement = isWithStatement;
generated$4.isAssignmentPattern = isAssignmentPattern;
generated$4.isArrayPattern = isArrayPattern;
generated$4.isArrowFunctionExpression = isArrowFunctionExpression;
generated$4.isClassBody = isClassBody;
generated$4.isClassExpression = isClassExpression;
generated$4.isClassDeclaration = isClassDeclaration;
generated$4.isExportAllDeclaration = isExportAllDeclaration;
generated$4.isExportDefaultDeclaration = isExportDefaultDeclaration;
generated$4.isExportNamedDeclaration = isExportNamedDeclaration;
generated$4.isExportSpecifier = isExportSpecifier;
generated$4.isForOfStatement = isForOfStatement;
generated$4.isImportDeclaration = isImportDeclaration;
generated$4.isImportDefaultSpecifier = isImportDefaultSpecifier;
generated$4.isImportNamespaceSpecifier = isImportNamespaceSpecifier;
generated$4.isImportSpecifier = isImportSpecifier;
generated$4.isMetaProperty = isMetaProperty;
generated$4.isClassMethod = isClassMethod;
generated$4.isObjectPattern = isObjectPattern;
generated$4.isSpreadElement = isSpreadElement;
generated$4.isSuper = isSuper;
generated$4.isTaggedTemplateExpression = isTaggedTemplateExpression;
generated$4.isTemplateElement = isTemplateElement;
generated$4.isTemplateLiteral = isTemplateLiteral;
generated$4.isYieldExpression = isYieldExpression;
generated$4.isAwaitExpression = isAwaitExpression;
generated$4.isImport = isImport;
generated$4.isBigIntLiteral = isBigIntLiteral;
generated$4.isExportNamespaceSpecifier = isExportNamespaceSpecifier;
generated$4.isOptionalMemberExpression = isOptionalMemberExpression;
generated$4.isOptionalCallExpression = isOptionalCallExpression;
generated$4.isClassProperty = isClassProperty;
generated$4.isClassPrivateProperty = isClassPrivateProperty;
generated$4.isClassPrivateMethod = isClassPrivateMethod;
generated$4.isPrivateName = isPrivateName;
generated$4.isAnyTypeAnnotation = isAnyTypeAnnotation;
generated$4.isArrayTypeAnnotation = isArrayTypeAnnotation;
generated$4.isBooleanTypeAnnotation = isBooleanTypeAnnotation;
generated$4.isBooleanLiteralTypeAnnotation = isBooleanLiteralTypeAnnotation;
generated$4.isNullLiteralTypeAnnotation = isNullLiteralTypeAnnotation;
generated$4.isClassImplements = isClassImplements;
generated$4.isDeclareClass = isDeclareClass;
generated$4.isDeclareFunction = isDeclareFunction;
generated$4.isDeclareInterface = isDeclareInterface;
generated$4.isDeclareModule = isDeclareModule;
generated$4.isDeclareModuleExports = isDeclareModuleExports;
generated$4.isDeclareTypeAlias = isDeclareTypeAlias;
generated$4.isDeclareOpaqueType = isDeclareOpaqueType;
generated$4.isDeclareVariable = isDeclareVariable;
generated$4.isDeclareExportDeclaration = isDeclareExportDeclaration;
generated$4.isDeclareExportAllDeclaration = isDeclareExportAllDeclaration;
generated$4.isDeclaredPredicate = isDeclaredPredicate;
generated$4.isExistsTypeAnnotation = isExistsTypeAnnotation;
generated$4.isFunctionTypeAnnotation = isFunctionTypeAnnotation;
generated$4.isFunctionTypeParam = isFunctionTypeParam;
generated$4.isGenericTypeAnnotation = isGenericTypeAnnotation;
generated$4.isInferredPredicate = isInferredPredicate;
generated$4.isInterfaceExtends = isInterfaceExtends;
generated$4.isInterfaceDeclaration = isInterfaceDeclaration;
generated$4.isInterfaceTypeAnnotation = isInterfaceTypeAnnotation;
generated$4.isIntersectionTypeAnnotation = isIntersectionTypeAnnotation;
generated$4.isMixedTypeAnnotation = isMixedTypeAnnotation;
generated$4.isEmptyTypeAnnotation = isEmptyTypeAnnotation;
generated$4.isNullableTypeAnnotation = isNullableTypeAnnotation;
generated$4.isNumberLiteralTypeAnnotation = isNumberLiteralTypeAnnotation;
generated$4.isNumberTypeAnnotation = isNumberTypeAnnotation;
generated$4.isObjectTypeAnnotation = isObjectTypeAnnotation;
generated$4.isObjectTypeInternalSlot = isObjectTypeInternalSlot;
generated$4.isObjectTypeCallProperty = isObjectTypeCallProperty;
generated$4.isObjectTypeIndexer = isObjectTypeIndexer;
generated$4.isObjectTypeProperty = isObjectTypeProperty;
generated$4.isObjectTypeSpreadProperty = isObjectTypeSpreadProperty;
generated$4.isOpaqueType = isOpaqueType;
generated$4.isQualifiedTypeIdentifier = isQualifiedTypeIdentifier;
generated$4.isStringLiteralTypeAnnotation = isStringLiteralTypeAnnotation;
generated$4.isStringTypeAnnotation = isStringTypeAnnotation;
generated$4.isSymbolTypeAnnotation = isSymbolTypeAnnotation;
generated$4.isThisTypeAnnotation = isThisTypeAnnotation;
generated$4.isTupleTypeAnnotation = isTupleTypeAnnotation;
generated$4.isTypeofTypeAnnotation = isTypeofTypeAnnotation;
generated$4.isTypeAlias = isTypeAlias;
generated$4.isTypeAnnotation = isTypeAnnotation;
generated$4.isTypeCastExpression = isTypeCastExpression;
generated$4.isTypeParameter = isTypeParameter;
generated$4.isTypeParameterDeclaration = isTypeParameterDeclaration;
generated$4.isTypeParameterInstantiation = isTypeParameterInstantiation;
generated$4.isUnionTypeAnnotation = isUnionTypeAnnotation;
generated$4.isVariance = isVariance;
generated$4.isVoidTypeAnnotation = isVoidTypeAnnotation;
generated$4.isEnumDeclaration = isEnumDeclaration;
generated$4.isEnumBooleanBody = isEnumBooleanBody;
generated$4.isEnumNumberBody = isEnumNumberBody;
generated$4.isEnumStringBody = isEnumStringBody;
generated$4.isEnumSymbolBody = isEnumSymbolBody;
generated$4.isEnumBooleanMember = isEnumBooleanMember;
generated$4.isEnumNumberMember = isEnumNumberMember;
generated$4.isEnumStringMember = isEnumStringMember;
generated$4.isEnumDefaultedMember = isEnumDefaultedMember;
generated$4.isIndexedAccessType = isIndexedAccessType;
generated$4.isOptionalIndexedAccessType = isOptionalIndexedAccessType;
generated$4.isJSXAttribute = isJSXAttribute;
generated$4.isJSXClosingElement = isJSXClosingElement;
generated$4.isJSXElement = isJSXElement;
generated$4.isJSXEmptyExpression = isJSXEmptyExpression;
generated$4.isJSXExpressionContainer = isJSXExpressionContainer;
generated$4.isJSXSpreadChild = isJSXSpreadChild;
generated$4.isJSXIdentifier = isJSXIdentifier;
generated$4.isJSXMemberExpression = isJSXMemberExpression;
generated$4.isJSXNamespacedName = isJSXNamespacedName;
generated$4.isJSXOpeningElement = isJSXOpeningElement;
generated$4.isJSXSpreadAttribute = isJSXSpreadAttribute;
generated$4.isJSXText = isJSXText;
generated$4.isJSXFragment = isJSXFragment;
generated$4.isJSXOpeningFragment = isJSXOpeningFragment;
generated$4.isJSXClosingFragment = isJSXClosingFragment;
generated$4.isNoop = isNoop;
generated$4.isPlaceholder = isPlaceholder;
generated$4.isV8IntrinsicIdentifier = isV8IntrinsicIdentifier;
generated$4.isArgumentPlaceholder = isArgumentPlaceholder;
generated$4.isBindExpression = isBindExpression;
generated$4.isImportAttribute = isImportAttribute;
generated$4.isDecorator = isDecorator;
generated$4.isDoExpression = isDoExpression;
generated$4.isExportDefaultSpecifier = isExportDefaultSpecifier;
generated$4.isRecordExpression = isRecordExpression;
generated$4.isTupleExpression = isTupleExpression;
generated$4.isDecimalLiteral = isDecimalLiteral;
generated$4.isStaticBlock = isStaticBlock;
generated$4.isModuleExpression = isModuleExpression;
generated$4.isTopicReference = isTopicReference;
generated$4.isPipelineTopicExpression = isPipelineTopicExpression;
generated$4.isPipelineBareFunction = isPipelineBareFunction;
generated$4.isPipelinePrimaryTopicReference = isPipelinePrimaryTopicReference;
generated$4.isTSParameterProperty = isTSParameterProperty;
generated$4.isTSDeclareFunction = isTSDeclareFunction;
generated$4.isTSDeclareMethod = isTSDeclareMethod;
generated$4.isTSQualifiedName = isTSQualifiedName;
generated$4.isTSCallSignatureDeclaration = isTSCallSignatureDeclaration;
generated$4.isTSConstructSignatureDeclaration = isTSConstructSignatureDeclaration;
generated$4.isTSPropertySignature = isTSPropertySignature;
generated$4.isTSMethodSignature = isTSMethodSignature;
generated$4.isTSIndexSignature = isTSIndexSignature;
generated$4.isTSAnyKeyword = isTSAnyKeyword;
generated$4.isTSBooleanKeyword = isTSBooleanKeyword;
generated$4.isTSBigIntKeyword = isTSBigIntKeyword;
generated$4.isTSIntrinsicKeyword = isTSIntrinsicKeyword;
generated$4.isTSNeverKeyword = isTSNeverKeyword;
generated$4.isTSNullKeyword = isTSNullKeyword;
generated$4.isTSNumberKeyword = isTSNumberKeyword;
generated$4.isTSObjectKeyword = isTSObjectKeyword;
generated$4.isTSStringKeyword = isTSStringKeyword;
generated$4.isTSSymbolKeyword = isTSSymbolKeyword;
generated$4.isTSUndefinedKeyword = isTSUndefinedKeyword;
generated$4.isTSUnknownKeyword = isTSUnknownKeyword;
generated$4.isTSVoidKeyword = isTSVoidKeyword;
generated$4.isTSThisType = isTSThisType;
generated$4.isTSFunctionType = isTSFunctionType;
generated$4.isTSConstructorType = isTSConstructorType;
generated$4.isTSTypeReference = isTSTypeReference;
generated$4.isTSTypePredicate = isTSTypePredicate;
generated$4.isTSTypeQuery = isTSTypeQuery;
generated$4.isTSTypeLiteral = isTSTypeLiteral;
generated$4.isTSArrayType = isTSArrayType;
generated$4.isTSTupleType = isTSTupleType;
generated$4.isTSOptionalType = isTSOptionalType;
generated$4.isTSRestType = isTSRestType;
generated$4.isTSNamedTupleMember = isTSNamedTupleMember;
generated$4.isTSUnionType = isTSUnionType;
generated$4.isTSIntersectionType = isTSIntersectionType;
generated$4.isTSConditionalType = isTSConditionalType;
generated$4.isTSInferType = isTSInferType;
generated$4.isTSParenthesizedType = isTSParenthesizedType;
generated$4.isTSTypeOperator = isTSTypeOperator;
generated$4.isTSIndexedAccessType = isTSIndexedAccessType;
generated$4.isTSMappedType = isTSMappedType;
generated$4.isTSLiteralType = isTSLiteralType;
generated$4.isTSExpressionWithTypeArguments = isTSExpressionWithTypeArguments;
generated$4.isTSInterfaceDeclaration = isTSInterfaceDeclaration;
generated$4.isTSInterfaceBody = isTSInterfaceBody;
generated$4.isTSTypeAliasDeclaration = isTSTypeAliasDeclaration;
generated$4.isTSAsExpression = isTSAsExpression;
generated$4.isTSTypeAssertion = isTSTypeAssertion;
generated$4.isTSEnumDeclaration = isTSEnumDeclaration;
generated$4.isTSEnumMember = isTSEnumMember;
generated$4.isTSModuleDeclaration = isTSModuleDeclaration;
generated$4.isTSModuleBlock = isTSModuleBlock;
generated$4.isTSImportType = isTSImportType;
generated$4.isTSImportEqualsDeclaration = isTSImportEqualsDeclaration;
generated$4.isTSExternalModuleReference = isTSExternalModuleReference;
generated$4.isTSNonNullExpression = isTSNonNullExpression;
generated$4.isTSExportAssignment = isTSExportAssignment;
generated$4.isTSNamespaceExportDeclaration = isTSNamespaceExportDeclaration;
generated$4.isTSTypeAnnotation = isTSTypeAnnotation;
generated$4.isTSTypeParameterInstantiation = isTSTypeParameterInstantiation;
generated$4.isTSTypeParameterDeclaration = isTSTypeParameterDeclaration;
generated$4.isTSTypeParameter = isTSTypeParameter;
generated$4.isExpression = isExpression;
generated$4.isBinary = isBinary;
generated$4.isScopable = isScopable;
generated$4.isBlockParent = isBlockParent;
generated$4.isBlock = isBlock;
generated$4.isStatement = isStatement;
generated$4.isTerminatorless = isTerminatorless;
generated$4.isCompletionStatement = isCompletionStatement;
generated$4.isConditional = isConditional;
generated$4.isLoop = isLoop;
generated$4.isWhile = isWhile;
generated$4.isExpressionWrapper = isExpressionWrapper;
generated$4.isFor = isFor;
generated$4.isForXStatement = isForXStatement;
generated$4.isFunction = isFunction;
generated$4.isFunctionParent = isFunctionParent;
generated$4.isPureish = isPureish;
generated$4.isDeclaration = isDeclaration;
generated$4.isPatternLike = isPatternLike;
generated$4.isLVal = isLVal;
generated$4.isTSEntityName = isTSEntityName;
generated$4.isLiteral = isLiteral;
generated$4.isImmutable = isImmutable$2;
generated$4.isUserWhitespacable = isUserWhitespacable;
generated$4.isMethod = isMethod;
generated$4.isObjectMember = isObjectMember;
generated$4.isProperty = isProperty;
generated$4.isUnaryLike = isUnaryLike;
generated$4.isPattern = isPattern;
generated$4.isClass = isClass;
generated$4.isModuleDeclaration = isModuleDeclaration;
generated$4.isExportDeclaration = isExportDeclaration;
generated$4.isModuleSpecifier = isModuleSpecifier;
generated$4.isPrivate = isPrivate;
generated$4.isFlow = isFlow;
generated$4.isFlowType = isFlowType;
generated$4.isFlowBaseAnnotation = isFlowBaseAnnotation;
generated$4.isFlowDeclaration = isFlowDeclaration;
generated$4.isFlowPredicate = isFlowPredicate;
generated$4.isEnumBody = isEnumBody;
generated$4.isEnumMember = isEnumMember;
generated$4.isJSX = isJSX;
generated$4.isTSTypeElement = isTSTypeElement;
generated$4.isTSType = isTSType;
generated$4.isTSBaseType = isTSBaseType;
generated$4.isNumberLiteral = isNumberLiteral;
generated$4.isRegexLiteral = isRegexLiteral;
generated$4.isRestProperty = isRestProperty;
generated$4.isSpreadProperty = isSpreadProperty;
var _shallowEqual = shallowEqual$1;
function isArrayExpression(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "ArrayExpression") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isAssignmentExpression(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "AssignmentExpression") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isBinaryExpression(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "BinaryExpression") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isInterpreterDirective(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "InterpreterDirective") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isDirective(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "Directive") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isDirectiveLiteral(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "DirectiveLiteral") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isBlockStatement(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "BlockStatement") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isBreakStatement(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "BreakStatement") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isCallExpression(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "CallExpression") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isCatchClause(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "CatchClause") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isConditionalExpression(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "ConditionalExpression") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isContinueStatement(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "ContinueStatement") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isDebuggerStatement(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "DebuggerStatement") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isDoWhileStatement(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "DoWhileStatement") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isEmptyStatement(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "EmptyStatement") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isExpressionStatement(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "ExpressionStatement") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isFile(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "File") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isForInStatement(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "ForInStatement") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isForStatement(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "ForStatement") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isFunctionDeclaration(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "FunctionDeclaration") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isFunctionExpression(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "FunctionExpression") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isIdentifier(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "Identifier") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isIfStatement(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "IfStatement") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isLabeledStatement(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "LabeledStatement") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isStringLiteral(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "StringLiteral") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isNumericLiteral(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "NumericLiteral") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isNullLiteral(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "NullLiteral") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isBooleanLiteral(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "BooleanLiteral") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isRegExpLiteral(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "RegExpLiteral") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isLogicalExpression(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "LogicalExpression") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isMemberExpression(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "MemberExpression") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isNewExpression(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "NewExpression") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isProgram(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "Program") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isObjectExpression(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "ObjectExpression") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isObjectMethod(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "ObjectMethod") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isObjectProperty(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "ObjectProperty") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isRestElement(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "RestElement") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isReturnStatement(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "ReturnStatement") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isSequenceExpression(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "SequenceExpression") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isParenthesizedExpression(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "ParenthesizedExpression") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isSwitchCase(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "SwitchCase") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isSwitchStatement(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "SwitchStatement") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isThisExpression(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "ThisExpression") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isThrowStatement(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "ThrowStatement") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isTryStatement(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "TryStatement") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isUnaryExpression(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "UnaryExpression") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isUpdateExpression(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "UpdateExpression") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isVariableDeclaration(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "VariableDeclaration") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isVariableDeclarator(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "VariableDeclarator") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isWhileStatement(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "WhileStatement") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isWithStatement(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "WithStatement") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isAssignmentPattern(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "AssignmentPattern") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isArrayPattern(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "ArrayPattern") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isArrowFunctionExpression(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "ArrowFunctionExpression") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isClassBody(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "ClassBody") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isClassExpression(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "ClassExpression") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isClassDeclaration(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "ClassDeclaration") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isExportAllDeclaration(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "ExportAllDeclaration") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isExportDefaultDeclaration(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "ExportDefaultDeclaration") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isExportNamedDeclaration(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "ExportNamedDeclaration") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isExportSpecifier(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "ExportSpecifier") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isForOfStatement(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "ForOfStatement") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isImportDeclaration(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "ImportDeclaration") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isImportDefaultSpecifier(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "ImportDefaultSpecifier") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isImportNamespaceSpecifier(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "ImportNamespaceSpecifier") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isImportSpecifier(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "ImportSpecifier") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isMetaProperty(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "MetaProperty") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isClassMethod(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "ClassMethod") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isObjectPattern(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "ObjectPattern") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isSpreadElement(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "SpreadElement") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isSuper(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "Super") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isTaggedTemplateExpression(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "TaggedTemplateExpression") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isTemplateElement(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "TemplateElement") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isTemplateLiteral(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "TemplateLiteral") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isYieldExpression(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "YieldExpression") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isAwaitExpression(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "AwaitExpression") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isImport(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "Import") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isBigIntLiteral(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "BigIntLiteral") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isExportNamespaceSpecifier(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "ExportNamespaceSpecifier") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isOptionalMemberExpression(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "OptionalMemberExpression") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isOptionalCallExpression(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "OptionalCallExpression") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isClassProperty(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "ClassProperty") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isClassPrivateProperty(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "ClassPrivateProperty") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isClassPrivateMethod(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "ClassPrivateMethod") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isPrivateName(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "PrivateName") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isAnyTypeAnnotation(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "AnyTypeAnnotation") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isArrayTypeAnnotation(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "ArrayTypeAnnotation") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isBooleanTypeAnnotation(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "BooleanTypeAnnotation") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isBooleanLiteralTypeAnnotation(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "BooleanLiteralTypeAnnotation") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isNullLiteralTypeAnnotation(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "NullLiteralTypeAnnotation") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isClassImplements(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "ClassImplements") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isDeclareClass(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "DeclareClass") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isDeclareFunction(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "DeclareFunction") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isDeclareInterface(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "DeclareInterface") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isDeclareModule(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "DeclareModule") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isDeclareModuleExports(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "DeclareModuleExports") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isDeclareTypeAlias(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "DeclareTypeAlias") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isDeclareOpaqueType(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "DeclareOpaqueType") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isDeclareVariable(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "DeclareVariable") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isDeclareExportDeclaration(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "DeclareExportDeclaration") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isDeclareExportAllDeclaration(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "DeclareExportAllDeclaration") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isDeclaredPredicate(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "DeclaredPredicate") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isExistsTypeAnnotation(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "ExistsTypeAnnotation") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isFunctionTypeAnnotation(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "FunctionTypeAnnotation") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isFunctionTypeParam(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "FunctionTypeParam") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isGenericTypeAnnotation(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "GenericTypeAnnotation") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isInferredPredicate(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "InferredPredicate") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isInterfaceExtends(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "InterfaceExtends") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isInterfaceDeclaration(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "InterfaceDeclaration") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isInterfaceTypeAnnotation(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "InterfaceTypeAnnotation") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isIntersectionTypeAnnotation(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "IntersectionTypeAnnotation") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isMixedTypeAnnotation(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "MixedTypeAnnotation") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isEmptyTypeAnnotation(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "EmptyTypeAnnotation") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isNullableTypeAnnotation(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "NullableTypeAnnotation") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isNumberLiteralTypeAnnotation(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "NumberLiteralTypeAnnotation") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isNumberTypeAnnotation(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "NumberTypeAnnotation") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isObjectTypeAnnotation(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "ObjectTypeAnnotation") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isObjectTypeInternalSlot(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "ObjectTypeInternalSlot") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isObjectTypeCallProperty(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "ObjectTypeCallProperty") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isObjectTypeIndexer(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "ObjectTypeIndexer") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isObjectTypeProperty(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "ObjectTypeProperty") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isObjectTypeSpreadProperty(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "ObjectTypeSpreadProperty") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isOpaqueType(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "OpaqueType") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isQualifiedTypeIdentifier(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "QualifiedTypeIdentifier") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isStringLiteralTypeAnnotation(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "StringLiteralTypeAnnotation") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isStringTypeAnnotation(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "StringTypeAnnotation") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isSymbolTypeAnnotation(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "SymbolTypeAnnotation") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isThisTypeAnnotation(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "ThisTypeAnnotation") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isTupleTypeAnnotation(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "TupleTypeAnnotation") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isTypeofTypeAnnotation(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "TypeofTypeAnnotation") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isTypeAlias(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "TypeAlias") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isTypeAnnotation(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "TypeAnnotation") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isTypeCastExpression(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "TypeCastExpression") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isTypeParameter(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "TypeParameter") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isTypeParameterDeclaration(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "TypeParameterDeclaration") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isTypeParameterInstantiation(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "TypeParameterInstantiation") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isUnionTypeAnnotation(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "UnionTypeAnnotation") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isVariance(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "Variance") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isVoidTypeAnnotation(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "VoidTypeAnnotation") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isEnumDeclaration(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "EnumDeclaration") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isEnumBooleanBody(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "EnumBooleanBody") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isEnumNumberBody(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "EnumNumberBody") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isEnumStringBody(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "EnumStringBody") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isEnumSymbolBody(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "EnumSymbolBody") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isEnumBooleanMember(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "EnumBooleanMember") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isEnumNumberMember(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "EnumNumberMember") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isEnumStringMember(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "EnumStringMember") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isEnumDefaultedMember(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "EnumDefaultedMember") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isIndexedAccessType(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "IndexedAccessType") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isOptionalIndexedAccessType(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "OptionalIndexedAccessType") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isJSXAttribute(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "JSXAttribute") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isJSXClosingElement(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "JSXClosingElement") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isJSXElement(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "JSXElement") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isJSXEmptyExpression(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "JSXEmptyExpression") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isJSXExpressionContainer(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "JSXExpressionContainer") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isJSXSpreadChild(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "JSXSpreadChild") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isJSXIdentifier(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "JSXIdentifier") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isJSXMemberExpression(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "JSXMemberExpression") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isJSXNamespacedName(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "JSXNamespacedName") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isJSXOpeningElement(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "JSXOpeningElement") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isJSXSpreadAttribute(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "JSXSpreadAttribute") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isJSXText(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "JSXText") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isJSXFragment(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "JSXFragment") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isJSXOpeningFragment(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "JSXOpeningFragment") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isJSXClosingFragment(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "JSXClosingFragment") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isNoop(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "Noop") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isPlaceholder(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "Placeholder") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isV8IntrinsicIdentifier(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "V8IntrinsicIdentifier") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isArgumentPlaceholder(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "ArgumentPlaceholder") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isBindExpression(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "BindExpression") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isImportAttribute(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "ImportAttribute") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isDecorator(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "Decorator") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isDoExpression(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "DoExpression") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isExportDefaultSpecifier(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "ExportDefaultSpecifier") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isRecordExpression(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "RecordExpression") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isTupleExpression(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "TupleExpression") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isDecimalLiteral(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "DecimalLiteral") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isStaticBlock(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "StaticBlock") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isModuleExpression(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "ModuleExpression") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isTopicReference(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "TopicReference") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isPipelineTopicExpression(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "PipelineTopicExpression") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isPipelineBareFunction(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "PipelineBareFunction") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isPipelinePrimaryTopicReference(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "PipelinePrimaryTopicReference") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isTSParameterProperty(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "TSParameterProperty") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isTSDeclareFunction(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "TSDeclareFunction") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isTSDeclareMethod(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "TSDeclareMethod") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isTSQualifiedName(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "TSQualifiedName") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isTSCallSignatureDeclaration(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "TSCallSignatureDeclaration") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isTSConstructSignatureDeclaration(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "TSConstructSignatureDeclaration") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isTSPropertySignature(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "TSPropertySignature") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isTSMethodSignature(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "TSMethodSignature") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isTSIndexSignature(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "TSIndexSignature") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isTSAnyKeyword(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "TSAnyKeyword") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isTSBooleanKeyword(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "TSBooleanKeyword") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isTSBigIntKeyword(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "TSBigIntKeyword") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isTSIntrinsicKeyword(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "TSIntrinsicKeyword") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isTSNeverKeyword(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "TSNeverKeyword") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isTSNullKeyword(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "TSNullKeyword") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isTSNumberKeyword(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "TSNumberKeyword") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isTSObjectKeyword(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "TSObjectKeyword") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isTSStringKeyword(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "TSStringKeyword") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isTSSymbolKeyword(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "TSSymbolKeyword") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isTSUndefinedKeyword(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "TSUndefinedKeyword") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isTSUnknownKeyword(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "TSUnknownKeyword") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isTSVoidKeyword(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "TSVoidKeyword") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isTSThisType(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "TSThisType") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isTSFunctionType(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "TSFunctionType") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isTSConstructorType(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "TSConstructorType") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isTSTypeReference(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "TSTypeReference") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isTSTypePredicate(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "TSTypePredicate") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isTSTypeQuery(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "TSTypeQuery") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isTSTypeLiteral(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "TSTypeLiteral") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isTSArrayType(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "TSArrayType") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isTSTupleType(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "TSTupleType") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isTSOptionalType(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "TSOptionalType") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isTSRestType(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "TSRestType") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isTSNamedTupleMember(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "TSNamedTupleMember") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isTSUnionType(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "TSUnionType") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isTSIntersectionType(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "TSIntersectionType") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isTSConditionalType(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "TSConditionalType") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isTSInferType(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "TSInferType") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isTSParenthesizedType(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "TSParenthesizedType") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isTSTypeOperator(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "TSTypeOperator") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isTSIndexedAccessType(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "TSIndexedAccessType") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isTSMappedType(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "TSMappedType") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isTSLiteralType(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "TSLiteralType") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isTSExpressionWithTypeArguments(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "TSExpressionWithTypeArguments") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isTSInterfaceDeclaration(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "TSInterfaceDeclaration") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isTSInterfaceBody(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "TSInterfaceBody") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isTSTypeAliasDeclaration(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "TSTypeAliasDeclaration") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isTSAsExpression(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "TSAsExpression") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isTSTypeAssertion(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "TSTypeAssertion") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isTSEnumDeclaration(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "TSEnumDeclaration") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isTSEnumMember(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "TSEnumMember") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isTSModuleDeclaration(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "TSModuleDeclaration") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isTSModuleBlock(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "TSModuleBlock") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isTSImportType(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "TSImportType") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isTSImportEqualsDeclaration(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "TSImportEqualsDeclaration") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isTSExternalModuleReference(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "TSExternalModuleReference") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isTSNonNullExpression(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "TSNonNullExpression") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isTSExportAssignment(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "TSExportAssignment") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isTSNamespaceExportDeclaration(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "TSNamespaceExportDeclaration") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isTSTypeAnnotation(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "TSTypeAnnotation") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isTSTypeParameterInstantiation(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "TSTypeParameterInstantiation") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isTSTypeParameterDeclaration(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "TSTypeParameterDeclaration") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isTSTypeParameter(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "TSTypeParameter") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isExpression(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if ("ArrayExpression" === nodeType || "AssignmentExpression" === nodeType || "BinaryExpression" === nodeType || "CallExpression" === nodeType || "ConditionalExpression" === nodeType || "FunctionExpression" === nodeType || "Identifier" === nodeType || "StringLiteral" === nodeType || "NumericLiteral" === nodeType || "NullLiteral" === nodeType || "BooleanLiteral" === nodeType || "RegExpLiteral" === nodeType || "LogicalExpression" === nodeType || "MemberExpression" === nodeType || "NewExpression" === nodeType || "ObjectExpression" === nodeType || "SequenceExpression" === nodeType || "ParenthesizedExpression" === nodeType || "ThisExpression" === nodeType || "UnaryExpression" === nodeType || "UpdateExpression" === nodeType || "ArrowFunctionExpression" === nodeType || "ClassExpression" === nodeType || "MetaProperty" === nodeType || "Super" === nodeType || "TaggedTemplateExpression" === nodeType || "TemplateLiteral" === nodeType || "YieldExpression" === nodeType || "AwaitExpression" === nodeType || "Import" === nodeType || "BigIntLiteral" === nodeType || "OptionalMemberExpression" === nodeType || "OptionalCallExpression" === nodeType || "TypeCastExpression" === nodeType || "JSXElement" === nodeType || "JSXFragment" === nodeType || "BindExpression" === nodeType || "DoExpression" === nodeType || "RecordExpression" === nodeType || "TupleExpression" === nodeType || "DecimalLiteral" === nodeType || "ModuleExpression" === nodeType || "TopicReference" === nodeType || "PipelineTopicExpression" === nodeType || "PipelineBareFunction" === nodeType || "PipelinePrimaryTopicReference" === nodeType || "TSAsExpression" === nodeType || "TSTypeAssertion" === nodeType || "TSNonNullExpression" === nodeType || nodeType === "Placeholder" && ("Expression" === node.expectedNode || "Identifier" === node.expectedNode || "StringLiteral" === node.expectedNode)) {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isBinary(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if ("BinaryExpression" === nodeType || "LogicalExpression" === nodeType) {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isScopable(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if ("BlockStatement" === nodeType || "CatchClause" === nodeType || "DoWhileStatement" === nodeType || "ForInStatement" === nodeType || "ForStatement" === nodeType || "FunctionDeclaration" === nodeType || "FunctionExpression" === nodeType || "Program" === nodeType || "ObjectMethod" === nodeType || "SwitchStatement" === nodeType || "WhileStatement" === nodeType || "ArrowFunctionExpression" === nodeType || "ClassExpression" === nodeType || "ClassDeclaration" === nodeType || "ForOfStatement" === nodeType || "ClassMethod" === nodeType || "ClassPrivateMethod" === nodeType || "StaticBlock" === nodeType || "TSModuleBlock" === nodeType || nodeType === "Placeholder" && "BlockStatement" === node.expectedNode) {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isBlockParent(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if ("BlockStatement" === nodeType || "CatchClause" === nodeType || "DoWhileStatement" === nodeType || "ForInStatement" === nodeType || "ForStatement" === nodeType || "FunctionDeclaration" === nodeType || "FunctionExpression" === nodeType || "Program" === nodeType || "ObjectMethod" === nodeType || "SwitchStatement" === nodeType || "WhileStatement" === nodeType || "ArrowFunctionExpression" === nodeType || "ForOfStatement" === nodeType || "ClassMethod" === nodeType || "ClassPrivateMethod" === nodeType || "StaticBlock" === nodeType || "TSModuleBlock" === nodeType || nodeType === "Placeholder" && "BlockStatement" === node.expectedNode) {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isBlock(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if ("BlockStatement" === nodeType || "Program" === nodeType || "TSModuleBlock" === nodeType || nodeType === "Placeholder" && "BlockStatement" === node.expectedNode) {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isStatement(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if ("BlockStatement" === nodeType || "BreakStatement" === nodeType || "ContinueStatement" === nodeType || "DebuggerStatement" === nodeType || "DoWhileStatement" === nodeType || "EmptyStatement" === nodeType || "ExpressionStatement" === nodeType || "ForInStatement" === nodeType || "ForStatement" === nodeType || "FunctionDeclaration" === nodeType || "IfStatement" === nodeType || "LabeledStatement" === nodeType || "ReturnStatement" === nodeType || "SwitchStatement" === nodeType || "ThrowStatement" === nodeType || "TryStatement" === nodeType || "VariableDeclaration" === nodeType || "WhileStatement" === nodeType || "WithStatement" === nodeType || "ClassDeclaration" === nodeType || "ExportAllDeclaration" === nodeType || "ExportDefaultDeclaration" === nodeType || "ExportNamedDeclaration" === nodeType || "ForOfStatement" === nodeType || "ImportDeclaration" === nodeType || "DeclareClass" === nodeType || "DeclareFunction" === nodeType || "DeclareInterface" === nodeType || "DeclareModule" === nodeType || "DeclareModuleExports" === nodeType || "DeclareTypeAlias" === nodeType || "DeclareOpaqueType" === nodeType || "DeclareVariable" === nodeType || "DeclareExportDeclaration" === nodeType || "DeclareExportAllDeclaration" === nodeType || "InterfaceDeclaration" === nodeType || "OpaqueType" === nodeType || "TypeAlias" === nodeType || "EnumDeclaration" === nodeType || "TSDeclareFunction" === nodeType || "TSInterfaceDeclaration" === nodeType || "TSTypeAliasDeclaration" === nodeType || "TSEnumDeclaration" === nodeType || "TSModuleDeclaration" === nodeType || "TSImportEqualsDeclaration" === nodeType || "TSExportAssignment" === nodeType || "TSNamespaceExportDeclaration" === nodeType || nodeType === "Placeholder" && ("Statement" === node.expectedNode || "Declaration" === node.expectedNode || "BlockStatement" === node.expectedNode)) {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isTerminatorless(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if ("BreakStatement" === nodeType || "ContinueStatement" === nodeType || "ReturnStatement" === nodeType || "ThrowStatement" === nodeType || "YieldExpression" === nodeType || "AwaitExpression" === nodeType) {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isCompletionStatement(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if ("BreakStatement" === nodeType || "ContinueStatement" === nodeType || "ReturnStatement" === nodeType || "ThrowStatement" === nodeType) {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isConditional(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if ("ConditionalExpression" === nodeType || "IfStatement" === nodeType) {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isLoop(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if ("DoWhileStatement" === nodeType || "ForInStatement" === nodeType || "ForStatement" === nodeType || "WhileStatement" === nodeType || "ForOfStatement" === nodeType) {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isWhile(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if ("DoWhileStatement" === nodeType || "WhileStatement" === nodeType) {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isExpressionWrapper(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if ("ExpressionStatement" === nodeType || "ParenthesizedExpression" === nodeType || "TypeCastExpression" === nodeType) {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isFor(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if ("ForInStatement" === nodeType || "ForStatement" === nodeType || "ForOfStatement" === nodeType) {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isForXStatement(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if ("ForInStatement" === nodeType || "ForOfStatement" === nodeType) {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isFunction(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if ("FunctionDeclaration" === nodeType || "FunctionExpression" === nodeType || "ObjectMethod" === nodeType || "ArrowFunctionExpression" === nodeType || "ClassMethod" === nodeType || "ClassPrivateMethod" === nodeType) {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isFunctionParent(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if ("FunctionDeclaration" === nodeType || "FunctionExpression" === nodeType || "ObjectMethod" === nodeType || "ArrowFunctionExpression" === nodeType || "ClassMethod" === nodeType || "ClassPrivateMethod" === nodeType) {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isPureish(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if ("FunctionDeclaration" === nodeType || "FunctionExpression" === nodeType || "StringLiteral" === nodeType || "NumericLiteral" === nodeType || "NullLiteral" === nodeType || "BooleanLiteral" === nodeType || "RegExpLiteral" === nodeType || "ArrowFunctionExpression" === nodeType || "BigIntLiteral" === nodeType || "DecimalLiteral" === nodeType || nodeType === "Placeholder" && "StringLiteral" === node.expectedNode) {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isDeclaration(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if ("FunctionDeclaration" === nodeType || "VariableDeclaration" === nodeType || "ClassDeclaration" === nodeType || "ExportAllDeclaration" === nodeType || "ExportDefaultDeclaration" === nodeType || "ExportNamedDeclaration" === nodeType || "ImportDeclaration" === nodeType || "DeclareClass" === nodeType || "DeclareFunction" === nodeType || "DeclareInterface" === nodeType || "DeclareModule" === nodeType || "DeclareModuleExports" === nodeType || "DeclareTypeAlias" === nodeType || "DeclareOpaqueType" === nodeType || "DeclareVariable" === nodeType || "DeclareExportDeclaration" === nodeType || "DeclareExportAllDeclaration" === nodeType || "InterfaceDeclaration" === nodeType || "OpaqueType" === nodeType || "TypeAlias" === nodeType || "EnumDeclaration" === nodeType || "TSDeclareFunction" === nodeType || "TSInterfaceDeclaration" === nodeType || "TSTypeAliasDeclaration" === nodeType || "TSEnumDeclaration" === nodeType || "TSModuleDeclaration" === nodeType || nodeType === "Placeholder" && "Declaration" === node.expectedNode) {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isPatternLike(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if ("Identifier" === nodeType || "RestElement" === nodeType || "AssignmentPattern" === nodeType || "ArrayPattern" === nodeType || "ObjectPattern" === nodeType || nodeType === "Placeholder" && ("Pattern" === node.expectedNode || "Identifier" === node.expectedNode)) {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isLVal(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if ("Identifier" === nodeType || "MemberExpression" === nodeType || "RestElement" === nodeType || "AssignmentPattern" === nodeType || "ArrayPattern" === nodeType || "ObjectPattern" === nodeType || "TSParameterProperty" === nodeType || nodeType === "Placeholder" && ("Pattern" === node.expectedNode || "Identifier" === node.expectedNode)) {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isTSEntityName(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if ("Identifier" === nodeType || "TSQualifiedName" === nodeType || nodeType === "Placeholder" && "Identifier" === node.expectedNode) {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isLiteral(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if ("StringLiteral" === nodeType || "NumericLiteral" === nodeType || "NullLiteral" === nodeType || "BooleanLiteral" === nodeType || "RegExpLiteral" === nodeType || "TemplateLiteral" === nodeType || "BigIntLiteral" === nodeType || "DecimalLiteral" === nodeType || nodeType === "Placeholder" && "StringLiteral" === node.expectedNode) {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isImmutable$2(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if ("StringLiteral" === nodeType || "NumericLiteral" === nodeType || "NullLiteral" === nodeType || "BooleanLiteral" === nodeType || "BigIntLiteral" === nodeType || "JSXAttribute" === nodeType || "JSXClosingElement" === nodeType || "JSXElement" === nodeType || "JSXExpressionContainer" === nodeType || "JSXSpreadChild" === nodeType || "JSXOpeningElement" === nodeType || "JSXText" === nodeType || "JSXFragment" === nodeType || "JSXOpeningFragment" === nodeType || "JSXClosingFragment" === nodeType || "DecimalLiteral" === nodeType || nodeType === "Placeholder" && "StringLiteral" === node.expectedNode) {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isUserWhitespacable(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if ("ObjectMethod" === nodeType || "ObjectProperty" === nodeType || "ObjectTypeInternalSlot" === nodeType || "ObjectTypeCallProperty" === nodeType || "ObjectTypeIndexer" === nodeType || "ObjectTypeProperty" === nodeType || "ObjectTypeSpreadProperty" === nodeType) {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isMethod(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if ("ObjectMethod" === nodeType || "ClassMethod" === nodeType || "ClassPrivateMethod" === nodeType) {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isObjectMember(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if ("ObjectMethod" === nodeType || "ObjectProperty" === nodeType) {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isProperty(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if ("ObjectProperty" === nodeType || "ClassProperty" === nodeType || "ClassPrivateProperty" === nodeType) {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isUnaryLike(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if ("UnaryExpression" === nodeType || "SpreadElement" === nodeType) {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isPattern(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if ("AssignmentPattern" === nodeType || "ArrayPattern" === nodeType || "ObjectPattern" === nodeType || nodeType === "Placeholder" && "Pattern" === node.expectedNode) {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isClass(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if ("ClassExpression" === nodeType || "ClassDeclaration" === nodeType) {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isModuleDeclaration(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if ("ExportAllDeclaration" === nodeType || "ExportDefaultDeclaration" === nodeType || "ExportNamedDeclaration" === nodeType || "ImportDeclaration" === nodeType) {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isExportDeclaration(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if ("ExportAllDeclaration" === nodeType || "ExportDefaultDeclaration" === nodeType || "ExportNamedDeclaration" === nodeType) {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isModuleSpecifier(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if ("ExportSpecifier" === nodeType || "ImportDefaultSpecifier" === nodeType || "ImportNamespaceSpecifier" === nodeType || "ImportSpecifier" === nodeType || "ExportNamespaceSpecifier" === nodeType || "ExportDefaultSpecifier" === nodeType) {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isPrivate(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if ("ClassPrivateProperty" === nodeType || "ClassPrivateMethod" === nodeType || "PrivateName" === nodeType) {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isFlow(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if ("AnyTypeAnnotation" === nodeType || "ArrayTypeAnnotation" === nodeType || "BooleanTypeAnnotation" === nodeType || "BooleanLiteralTypeAnnotation" === nodeType || "NullLiteralTypeAnnotation" === nodeType || "ClassImplements" === nodeType || "DeclareClass" === nodeType || "DeclareFunction" === nodeType || "DeclareInterface" === nodeType || "DeclareModule" === nodeType || "DeclareModuleExports" === nodeType || "DeclareTypeAlias" === nodeType || "DeclareOpaqueType" === nodeType || "DeclareVariable" === nodeType || "DeclareExportDeclaration" === nodeType || "DeclareExportAllDeclaration" === nodeType || "DeclaredPredicate" === nodeType || "ExistsTypeAnnotation" === nodeType || "FunctionTypeAnnotation" === nodeType || "FunctionTypeParam" === nodeType || "GenericTypeAnnotation" === nodeType || "InferredPredicate" === nodeType || "InterfaceExtends" === nodeType || "InterfaceDeclaration" === nodeType || "InterfaceTypeAnnotation" === nodeType || "IntersectionTypeAnnotation" === nodeType || "MixedTypeAnnotation" === nodeType || "EmptyTypeAnnotation" === nodeType || "NullableTypeAnnotation" === nodeType || "NumberLiteralTypeAnnotation" === nodeType || "NumberTypeAnnotation" === nodeType || "ObjectTypeAnnotation" === nodeType || "ObjectTypeInternalSlot" === nodeType || "ObjectTypeCallProperty" === nodeType || "ObjectTypeIndexer" === nodeType || "ObjectTypeProperty" === nodeType || "ObjectTypeSpreadProperty" === nodeType || "OpaqueType" === nodeType || "QualifiedTypeIdentifier" === nodeType || "StringLiteralTypeAnnotation" === nodeType || "StringTypeAnnotation" === nodeType || "SymbolTypeAnnotation" === nodeType || "ThisTypeAnnotation" === nodeType || "TupleTypeAnnotation" === nodeType || "TypeofTypeAnnotation" === nodeType || "TypeAlias" === nodeType || "TypeAnnotation" === nodeType || "TypeCastExpression" === nodeType || "TypeParameter" === nodeType || "TypeParameterDeclaration" === nodeType || "TypeParameterInstantiation" === nodeType || "UnionTypeAnnotation" === nodeType || "Variance" === nodeType || "VoidTypeAnnotation" === nodeType || "IndexedAccessType" === nodeType || "OptionalIndexedAccessType" === nodeType) {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isFlowType(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if ("AnyTypeAnnotation" === nodeType || "ArrayTypeAnnotation" === nodeType || "BooleanTypeAnnotation" === nodeType || "BooleanLiteralTypeAnnotation" === nodeType || "NullLiteralTypeAnnotation" === nodeType || "ExistsTypeAnnotation" === nodeType || "FunctionTypeAnnotation" === nodeType || "GenericTypeAnnotation" === nodeType || "InterfaceTypeAnnotation" === nodeType || "IntersectionTypeAnnotation" === nodeType || "MixedTypeAnnotation" === nodeType || "EmptyTypeAnnotation" === nodeType || "NullableTypeAnnotation" === nodeType || "NumberLiteralTypeAnnotation" === nodeType || "NumberTypeAnnotation" === nodeType || "ObjectTypeAnnotation" === nodeType || "StringLiteralTypeAnnotation" === nodeType || "StringTypeAnnotation" === nodeType || "SymbolTypeAnnotation" === nodeType || "ThisTypeAnnotation" === nodeType || "TupleTypeAnnotation" === nodeType || "TypeofTypeAnnotation" === nodeType || "UnionTypeAnnotation" === nodeType || "VoidTypeAnnotation" === nodeType || "IndexedAccessType" === nodeType || "OptionalIndexedAccessType" === nodeType) {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isFlowBaseAnnotation(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if ("AnyTypeAnnotation" === nodeType || "BooleanTypeAnnotation" === nodeType || "NullLiteralTypeAnnotation" === nodeType || "MixedTypeAnnotation" === nodeType || "EmptyTypeAnnotation" === nodeType || "NumberTypeAnnotation" === nodeType || "StringTypeAnnotation" === nodeType || "SymbolTypeAnnotation" === nodeType || "ThisTypeAnnotation" === nodeType || "VoidTypeAnnotation" === nodeType) {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isFlowDeclaration(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if ("DeclareClass" === nodeType || "DeclareFunction" === nodeType || "DeclareInterface" === nodeType || "DeclareModule" === nodeType || "DeclareModuleExports" === nodeType || "DeclareTypeAlias" === nodeType || "DeclareOpaqueType" === nodeType || "DeclareVariable" === nodeType || "DeclareExportDeclaration" === nodeType || "DeclareExportAllDeclaration" === nodeType || "InterfaceDeclaration" === nodeType || "OpaqueType" === nodeType || "TypeAlias" === nodeType) {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isFlowPredicate(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if ("DeclaredPredicate" === nodeType || "InferredPredicate" === nodeType) {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isEnumBody(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if ("EnumBooleanBody" === nodeType || "EnumNumberBody" === nodeType || "EnumStringBody" === nodeType || "EnumSymbolBody" === nodeType) {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isEnumMember(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if ("EnumBooleanMember" === nodeType || "EnumNumberMember" === nodeType || "EnumStringMember" === nodeType || "EnumDefaultedMember" === nodeType) {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isJSX(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if ("JSXAttribute" === nodeType || "JSXClosingElement" === nodeType || "JSXElement" === nodeType || "JSXEmptyExpression" === nodeType || "JSXExpressionContainer" === nodeType || "JSXSpreadChild" === nodeType || "JSXIdentifier" === nodeType || "JSXMemberExpression" === nodeType || "JSXNamespacedName" === nodeType || "JSXOpeningElement" === nodeType || "JSXSpreadAttribute" === nodeType || "JSXText" === nodeType || "JSXFragment" === nodeType || "JSXOpeningFragment" === nodeType || "JSXClosingFragment" === nodeType) {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isTSTypeElement(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if ("TSCallSignatureDeclaration" === nodeType || "TSConstructSignatureDeclaration" === nodeType || "TSPropertySignature" === nodeType || "TSMethodSignature" === nodeType || "TSIndexSignature" === nodeType) {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isTSType(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if ("TSAnyKeyword" === nodeType || "TSBooleanKeyword" === nodeType || "TSBigIntKeyword" === nodeType || "TSIntrinsicKeyword" === nodeType || "TSNeverKeyword" === nodeType || "TSNullKeyword" === nodeType || "TSNumberKeyword" === nodeType || "TSObjectKeyword" === nodeType || "TSStringKeyword" === nodeType || "TSSymbolKeyword" === nodeType || "TSUndefinedKeyword" === nodeType || "TSUnknownKeyword" === nodeType || "TSVoidKeyword" === nodeType || "TSThisType" === nodeType || "TSFunctionType" === nodeType || "TSConstructorType" === nodeType || "TSTypeReference" === nodeType || "TSTypePredicate" === nodeType || "TSTypeQuery" === nodeType || "TSTypeLiteral" === nodeType || "TSArrayType" === nodeType || "TSTupleType" === nodeType || "TSOptionalType" === nodeType || "TSRestType" === nodeType || "TSUnionType" === nodeType || "TSIntersectionType" === nodeType || "TSConditionalType" === nodeType || "TSInferType" === nodeType || "TSParenthesizedType" === nodeType || "TSTypeOperator" === nodeType || "TSIndexedAccessType" === nodeType || "TSMappedType" === nodeType || "TSLiteralType" === nodeType || "TSExpressionWithTypeArguments" === nodeType || "TSImportType" === nodeType) {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isTSBaseType(node, opts) {
    if (!node) return false;
    var nodeType = node.type;
    if ("TSAnyKeyword" === nodeType || "TSBooleanKeyword" === nodeType || "TSBigIntKeyword" === nodeType || "TSIntrinsicKeyword" === nodeType || "TSNeverKeyword" === nodeType || "TSNullKeyword" === nodeType || "TSNumberKeyword" === nodeType || "TSObjectKeyword" === nodeType || "TSStringKeyword" === nodeType || "TSSymbolKeyword" === nodeType || "TSUndefinedKeyword" === nodeType || "TSUnknownKeyword" === nodeType || "TSVoidKeyword" === nodeType || "TSThisType" === nodeType || "TSLiteralType" === nodeType) {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isNumberLiteral(node, opts) {
    console.trace("The node type NumberLiteral has been renamed to NumericLiteral");
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "NumberLiteral") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isRegexLiteral(node, opts) {
    console.trace("The node type RegexLiteral has been renamed to RegExpLiteral");
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "RegexLiteral") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isRestProperty(node, opts) {
    console.trace("The node type RestProperty has been renamed to RestElement");
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "RestProperty") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}
function isSpreadProperty(node, opts) {
    console.trace("The node type SpreadProperty has been renamed to SpreadElement");
    if (!node) return false;
    var nodeType = node.type;
    if (nodeType === "SpreadProperty") {
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    }
    return false;
}

Object.defineProperty(matchesPattern$1, "__esModule", {
    value: true
});
matchesPattern$1.default = matchesPattern;
var _generated$o = generated$4;
function matchesPattern(member, match, allowPartial) {
    if (!(0, _generated$o.isMemberExpression)(member)) return false;
    var parts = Array.isArray(match) ? match : match.split(".");
    var nodes = [];
    var node;
    for(node = member; (0, _generated$o.isMemberExpression)(node); node = node.object){
        nodes.push(node.property);
    }
    nodes.push(node);
    if (nodes.length < parts.length) return false;
    if (!allowPartial && nodes.length > parts.length) return false;
    for(var i = 0, j = nodes.length - 1; i < parts.length; i++, j--){
        var node1 = nodes[j];
        var value = void 0;
        if ((0, _generated$o.isIdentifier)(node1)) {
            value = node1.name;
        } else if ((0, _generated$o.isStringLiteral)(node1)) {
            value = node1.value;
        } else if ((0, _generated$o.isThisExpression)(node1)) {
            value = "this";
        } else {
            return false;
        }
        if (parts[i] !== value) return false;
    }
    return true;
}

Object.defineProperty(buildMatchMemberExpression$1, "__esModule", {
    value: true
});
buildMatchMemberExpression$1.default = buildMatchMemberExpression;
var _matchesPattern = matchesPattern$1;
function buildMatchMemberExpression(match, allowPartial) {
    var parts = match.split(".");
    return function(member) {
        return (0, _matchesPattern.default)(member, parts, allowPartial);
    };
}

Object.defineProperty(isReactComponent$1, "__esModule", {
    value: true
});
isReactComponent$1.default = void 0;
var _buildMatchMemberExpression = buildMatchMemberExpression$1;
var isReactComponent = (0, _buildMatchMemberExpression.default)("React.Component");
var _default$4 = isReactComponent;
isReactComponent$1.default = _default$4;

var isCompatTag$1 = {};

Object.defineProperty(isCompatTag$1, "__esModule", {
    value: true
});
isCompatTag$1.default = isCompatTag;
function isCompatTag(tagName) {
    return !!tagName && /^[a-z]/.test(tagName);
}

var buildChildren$1 = {};

var cleanJSXElementLiteralChild$1 = {};

var generated$3 = {};

var builder$1 = {};

var definitions = {};

var _typeof$4 = function(obj) {
    "@swc/helpers - typeof";
    return obj && typeof Symbol !== "undefined" && obj.constructor === Symbol ? "symbol" : typeof obj;
};
var toFastProperties;
var hasRequiredToFastProperties;
function requireToFastProperties() {
    if (hasRequiredToFastProperties) return toFastProperties;
    hasRequiredToFastProperties = 1;
    var fastProto = null;
    // Creates an object with permanently fast properties in V8. See Toon Verwaest's
    // post https://medium.com/@tverwaes/setting-up-prototypes-in-v8-ec9c9491dfe2#5f62
    // for more details. Use %HasFastProperties(object) and the Node.js flag
    // --allow-natives-syntax to check whether an object has fast properties.
    function FastObject(o) {
        // A prototype object will have "fast properties" enabled once it is checked
        // against the inline property cache of a function, e.g. fastProto.property:
        // https://github.com/v8/v8/blob/6.0.122/test/mjsunit/fast-prototype.js#L48-L63
        if (fastProto !== null && _typeof$4(fastProto.property)) {
            var result = fastProto;
            fastProto = FastObject.prototype = null;
            return result;
        }
        fastProto = FastObject.prototype = o == null ? Object.create(null) : o;
        return new FastObject;
    }
    // Initialize the inline property cache of FastObject
    FastObject();
    toFastProperties = function toFastproperties(o) {
        return FastObject(o);
    };
    return toFastProperties;
}

var core = {};

var is = {};

var isType = {};

function _createForOfIteratorHelperLoose$i(o, allowArrayLike) {
    var it = typeof Symbol !== "undefined" && o[Symbol.iterator] || o["@@iterator"];
    if (it) return (it = it.call(o)).next.bind(it);
    if (Array.isArray(o) || (it = unsupportedIterableToArray(o)) || allowArrayLike && o && typeof o.length === "number") {
        if (it) o = it;
        var i = 0;
        return function() {
            if (i >= o.length) return {
                done: true
            };
            return {
                done: false,
                value: o[i++]
            };
        };
    }
    throw new TypeError("Invalid attempt to iterate non-iterable instance.\\nIn order to be iterable, non-array objects must have a [Symbol.iterator]() method.");
}
var hasRequiredIsType;
function requireIsType() {
    var isType$1 = function isType(nodeType, targetType) {
        if (nodeType === targetType) return true;
        if (_definitions.ALIAS_KEYS[targetType]) return false;
        var aliases = _definitions.FLIPPED_ALIAS_KEYS[targetType];
        if (aliases) {
            if (aliases[0] === nodeType) return true;
            for(var _iterator = _createForOfIteratorHelperLoose$i(aliases), _step; !(_step = _iterator()).done;){
                var alias = _step.value;
                if (nodeType === alias) return true;
            }
        }
        return false;
    };
    if (hasRequiredIsType) return isType;
    hasRequiredIsType = 1;
    Object.defineProperty(isType, "__esModule", {
        value: true
    });
    isType.default = isType$1;
    var _definitions = requireDefinitions();
    return isType;
}

var isPlaceholderType = {};

function _createForOfIteratorHelperLoose$h(o, allowArrayLike) {
    var it = typeof Symbol !== "undefined" && o[Symbol.iterator] || o["@@iterator"];
    if (it) return (it = it.call(o)).next.bind(it);
    if (Array.isArray(o) || (it = unsupportedIterableToArray(o)) || allowArrayLike && o && typeof o.length === "number") {
        if (it) o = it;
        var i = 0;
        return function() {
            if (i >= o.length) return {
                done: true
            };
            return {
                done: false,
                value: o[i++]
            };
        };
    }
    throw new TypeError("Invalid attempt to iterate non-iterable instance.\\nIn order to be iterable, non-array objects must have a [Symbol.iterator]() method.");
}
var hasRequiredIsPlaceholderType;
function requireIsPlaceholderType() {
    var isPlaceholderType$1 = function isPlaceholderType(placeholderType, targetType) {
        if (placeholderType === targetType) return true;
        var aliases = _definitions.PLACEHOLDERS_ALIAS[placeholderType];
        if (aliases) {
            for(var _iterator = _createForOfIteratorHelperLoose$h(aliases), _step; !(_step = _iterator()).done;){
                var alias = _step.value;
                if (targetType === alias) return true;
            }
        }
        return false;
    };
    if (hasRequiredIsPlaceholderType) return isPlaceholderType;
    hasRequiredIsPlaceholderType = 1;
    Object.defineProperty(isPlaceholderType, "__esModule", {
        value: true
    });
    isPlaceholderType.default = isPlaceholderType$1;
    var _definitions = requireDefinitions();
    return isPlaceholderType;
}

var hasRequiredIs;
function requireIs() {
    var is$1 = function is(type, node, opts) {
        if (!node) return false;
        var matches = (0, _isType.default)(node.type, type);
        if (!matches) {
            if (!opts && node.type === "Placeholder" && type in _definitions.FLIPPED_ALIAS_KEYS) {
                return (0, _isPlaceholderType.default)(node.expectedNode, type);
            }
            return false;
        }
        if (typeof opts === "undefined") {
            return true;
        } else {
            return (0, _shallowEqual.default)(node, opts);
        }
    };
    if (hasRequiredIs) return is;
    hasRequiredIs = 1;
    Object.defineProperty(is, "__esModule", {
        value: true
    });
    is.default = is$1;
    var _shallowEqual = shallowEqual$1;
    var _isType = requireIsType();
    var _isPlaceholderType = requireIsPlaceholderType();
    var _definitions = requireDefinitions();
    return is;
}

var isValidIdentifier$1 = {};

var lib = {};

var identifier$1 = {};

Object.defineProperty(identifier$1, "__esModule", {
    value: true
});
identifier$1.isIdentifierStart = isIdentifierStart;
identifier$1.isIdentifierChar = isIdentifierChar;
identifier$1.isIdentifierName = isIdentifierName;
var nonASCIIidentifierStartChars = "\xaa\xb5\xba\xc0-\xd6\xd8-\xf6\xf8-ˁˆ-ˑˠ-ˤˬˮͰ-ʹͶͷͺ-ͽͿΆΈ-ΊΌΎ-ΡΣ-ϵϷ-ҁҊ-ԯԱ-Ֆՙՠ-ֈא-תׯ-ײؠ-يٮٯٱ-ۓەۥۦۮۯۺ-ۼۿܐܒ-ܯݍ-ޥޱߊ-ߪߴߵߺࠀ-ࠕࠚࠤࠨࡀ-ࡘࡠ-ࡪࢠ-ࢴࢶ-ࣇऄ-हऽॐक़-ॡॱ-ঀঅ-ঌএঐও-নপ-রলশ-হঽৎড়ঢ়য়-ৡৰৱৼਅ-ਊਏਐਓ-ਨਪ-ਰਲਲ਼ਵਸ਼ਸਹਖ਼-ੜਫ਼ੲ-ੴઅ-ઍએ-ઑઓ-નપ-રલળવ-હઽૐૠૡૹଅ-ଌଏଐଓ-ନପ-ରଲଳଵ-ହଽଡ଼ଢ଼ୟ-ୡୱஃஅ-ஊஎ-ஐஒ-கஙசஜஞடணதந-பம-ஹௐఅ-ఌఎ-ఐఒ-నప-హఽౘ-ౚౠౡಀಅ-ಌಎ-ಐಒ-ನಪ-ಳವ-ಹಽೞೠೡೱೲഄ-ഌഎ-ഐഒ-ഺഽൎൔ-ൖൟ-ൡൺ-ൿඅ-ඖක-නඳ-රලව-ෆก-ะาำเ-ๆກຂຄຆ-ຊຌ-ຣລວ-ະາຳຽເ-ໄໆໜ-ໟༀཀ-ཇཉ-ཬྈ-ྌက-ဪဿၐ-ၕၚ-ၝၡၥၦၮ-ၰၵ-ႁႎႠ-ჅჇჍა-ჺჼ-ቈቊ-ቍቐ-ቖቘቚ-ቝበ-ኈኊ-ኍነ-ኰኲ-ኵኸ-ኾዀዂ-ዅወ-ዖዘ-ጐጒ-ጕጘ-ፚᎀ-ᎏᎠ-Ᏽᏸ-ᏽᐁ-ᙬᙯ-ᙿᚁ-ᚚᚠ-ᛪᛮ-ᛸᜀ-ᜌᜎ-ᜑᜠ-ᜱᝀ-ᝑᝠ-ᝬᝮ-ᝰក-ឳៗៜᠠ-ᡸᢀ-ᢨᢪᢰ-ᣵᤀ-ᤞᥐ-ᥭᥰ-ᥴᦀ-ᦫᦰ-ᧉᨀ-ᨖᨠ-ᩔᪧᬅ-ᬳᭅ-ᭋᮃ-ᮠᮮᮯᮺ-ᯥᰀ-ᰣᱍ-ᱏᱚ-ᱽᲀ-ᲈᲐ-ᲺᲽ-Ჿᳩ-ᳬᳮ-ᳳᳵᳶᳺᴀ-ᶿḀ-ἕἘ-Ἕἠ-ὅὈ-Ὅὐ-ὗὙὛὝὟ-ώᾀ-ᾴᾶ-ᾼιῂ-ῄῆ-ῌῐ-ΐῖ-Ίῠ-Ῥῲ-ῴῶ-ῼⁱⁿₐ-ₜℂℇℊ-ℓℕ℘-ℝℤΩℨK-ℹℼ-ℿⅅ-ⅉⅎⅠ-ↈⰀ-Ⱞⰰ-ⱞⱠ-ⳤⳫ-ⳮⳲⳳⴀ-ⴥⴧⴭⴰ-ⵧⵯⶀ-ⶖⶠ-ⶦⶨ-ⶮⶰ-ⶶⶸ-ⶾⷀ-ⷆⷈ-ⷎⷐ-ⷖⷘ-ⷞ々-〇〡-〩〱-〵〸-〼ぁ-ゖ゛-ゟァ-ヺー-ヿㄅ-ㄯㄱ-ㆎㆠ-ㆿㇰ-ㇿ㐀-䶿一-鿼ꀀ-ꒌꓐ-ꓽꔀ-ꘌꘐ-ꘟꘪꘫꙀ-ꙮꙿ-ꚝꚠ-ꛯꜗ-ꜟꜢ-ꞈꞋ-ꞿꟂ-ꟊꟵ-ꠁꠃ-ꠅꠇ-ꠊꠌ-ꠢꡀ-ꡳꢂ-ꢳꣲ-ꣷꣻꣽꣾꤊ-ꤥꤰ-ꥆꥠ-ꥼꦄ-ꦲꧏꧠ-ꧤꧦ-ꧯꧺ-ꧾꨀ-ꨨꩀ-ꩂꩄ-ꩋꩠ-ꩶꩺꩾ-ꪯꪱꪵꪶꪹ-ꪽꫀꫂꫛ-ꫝꫠ-ꫪꫲ-ꫴꬁ-ꬆꬉ-ꬎꬑ-ꬖꬠ-ꬦꬨ-ꬮꬰ-ꭚꭜ-ꭩꭰ-ꯢ가-힣ힰ-ퟆퟋ-ퟻ豈-舘並-龎ﬀ-ﬆﬓ-ﬗיִײַ-ﬨשׁ-זּטּ-לּמּנּסּףּפּצּ-ﮱﯓ-ﴽﵐ-ﶏﶒ-ﷇﷰ-ﷻﹰ-ﹴﹶ-ﻼＡ-Ｚａ-ｚｦ-ﾾￂ-ￇￊ-ￏￒ-ￗￚ-ￜ";
var nonASCIIidentifierChars = "‌‍\xb7̀-ͯ·҃-֑҇-ׇֽֿׁׂׅׄؐ-ًؚ-٩ٰۖ-ۜ۟-۪ۤۧۨ-ۭ۰-۹ܑܰ-݊ަ-ް߀-߉߫-߽߳ࠖ-࠙ࠛ-ࠣࠥ-ࠧࠩ-࡙࠭-࡛࣓-ࣣ࣡-ःऺ-़ा-ॏ॑-ॗॢॣ०-९ঁ-ঃ়া-ৄেৈো-্ৗৢৣ০-৯৾ਁ-ਃ਼ਾ-ੂੇੈੋ-੍ੑ੦-ੱੵઁ-ઃ઼ા-ૅે-ૉો-્ૢૣ૦-૯ૺ-૿ଁ-ଃ଼ା-ୄେୈୋ-୍୕-ୗୢୣ୦-୯ஂா-ூெ-ைொ-்ௗ௦-௯ఀ-ఄా-ౄె-ైొ-్ౕౖౢౣ౦-౯ಁ-ಃ಼ಾ-ೄೆ-ೈೊ-್ೕೖೢೣ೦-೯ഀ-ഃ഻഼ാ-ൄെ-ൈൊ-്ൗൢൣ൦-൯ඁ-ඃ්ා-ුූෘ-ෟ෦-෯ෲෳัิ-ฺ็-๎๐-๙ັິ-ຼ່-ໍ໐-໙༘༙༠-༩༹༵༷༾༿ཱ-྄྆྇ྍ-ྗྙ-ྼ࿆ါ-ှ၀-၉ၖ-ၙၞ-ၠၢ-ၤၧ-ၭၱ-ၴႂ-ႍႏ-ႝ፝-፟፩-፱ᜒ-᜔ᜲ-᜴ᝒᝓᝲᝳ឴-៓៝០-៩᠋-᠍᠐-᠙ᢩᤠ-ᤫᤰ-᤻᥆-᥏᧐-᧚ᨗ-ᨛᩕ-ᩞ᩠-᩿᩼-᪉᪐-᪙᪰-᪽ᪿᫀᬀ-ᬄ᬴-᭄᭐-᭙᭫-᭳ᮀ-ᮂᮡ-ᮭ᮰-᮹᯦-᯳ᰤ-᰷᱀-᱉᱐-᱙᳐-᳔᳒-᳨᳭᳴᳷-᳹᷀-᷹᷻-᷿‿⁀⁔⃐-⃥⃜⃡-⃰⳯-⵿⳱ⷠ-〪ⷿ-゙゚〯꘠-꘩꙯ꙴ-꙽ꚞꚟ꛰꛱ꠂ꠆ꠋꠣ-ꠧ꠬ꢀꢁꢴ-ꣅ꣐-꣙꣠-꣱ꣿ-꤉ꤦ-꤭ꥇ-꥓ꦀ-ꦃ꦳-꧀꧐-꧙ꧥ꧰-꧹ꨩ-ꨶꩃꩌꩍ꩐-꩙ꩻ-ꩽꪰꪲ-ꪴꪷꪸꪾ꪿꫁ꫫ-ꫯꫵ꫶ꯣ-ꯪ꯬꯭꯰-꯹ﬞ︀-️︠-︯︳︴﹍-﹏０-９＿";
var nonASCIIidentifierStart = new RegExp("[" + nonASCIIidentifierStartChars + "]");
var nonASCIIidentifier = new RegExp("[" + nonASCIIidentifierStartChars + nonASCIIidentifierChars + "]");
nonASCIIidentifierStartChars = nonASCIIidentifierChars = null;
var astralIdentifierStartCodes = [
    0,
    11,
    2,
    25,
    2,
    18,
    2,
    1,
    2,
    14,
    3,
    13,
    35,
    122,
    70,
    52,
    268,
    28,
    4,
    48,
    48,
    31,
    14,
    29,
    6,
    37,
    11,
    29,
    3,
    35,
    5,
    7,
    2,
    4,
    43,
    157,
    19,
    35,
    5,
    35,
    5,
    39,
    9,
    51,
    157,
    310,
    10,
    21,
    11,
    7,
    153,
    5,
    3,
    0,
    2,
    43,
    2,
    1,
    4,
    0,
    3,
    22,
    11,
    22,
    10,
    30,
    66,
    18,
    2,
    1,
    11,
    21,
    11,
    25,
    71,
    55,
    7,
    1,
    65,
    0,
    16,
    3,
    2,
    2,
    2,
    28,
    43,
    28,
    4,
    28,
    36,
    7,
    2,
    27,
    28,
    53,
    11,
    21,
    11,
    18,
    14,
    17,
    111,
    72,
    56,
    50,
    14,
    50,
    14,
    35,
    349,
    41,
    7,
    1,
    79,
    28,
    11,
    0,
    9,
    21,
    107,
    20,
    28,
    22,
    13,
    52,
    76,
    44,
    33,
    24,
    27,
    35,
    30,
    0,
    3,
    0,
    9,
    34,
    4,
    0,
    13,
    47,
    15,
    3,
    22,
    0,
    2,
    0,
    36,
    17,
    2,
    24,
    85,
    6,
    2,
    0,
    2,
    3,
    2,
    14,
    2,
    9,
    8,
    46,
    39,
    7,
    3,
    1,
    3,
    21,
    2,
    6,
    2,
    1,
    2,
    4,
    4,
    0,
    19,
    0,
    13,
    4,
    159,
    52,
    19,
    3,
    21,
    2,
    31,
    47,
    21,
    1,
    2,
    0,
    185,
    46,
    42,
    3,
    37,
    47,
    21,
    0,
    60,
    42,
    14,
    0,
    72,
    26,
    230,
    43,
    117,
    63,
    32,
    7,
    3,
    0,
    3,
    7,
    2,
    1,
    2,
    23,
    16,
    0,
    2,
    0,
    95,
    7,
    3,
    38,
    17,
    0,
    2,
    0,
    29,
    0,
    11,
    39,
    8,
    0,
    22,
    0,
    12,
    45,
    20,
    0,
    35,
    56,
    264,
    8,
    2,
    36,
    18,
    0,
    50,
    29,
    113,
    6,
    2,
    1,
    2,
    37,
    22,
    0,
    26,
    5,
    2,
    1,
    2,
    31,
    15,
    0,
    328,
    18,
    190,
    0,
    80,
    921,
    103,
    110,
    18,
    195,
    2749,
    1070,
    4050,
    582,
    8634,
    568,
    8,
    30,
    114,
    29,
    19,
    47,
    17,
    3,
    32,
    20,
    6,
    18,
    689,
    63,
    129,
    74,
    6,
    0,
    67,
    12,
    65,
    1,
    2,
    0,
    29,
    6135,
    9,
    1237,
    43,
    8,
    8952,
    286,
    50,
    2,
    18,
    3,
    9,
    395,
    2309,
    106,
    6,
    12,
    4,
    8,
    8,
    9,
    5991,
    84,
    2,
    70,
    2,
    1,
    3,
    0,
    3,
    1,
    3,
    3,
    2,
    11,
    2,
    0,
    2,
    6,
    2,
    64,
    2,
    3,
    3,
    7,
    2,
    6,
    2,
    27,
    2,
    3,
    2,
    4,
    2,
    0,
    4,
    6,
    2,
    339,
    3,
    24,
    2,
    24,
    2,
    30,
    2,
    24,
    2,
    30,
    2,
    24,
    2,
    30,
    2,
    24,
    2,
    30,
    2,
    24,
    2,
    7,
    2357,
    44,
    11,
    6,
    17,
    0,
    370,
    43,
    1301,
    196,
    60,
    67,
    8,
    0,
    1205,
    3,
    2,
    26,
    2,
    1,
    2,
    0,
    3,
    0,
    2,
    9,
    2,
    3,
    2,
    0,
    2,
    0,
    7,
    0,
    5,
    0,
    2,
    0,
    2,
    0,
    2,
    2,
    2,
    1,
    2,
    0,
    3,
    0,
    2,
    0,
    2,
    0,
    2,
    0,
    2,
    0,
    2,
    1,
    2,
    0,
    3,
    3,
    2,
    6,
    2,
    3,
    2,
    3,
    2,
    0,
    2,
    9,
    2,
    16,
    6,
    2,
    2,
    4,
    2,
    16,
    4421,
    42717,
    35,
    4148,
    12,
    221,
    3,
    5761,
    15,
    7472,
    3104,
    541,
    1507,
    4938
];
var astralIdentifierCodes = [
    509,
    0,
    227,
    0,
    150,
    4,
    294,
    9,
    1368,
    2,
    2,
    1,
    6,
    3,
    41,
    2,
    5,
    0,
    166,
    1,
    574,
    3,
    9,
    9,
    370,
    1,
    154,
    10,
    176,
    2,
    54,
    14,
    32,
    9,
    16,
    3,
    46,
    10,
    54,
    9,
    7,
    2,
    37,
    13,
    2,
    9,
    6,
    1,
    45,
    0,
    13,
    2,
    49,
    13,
    9,
    3,
    2,
    11,
    83,
    11,
    7,
    0,
    161,
    11,
    6,
    9,
    7,
    3,
    56,
    1,
    2,
    6,
    3,
    1,
    3,
    2,
    10,
    0,
    11,
    1,
    3,
    6,
    4,
    4,
    193,
    17,
    10,
    9,
    5,
    0,
    82,
    19,
    13,
    9,
    214,
    6,
    3,
    8,
    28,
    1,
    83,
    16,
    16,
    9,
    82,
    12,
    9,
    9,
    84,
    14,
    5,
    9,
    243,
    14,
    166,
    9,
    71,
    5,
    2,
    1,
    3,
    3,
    2,
    0,
    2,
    1,
    13,
    9,
    120,
    6,
    3,
    6,
    4,
    0,
    29,
    9,
    41,
    6,
    2,
    3,
    9,
    0,
    10,
    10,
    47,
    15,
    406,
    7,
    2,
    7,
    17,
    9,
    57,
    21,
    2,
    13,
    123,
    5,
    4,
    0,
    2,
    1,
    2,
    6,
    2,
    0,
    9,
    9,
    49,
    4,
    2,
    1,
    2,
    4,
    9,
    9,
    330,
    3,
    19306,
    9,
    135,
    4,
    60,
    6,
    26,
    9,
    1014,
    0,
    2,
    54,
    8,
    3,
    82,
    0,
    12,
    1,
    19628,
    1,
    5319,
    4,
    4,
    5,
    9,
    7,
    3,
    6,
    31,
    3,
    149,
    2,
    1418,
    49,
    513,
    54,
    5,
    49,
    9,
    0,
    15,
    0,
    23,
    4,
    2,
    14,
    1361,
    6,
    2,
    16,
    3,
    6,
    2,
    1,
    2,
    4,
    262,
    6,
    10,
    9,
    419,
    13,
    1495,
    6,
    110,
    6,
    6,
    9,
    4759,
    9,
    787719,
    239
];
function isInAstralSet(code, set) {
    var pos = 0x10000;
    for(var i = 0, length = set.length; i < length; i += 2){
        pos += set[i];
        if (pos > code) return false;
        pos += set[i + 1];
        if (pos >= code) return true;
    }
    return false;
}
function isIdentifierStart(code) {
    if (code < 65) return code === 36;
    if (code <= 90) return true;
    if (code < 97) return code === 95;
    if (code <= 122) return true;
    if (code <= 0xffff) {
        return code >= 0xaa && nonASCIIidentifierStart.test(String.fromCharCode(code));
    }
    return isInAstralSet(code, astralIdentifierStartCodes);
}
function isIdentifierChar(code) {
    if (code < 48) return code === 36;
    if (code < 58) return true;
    if (code < 65) return false;
    if (code <= 90) return true;
    if (code < 97) return code === 95;
    if (code <= 122) return true;
    if (code <= 0xffff) {
        return code >= 0xaa && nonASCIIidentifier.test(String.fromCharCode(code));
    }
    return isInAstralSet(code, astralIdentifierStartCodes) || isInAstralSet(code, astralIdentifierCodes);
}
function isIdentifierName(name) {
    var isFirst = true;
    for(var i = 0; i < name.length; i++){
        var cp = name.charCodeAt(i);
        if ((cp & 0xfc00) === 0xd800 && i + 1 < name.length) {
            var trail = name.charCodeAt(++i);
            if ((trail & 0xfc00) === 0xdc00) {
                cp = 0x10000 + ((cp & 0x3ff) << 10) + (trail & 0x3ff);
            }
        }
        if (isFirst) {
            isFirst = false;
            if (!isIdentifierStart(cp)) {
                return false;
            }
        } else if (!isIdentifierChar(cp)) {
            return false;
        }
    }
    return !isFirst;
}

var keyword = {};

Object.defineProperty(keyword, "__esModule", {
    value: true
});
keyword.isReservedWord = isReservedWord;
keyword.isStrictReservedWord = isStrictReservedWord;
keyword.isStrictBindOnlyReservedWord = isStrictBindOnlyReservedWord;
keyword.isStrictBindReservedWord = isStrictBindReservedWord;
keyword.isKeyword = isKeyword;
var reservedWords = {
    keyword: [
        "break",
        "case",
        "catch",
        "continue",
        "debugger",
        "default",
        "do",
        "else",
        "finally",
        "for",
        "function",
        "if",
        "return",
        "switch",
        "throw",
        "try",
        "var",
        "const",
        "while",
        "with",
        "new",
        "this",
        "super",
        "class",
        "extends",
        "export",
        "import",
        "null",
        "true",
        "false",
        "in",
        "instanceof",
        "typeof",
        "void",
        "delete"
    ],
    strict: [
        "implements",
        "interface",
        "let",
        "package",
        "private",
        "protected",
        "public",
        "static",
        "yield"
    ],
    strictBind: [
        "eval",
        "arguments"
    ]
};
var keywords = new Set(reservedWords.keyword);
var reservedWordsStrictSet = new Set(reservedWords.strict);
var reservedWordsStrictBindSet = new Set(reservedWords.strictBind);
function isReservedWord(word, inModule) {
    return inModule && word === "await" || word === "enum";
}
function isStrictReservedWord(word, inModule) {
    return isReservedWord(word, inModule) || reservedWordsStrictSet.has(word);
}
function isStrictBindOnlyReservedWord(word) {
    return reservedWordsStrictBindSet.has(word);
}
function isStrictBindReservedWord(word, inModule) {
    return isStrictReservedWord(word, inModule) || isStrictBindOnlyReservedWord(word);
}
function isKeyword(word) {
    return keywords.has(word);
}

(function(exports) {
    Object.defineProperty(exports, "__esModule", {
        value: true
    });
    Object.defineProperty(exports, "isIdentifierName", {
        enumerable: true,
        get: function get() {
            return _identifier.isIdentifierName;
        }
    });
    Object.defineProperty(exports, "isIdentifierChar", {
        enumerable: true,
        get: function get() {
            return _identifier.isIdentifierChar;
        }
    });
    Object.defineProperty(exports, "isIdentifierStart", {
        enumerable: true,
        get: function get() {
            return _identifier.isIdentifierStart;
        }
    });
    Object.defineProperty(exports, "isReservedWord", {
        enumerable: true,
        get: function get() {
            return _keyword.isReservedWord;
        }
    });
    Object.defineProperty(exports, "isStrictBindOnlyReservedWord", {
        enumerable: true,
        get: function get() {
            return _keyword.isStrictBindOnlyReservedWord;
        }
    });
    Object.defineProperty(exports, "isStrictBindReservedWord", {
        enumerable: true,
        get: function get() {
            return _keyword.isStrictBindReservedWord;
        }
    });
    Object.defineProperty(exports, "isStrictReservedWord", {
        enumerable: true,
        get: function get() {
            return _keyword.isStrictReservedWord;
        }
    });
    Object.defineProperty(exports, "isKeyword", {
        enumerable: true,
        get: function get() {
            return _keyword.isKeyword;
        }
    });
    var _identifier = identifier$1;
    var _keyword = keyword;
})(lib);

Object.defineProperty(isValidIdentifier$1, "__esModule", {
    value: true
});
isValidIdentifier$1.default = isValidIdentifier;
var _helperValidatorIdentifier$1 = lib;
function isValidIdentifier(name, reserved) {
    if (reserved === void 0) reserved = true;
    if (typeof name !== "string") return false;
    if (reserved) {
        if ((0, _helperValidatorIdentifier$1.isKeyword)(name) || (0, _helperValidatorIdentifier$1.isStrictReservedWord)(name, true)) {
            return false;
        }
    }
    return (0, _helperValidatorIdentifier$1.isIdentifierName)(name);
}

var constants = {};

function _arrayLikeToArray$3(arr, len) {
    if (len == null || len > arr.length) len = arr.length;
    for(var i = 0, arr2 = new Array(len); i < len; i++)arr2[i] = arr[i];
    return arr2;
}
function _arrayWithoutHoles$3(arr) {
    if (Array.isArray(arr)) return _arrayLikeToArray$3(arr);
}
function _iterableToArray$3(iter) {
    if (typeof Symbol !== "undefined" && iter[Symbol.iterator] != null || iter["@@iterator"] != null) return Array.from(iter);
}
function _nonIterableSpread$3() {
    throw new TypeError("Invalid attempt to spread non-iterable instance.\\nIn order to be iterable, non-array objects must have a [Symbol.iterator]() method.");
}
function _toConsumableArray$3(arr) {
    return _arrayWithoutHoles$3(arr) || _iterableToArray$3(arr) || _unsupportedIterableToArray$3(arr) || _nonIterableSpread$3();
}
function _unsupportedIterableToArray$3(o, minLen) {
    if (!o) return;
    if (typeof o === "string") return _arrayLikeToArray$3(o, minLen);
    var n = Object.prototype.toString.call(o).slice(8, -1);
    if (n === "Object" && o.constructor) n = o.constructor.name;
    if (n === "Map" || n === "Set") return Array.from(n);
    if (n === "Arguments" || /^(?:Ui|I)nt(?:8|16|32)(?:Clamped)?Array$/.test(n)) return _arrayLikeToArray$3(o, minLen);
}
Object.defineProperty(constants, "__esModule", {
    value: true
});
constants.NOT_LOCAL_BINDING = constants.BLOCK_SCOPED_SYMBOL = constants.INHERIT_KEYS = constants.UNARY_OPERATORS = constants.STRING_UNARY_OPERATORS = constants.NUMBER_UNARY_OPERATORS = constants.BOOLEAN_UNARY_OPERATORS = constants.ASSIGNMENT_OPERATORS = constants.BINARY_OPERATORS = constants.NUMBER_BINARY_OPERATORS = constants.BOOLEAN_BINARY_OPERATORS = constants.COMPARISON_BINARY_OPERATORS = constants.EQUALITY_BINARY_OPERATORS = constants.BOOLEAN_NUMBER_BINARY_OPERATORS = constants.UPDATE_OPERATORS = constants.LOGICAL_OPERATORS = constants.COMMENT_KEYS = constants.FOR_INIT_KEYS = constants.FLATTENABLE_KEYS = constants.STATEMENT_OR_BLOCK_KEYS = void 0;
var STATEMENT_OR_BLOCK_KEYS = [
    "consequent",
    "body",
    "alternate"
];
constants.STATEMENT_OR_BLOCK_KEYS = STATEMENT_OR_BLOCK_KEYS;
var FLATTENABLE_KEYS = [
    "body",
    "expressions"
];
constants.FLATTENABLE_KEYS = FLATTENABLE_KEYS;
var FOR_INIT_KEYS = [
    "left",
    "init"
];
constants.FOR_INIT_KEYS = FOR_INIT_KEYS;
var COMMENT_KEYS = [
    "leadingComments",
    "trailingComments",
    "innerComments"
];
constants.COMMENT_KEYS = COMMENT_KEYS;
var LOGICAL_OPERATORS = [
    "||",
    "&&",
    "??"
];
constants.LOGICAL_OPERATORS = LOGICAL_OPERATORS;
var UPDATE_OPERATORS = [
    "++",
    "--"
];
constants.UPDATE_OPERATORS = UPDATE_OPERATORS;
var BOOLEAN_NUMBER_BINARY_OPERATORS = [
    ">",
    "<",
    ">=",
    "<="
];
constants.BOOLEAN_NUMBER_BINARY_OPERATORS = BOOLEAN_NUMBER_BINARY_OPERATORS;
var EQUALITY_BINARY_OPERATORS = [
    "==",
    "===",
    "!=",
    "!=="
];
constants.EQUALITY_BINARY_OPERATORS = EQUALITY_BINARY_OPERATORS;
var COMPARISON_BINARY_OPERATORS = _toConsumableArray$3(EQUALITY_BINARY_OPERATORS).concat([
    "in",
    "instanceof"
]);
constants.COMPARISON_BINARY_OPERATORS = COMPARISON_BINARY_OPERATORS;
var BOOLEAN_BINARY_OPERATORS = _toConsumableArray$3(COMPARISON_BINARY_OPERATORS).concat(_toConsumableArray$3(BOOLEAN_NUMBER_BINARY_OPERATORS));
constants.BOOLEAN_BINARY_OPERATORS = BOOLEAN_BINARY_OPERATORS;
var NUMBER_BINARY_OPERATORS = [
    "-",
    "/",
    "%",
    "*",
    "**",
    "&",
    "|",
    ">>",
    ">>>",
    "<<",
    "^"
];
constants.NUMBER_BINARY_OPERATORS = NUMBER_BINARY_OPERATORS;
var BINARY_OPERATORS = [
    "+"
].concat(_toConsumableArray$3(NUMBER_BINARY_OPERATORS), _toConsumableArray$3(BOOLEAN_BINARY_OPERATORS));
constants.BINARY_OPERATORS = BINARY_OPERATORS;
var ASSIGNMENT_OPERATORS = [
    "=",
    "+="
].concat(_toConsumableArray$3(NUMBER_BINARY_OPERATORS.map(function(op) {
    return op + "=";
})), _toConsumableArray$3(LOGICAL_OPERATORS.map(function(op) {
    return op + "=";
})));
constants.ASSIGNMENT_OPERATORS = ASSIGNMENT_OPERATORS;
var BOOLEAN_UNARY_OPERATORS = [
    "delete",
    "!"
];
constants.BOOLEAN_UNARY_OPERATORS = BOOLEAN_UNARY_OPERATORS;
var NUMBER_UNARY_OPERATORS = [
    "+",
    "-",
    "~"
];
constants.NUMBER_UNARY_OPERATORS = NUMBER_UNARY_OPERATORS;
var STRING_UNARY_OPERATORS = [
    "typeof"
];
constants.STRING_UNARY_OPERATORS = STRING_UNARY_OPERATORS;
var UNARY_OPERATORS = [
    "void",
    "throw"
].concat(_toConsumableArray$3(BOOLEAN_UNARY_OPERATORS), _toConsumableArray$3(NUMBER_UNARY_OPERATORS), _toConsumableArray$3(STRING_UNARY_OPERATORS));
constants.UNARY_OPERATORS = UNARY_OPERATORS;
var INHERIT_KEYS = {
    optional: [
        "typeAnnotation",
        "typeParameters",
        "returnType"
    ],
    force: [
        "start",
        "loc",
        "end"
    ]
};
constants.INHERIT_KEYS = INHERIT_KEYS;
var BLOCK_SCOPED_SYMBOL = Symbol.for("var used to be block scoped");
constants.BLOCK_SCOPED_SYMBOL = BLOCK_SCOPED_SYMBOL;
var NOT_LOCAL_BINDING = Symbol.for("should not be considered a local binding");
constants.NOT_LOCAL_BINDING = NOT_LOCAL_BINDING;

var utils = {};

var validate = {};

var hasRequiredValidate;
function requireValidate() {
    var validate$1 = function validate(node, key, val) {
        if (!node) return;
        var fields = _definitions.NODE_FIELDS[node.type];
        if (!fields) return;
        var field = fields[key];
        validateField(node, key, val, field);
        validateChild(node, key, val);
    };
    var validateField = function validateField(node, key, val, field) {
        if (!(field != null && field.validate)) return;
        if (field.optional && val == null) return;
        field.validate(node, key, val);
    };
    var validateChild = function validateChild(node, key, val) {
        if (val == null) return;
        var validate = _definitions.NODE_PARENT_VALIDATIONS[val.type];
        if (!validate) return;
        validate(node, key, val);
    };
    if (hasRequiredValidate) return validate;
    hasRequiredValidate = 1;
    Object.defineProperty(validate, "__esModule", {
        value: true
    });
    validate.default = validate$1;
    validate.validateField = validateField;
    validate.validateChild = validateChild;
    var _definitions = requireDefinitions();
    return validate;
}

function _instanceof(left, right) {
    if (right != null && typeof Symbol !== "undefined" && right[Symbol.hasInstance]) {
        return !!right[Symbol.hasInstance](left);
    } else {
        return left instanceof right;
    }
}
var _typeof$3 = function(obj) {
    "@swc/helpers - typeof";
    return obj && typeof Symbol !== "undefined" && obj.constructor === Symbol ? "symbol" : typeof obj;
};
function _createForOfIteratorHelperLoose$g(o, allowArrayLike) {
    var it = typeof Symbol !== "undefined" && o[Symbol.iterator] || o["@@iterator"];
    if (it) return (it = it.call(o)).next.bind(it);
    if (Array.isArray(o) || (it = unsupportedIterableToArray(o)) || allowArrayLike && o && typeof o.length === "number") {
        if (it) o = it;
        var i = 0;
        return function() {
            if (i >= o.length) return {
                done: true
            };
            return {
                done: false,
                value: o[i++]
            };
        };
    }
    throw new TypeError("Invalid attempt to iterate non-iterable instance.\\nIn order to be iterable, non-array objects must have a [Symbol.iterator]() method.");
}
var hasRequiredUtils;
function requireUtils() {
    var getType = function getType(val) {
        if (Array.isArray(val)) {
            return "array";
        } else if (val === null) {
            return "null";
        } else {
            return typeof val === "undefined" ? "undefined" : _typeof$3(val);
        }
    };
    var validate = function validate(validate) {
        return {
            validate: validate
        };
    };
    var typeIs = function typeIs(typeName) {
        return typeof typeName === "string" ? assertNodeType(typeName) : assertNodeType.apply(void 0, typeName);
    };
    var validateType = function validateType(typeName) {
        return validate(typeIs(typeName));
    };
    var validateOptional = function validateOptional(validate) {
        return {
            validate: validate,
            optional: true
        };
    };
    var validateOptionalType = function validateOptionalType(typeName) {
        return {
            validate: typeIs(typeName),
            optional: true
        };
    };
    var arrayOf = function arrayOf(elementType) {
        return chain(assertValueType("array"), assertEach(elementType));
    };
    var arrayOfType = function arrayOfType(typeName) {
        return arrayOf(typeIs(typeName));
    };
    var validateArrayOfType = function validateArrayOfType(typeName) {
        return validate(arrayOfType(typeName));
    };
    var assertEach = function assertEach(callback) {
        function validator(node, key, val) {
            if (!Array.isArray(val)) return;
            for(var i = 0; i < val.length; i++){
                var subkey = key + "[" + i + "]";
                var v = val[i];
                callback(node, subkey, v);
                if (process.env.BABEL_TYPES_8_BREAKING) (0, _validate.validateChild)(node, subkey, v);
            }
        }
        validator.each = callback;
        return validator;
    };
    var assertOneOf = function assertOneOf() {
        for(var _len = arguments.length, values = new Array(_len), _key = 0; _key < _len; _key++){
            values[_key] = arguments[_key];
        }
        function validate(node, key, val) {
            if (values.indexOf(val) < 0) {
                throw new TypeError("Property " + key + " expected value to be one of " + JSON.stringify(values) + " but got " + JSON.stringify(val));
            }
        }
        validate.oneOf = values;
        return validate;
    };
    var assertNodeType = function assertNodeType() {
        for(var _len = arguments.length, types = new Array(_len), _key = 0; _key < _len; _key++){
            types[_key] = arguments[_key];
        }
        function validate(node, key, val) {
            for(var _iterator = _createForOfIteratorHelperLoose$g(types), _step; !(_step = _iterator()).done;){
                var type = _step.value;
                if ((0, _is.default)(type, val)) {
                    (0, _validate.validateChild)(node, key, val);
                    return;
                }
            }
            throw new TypeError("Property " + key + " of " + node.type + " expected node to be of a type " + JSON.stringify(types) + " but instead got " + JSON.stringify(val == null ? void 0 : val.type));
        }
        validate.oneOfNodeTypes = types;
        return validate;
    };
    var assertNodeOrValueType = function assertNodeOrValueType() {
        for(var _len = arguments.length, types = new Array(_len), _key = 0; _key < _len; _key++){
            types[_key] = arguments[_key];
        }
        function validate(node, key, val) {
            for(var _iterator = _createForOfIteratorHelperLoose$g(types), _step; !(_step = _iterator()).done;){
                var type = _step.value;
                if (getType(val) === type || (0, _is.default)(type, val)) {
                    (0, _validate.validateChild)(node, key, val);
                    return;
                }
            }
            throw new TypeError("Property " + key + " of " + node.type + " expected node to be of a type " + JSON.stringify(types) + " but instead got " + JSON.stringify(val == null ? void 0 : val.type));
        }
        validate.oneOfNodeOrValueTypes = types;
        return validate;
    };
    var assertValueType = function assertValueType(type) {
        function validate(node, key, val) {
            var valid = getType(val) === type;
            if (!valid) {
                throw new TypeError("Property " + key + " expected type of " + type + " but got " + getType(val));
            }
        }
        validate.type = type;
        return validate;
    };
    var assertShape = function assertShape(shape) {
        function validate(node, key, val) {
            var errors = [];
            for(var _iterator = _createForOfIteratorHelperLoose$g(Object.keys(shape)), _step; !(_step = _iterator()).done;){
                var property = _step.value;
                try {
                    (0, _validate.validateField)(node, property, val[property], shape[property]);
                } catch (error) {
                    if (_instanceof(error, TypeError)) {
                        errors.push(error.message);
                        continue;
                    }
                    throw error;
                }
            }
            if (errors.length) {
                throw new TypeError("Property " + key + " of " + node.type + " expected to have the following:\n" + errors.join("\n"));
            }
        }
        validate.shapeOf = shape;
        return validate;
    };
    var assertOptionalChainStart = function assertOptionalChainStart() {
        function validate(node) {
            var _current;
            var current = node;
            while(node){
                var type = current.type;
                if (type === "OptionalCallExpression") {
                    if (current.optional) return;
                    current = current.callee;
                    continue;
                }
                if (type === "OptionalMemberExpression") {
                    if (current.optional) return;
                    current = current.object;
                    continue;
                }
                break;
            }
            throw new TypeError("Non-optional " + node.type + " must chain from an optional OptionalMemberExpression or OptionalCallExpression. Found chain from " + ((_current = current) == null ? void 0 : _current.type));
        }
        return validate;
    };
    var chain = function chain() {
        for(var _len = arguments.length, fns = new Array(_len), _key = 0; _key < _len; _key++){
            fns[_key] = arguments[_key];
        }
        function validate() {
            for(var _len = arguments.length, args = new Array(_len), _key = 0; _key < _len; _key++){
                args[_key] = arguments[_key];
            }
            for(var _iterator = _createForOfIteratorHelperLoose$g(fns), _step; !(_step = _iterator()).done;){
                var fn = _step.value;
                fn.apply(void 0, args);
            }
        }
        validate.chainOf = fns;
        if (fns.length >= 2 && "type" in fns[0] && fns[0].type === "array" && !("each" in fns[1])) {
            throw new Error('An assertValueType("array") validator can only be followed by an assertEach(...) validator.');
        }
        return validate;
    };
    var defineType = function defineType(type, opts) {
        if (opts === void 0) opts = {};
        var inherits = opts.inherits && store[opts.inherits] || {};
        var fields = opts.fields;
        if (!fields) {
            fields = {};
            if (inherits.fields) {
                var keys = Object.getOwnPropertyNames(inherits.fields);
                for(var _iterator = _createForOfIteratorHelperLoose$g(keys), _step; !(_step = _iterator()).done;){
                    var key = _step.value;
                    var field = inherits.fields[key];
                    var def = field.default;
                    if (Array.isArray(def) ? def.length > 0 : def && typeof def === "object") {
                        throw new Error("field defaults can only be primitives or empty arrays currently");
                    }
                    fields[key] = {
                        default: Array.isArray(def) ? [] : def,
                        optional: field.optional,
                        validate: field.validate
                    };
                }
            }
        }
        var visitor = opts.visitor || inherits.visitor || [];
        var aliases = opts.aliases || inherits.aliases || [];
        var builder = opts.builder || inherits.builder || opts.visitor || [];
        for(var _iterator1 = _createForOfIteratorHelperLoose$g(Object.keys(opts)), _step1; !(_step1 = _iterator1()).done;){
            var k = _step1.value;
            if (validTypeOpts.indexOf(k) === -1) {
                throw new Error('Unknown type option "' + k + '" on ' + type);
            }
        }
        if (opts.deprecatedAlias) {
            DEPRECATED_KEYS[opts.deprecatedAlias] = type;
        }
        for(var _iterator2 = _createForOfIteratorHelperLoose$g(visitor.concat(builder)), _step2; !(_step2 = _iterator2()).done;){
            var key1 = _step2.value;
            fields[key1] = fields[key1] || {};
        }
        for(var _iterator3 = _createForOfIteratorHelperLoose$g(Object.keys(fields)), _step3; !(_step3 = _iterator3()).done;){
            var key2 = _step3.value;
            var field1 = fields[key2];
            if (field1.default !== undefined && builder.indexOf(key2) === -1) {
                field1.optional = true;
            }
            if (field1.default === undefined) {
                field1.default = null;
            } else if (!field1.validate && field1.default != null) {
                field1.validate = assertValueType(getType(field1.default));
            }
            for(var _iterator4 = _createForOfIteratorHelperLoose$g(Object.keys(field1)), _step4; !(_step4 = _iterator4()).done;){
                var k1 = _step4.value;
                if (validFieldKeys.indexOf(k1) === -1) {
                    throw new Error('Unknown field key "' + k1 + '" on ' + type + "." + key2);
                }
            }
        }
        VISITOR_KEYS[type] = opts.visitor = visitor;
        BUILDER_KEYS[type] = opts.builder = builder;
        NODE_FIELDS[type] = opts.fields = fields;
        ALIAS_KEYS[type] = opts.aliases = aliases;
        aliases.forEach(function(alias) {
            FLIPPED_ALIAS_KEYS[alias] = FLIPPED_ALIAS_KEYS[alias] || [];
            FLIPPED_ALIAS_KEYS[alias].push(type);
        });
        if (opts.validate) {
            NODE_PARENT_VALIDATIONS[type] = opts.validate;
        }
        store[type] = opts;
    };
    if (hasRequiredUtils) return utils;
    hasRequiredUtils = 1;
    Object.defineProperty(utils, "__esModule", {
        value: true
    });
    utils.validate = validate;
    utils.typeIs = typeIs;
    utils.validateType = validateType;
    utils.validateOptional = validateOptional;
    utils.validateOptionalType = validateOptionalType;
    utils.arrayOf = arrayOf;
    utils.arrayOfType = arrayOfType;
    utils.validateArrayOfType = validateArrayOfType;
    utils.assertEach = assertEach;
    utils.assertOneOf = assertOneOf;
    utils.assertNodeType = assertNodeType;
    utils.assertNodeOrValueType = assertNodeOrValueType;
    utils.assertValueType = assertValueType;
    utils.assertShape = assertShape;
    utils.assertOptionalChainStart = assertOptionalChainStart;
    utils.chain = chain;
    utils.default = defineType;
    utils.NODE_PARENT_VALIDATIONS = utils.DEPRECATED_KEYS = utils.BUILDER_KEYS = utils.NODE_FIELDS = utils.FLIPPED_ALIAS_KEYS = utils.ALIAS_KEYS = utils.VISITOR_KEYS = void 0;
    var _is = requireIs();
    var _validate = requireValidate();
    var VISITOR_KEYS = {};
    utils.VISITOR_KEYS = VISITOR_KEYS;
    var ALIAS_KEYS = {};
    utils.ALIAS_KEYS = ALIAS_KEYS;
    var FLIPPED_ALIAS_KEYS = {};
    utils.FLIPPED_ALIAS_KEYS = FLIPPED_ALIAS_KEYS;
    var NODE_FIELDS = {};
    utils.NODE_FIELDS = NODE_FIELDS;
    var BUILDER_KEYS = {};
    utils.BUILDER_KEYS = BUILDER_KEYS;
    var DEPRECATED_KEYS = {};
    utils.DEPRECATED_KEYS = DEPRECATED_KEYS;
    var NODE_PARENT_VALIDATIONS = {};
    utils.NODE_PARENT_VALIDATIONS = NODE_PARENT_VALIDATIONS;
    var validTypeOpts = [
        "aliases",
        "builder",
        "deprecatedAlias",
        "fields",
        "inherits",
        "visitor",
        "validate"
    ];
    var validFieldKeys = [
        "default",
        "optional",
        "validate"
    ];
    var store = {};
    return utils;
}

function _arrayLikeToArray$2(arr, len) {
    if (len == null || len > arr.length) len = arr.length;
    for(var i = 0, arr2 = new Array(len); i < len; i++)arr2[i] = arr[i];
    return arr2;
}
function _arrayWithoutHoles$2(arr) {
    if (Array.isArray(arr)) return _arrayLikeToArray$2(arr);
}
function _iterableToArray$2(iter) {
    if (typeof Symbol !== "undefined" && iter[Symbol.iterator] != null || iter["@@iterator"] != null) return Array.from(iter);
}
function _nonIterableSpread$2() {
    throw new TypeError("Invalid attempt to spread non-iterable instance.\\nIn order to be iterable, non-array objects must have a [Symbol.iterator]() method.");
}
function _toConsumableArray$2(arr) {
    return _arrayWithoutHoles$2(arr) || _iterableToArray$2(arr) || _unsupportedIterableToArray$2(arr) || _nonIterableSpread$2();
}
function _unsupportedIterableToArray$2(o, minLen) {
    if (!o) return;
    if (typeof o === "string") return _arrayLikeToArray$2(o, minLen);
    var n = Object.prototype.toString.call(o).slice(8, -1);
    if (n === "Object" && o.constructor) n = o.constructor.name;
    if (n === "Map" || n === "Set") return Array.from(n);
    if (n === "Arguments" || /^(?:Ui|I)nt(?:8|16|32)(?:Clamped)?Array$/.test(n)) return _arrayLikeToArray$2(o, minLen);
}
var hasRequiredCore;
function requireCore() {
    if (hasRequiredCore) return core;
    hasRequiredCore = 1;
    Object.defineProperty(core, "__esModule", {
        value: true
    });
    core.classMethodOrDeclareMethodCommon = core.classMethodOrPropertyCommon = core.patternLikeCommon = core.functionDeclarationCommon = core.functionTypeAnnotationCommon = core.functionCommon = void 0;
    var _is = requireIs();
    var _isValidIdentifier = isValidIdentifier$1;
    var _helperValidatorIdentifier = lib;
    var _constants = constants;
    var _utils = requireUtils();
    (0, _utils.default)("ArrayExpression", {
        fields: {
            elements: {
                validate: (0, _utils.chain)((0, _utils.assertValueType)("array"), (0, _utils.assertEach)((0, _utils.assertNodeOrValueType)("null", "Expression", "SpreadElement"))),
                default: !process.env.BABEL_TYPES_8_BREAKING ? [] : undefined
            }
        },
        visitor: [
            "elements"
        ],
        aliases: [
            "Expression"
        ]
    });
    (0, _utils.default)("AssignmentExpression", {
        fields: {
            operator: {
                validate: function() {
                    if (!process.env.BABEL_TYPES_8_BREAKING) {
                        return (0, _utils.assertValueType)("string");
                    }
                    var identifier = (_utils.assertOneOf).apply(this, _constants.ASSIGNMENT_OPERATORS);
                    var pattern = (0, _utils.assertOneOf)("=");
                    return function(node, key, val) {
                        var validator = (0, _is.default)("Pattern", node.left) ? pattern : identifier;
                        validator(node, key, val);
                    };
                }()
            },
            left: {
                validate: !process.env.BABEL_TYPES_8_BREAKING ? (0, _utils.assertNodeType)("LVal") : (0, _utils.assertNodeType)("Identifier", "MemberExpression", "ArrayPattern", "ObjectPattern")
            },
            right: {
                validate: (0, _utils.assertNodeType)("Expression")
            }
        },
        builder: [
            "operator",
            "left",
            "right"
        ],
        visitor: [
            "left",
            "right"
        ],
        aliases: [
            "Expression"
        ]
    });
    (0, _utils.default)("BinaryExpression", {
        builder: [
            "operator",
            "left",
            "right"
        ],
        fields: {
            operator: {
                validate: (_utils.assertOneOf).apply(this, _constants.BINARY_OPERATORS)
            },
            left: {
                validate: function() {
                    var expression = (0, _utils.assertNodeType)("Expression");
                    var inOp = (0, _utils.assertNodeType)("Expression", "PrivateName");
                    var validator = function validator(node, key, val) {
                        var validator = node.operator === "in" ? inOp : expression;
                        validator(node, key, val);
                    };
                    validator.oneOfNodeTypes = [
                        "Expression",
                        "PrivateName"
                    ];
                    return validator;
                }()
            },
            right: {
                validate: (0, _utils.assertNodeType)("Expression")
            }
        },
        visitor: [
            "left",
            "right"
        ],
        aliases: [
            "Binary",
            "Expression"
        ]
    });
    (0, _utils.default)("InterpreterDirective", {
        builder: [
            "value"
        ],
        fields: {
            value: {
                validate: (0, _utils.assertValueType)("string")
            }
        }
    });
    (0, _utils.default)("Directive", {
        visitor: [
            "value"
        ],
        fields: {
            value: {
                validate: (0, _utils.assertNodeType)("DirectiveLiteral")
            }
        }
    });
    (0, _utils.default)("DirectiveLiteral", {
        builder: [
            "value"
        ],
        fields: {
            value: {
                validate: (0, _utils.assertValueType)("string")
            }
        }
    });
    (0, _utils.default)("BlockStatement", {
        builder: [
            "body",
            "directives"
        ],
        visitor: [
            "directives",
            "body"
        ],
        fields: {
            directives: {
                validate: (0, _utils.chain)((0, _utils.assertValueType)("array"), (0, _utils.assertEach)((0, _utils.assertNodeType)("Directive"))),
                default: []
            },
            body: {
                validate: (0, _utils.chain)((0, _utils.assertValueType)("array"), (0, _utils.assertEach)((0, _utils.assertNodeType)("Statement")))
            }
        },
        aliases: [
            "Scopable",
            "BlockParent",
            "Block",
            "Statement"
        ]
    });
    (0, _utils.default)("BreakStatement", {
        visitor: [
            "label"
        ],
        fields: {
            label: {
                validate: (0, _utils.assertNodeType)("Identifier"),
                optional: true
            }
        },
        aliases: [
            "Statement",
            "Terminatorless",
            "CompletionStatement"
        ]
    });
    (0, _utils.default)("CallExpression", {
        visitor: [
            "callee",
            "arguments",
            "typeParameters",
            "typeArguments"
        ],
        builder: [
            "callee",
            "arguments"
        ],
        aliases: [
            "Expression"
        ],
        fields: Object.assign({
            callee: {
                validate: (0, _utils.assertNodeType)("Expression", "V8IntrinsicIdentifier")
            },
            arguments: {
                validate: (0, _utils.chain)((0, _utils.assertValueType)("array"), (0, _utils.assertEach)((0, _utils.assertNodeType)("Expression", "SpreadElement", "JSXNamespacedName", "ArgumentPlaceholder")))
            }
        }, !process.env.BABEL_TYPES_8_BREAKING ? {
            optional: {
                validate: (0, _utils.assertOneOf)(true, false),
                optional: true
            }
        } : {}, {
            typeArguments: {
                validate: (0, _utils.assertNodeType)("TypeParameterInstantiation"),
                optional: true
            },
            typeParameters: {
                validate: (0, _utils.assertNodeType)("TSTypeParameterInstantiation"),
                optional: true
            }
        })
    });
    (0, _utils.default)("CatchClause", {
        visitor: [
            "param",
            "body"
        ],
        fields: {
            param: {
                validate: (0, _utils.assertNodeType)("Identifier", "ArrayPattern", "ObjectPattern"),
                optional: true
            },
            body: {
                validate: (0, _utils.assertNodeType)("BlockStatement")
            }
        },
        aliases: [
            "Scopable",
            "BlockParent"
        ]
    });
    (0, _utils.default)("ConditionalExpression", {
        visitor: [
            "test",
            "consequent",
            "alternate"
        ],
        fields: {
            test: {
                validate: (0, _utils.assertNodeType)("Expression")
            },
            consequent: {
                validate: (0, _utils.assertNodeType)("Expression")
            },
            alternate: {
                validate: (0, _utils.assertNodeType)("Expression")
            }
        },
        aliases: [
            "Expression",
            "Conditional"
        ]
    });
    (0, _utils.default)("ContinueStatement", {
        visitor: [
            "label"
        ],
        fields: {
            label: {
                validate: (0, _utils.assertNodeType)("Identifier"),
                optional: true
            }
        },
        aliases: [
            "Statement",
            "Terminatorless",
            "CompletionStatement"
        ]
    });
    (0, _utils.default)("DebuggerStatement", {
        aliases: [
            "Statement"
        ]
    });
    (0, _utils.default)("DoWhileStatement", {
        visitor: [
            "test",
            "body"
        ],
        fields: {
            test: {
                validate: (0, _utils.assertNodeType)("Expression")
            },
            body: {
                validate: (0, _utils.assertNodeType)("Statement")
            }
        },
        aliases: [
            "Statement",
            "BlockParent",
            "Loop",
            "While",
            "Scopable"
        ]
    });
    (0, _utils.default)("EmptyStatement", {
        aliases: [
            "Statement"
        ]
    });
    (0, _utils.default)("ExpressionStatement", {
        visitor: [
            "expression"
        ],
        fields: {
            expression: {
                validate: (0, _utils.assertNodeType)("Expression")
            }
        },
        aliases: [
            "Statement",
            "ExpressionWrapper"
        ]
    });
    (0, _utils.default)("File", {
        builder: [
            "program",
            "comments",
            "tokens"
        ],
        visitor: [
            "program"
        ],
        fields: {
            program: {
                validate: (0, _utils.assertNodeType)("Program")
            },
            comments: {
                validate: !process.env.BABEL_TYPES_8_BREAKING ? Object.assign(function() {}, {
                    each: {
                        oneOfNodeTypes: [
                            "CommentBlock",
                            "CommentLine"
                        ]
                    }
                }) : (0, _utils.assertEach)((0, _utils.assertNodeType)("CommentBlock", "CommentLine")),
                optional: true
            },
            tokens: {
                validate: (0, _utils.assertEach)(Object.assign(function() {}, {
                    type: "any"
                })),
                optional: true
            }
        }
    });
    (0, _utils.default)("ForInStatement", {
        visitor: [
            "left",
            "right",
            "body"
        ],
        aliases: [
            "Scopable",
            "Statement",
            "For",
            "BlockParent",
            "Loop",
            "ForXStatement"
        ],
        fields: {
            left: {
                validate: !process.env.BABEL_TYPES_8_BREAKING ? (0, _utils.assertNodeType)("VariableDeclaration", "LVal") : (0, _utils.assertNodeType)("VariableDeclaration", "Identifier", "MemberExpression", "ArrayPattern", "ObjectPattern")
            },
            right: {
                validate: (0, _utils.assertNodeType)("Expression")
            },
            body: {
                validate: (0, _utils.assertNodeType)("Statement")
            }
        }
    });
    (0, _utils.default)("ForStatement", {
        visitor: [
            "init",
            "test",
            "update",
            "body"
        ],
        aliases: [
            "Scopable",
            "Statement",
            "For",
            "BlockParent",
            "Loop"
        ],
        fields: {
            init: {
                validate: (0, _utils.assertNodeType)("VariableDeclaration", "Expression"),
                optional: true
            },
            test: {
                validate: (0, _utils.assertNodeType)("Expression"),
                optional: true
            },
            update: {
                validate: (0, _utils.assertNodeType)("Expression"),
                optional: true
            },
            body: {
                validate: (0, _utils.assertNodeType)("Statement")
            }
        }
    });
    var functionCommon = {
        params: {
            validate: (0, _utils.chain)((0, _utils.assertValueType)("array"), (0, _utils.assertEach)((0, _utils.assertNodeType)("Identifier", "Pattern", "RestElement")))
        },
        generator: {
            default: false
        },
        async: {
            default: false
        }
    };
    core.functionCommon = functionCommon;
    var functionTypeAnnotationCommon = {
        returnType: {
            validate: (0, _utils.assertNodeType)("TypeAnnotation", "TSTypeAnnotation", "Noop"),
            optional: true
        },
        typeParameters: {
            validate: (0, _utils.assertNodeType)("TypeParameterDeclaration", "TSTypeParameterDeclaration", "Noop"),
            optional: true
        }
    };
    core.functionTypeAnnotationCommon = functionTypeAnnotationCommon;
    var functionDeclarationCommon = Object.assign({}, functionCommon, {
        declare: {
            validate: (0, _utils.assertValueType)("boolean"),
            optional: true
        },
        id: {
            validate: (0, _utils.assertNodeType)("Identifier"),
            optional: true
        }
    });
    core.functionDeclarationCommon = functionDeclarationCommon;
    (0, _utils.default)("FunctionDeclaration", {
        builder: [
            "id",
            "params",
            "body",
            "generator",
            "async"
        ],
        visitor: [
            "id",
            "params",
            "body",
            "returnType",
            "typeParameters"
        ],
        fields: Object.assign({}, functionDeclarationCommon, functionTypeAnnotationCommon, {
            body: {
                validate: (0, _utils.assertNodeType)("BlockStatement")
            }
        }),
        aliases: [
            "Scopable",
            "Function",
            "BlockParent",
            "FunctionParent",
            "Statement",
            "Pureish",
            "Declaration"
        ],
        validate: function() {
            if (!process.env.BABEL_TYPES_8_BREAKING) return function() {};
            var identifier = (0, _utils.assertNodeType)("Identifier");
            return function(parent, key, node) {
                if (!(0, _is.default)("ExportDefaultDeclaration", parent)) {
                    identifier(node, "id", node.id);
                }
            };
        }()
    });
    (0, _utils.default)("FunctionExpression", {
        inherits: "FunctionDeclaration",
        aliases: [
            "Scopable",
            "Function",
            "BlockParent",
            "FunctionParent",
            "Expression",
            "Pureish"
        ],
        fields: Object.assign({}, functionCommon, functionTypeAnnotationCommon, {
            id: {
                validate: (0, _utils.assertNodeType)("Identifier"),
                optional: true
            },
            body: {
                validate: (0, _utils.assertNodeType)("BlockStatement")
            }
        })
    });
    var patternLikeCommon = {
        typeAnnotation: {
            validate: (0, _utils.assertNodeType)("TypeAnnotation", "TSTypeAnnotation", "Noop"),
            optional: true
        },
        decorators: {
            validate: (0, _utils.chain)((0, _utils.assertValueType)("array"), (0, _utils.assertEach)((0, _utils.assertNodeType)("Decorator")))
        }
    };
    core.patternLikeCommon = patternLikeCommon;
    (0, _utils.default)("Identifier", {
        builder: [
            "name"
        ],
        visitor: [
            "typeAnnotation",
            "decorators"
        ],
        aliases: [
            "Expression",
            "PatternLike",
            "LVal",
            "TSEntityName"
        ],
        fields: Object.assign({}, patternLikeCommon, {
            name: {
                validate: (0, _utils.chain)((0, _utils.assertValueType)("string"), Object.assign(function(node, key, val) {
                    if (!process.env.BABEL_TYPES_8_BREAKING) return;
                    if (!(0, _isValidIdentifier.default)(val, false)) {
                        throw new TypeError('"' + val + '" is not a valid identifier name');
                    }
                }, {
                    type: "string"
                }))
            },
            optional: {
                validate: (0, _utils.assertValueType)("boolean"),
                optional: true
            }
        }),
        validate: function validate(parent, key, node) {
            if (!process.env.BABEL_TYPES_8_BREAKING) return;
            var match = /\.(\w+)$/.exec(key);
            if (!match) return;
            var parentKey = match[1];
            var nonComp = {
                computed: false
            };
            if (parentKey === "property") {
                if ((0, _is.default)("MemberExpression", parent, nonComp)) return;
                if ((0, _is.default)("OptionalMemberExpression", parent, nonComp)) return;
            } else if (parentKey === "key") {
                if ((0, _is.default)("Property", parent, nonComp)) return;
                if ((0, _is.default)("Method", parent, nonComp)) return;
            } else if (parentKey === "exported") {
                if ((0, _is.default)("ExportSpecifier", parent)) return;
            } else if (parentKey === "imported") {
                if ((0, _is.default)("ImportSpecifier", parent, {
                    imported: node
                })) return;
            } else if (parentKey === "meta") {
                if ((0, _is.default)("MetaProperty", parent, {
                    meta: node
                })) return;
            }
            if (((0, _helperValidatorIdentifier.isKeyword)(node.name) || (0, _helperValidatorIdentifier.isReservedWord)(node.name, false)) && node.name !== "this") {
                throw new TypeError('"' + node.name + '" is not a valid identifier');
            }
        }
    });
    (0, _utils.default)("IfStatement", {
        visitor: [
            "test",
            "consequent",
            "alternate"
        ],
        aliases: [
            "Statement",
            "Conditional"
        ],
        fields: {
            test: {
                validate: (0, _utils.assertNodeType)("Expression")
            },
            consequent: {
                validate: (0, _utils.assertNodeType)("Statement")
            },
            alternate: {
                optional: true,
                validate: (0, _utils.assertNodeType)("Statement")
            }
        }
    });
    (0, _utils.default)("LabeledStatement", {
        visitor: [
            "label",
            "body"
        ],
        aliases: [
            "Statement"
        ],
        fields: {
            label: {
                validate: (0, _utils.assertNodeType)("Identifier")
            },
            body: {
                validate: (0, _utils.assertNodeType)("Statement")
            }
        }
    });
    (0, _utils.default)("StringLiteral", {
        builder: [
            "value"
        ],
        fields: {
            value: {
                validate: (0, _utils.assertValueType)("string")
            }
        },
        aliases: [
            "Expression",
            "Pureish",
            "Literal",
            "Immutable"
        ]
    });
    (0, _utils.default)("NumericLiteral", {
        builder: [
            "value"
        ],
        deprecatedAlias: "NumberLiteral",
        fields: {
            value: {
                validate: (0, _utils.assertValueType)("number")
            }
        },
        aliases: [
            "Expression",
            "Pureish",
            "Literal",
            "Immutable"
        ]
    });
    (0, _utils.default)("NullLiteral", {
        aliases: [
            "Expression",
            "Pureish",
            "Literal",
            "Immutable"
        ]
    });
    (0, _utils.default)("BooleanLiteral", {
        builder: [
            "value"
        ],
        fields: {
            value: {
                validate: (0, _utils.assertValueType)("boolean")
            }
        },
        aliases: [
            "Expression",
            "Pureish",
            "Literal",
            "Immutable"
        ]
    });
    (0, _utils.default)("RegExpLiteral", {
        builder: [
            "pattern",
            "flags"
        ],
        deprecatedAlias: "RegexLiteral",
        aliases: [
            "Expression",
            "Pureish",
            "Literal"
        ],
        fields: {
            pattern: {
                validate: (0, _utils.assertValueType)("string")
            },
            flags: {
                validate: (0, _utils.chain)((0, _utils.assertValueType)("string"), Object.assign(function(node, key, val) {
                    if (!process.env.BABEL_TYPES_8_BREAKING) return;
                    var invalid = /[^gimsuy]/.exec(val);
                    if (invalid) {
                        throw new TypeError('"' + invalid[0] + '" is not a valid RegExp flag');
                    }
                }, {
                    type: "string"
                })),
                default: ""
            }
        }
    });
    (0, _utils.default)("LogicalExpression", {
        builder: [
            "operator",
            "left",
            "right"
        ],
        visitor: [
            "left",
            "right"
        ],
        aliases: [
            "Binary",
            "Expression"
        ],
        fields: {
            operator: {
                validate: (_utils.assertOneOf).apply(this, _constants.LOGICAL_OPERATORS)
            },
            left: {
                validate: (0, _utils.assertNodeType)("Expression")
            },
            right: {
                validate: (0, _utils.assertNodeType)("Expression")
            }
        }
    });
    (0, _utils.default)("MemberExpression", {
        builder: [
            "object",
            "property",
            "computed"
        ].concat(_toConsumableArray$2(!process.env.BABEL_TYPES_8_BREAKING ? [
            "optional"
        ] : [])),
        visitor: [
            "object",
            "property"
        ],
        aliases: [
            "Expression",
            "LVal"
        ],
        fields: Object.assign({
            object: {
                validate: (0, _utils.assertNodeType)("Expression")
            },
            property: {
                validate: function() {
                    var normal = (0, _utils.assertNodeType)("Identifier", "PrivateName");
                    var computed = (0, _utils.assertNodeType)("Expression");
                    var validator = function validator(node, key, val) {
                        var validator = node.computed ? computed : normal;
                        validator(node, key, val);
                    };
                    validator.oneOfNodeTypes = [
                        "Expression",
                        "Identifier",
                        "PrivateName"
                    ];
                    return validator;
                }()
            },
            computed: {
                default: false
            }
        }, !process.env.BABEL_TYPES_8_BREAKING ? {
            optional: {
                validate: (0, _utils.assertOneOf)(true, false),
                optional: true
            }
        } : {})
    });
    (0, _utils.default)("NewExpression", {
        inherits: "CallExpression"
    });
    (0, _utils.default)("Program", {
        visitor: [
            "directives",
            "body"
        ],
        builder: [
            "body",
            "directives",
            "sourceType",
            "interpreter"
        ],
        fields: {
            sourceFile: {
                validate: (0, _utils.assertValueType)("string")
            },
            sourceType: {
                validate: (0, _utils.assertOneOf)("script", "module"),
                default: "script"
            },
            interpreter: {
                validate: (0, _utils.assertNodeType)("InterpreterDirective"),
                default: null,
                optional: true
            },
            directives: {
                validate: (0, _utils.chain)((0, _utils.assertValueType)("array"), (0, _utils.assertEach)((0, _utils.assertNodeType)("Directive"))),
                default: []
            },
            body: {
                validate: (0, _utils.chain)((0, _utils.assertValueType)("array"), (0, _utils.assertEach)((0, _utils.assertNodeType)("Statement")))
            }
        },
        aliases: [
            "Scopable",
            "BlockParent",
            "Block"
        ]
    });
    (0, _utils.default)("ObjectExpression", {
        visitor: [
            "properties"
        ],
        aliases: [
            "Expression"
        ],
        fields: {
            properties: {
                validate: (0, _utils.chain)((0, _utils.assertValueType)("array"), (0, _utils.assertEach)((0, _utils.assertNodeType)("ObjectMethod", "ObjectProperty", "SpreadElement")))
            }
        }
    });
    (0, _utils.default)("ObjectMethod", {
        builder: [
            "kind",
            "key",
            "params",
            "body",
            "computed",
            "generator",
            "async"
        ],
        fields: Object.assign({}, functionCommon, functionTypeAnnotationCommon, {
            kind: Object.assign({
                validate: (0, _utils.assertOneOf)("method", "get", "set")
            }, !process.env.BABEL_TYPES_8_BREAKING ? {
                default: "method"
            } : {}),
            computed: {
                default: false
            },
            key: {
                validate: function() {
                    var normal = (0, _utils.assertNodeType)("Identifier", "StringLiteral", "NumericLiteral");
                    var computed = (0, _utils.assertNodeType)("Expression");
                    var validator = function validator(node, key, val) {
                        var validator = node.computed ? computed : normal;
                        validator(node, key, val);
                    };
                    validator.oneOfNodeTypes = [
                        "Expression",
                        "Identifier",
                        "StringLiteral",
                        "NumericLiteral"
                    ];
                    return validator;
                }()
            },
            decorators: {
                validate: (0, _utils.chain)((0, _utils.assertValueType)("array"), (0, _utils.assertEach)((0, _utils.assertNodeType)("Decorator"))),
                optional: true
            },
            body: {
                validate: (0, _utils.assertNodeType)("BlockStatement")
            }
        }),
        visitor: [
            "key",
            "params",
            "body",
            "decorators",
            "returnType",
            "typeParameters"
        ],
        aliases: [
            "UserWhitespacable",
            "Function",
            "Scopable",
            "BlockParent",
            "FunctionParent",
            "Method",
            "ObjectMember"
        ]
    });
    (0, _utils.default)("ObjectProperty", {
        builder: [
            "key",
            "value",
            "computed",
            "shorthand"
        ].concat(_toConsumableArray$2(!process.env.BABEL_TYPES_8_BREAKING ? [
            "decorators"
        ] : [])),
        fields: {
            computed: {
                default: false
            },
            key: {
                validate: function() {
                    var normal = (0, _utils.assertNodeType)("Identifier", "StringLiteral", "NumericLiteral");
                    var computed = (0, _utils.assertNodeType)("Expression");
                    var validator = function validator(node, key, val) {
                        var validator = node.computed ? computed : normal;
                        validator(node, key, val);
                    };
                    validator.oneOfNodeTypes = [
                        "Expression",
                        "Identifier",
                        "StringLiteral",
                        "NumericLiteral"
                    ];
                    return validator;
                }()
            },
            value: {
                validate: (0, _utils.assertNodeType)("Expression", "PatternLike")
            },
            shorthand: {
                validate: (0, _utils.chain)((0, _utils.assertValueType)("boolean"), Object.assign(function(node, key, val) {
                    if (!process.env.BABEL_TYPES_8_BREAKING) return;
                    if (val && node.computed) {
                        throw new TypeError("Property shorthand of ObjectProperty cannot be true if computed is true");
                    }
                }, {
                    type: "boolean"
                }), function(node, key, val) {
                    if (!process.env.BABEL_TYPES_8_BREAKING) return;
                    if (val && !(0, _is.default)("Identifier", node.key)) {
                        throw new TypeError("Property shorthand of ObjectProperty cannot be true if key is not an Identifier");
                    }
                }),
                default: false
            },
            decorators: {
                validate: (0, _utils.chain)((0, _utils.assertValueType)("array"), (0, _utils.assertEach)((0, _utils.assertNodeType)("Decorator"))),
                optional: true
            }
        },
        visitor: [
            "key",
            "value",
            "decorators"
        ],
        aliases: [
            "UserWhitespacable",
            "Property",
            "ObjectMember"
        ],
        validate: function() {
            var pattern = (0, _utils.assertNodeType)("Identifier", "Pattern");
            var expression = (0, _utils.assertNodeType)("Expression");
            return function(parent, key, node) {
                if (!process.env.BABEL_TYPES_8_BREAKING) return;
                var validator = (0, _is.default)("ObjectPattern", parent) ? pattern : expression;
                validator(node, "value", node.value);
            };
        }()
    });
    (0, _utils.default)("RestElement", {
        visitor: [
            "argument",
            "typeAnnotation"
        ],
        builder: [
            "argument"
        ],
        aliases: [
            "LVal",
            "PatternLike"
        ],
        deprecatedAlias: "RestProperty",
        fields: Object.assign({}, patternLikeCommon, {
            argument: {
                validate: !process.env.BABEL_TYPES_8_BREAKING ? (0, _utils.assertNodeType)("LVal") : (0, _utils.assertNodeType)("Identifier", "Pattern", "MemberExpression")
            },
            optional: {
                validate: (0, _utils.assertValueType)("boolean"),
                optional: true
            }
        }),
        validate: function validate(parent, key) {
            if (!process.env.BABEL_TYPES_8_BREAKING) return;
            var match = /(\w+)\[(\d+)\]/.exec(key);
            if (!match) throw new Error("Internal Babel error: malformed key.");
            var listKey = match[1], index = match[2];
            if (parent[listKey].length > index + 1) {
                throw new TypeError("RestElement must be last element of " + listKey);
            }
        }
    });
    (0, _utils.default)("ReturnStatement", {
        visitor: [
            "argument"
        ],
        aliases: [
            "Statement",
            "Terminatorless",
            "CompletionStatement"
        ],
        fields: {
            argument: {
                validate: (0, _utils.assertNodeType)("Expression"),
                optional: true
            }
        }
    });
    (0, _utils.default)("SequenceExpression", {
        visitor: [
            "expressions"
        ],
        fields: {
            expressions: {
                validate: (0, _utils.chain)((0, _utils.assertValueType)("array"), (0, _utils.assertEach)((0, _utils.assertNodeType)("Expression")))
            }
        },
        aliases: [
            "Expression"
        ]
    });
    (0, _utils.default)("ParenthesizedExpression", {
        visitor: [
            "expression"
        ],
        aliases: [
            "Expression",
            "ExpressionWrapper"
        ],
        fields: {
            expression: {
                validate: (0, _utils.assertNodeType)("Expression")
            }
        }
    });
    (0, _utils.default)("SwitchCase", {
        visitor: [
            "test",
            "consequent"
        ],
        fields: {
            test: {
                validate: (0, _utils.assertNodeType)("Expression"),
                optional: true
            },
            consequent: {
                validate: (0, _utils.chain)((0, _utils.assertValueType)("array"), (0, _utils.assertEach)((0, _utils.assertNodeType)("Statement")))
            }
        }
    });
    (0, _utils.default)("SwitchStatement", {
        visitor: [
            "discriminant",
            "cases"
        ],
        aliases: [
            "Statement",
            "BlockParent",
            "Scopable"
        ],
        fields: {
            discriminant: {
                validate: (0, _utils.assertNodeType)("Expression")
            },
            cases: {
                validate: (0, _utils.chain)((0, _utils.assertValueType)("array"), (0, _utils.assertEach)((0, _utils.assertNodeType)("SwitchCase")))
            }
        }
    });
    (0, _utils.default)("ThisExpression", {
        aliases: [
            "Expression"
        ]
    });
    (0, _utils.default)("ThrowStatement", {
        visitor: [
            "argument"
        ],
        aliases: [
            "Statement",
            "Terminatorless",
            "CompletionStatement"
        ],
        fields: {
            argument: {
                validate: (0, _utils.assertNodeType)("Expression")
            }
        }
    });
    (0, _utils.default)("TryStatement", {
        visitor: [
            "block",
            "handler",
            "finalizer"
        ],
        aliases: [
            "Statement"
        ],
        fields: {
            block: {
                validate: (0, _utils.chain)((0, _utils.assertNodeType)("BlockStatement"), Object.assign(function(node) {
                    if (!process.env.BABEL_TYPES_8_BREAKING) return;
                    if (!node.handler && !node.finalizer) {
                        throw new TypeError("TryStatement expects either a handler or finalizer, or both");
                    }
                }, {
                    oneOfNodeTypes: [
                        "BlockStatement"
                    ]
                }))
            },
            handler: {
                optional: true,
                validate: (0, _utils.assertNodeType)("CatchClause")
            },
            finalizer: {
                optional: true,
                validate: (0, _utils.assertNodeType)("BlockStatement")
            }
        }
    });
    (0, _utils.default)("UnaryExpression", {
        builder: [
            "operator",
            "argument",
            "prefix"
        ],
        fields: {
            prefix: {
                default: true
            },
            argument: {
                validate: (0, _utils.assertNodeType)("Expression")
            },
            operator: {
                validate: (_utils.assertOneOf).apply(this, _constants.UNARY_OPERATORS)
            }
        },
        visitor: [
            "argument"
        ],
        aliases: [
            "UnaryLike",
            "Expression"
        ]
    });
    (0, _utils.default)("UpdateExpression", {
        builder: [
            "operator",
            "argument",
            "prefix"
        ],
        fields: {
            prefix: {
                default: false
            },
            argument: {
                validate: !process.env.BABEL_TYPES_8_BREAKING ? (0, _utils.assertNodeType)("Expression") : (0, _utils.assertNodeType)("Identifier", "MemberExpression")
            },
            operator: {
                validate: (_utils.assertOneOf).apply(this, _constants.UPDATE_OPERATORS)
            }
        },
        visitor: [
            "argument"
        ],
        aliases: [
            "Expression"
        ]
    });
    (0, _utils.default)("VariableDeclaration", {
        builder: [
            "kind",
            "declarations"
        ],
        visitor: [
            "declarations"
        ],
        aliases: [
            "Statement",
            "Declaration"
        ],
        fields: {
            declare: {
                validate: (0, _utils.assertValueType)("boolean"),
                optional: true
            },
            kind: {
                validate: (0, _utils.assertOneOf)("var", "let", "const")
            },
            declarations: {
                validate: (0, _utils.chain)((0, _utils.assertValueType)("array"), (0, _utils.assertEach)((0, _utils.assertNodeType)("VariableDeclarator")))
            }
        },
        validate: function validate(parent, key, node) {
            if (!process.env.BABEL_TYPES_8_BREAKING) return;
            if (!(0, _is.default)("ForXStatement", parent, {
                left: node
            })) return;
            if (node.declarations.length !== 1) {
                throw new TypeError("Exactly one VariableDeclarator is required in the VariableDeclaration of a " + parent.type);
            }
        }
    });
    (0, _utils.default)("VariableDeclarator", {
        visitor: [
            "id",
            "init"
        ],
        fields: {
            id: {
                validate: function() {
                    if (!process.env.BABEL_TYPES_8_BREAKING) {
                        return (0, _utils.assertNodeType)("LVal");
                    }
                    var normal = (0, _utils.assertNodeType)("Identifier", "ArrayPattern", "ObjectPattern");
                    var without = (0, _utils.assertNodeType)("Identifier");
                    return function(node, key, val) {
                        var validator = node.init ? normal : without;
                        validator(node, key, val);
                    };
                }()
            },
            definite: {
                optional: true,
                validate: (0, _utils.assertValueType)("boolean")
            },
            init: {
                optional: true,
                validate: (0, _utils.assertNodeType)("Expression")
            }
        }
    });
    (0, _utils.default)("WhileStatement", {
        visitor: [
            "test",
            "body"
        ],
        aliases: [
            "Statement",
            "BlockParent",
            "Loop",
            "While",
            "Scopable"
        ],
        fields: {
            test: {
                validate: (0, _utils.assertNodeType)("Expression")
            },
            body: {
                validate: (0, _utils.assertNodeType)("Statement")
            }
        }
    });
    (0, _utils.default)("WithStatement", {
        visitor: [
            "object",
            "body"
        ],
        aliases: [
            "Statement"
        ],
        fields: {
            object: {
                validate: (0, _utils.assertNodeType)("Expression")
            },
            body: {
                validate: (0, _utils.assertNodeType)("Statement")
            }
        }
    });
    (0, _utils.default)("AssignmentPattern", {
        visitor: [
            "left",
            "right",
            "decorators"
        ],
        builder: [
            "left",
            "right"
        ],
        aliases: [
            "Pattern",
            "PatternLike",
            "LVal"
        ],
        fields: Object.assign({}, patternLikeCommon, {
            left: {
                validate: (0, _utils.assertNodeType)("Identifier", "ObjectPattern", "ArrayPattern", "MemberExpression")
            },
            right: {
                validate: (0, _utils.assertNodeType)("Expression")
            },
            decorators: {
                validate: (0, _utils.chain)((0, _utils.assertValueType)("array"), (0, _utils.assertEach)((0, _utils.assertNodeType)("Decorator"))),
                optional: true
            }
        })
    });
    (0, _utils.default)("ArrayPattern", {
        visitor: [
            "elements",
            "typeAnnotation"
        ],
        builder: [
            "elements"
        ],
        aliases: [
            "Pattern",
            "PatternLike",
            "LVal"
        ],
        fields: Object.assign({}, patternLikeCommon, {
            elements: {
                validate: (0, _utils.chain)((0, _utils.assertValueType)("array"), (0, _utils.assertEach)((0, _utils.assertNodeOrValueType)("null", "PatternLike")))
            },
            decorators: {
                validate: (0, _utils.chain)((0, _utils.assertValueType)("array"), (0, _utils.assertEach)((0, _utils.assertNodeType)("Decorator"))),
                optional: true
            },
            optional: {
                validate: (0, _utils.assertValueType)("boolean"),
                optional: true
            }
        })
    });
    (0, _utils.default)("ArrowFunctionExpression", {
        builder: [
            "params",
            "body",
            "async"
        ],
        visitor: [
            "params",
            "body",
            "returnType",
            "typeParameters"
        ],
        aliases: [
            "Scopable",
            "Function",
            "BlockParent",
            "FunctionParent",
            "Expression",
            "Pureish"
        ],
        fields: Object.assign({}, functionCommon, functionTypeAnnotationCommon, {
            expression: {
                validate: (0, _utils.assertValueType)("boolean")
            },
            body: {
                validate: (0, _utils.assertNodeType)("BlockStatement", "Expression")
            }
        })
    });
    (0, _utils.default)("ClassBody", {
        visitor: [
            "body"
        ],
        fields: {
            body: {
                validate: (0, _utils.chain)((0, _utils.assertValueType)("array"), (0, _utils.assertEach)((0, _utils.assertNodeType)("ClassMethod", "ClassPrivateMethod", "ClassProperty", "ClassPrivateProperty", "TSDeclareMethod", "TSIndexSignature")))
            }
        }
    });
    (0, _utils.default)("ClassExpression", {
        builder: [
            "id",
            "superClass",
            "body",
            "decorators"
        ],
        visitor: [
            "id",
            "body",
            "superClass",
            "mixins",
            "typeParameters",
            "superTypeParameters",
            "implements",
            "decorators"
        ],
        aliases: [
            "Scopable",
            "Class",
            "Expression"
        ],
        fields: {
            id: {
                validate: (0, _utils.assertNodeType)("Identifier"),
                optional: true
            },
            typeParameters: {
                validate: (0, _utils.assertNodeType)("TypeParameterDeclaration", "TSTypeParameterDeclaration", "Noop"),
                optional: true
            },
            body: {
                validate: (0, _utils.assertNodeType)("ClassBody")
            },
            superClass: {
                optional: true,
                validate: (0, _utils.assertNodeType)("Expression")
            },
            superTypeParameters: {
                validate: (0, _utils.assertNodeType)("TypeParameterInstantiation", "TSTypeParameterInstantiation"),
                optional: true
            },
            implements: {
                validate: (0, _utils.chain)((0, _utils.assertValueType)("array"), (0, _utils.assertEach)((0, _utils.assertNodeType)("TSExpressionWithTypeArguments", "ClassImplements"))),
                optional: true
            },
            decorators: {
                validate: (0, _utils.chain)((0, _utils.assertValueType)("array"), (0, _utils.assertEach)((0, _utils.assertNodeType)("Decorator"))),
                optional: true
            },
            mixins: {
                validate: (0, _utils.assertNodeType)("InterfaceExtends"),
                optional: true
            }
        }
    });
    (0, _utils.default)("ClassDeclaration", {
        inherits: "ClassExpression",
        aliases: [
            "Scopable",
            "Class",
            "Statement",
            "Declaration"
        ],
        fields: {
            id: {
                validate: (0, _utils.assertNodeType)("Identifier")
            },
            typeParameters: {
                validate: (0, _utils.assertNodeType)("TypeParameterDeclaration", "TSTypeParameterDeclaration", "Noop"),
                optional: true
            },
            body: {
                validate: (0, _utils.assertNodeType)("ClassBody")
            },
            superClass: {
                optional: true,
                validate: (0, _utils.assertNodeType)("Expression")
            },
            superTypeParameters: {
                validate: (0, _utils.assertNodeType)("TypeParameterInstantiation", "TSTypeParameterInstantiation"),
                optional: true
            },
            implements: {
                validate: (0, _utils.chain)((0, _utils.assertValueType)("array"), (0, _utils.assertEach)((0, _utils.assertNodeType)("TSExpressionWithTypeArguments", "ClassImplements"))),
                optional: true
            },
            decorators: {
                validate: (0, _utils.chain)((0, _utils.assertValueType)("array"), (0, _utils.assertEach)((0, _utils.assertNodeType)("Decorator"))),
                optional: true
            },
            mixins: {
                validate: (0, _utils.assertNodeType)("InterfaceExtends"),
                optional: true
            },
            declare: {
                validate: (0, _utils.assertValueType)("boolean"),
                optional: true
            },
            abstract: {
                validate: (0, _utils.assertValueType)("boolean"),
                optional: true
            }
        },
        validate: function() {
            var identifier = (0, _utils.assertNodeType)("Identifier");
            return function(parent, key, node) {
                if (!process.env.BABEL_TYPES_8_BREAKING) return;
                if (!(0, _is.default)("ExportDefaultDeclaration", parent)) {
                    identifier(node, "id", node.id);
                }
            };
        }()
    });
    (0, _utils.default)("ExportAllDeclaration", {
        visitor: [
            "source"
        ],
        aliases: [
            "Statement",
            "Declaration",
            "ModuleDeclaration",
            "ExportDeclaration"
        ],
        fields: {
            source: {
                validate: (0, _utils.assertNodeType)("StringLiteral")
            },
            exportKind: (0, _utils.validateOptional)((0, _utils.assertOneOf)("type", "value")),
            assertions: {
                optional: true,
                validate: (0, _utils.chain)((0, _utils.assertValueType)("array"), (0, _utils.assertEach)((0, _utils.assertNodeType)("ImportAttribute")))
            }
        }
    });
    (0, _utils.default)("ExportDefaultDeclaration", {
        visitor: [
            "declaration"
        ],
        aliases: [
            "Statement",
            "Declaration",
            "ModuleDeclaration",
            "ExportDeclaration"
        ],
        fields: {
            declaration: {
                validate: (0, _utils.assertNodeType)("FunctionDeclaration", "TSDeclareFunction", "ClassDeclaration", "Expression")
            },
            exportKind: (0, _utils.validateOptional)((0, _utils.assertOneOf)("value"))
        }
    });
    (0, _utils.default)("ExportNamedDeclaration", {
        visitor: [
            "declaration",
            "specifiers",
            "source"
        ],
        aliases: [
            "Statement",
            "Declaration",
            "ModuleDeclaration",
            "ExportDeclaration"
        ],
        fields: {
            declaration: {
                optional: true,
                validate: (0, _utils.chain)((0, _utils.assertNodeType)("Declaration"), Object.assign(function(node, key, val) {
                    if (!process.env.BABEL_TYPES_8_BREAKING) return;
                    if (val && node.specifiers.length) {
                        throw new TypeError("Only declaration or specifiers is allowed on ExportNamedDeclaration");
                    }
                }, {
                    oneOfNodeTypes: [
                        "Declaration"
                    ]
                }), function(node, key, val) {
                    if (!process.env.BABEL_TYPES_8_BREAKING) return;
                    if (val && node.source) {
                        throw new TypeError("Cannot export a declaration from a source");
                    }
                })
            },
            assertions: {
                optional: true,
                validate: (0, _utils.chain)((0, _utils.assertValueType)("array"), (0, _utils.assertEach)((0, _utils.assertNodeType)("ImportAttribute")))
            },
            specifiers: {
                default: [],
                validate: (0, _utils.chain)((0, _utils.assertValueType)("array"), (0, _utils.assertEach)(function() {
                    var sourced = (0, _utils.assertNodeType)("ExportSpecifier", "ExportDefaultSpecifier", "ExportNamespaceSpecifier");
                    var sourceless = (0, _utils.assertNodeType)("ExportSpecifier");
                    if (!process.env.BABEL_TYPES_8_BREAKING) return sourced;
                    return function(node, key, val) {
                        var validator = node.source ? sourced : sourceless;
                        validator(node, key, val);
                    };
                }()))
            },
            source: {
                validate: (0, _utils.assertNodeType)("StringLiteral"),
                optional: true
            },
            exportKind: (0, _utils.validateOptional)((0, _utils.assertOneOf)("type", "value"))
        }
    });
    (0, _utils.default)("ExportSpecifier", {
        visitor: [
            "local",
            "exported"
        ],
        aliases: [
            "ModuleSpecifier"
        ],
        fields: {
            local: {
                validate: (0, _utils.assertNodeType)("Identifier")
            },
            exported: {
                validate: (0, _utils.assertNodeType)("Identifier", "StringLiteral")
            }
        }
    });
    (0, _utils.default)("ForOfStatement", {
        visitor: [
            "left",
            "right",
            "body"
        ],
        builder: [
            "left",
            "right",
            "body",
            "await"
        ],
        aliases: [
            "Scopable",
            "Statement",
            "For",
            "BlockParent",
            "Loop",
            "ForXStatement"
        ],
        fields: {
            left: {
                validate: function() {
                    if (!process.env.BABEL_TYPES_8_BREAKING) {
                        return (0, _utils.assertNodeType)("VariableDeclaration", "LVal");
                    }
                    var declaration = (0, _utils.assertNodeType)("VariableDeclaration");
                    var lval = (0, _utils.assertNodeType)("Identifier", "MemberExpression", "ArrayPattern", "ObjectPattern");
                    return function(node, key, val) {
                        if ((0, _is.default)("VariableDeclaration", val)) {
                            declaration(node, key, val);
                        } else {
                            lval(node, key, val);
                        }
                    };
                }()
            },
            right: {
                validate: (0, _utils.assertNodeType)("Expression")
            },
            body: {
                validate: (0, _utils.assertNodeType)("Statement")
            },
            await: {
                default: false
            }
        }
    });
    (0, _utils.default)("ImportDeclaration", {
        visitor: [
            "specifiers",
            "source"
        ],
        aliases: [
            "Statement",
            "Declaration",
            "ModuleDeclaration"
        ],
        fields: {
            assertions: {
                optional: true,
                validate: (0, _utils.chain)((0, _utils.assertValueType)("array"), (0, _utils.assertEach)((0, _utils.assertNodeType)("ImportAttribute")))
            },
            specifiers: {
                validate: (0, _utils.chain)((0, _utils.assertValueType)("array"), (0, _utils.assertEach)((0, _utils.assertNodeType)("ImportSpecifier", "ImportDefaultSpecifier", "ImportNamespaceSpecifier")))
            },
            source: {
                validate: (0, _utils.assertNodeType)("StringLiteral")
            },
            importKind: {
                validate: (0, _utils.assertOneOf)("type", "typeof", "value"),
                optional: true
            }
        }
    });
    (0, _utils.default)("ImportDefaultSpecifier", {
        visitor: [
            "local"
        ],
        aliases: [
            "ModuleSpecifier"
        ],
        fields: {
            local: {
                validate: (0, _utils.assertNodeType)("Identifier")
            }
        }
    });
    (0, _utils.default)("ImportNamespaceSpecifier", {
        visitor: [
            "local"
        ],
        aliases: [
            "ModuleSpecifier"
        ],
        fields: {
            local: {
                validate: (0, _utils.assertNodeType)("Identifier")
            }
        }
    });
    (0, _utils.default)("ImportSpecifier", {
        visitor: [
            "local",
            "imported"
        ],
        aliases: [
            "ModuleSpecifier"
        ],
        fields: {
            local: {
                validate: (0, _utils.assertNodeType)("Identifier")
            },
            imported: {
                validate: (0, _utils.assertNodeType)("Identifier", "StringLiteral")
            },
            importKind: {
                validate: (0, _utils.assertOneOf)("type", "typeof"),
                optional: true
            }
        }
    });
    (0, _utils.default)("MetaProperty", {
        visitor: [
            "meta",
            "property"
        ],
        aliases: [
            "Expression"
        ],
        fields: {
            meta: {
                validate: (0, _utils.chain)((0, _utils.assertNodeType)("Identifier"), Object.assign(function(node, key, val) {
                    if (!process.env.BABEL_TYPES_8_BREAKING) return;
                    var property;
                    switch(val.name){
                        case "function":
                            property = "sent";
                            break;
                        case "new":
                            property = "target";
                            break;
                        case "import":
                            property = "meta";
                            break;
                    }
                    if (!(0, _is.default)("Identifier", node.property, {
                        name: property
                    })) {
                        throw new TypeError("Unrecognised MetaProperty");
                    }
                }, {
                    oneOfNodeTypes: [
                        "Identifier"
                    ]
                }))
            },
            property: {
                validate: (0, _utils.assertNodeType)("Identifier")
            }
        }
    });
    var classMethodOrPropertyCommon = {
        abstract: {
            validate: (0, _utils.assertValueType)("boolean"),
            optional: true
        },
        accessibility: {
            validate: (0, _utils.assertOneOf)("public", "private", "protected"),
            optional: true
        },
        static: {
            default: false
        },
        override: {
            default: false
        },
        computed: {
            default: false
        },
        optional: {
            validate: (0, _utils.assertValueType)("boolean"),
            optional: true
        },
        key: {
            validate: (0, _utils.chain)(function() {
                var normal = (0, _utils.assertNodeType)("Identifier", "StringLiteral", "NumericLiteral");
                var computed = (0, _utils.assertNodeType)("Expression");
                return function(node, key, val) {
                    var validator = node.computed ? computed : normal;
                    validator(node, key, val);
                };
            }(), (0, _utils.assertNodeType)("Identifier", "StringLiteral", "NumericLiteral", "Expression"))
        }
    };
    core.classMethodOrPropertyCommon = classMethodOrPropertyCommon;
    var classMethodOrDeclareMethodCommon = Object.assign({}, functionCommon, classMethodOrPropertyCommon, {
        params: {
            validate: (0, _utils.chain)((0, _utils.assertValueType)("array"), (0, _utils.assertEach)((0, _utils.assertNodeType)("Identifier", "Pattern", "RestElement", "TSParameterProperty")))
        },
        kind: {
            validate: (0, _utils.assertOneOf)("get", "set", "method", "constructor"),
            default: "method"
        },
        access: {
            validate: (0, _utils.chain)((0, _utils.assertValueType)("string"), (0, _utils.assertOneOf)("public", "private", "protected")),
            optional: true
        },
        decorators: {
            validate: (0, _utils.chain)((0, _utils.assertValueType)("array"), (0, _utils.assertEach)((0, _utils.assertNodeType)("Decorator"))),
            optional: true
        }
    });
    core.classMethodOrDeclareMethodCommon = classMethodOrDeclareMethodCommon;
    (0, _utils.default)("ClassMethod", {
        aliases: [
            "Function",
            "Scopable",
            "BlockParent",
            "FunctionParent",
            "Method"
        ],
        builder: [
            "kind",
            "key",
            "params",
            "body",
            "computed",
            "static",
            "generator",
            "async"
        ],
        visitor: [
            "key",
            "params",
            "body",
            "decorators",
            "returnType",
            "typeParameters"
        ],
        fields: Object.assign({}, classMethodOrDeclareMethodCommon, functionTypeAnnotationCommon, {
            body: {
                validate: (0, _utils.assertNodeType)("BlockStatement")
            }
        })
    });
    (0, _utils.default)("ObjectPattern", {
        visitor: [
            "properties",
            "typeAnnotation",
            "decorators"
        ],
        builder: [
            "properties"
        ],
        aliases: [
            "Pattern",
            "PatternLike",
            "LVal"
        ],
        fields: Object.assign({}, patternLikeCommon, {
            properties: {
                validate: (0, _utils.chain)((0, _utils.assertValueType)("array"), (0, _utils.assertEach)((0, _utils.assertNodeType)("RestElement", "ObjectProperty")))
            }
        })
    });
    (0, _utils.default)("SpreadElement", {
        visitor: [
            "argument"
        ],
        aliases: [
            "UnaryLike"
        ],
        deprecatedAlias: "SpreadProperty",
        fields: {
            argument: {
                validate: (0, _utils.assertNodeType)("Expression")
            }
        }
    });
    (0, _utils.default)("Super", {
        aliases: [
            "Expression"
        ]
    });
    (0, _utils.default)("TaggedTemplateExpression", {
        visitor: [
            "tag",
            "quasi",
            "typeParameters"
        ],
        builder: [
            "tag",
            "quasi"
        ],
        aliases: [
            "Expression"
        ],
        fields: {
            tag: {
                validate: (0, _utils.assertNodeType)("Expression")
            },
            quasi: {
                validate: (0, _utils.assertNodeType)("TemplateLiteral")
            },
            typeParameters: {
                validate: (0, _utils.assertNodeType)("TypeParameterInstantiation", "TSTypeParameterInstantiation"),
                optional: true
            }
        }
    });
    (0, _utils.default)("TemplateElement", {
        builder: [
            "value",
            "tail"
        ],
        fields: {
            value: {
                validate: (0, _utils.assertShape)({
                    raw: {
                        validate: (0, _utils.assertValueType)("string")
                    },
                    cooked: {
                        validate: (0, _utils.assertValueType)("string"),
                        optional: true
                    }
                })
            },
            tail: {
                default: false
            }
        }
    });
    (0, _utils.default)("TemplateLiteral", {
        visitor: [
            "quasis",
            "expressions"
        ],
        aliases: [
            "Expression",
            "Literal"
        ],
        fields: {
            quasis: {
                validate: (0, _utils.chain)((0, _utils.assertValueType)("array"), (0, _utils.assertEach)((0, _utils.assertNodeType)("TemplateElement")))
            },
            expressions: {
                validate: (0, _utils.chain)((0, _utils.assertValueType)("array"), (0, _utils.assertEach)((0, _utils.assertNodeType)("Expression", "TSType")), function(node, key, val) {
                    if (node.quasis.length !== val.length + 1) {
                        throw new TypeError("Number of " + node.type + " quasis should be exactly one more than the number of expressions.\nExpected " + (val.length + 1) + " quasis but got " + node.quasis.length);
                    }
                })
            }
        }
    });
    (0, _utils.default)("YieldExpression", {
        builder: [
            "argument",
            "delegate"
        ],
        visitor: [
            "argument"
        ],
        aliases: [
            "Expression",
            "Terminatorless"
        ],
        fields: {
            delegate: {
                validate: (0, _utils.chain)((0, _utils.assertValueType)("boolean"), Object.assign(function(node, key, val) {
                    if (!process.env.BABEL_TYPES_8_BREAKING) return;
                    if (val && !node.argument) {
                        throw new TypeError("Property delegate of YieldExpression cannot be true if there is no argument");
                    }
                }, {
                    type: "boolean"
                })),
                default: false
            },
            argument: {
                optional: true,
                validate: (0, _utils.assertNodeType)("Expression")
            }
        }
    });
    (0, _utils.default)("AwaitExpression", {
        builder: [
            "argument"
        ],
        visitor: [
            "argument"
        ],
        aliases: [
            "Expression",
            "Terminatorless"
        ],
        fields: {
            argument: {
                validate: (0, _utils.assertNodeType)("Expression")
            }
        }
    });
    (0, _utils.default)("Import", {
        aliases: [
            "Expression"
        ]
    });
    (0, _utils.default)("BigIntLiteral", {
        builder: [
            "value"
        ],
        fields: {
            value: {
                validate: (0, _utils.assertValueType)("string")
            }
        },
        aliases: [
            "Expression",
            "Pureish",
            "Literal",
            "Immutable"
        ]
    });
    (0, _utils.default)("ExportNamespaceSpecifier", {
        visitor: [
            "exported"
        ],
        aliases: [
            "ModuleSpecifier"
        ],
        fields: {
            exported: {
                validate: (0, _utils.assertNodeType)("Identifier")
            }
        }
    });
    (0, _utils.default)("OptionalMemberExpression", {
        builder: [
            "object",
            "property",
            "computed",
            "optional"
        ],
        visitor: [
            "object",
            "property"
        ],
        aliases: [
            "Expression"
        ],
        fields: {
            object: {
                validate: (0, _utils.assertNodeType)("Expression")
            },
            property: {
                validate: function() {
                    var normal = (0, _utils.assertNodeType)("Identifier");
                    var computed = (0, _utils.assertNodeType)("Expression");
                    var validator = function validator(node, key, val) {
                        var validator = node.computed ? computed : normal;
                        validator(node, key, val);
                    };
                    validator.oneOfNodeTypes = [
                        "Expression",
                        "Identifier"
                    ];
                    return validator;
                }()
            },
            computed: {
                default: false
            },
            optional: {
                validate: !process.env.BABEL_TYPES_8_BREAKING ? (0, _utils.assertValueType)("boolean") : (0, _utils.chain)((0, _utils.assertValueType)("boolean"), (0, _utils.assertOptionalChainStart)())
            }
        }
    });
    (0, _utils.default)("OptionalCallExpression", {
        visitor: [
            "callee",
            "arguments",
            "typeParameters",
            "typeArguments"
        ],
        builder: [
            "callee",
            "arguments",
            "optional"
        ],
        aliases: [
            "Expression"
        ],
        fields: {
            callee: {
                validate: (0, _utils.assertNodeType)("Expression")
            },
            arguments: {
                validate: (0, _utils.chain)((0, _utils.assertValueType)("array"), (0, _utils.assertEach)((0, _utils.assertNodeType)("Expression", "SpreadElement", "JSXNamespacedName", "ArgumentPlaceholder")))
            },
            optional: {
                validate: !process.env.BABEL_TYPES_8_BREAKING ? (0, _utils.assertValueType)("boolean") : (0, _utils.chain)((0, _utils.assertValueType)("boolean"), (0, _utils.assertOptionalChainStart)())
            },
            typeArguments: {
                validate: (0, _utils.assertNodeType)("TypeParameterInstantiation"),
                optional: true
            },
            typeParameters: {
                validate: (0, _utils.assertNodeType)("TSTypeParameterInstantiation"),
                optional: true
            }
        }
    });
    (0, _utils.default)("ClassProperty", {
        visitor: [
            "key",
            "value",
            "typeAnnotation",
            "decorators"
        ],
        builder: [
            "key",
            "value",
            "typeAnnotation",
            "decorators",
            "computed",
            "static"
        ],
        aliases: [
            "Property"
        ],
        fields: Object.assign({}, classMethodOrPropertyCommon, {
            value: {
                validate: (0, _utils.assertNodeType)("Expression"),
                optional: true
            },
            definite: {
                validate: (0, _utils.assertValueType)("boolean"),
                optional: true
            },
            typeAnnotation: {
                validate: (0, _utils.assertNodeType)("TypeAnnotation", "TSTypeAnnotation", "Noop"),
                optional: true
            },
            decorators: {
                validate: (0, _utils.chain)((0, _utils.assertValueType)("array"), (0, _utils.assertEach)((0, _utils.assertNodeType)("Decorator"))),
                optional: true
            },
            readonly: {
                validate: (0, _utils.assertValueType)("boolean"),
                optional: true
            },
            declare: {
                validate: (0, _utils.assertValueType)("boolean"),
                optional: true
            },
            variance: {
                validate: (0, _utils.assertNodeType)("Variance"),
                optional: true
            }
        })
    });
    (0, _utils.default)("ClassPrivateProperty", {
        visitor: [
            "key",
            "value",
            "decorators",
            "typeAnnotation"
        ],
        builder: [
            "key",
            "value",
            "decorators",
            "static"
        ],
        aliases: [
            "Property",
            "Private"
        ],
        fields: {
            key: {
                validate: (0, _utils.assertNodeType)("PrivateName")
            },
            value: {
                validate: (0, _utils.assertNodeType)("Expression"),
                optional: true
            },
            typeAnnotation: {
                validate: (0, _utils.assertNodeType)("TypeAnnotation", "TSTypeAnnotation", "Noop"),
                optional: true
            },
            decorators: {
                validate: (0, _utils.chain)((0, _utils.assertValueType)("array"), (0, _utils.assertEach)((0, _utils.assertNodeType)("Decorator"))),
                optional: true
            },
            readonly: {
                validate: (0, _utils.assertValueType)("boolean"),
                optional: true
            },
            definite: {
                validate: (0, _utils.assertValueType)("boolean"),
                optional: true
            },
            variance: {
                validate: (0, _utils.assertNodeType)("Variance"),
                optional: true
            }
        }
    });
    (0, _utils.default)("ClassPrivateMethod", {
        builder: [
            "kind",
            "key",
            "params",
            "body",
            "static"
        ],
        visitor: [
            "key",
            "params",
            "body",
            "decorators",
            "returnType",
            "typeParameters"
        ],
        aliases: [
            "Function",
            "Scopable",
            "BlockParent",
            "FunctionParent",
            "Method",
            "Private"
        ],
        fields: Object.assign({}, classMethodOrDeclareMethodCommon, functionTypeAnnotationCommon, {
            key: {
                validate: (0, _utils.assertNodeType)("PrivateName")
            },
            body: {
                validate: (0, _utils.assertNodeType)("BlockStatement")
            }
        })
    });
    (0, _utils.default)("PrivateName", {
        visitor: [
            "id"
        ],
        aliases: [
            "Private"
        ],
        fields: {
            id: {
                validate: (0, _utils.assertNodeType)("Identifier")
            }
        }
    });
    return core;
}

var flow = {};

var hasRequiredFlow;
function requireFlow() {
    if (hasRequiredFlow) return flow;
    hasRequiredFlow = 1;
    var _utils = requireUtils();
    var defineInterfaceishType = function(name, typeParameterType) {
        if (typeParameterType === void 0) typeParameterType = "TypeParameterDeclaration";
        (0, _utils.default)(name, {
            builder: [
                "id",
                "typeParameters",
                "extends",
                "body"
            ],
            visitor: [
                "id",
                "typeParameters",
                "extends",
                "mixins",
                "implements",
                "body"
            ],
            aliases: [
                "Flow",
                "FlowDeclaration",
                "Statement",
                "Declaration"
            ],
            fields: {
                id: (0, _utils.validateType)("Identifier"),
                typeParameters: (0, _utils.validateOptionalType)(typeParameterType),
                extends: (0, _utils.validateOptional)((0, _utils.arrayOfType)("InterfaceExtends")),
                mixins: (0, _utils.validateOptional)((0, _utils.arrayOfType)("InterfaceExtends")),
                implements: (0, _utils.validateOptional)((0, _utils.arrayOfType)("ClassImplements")),
                body: (0, _utils.validateType)("ObjectTypeAnnotation")
            }
        });
    };
    (0, _utils.default)("AnyTypeAnnotation", {
        aliases: [
            "Flow",
            "FlowType",
            "FlowBaseAnnotation"
        ]
    });
    (0, _utils.default)("ArrayTypeAnnotation", {
        visitor: [
            "elementType"
        ],
        aliases: [
            "Flow",
            "FlowType"
        ],
        fields: {
            elementType: (0, _utils.validateType)("FlowType")
        }
    });
    (0, _utils.default)("BooleanTypeAnnotation", {
        aliases: [
            "Flow",
            "FlowType",
            "FlowBaseAnnotation"
        ]
    });
    (0, _utils.default)("BooleanLiteralTypeAnnotation", {
        builder: [
            "value"
        ],
        aliases: [
            "Flow",
            "FlowType"
        ],
        fields: {
            value: (0, _utils.validate)((0, _utils.assertValueType)("boolean"))
        }
    });
    (0, _utils.default)("NullLiteralTypeAnnotation", {
        aliases: [
            "Flow",
            "FlowType",
            "FlowBaseAnnotation"
        ]
    });
    (0, _utils.default)("ClassImplements", {
        visitor: [
            "id",
            "typeParameters"
        ],
        aliases: [
            "Flow"
        ],
        fields: {
            id: (0, _utils.validateType)("Identifier"),
            typeParameters: (0, _utils.validateOptionalType)("TypeParameterInstantiation")
        }
    });
    defineInterfaceishType("DeclareClass");
    (0, _utils.default)("DeclareFunction", {
        visitor: [
            "id"
        ],
        aliases: [
            "Flow",
            "FlowDeclaration",
            "Statement",
            "Declaration"
        ],
        fields: {
            id: (0, _utils.validateType)("Identifier"),
            predicate: (0, _utils.validateOptionalType)("DeclaredPredicate")
        }
    });
    defineInterfaceishType("DeclareInterface");
    (0, _utils.default)("DeclareModule", {
        builder: [
            "id",
            "body",
            "kind"
        ],
        visitor: [
            "id",
            "body"
        ],
        aliases: [
            "Flow",
            "FlowDeclaration",
            "Statement",
            "Declaration"
        ],
        fields: {
            id: (0, _utils.validateType)([
                "Identifier",
                "StringLiteral"
            ]),
            body: (0, _utils.validateType)("BlockStatement"),
            kind: (0, _utils.validateOptional)((0, _utils.assertOneOf)("CommonJS", "ES"))
        }
    });
    (0, _utils.default)("DeclareModuleExports", {
        visitor: [
            "typeAnnotation"
        ],
        aliases: [
            "Flow",
            "FlowDeclaration",
            "Statement",
            "Declaration"
        ],
        fields: {
            typeAnnotation: (0, _utils.validateType)("TypeAnnotation")
        }
    });
    (0, _utils.default)("DeclareTypeAlias", {
        visitor: [
            "id",
            "typeParameters",
            "right"
        ],
        aliases: [
            "Flow",
            "FlowDeclaration",
            "Statement",
            "Declaration"
        ],
        fields: {
            id: (0, _utils.validateType)("Identifier"),
            typeParameters: (0, _utils.validateOptionalType)("TypeParameterDeclaration"),
            right: (0, _utils.validateType)("FlowType")
        }
    });
    (0, _utils.default)("DeclareOpaqueType", {
        visitor: [
            "id",
            "typeParameters",
            "supertype"
        ],
        aliases: [
            "Flow",
            "FlowDeclaration",
            "Statement",
            "Declaration"
        ],
        fields: {
            id: (0, _utils.validateType)("Identifier"),
            typeParameters: (0, _utils.validateOptionalType)("TypeParameterDeclaration"),
            supertype: (0, _utils.validateOptionalType)("FlowType"),
            impltype: (0, _utils.validateOptionalType)("FlowType")
        }
    });
    (0, _utils.default)("DeclareVariable", {
        visitor: [
            "id"
        ],
        aliases: [
            "Flow",
            "FlowDeclaration",
            "Statement",
            "Declaration"
        ],
        fields: {
            id: (0, _utils.validateType)("Identifier")
        }
    });
    (0, _utils.default)("DeclareExportDeclaration", {
        visitor: [
            "declaration",
            "specifiers",
            "source"
        ],
        aliases: [
            "Flow",
            "FlowDeclaration",
            "Statement",
            "Declaration"
        ],
        fields: {
            declaration: (0, _utils.validateOptionalType)("Flow"),
            specifiers: (0, _utils.validateOptional)((0, _utils.arrayOfType)([
                "ExportSpecifier",
                "ExportNamespaceSpecifier"
            ])),
            source: (0, _utils.validateOptionalType)("StringLiteral"),
            default: (0, _utils.validateOptional)((0, _utils.assertValueType)("boolean"))
        }
    });
    (0, _utils.default)("DeclareExportAllDeclaration", {
        visitor: [
            "source"
        ],
        aliases: [
            "Flow",
            "FlowDeclaration",
            "Statement",
            "Declaration"
        ],
        fields: {
            source: (0, _utils.validateType)("StringLiteral"),
            exportKind: (0, _utils.validateOptional)((0, _utils.assertOneOf)("type", "value"))
        }
    });
    (0, _utils.default)("DeclaredPredicate", {
        visitor: [
            "value"
        ],
        aliases: [
            "Flow",
            "FlowPredicate"
        ],
        fields: {
            value: (0, _utils.validateType)("Flow")
        }
    });
    (0, _utils.default)("ExistsTypeAnnotation", {
        aliases: [
            "Flow",
            "FlowType"
        ]
    });
    (0, _utils.default)("FunctionTypeAnnotation", {
        visitor: [
            "typeParameters",
            "params",
            "rest",
            "returnType"
        ],
        aliases: [
            "Flow",
            "FlowType"
        ],
        fields: {
            typeParameters: (0, _utils.validateOptionalType)("TypeParameterDeclaration"),
            params: (0, _utils.validate)((0, _utils.arrayOfType)("FunctionTypeParam")),
            rest: (0, _utils.validateOptionalType)("FunctionTypeParam"),
            this: (0, _utils.validateOptionalType)("FunctionTypeParam"),
            returnType: (0, _utils.validateType)("FlowType")
        }
    });
    (0, _utils.default)("FunctionTypeParam", {
        visitor: [
            "name",
            "typeAnnotation"
        ],
        aliases: [
            "Flow"
        ],
        fields: {
            name: (0, _utils.validateOptionalType)("Identifier"),
            typeAnnotation: (0, _utils.validateType)("FlowType"),
            optional: (0, _utils.validateOptional)((0, _utils.assertValueType)("boolean"))
        }
    });
    (0, _utils.default)("GenericTypeAnnotation", {
        visitor: [
            "id",
            "typeParameters"
        ],
        aliases: [
            "Flow",
            "FlowType"
        ],
        fields: {
            id: (0, _utils.validateType)([
                "Identifier",
                "QualifiedTypeIdentifier"
            ]),
            typeParameters: (0, _utils.validateOptionalType)("TypeParameterInstantiation")
        }
    });
    (0, _utils.default)("InferredPredicate", {
        aliases: [
            "Flow",
            "FlowPredicate"
        ]
    });
    (0, _utils.default)("InterfaceExtends", {
        visitor: [
            "id",
            "typeParameters"
        ],
        aliases: [
            "Flow"
        ],
        fields: {
            id: (0, _utils.validateType)([
                "Identifier",
                "QualifiedTypeIdentifier"
            ]),
            typeParameters: (0, _utils.validateOptionalType)("TypeParameterInstantiation")
        }
    });
    defineInterfaceishType("InterfaceDeclaration");
    (0, _utils.default)("InterfaceTypeAnnotation", {
        visitor: [
            "extends",
            "body"
        ],
        aliases: [
            "Flow",
            "FlowType"
        ],
        fields: {
            extends: (0, _utils.validateOptional)((0, _utils.arrayOfType)("InterfaceExtends")),
            body: (0, _utils.validateType)("ObjectTypeAnnotation")
        }
    });
    (0, _utils.default)("IntersectionTypeAnnotation", {
        visitor: [
            "types"
        ],
        aliases: [
            "Flow",
            "FlowType"
        ],
        fields: {
            types: (0, _utils.validate)((0, _utils.arrayOfType)("FlowType"))
        }
    });
    (0, _utils.default)("MixedTypeAnnotation", {
        aliases: [
            "Flow",
            "FlowType",
            "FlowBaseAnnotation"
        ]
    });
    (0, _utils.default)("EmptyTypeAnnotation", {
        aliases: [
            "Flow",
            "FlowType",
            "FlowBaseAnnotation"
        ]
    });
    (0, _utils.default)("NullableTypeAnnotation", {
        visitor: [
            "typeAnnotation"
        ],
        aliases: [
            "Flow",
            "FlowType"
        ],
        fields: {
            typeAnnotation: (0, _utils.validateType)("FlowType")
        }
    });
    (0, _utils.default)("NumberLiteralTypeAnnotation", {
        builder: [
            "value"
        ],
        aliases: [
            "Flow",
            "FlowType"
        ],
        fields: {
            value: (0, _utils.validate)((0, _utils.assertValueType)("number"))
        }
    });
    (0, _utils.default)("NumberTypeAnnotation", {
        aliases: [
            "Flow",
            "FlowType",
            "FlowBaseAnnotation"
        ]
    });
    (0, _utils.default)("ObjectTypeAnnotation", {
        visitor: [
            "properties",
            "indexers",
            "callProperties",
            "internalSlots"
        ],
        aliases: [
            "Flow",
            "FlowType"
        ],
        builder: [
            "properties",
            "indexers",
            "callProperties",
            "internalSlots",
            "exact"
        ],
        fields: {
            properties: (0, _utils.validate)((0, _utils.arrayOfType)([
                "ObjectTypeProperty",
                "ObjectTypeSpreadProperty"
            ])),
            indexers: (0, _utils.validateOptional)((0, _utils.arrayOfType)("ObjectTypeIndexer")),
            callProperties: (0, _utils.validateOptional)((0, _utils.arrayOfType)("ObjectTypeCallProperty")),
            internalSlots: (0, _utils.validateOptional)((0, _utils.arrayOfType)("ObjectTypeInternalSlot")),
            exact: {
                validate: (0, _utils.assertValueType)("boolean"),
                default: false
            },
            inexact: (0, _utils.validateOptional)((0, _utils.assertValueType)("boolean"))
        }
    });
    (0, _utils.default)("ObjectTypeInternalSlot", {
        visitor: [
            "id",
            "value",
            "optional",
            "static",
            "method"
        ],
        aliases: [
            "Flow",
            "UserWhitespacable"
        ],
        fields: {
            id: (0, _utils.validateType)("Identifier"),
            value: (0, _utils.validateType)("FlowType"),
            optional: (0, _utils.validate)((0, _utils.assertValueType)("boolean")),
            static: (0, _utils.validate)((0, _utils.assertValueType)("boolean")),
            method: (0, _utils.validate)((0, _utils.assertValueType)("boolean"))
        }
    });
    (0, _utils.default)("ObjectTypeCallProperty", {
        visitor: [
            "value"
        ],
        aliases: [
            "Flow",
            "UserWhitespacable"
        ],
        fields: {
            value: (0, _utils.validateType)("FlowType"),
            static: (0, _utils.validate)((0, _utils.assertValueType)("boolean"))
        }
    });
    (0, _utils.default)("ObjectTypeIndexer", {
        visitor: [
            "id",
            "key",
            "value",
            "variance"
        ],
        aliases: [
            "Flow",
            "UserWhitespacable"
        ],
        fields: {
            id: (0, _utils.validateOptionalType)("Identifier"),
            key: (0, _utils.validateType)("FlowType"),
            value: (0, _utils.validateType)("FlowType"),
            static: (0, _utils.validate)((0, _utils.assertValueType)("boolean")),
            variance: (0, _utils.validateOptionalType)("Variance")
        }
    });
    (0, _utils.default)("ObjectTypeProperty", {
        visitor: [
            "key",
            "value",
            "variance"
        ],
        aliases: [
            "Flow",
            "UserWhitespacable"
        ],
        fields: {
            key: (0, _utils.validateType)([
                "Identifier",
                "StringLiteral"
            ]),
            value: (0, _utils.validateType)("FlowType"),
            kind: (0, _utils.validate)((0, _utils.assertOneOf)("init", "get", "set")),
            static: (0, _utils.validate)((0, _utils.assertValueType)("boolean")),
            proto: (0, _utils.validate)((0, _utils.assertValueType)("boolean")),
            optional: (0, _utils.validate)((0, _utils.assertValueType)("boolean")),
            variance: (0, _utils.validateOptionalType)("Variance"),
            method: (0, _utils.validate)((0, _utils.assertValueType)("boolean"))
        }
    });
    (0, _utils.default)("ObjectTypeSpreadProperty", {
        visitor: [
            "argument"
        ],
        aliases: [
            "Flow",
            "UserWhitespacable"
        ],
        fields: {
            argument: (0, _utils.validateType)("FlowType")
        }
    });
    (0, _utils.default)("OpaqueType", {
        visitor: [
            "id",
            "typeParameters",
            "supertype",
            "impltype"
        ],
        aliases: [
            "Flow",
            "FlowDeclaration",
            "Statement",
            "Declaration"
        ],
        fields: {
            id: (0, _utils.validateType)("Identifier"),
            typeParameters: (0, _utils.validateOptionalType)("TypeParameterDeclaration"),
            supertype: (0, _utils.validateOptionalType)("FlowType"),
            impltype: (0, _utils.validateType)("FlowType")
        }
    });
    (0, _utils.default)("QualifiedTypeIdentifier", {
        visitor: [
            "id",
            "qualification"
        ],
        aliases: [
            "Flow"
        ],
        fields: {
            id: (0, _utils.validateType)("Identifier"),
            qualification: (0, _utils.validateType)([
                "Identifier",
                "QualifiedTypeIdentifier"
            ])
        }
    });
    (0, _utils.default)("StringLiteralTypeAnnotation", {
        builder: [
            "value"
        ],
        aliases: [
            "Flow",
            "FlowType"
        ],
        fields: {
            value: (0, _utils.validate)((0, _utils.assertValueType)("string"))
        }
    });
    (0, _utils.default)("StringTypeAnnotation", {
        aliases: [
            "Flow",
            "FlowType",
            "FlowBaseAnnotation"
        ]
    });
    (0, _utils.default)("SymbolTypeAnnotation", {
        aliases: [
            "Flow",
            "FlowType",
            "FlowBaseAnnotation"
        ]
    });
    (0, _utils.default)("ThisTypeAnnotation", {
        aliases: [
            "Flow",
            "FlowType",
            "FlowBaseAnnotation"
        ]
    });
    (0, _utils.default)("TupleTypeAnnotation", {
        visitor: [
            "types"
        ],
        aliases: [
            "Flow",
            "FlowType"
        ],
        fields: {
            types: (0, _utils.validate)((0, _utils.arrayOfType)("FlowType"))
        }
    });
    (0, _utils.default)("TypeofTypeAnnotation", {
        visitor: [
            "argument"
        ],
        aliases: [
            "Flow",
            "FlowType"
        ],
        fields: {
            argument: (0, _utils.validateType)("FlowType")
        }
    });
    (0, _utils.default)("TypeAlias", {
        visitor: [
            "id",
            "typeParameters",
            "right"
        ],
        aliases: [
            "Flow",
            "FlowDeclaration",
            "Statement",
            "Declaration"
        ],
        fields: {
            id: (0, _utils.validateType)("Identifier"),
            typeParameters: (0, _utils.validateOptionalType)("TypeParameterDeclaration"),
            right: (0, _utils.validateType)("FlowType")
        }
    });
    (0, _utils.default)("TypeAnnotation", {
        aliases: [
            "Flow"
        ],
        visitor: [
            "typeAnnotation"
        ],
        fields: {
            typeAnnotation: (0, _utils.validateType)("FlowType")
        }
    });
    (0, _utils.default)("TypeCastExpression", {
        visitor: [
            "expression",
            "typeAnnotation"
        ],
        aliases: [
            "Flow",
            "ExpressionWrapper",
            "Expression"
        ],
        fields: {
            expression: (0, _utils.validateType)("Expression"),
            typeAnnotation: (0, _utils.validateType)("TypeAnnotation")
        }
    });
    (0, _utils.default)("TypeParameter", {
        aliases: [
            "Flow"
        ],
        visitor: [
            "bound",
            "default",
            "variance"
        ],
        fields: {
            name: (0, _utils.validate)((0, _utils.assertValueType)("string")),
            bound: (0, _utils.validateOptionalType)("TypeAnnotation"),
            default: (0, _utils.validateOptionalType)("FlowType"),
            variance: (0, _utils.validateOptionalType)("Variance")
        }
    });
    (0, _utils.default)("TypeParameterDeclaration", {
        aliases: [
            "Flow"
        ],
        visitor: [
            "params"
        ],
        fields: {
            params: (0, _utils.validate)((0, _utils.arrayOfType)("TypeParameter"))
        }
    });
    (0, _utils.default)("TypeParameterInstantiation", {
        aliases: [
            "Flow"
        ],
        visitor: [
            "params"
        ],
        fields: {
            params: (0, _utils.validate)((0, _utils.arrayOfType)("FlowType"))
        }
    });
    (0, _utils.default)("UnionTypeAnnotation", {
        visitor: [
            "types"
        ],
        aliases: [
            "Flow",
            "FlowType"
        ],
        fields: {
            types: (0, _utils.validate)((0, _utils.arrayOfType)("FlowType"))
        }
    });
    (0, _utils.default)("Variance", {
        aliases: [
            "Flow"
        ],
        builder: [
            "kind"
        ],
        fields: {
            kind: (0, _utils.validate)((0, _utils.assertOneOf)("minus", "plus"))
        }
    });
    (0, _utils.default)("VoidTypeAnnotation", {
        aliases: [
            "Flow",
            "FlowType",
            "FlowBaseAnnotation"
        ]
    });
    (0, _utils.default)("EnumDeclaration", {
        aliases: [
            "Statement",
            "Declaration"
        ],
        visitor: [
            "id",
            "body"
        ],
        fields: {
            id: (0, _utils.validateType)("Identifier"),
            body: (0, _utils.validateType)([
                "EnumBooleanBody",
                "EnumNumberBody",
                "EnumStringBody",
                "EnumSymbolBody"
            ])
        }
    });
    (0, _utils.default)("EnumBooleanBody", {
        aliases: [
            "EnumBody"
        ],
        visitor: [
            "members"
        ],
        fields: {
            explicitType: (0, _utils.validate)((0, _utils.assertValueType)("boolean")),
            members: (0, _utils.validateArrayOfType)("EnumBooleanMember"),
            hasUnknownMembers: (0, _utils.validate)((0, _utils.assertValueType)("boolean"))
        }
    });
    (0, _utils.default)("EnumNumberBody", {
        aliases: [
            "EnumBody"
        ],
        visitor: [
            "members"
        ],
        fields: {
            explicitType: (0, _utils.validate)((0, _utils.assertValueType)("boolean")),
            members: (0, _utils.validateArrayOfType)("EnumNumberMember"),
            hasUnknownMembers: (0, _utils.validate)((0, _utils.assertValueType)("boolean"))
        }
    });
    (0, _utils.default)("EnumStringBody", {
        aliases: [
            "EnumBody"
        ],
        visitor: [
            "members"
        ],
        fields: {
            explicitType: (0, _utils.validate)((0, _utils.assertValueType)("boolean")),
            members: (0, _utils.validateArrayOfType)([
                "EnumStringMember",
                "EnumDefaultedMember"
            ]),
            hasUnknownMembers: (0, _utils.validate)((0, _utils.assertValueType)("boolean"))
        }
    });
    (0, _utils.default)("EnumSymbolBody", {
        aliases: [
            "EnumBody"
        ],
        visitor: [
            "members"
        ],
        fields: {
            members: (0, _utils.validateArrayOfType)("EnumDefaultedMember"),
            hasUnknownMembers: (0, _utils.validate)((0, _utils.assertValueType)("boolean"))
        }
    });
    (0, _utils.default)("EnumBooleanMember", {
        aliases: [
            "EnumMember"
        ],
        visitor: [
            "id"
        ],
        fields: {
            id: (0, _utils.validateType)("Identifier"),
            init: (0, _utils.validateType)("BooleanLiteral")
        }
    });
    (0, _utils.default)("EnumNumberMember", {
        aliases: [
            "EnumMember"
        ],
        visitor: [
            "id",
            "init"
        ],
        fields: {
            id: (0, _utils.validateType)("Identifier"),
            init: (0, _utils.validateType)("NumericLiteral")
        }
    });
    (0, _utils.default)("EnumStringMember", {
        aliases: [
            "EnumMember"
        ],
        visitor: [
            "id",
            "init"
        ],
        fields: {
            id: (0, _utils.validateType)("Identifier"),
            init: (0, _utils.validateType)("StringLiteral")
        }
    });
    (0, _utils.default)("EnumDefaultedMember", {
        aliases: [
            "EnumMember"
        ],
        visitor: [
            "id"
        ],
        fields: {
            id: (0, _utils.validateType)("Identifier")
        }
    });
    (0, _utils.default)("IndexedAccessType", {
        visitor: [
            "objectType",
            "indexType"
        ],
        aliases: [
            "Flow",
            "FlowType"
        ],
        fields: {
            objectType: (0, _utils.validateType)("FlowType"),
            indexType: (0, _utils.validateType)("FlowType")
        }
    });
    (0, _utils.default)("OptionalIndexedAccessType", {
        visitor: [
            "objectType",
            "indexType"
        ],
        aliases: [
            "Flow",
            "FlowType"
        ],
        fields: {
            objectType: (0, _utils.validateType)("FlowType"),
            indexType: (0, _utils.validateType)("FlowType"),
            optional: (0, _utils.validate)((0, _utils.assertValueType)("boolean"))
        }
    });
    return flow;
}

var jsx = {};

var hasRequiredJsx;
function requireJsx() {
    if (hasRequiredJsx) return jsx;
    hasRequiredJsx = 1;
    var _utils = requireUtils();
    (0, _utils.default)("JSXAttribute", {
        visitor: [
            "name",
            "value"
        ],
        aliases: [
            "JSX",
            "Immutable"
        ],
        fields: {
            name: {
                validate: (0, _utils.assertNodeType)("JSXIdentifier", "JSXNamespacedName")
            },
            value: {
                optional: true,
                validate: (0, _utils.assertNodeType)("JSXElement", "JSXFragment", "StringLiteral", "JSXExpressionContainer")
            }
        }
    });
    (0, _utils.default)("JSXClosingElement", {
        visitor: [
            "name"
        ],
        aliases: [
            "JSX",
            "Immutable"
        ],
        fields: {
            name: {
                validate: (0, _utils.assertNodeType)("JSXIdentifier", "JSXMemberExpression", "JSXNamespacedName")
            }
        }
    });
    (0, _utils.default)("JSXElement", {
        builder: [
            "openingElement",
            "closingElement",
            "children",
            "selfClosing"
        ],
        visitor: [
            "openingElement",
            "children",
            "closingElement"
        ],
        aliases: [
            "JSX",
            "Immutable",
            "Expression"
        ],
        fields: {
            openingElement: {
                validate: (0, _utils.assertNodeType)("JSXOpeningElement")
            },
            closingElement: {
                optional: true,
                validate: (0, _utils.assertNodeType)("JSXClosingElement")
            },
            children: {
                validate: (0, _utils.chain)((0, _utils.assertValueType)("array"), (0, _utils.assertEach)((0, _utils.assertNodeType)("JSXText", "JSXExpressionContainer", "JSXSpreadChild", "JSXElement", "JSXFragment")))
            },
            selfClosing: {
                validate: (0, _utils.assertValueType)("boolean"),
                optional: true
            }
        }
    });
    (0, _utils.default)("JSXEmptyExpression", {
        aliases: [
            "JSX"
        ]
    });
    (0, _utils.default)("JSXExpressionContainer", {
        visitor: [
            "expression"
        ],
        aliases: [
            "JSX",
            "Immutable"
        ],
        fields: {
            expression: {
                validate: (0, _utils.assertNodeType)("Expression", "JSXEmptyExpression")
            }
        }
    });
    (0, _utils.default)("JSXSpreadChild", {
        visitor: [
            "expression"
        ],
        aliases: [
            "JSX",
            "Immutable"
        ],
        fields: {
            expression: {
                validate: (0, _utils.assertNodeType)("Expression")
            }
        }
    });
    (0, _utils.default)("JSXIdentifier", {
        builder: [
            "name"
        ],
        aliases: [
            "JSX"
        ],
        fields: {
            name: {
                validate: (0, _utils.assertValueType)("string")
            }
        }
    });
    (0, _utils.default)("JSXMemberExpression", {
        visitor: [
            "object",
            "property"
        ],
        aliases: [
            "JSX"
        ],
        fields: {
            object: {
                validate: (0, _utils.assertNodeType)("JSXMemberExpression", "JSXIdentifier")
            },
            property: {
                validate: (0, _utils.assertNodeType)("JSXIdentifier")
            }
        }
    });
    (0, _utils.default)("JSXNamespacedName", {
        visitor: [
            "namespace",
            "name"
        ],
        aliases: [
            "JSX"
        ],
        fields: {
            namespace: {
                validate: (0, _utils.assertNodeType)("JSXIdentifier")
            },
            name: {
                validate: (0, _utils.assertNodeType)("JSXIdentifier")
            }
        }
    });
    (0, _utils.default)("JSXOpeningElement", {
        builder: [
            "name",
            "attributes",
            "selfClosing"
        ],
        visitor: [
            "name",
            "attributes"
        ],
        aliases: [
            "JSX",
            "Immutable"
        ],
        fields: {
            name: {
                validate: (0, _utils.assertNodeType)("JSXIdentifier", "JSXMemberExpression", "JSXNamespacedName")
            },
            selfClosing: {
                default: false
            },
            attributes: {
                validate: (0, _utils.chain)((0, _utils.assertValueType)("array"), (0, _utils.assertEach)((0, _utils.assertNodeType)("JSXAttribute", "JSXSpreadAttribute")))
            },
            typeParameters: {
                validate: (0, _utils.assertNodeType)("TypeParameterInstantiation", "TSTypeParameterInstantiation"),
                optional: true
            }
        }
    });
    (0, _utils.default)("JSXSpreadAttribute", {
        visitor: [
            "argument"
        ],
        aliases: [
            "JSX"
        ],
        fields: {
            argument: {
                validate: (0, _utils.assertNodeType)("Expression")
            }
        }
    });
    (0, _utils.default)("JSXText", {
        aliases: [
            "JSX",
            "Immutable"
        ],
        builder: [
            "value"
        ],
        fields: {
            value: {
                validate: (0, _utils.assertValueType)("string")
            }
        }
    });
    (0, _utils.default)("JSXFragment", {
        builder: [
            "openingFragment",
            "closingFragment",
            "children"
        ],
        visitor: [
            "openingFragment",
            "children",
            "closingFragment"
        ],
        aliases: [
            "JSX",
            "Immutable",
            "Expression"
        ],
        fields: {
            openingFragment: {
                validate: (0, _utils.assertNodeType)("JSXOpeningFragment")
            },
            closingFragment: {
                validate: (0, _utils.assertNodeType)("JSXClosingFragment")
            },
            children: {
                validate: (0, _utils.chain)((0, _utils.assertValueType)("array"), (0, _utils.assertEach)((0, _utils.assertNodeType)("JSXText", "JSXExpressionContainer", "JSXSpreadChild", "JSXElement", "JSXFragment")))
            }
        }
    });
    (0, _utils.default)("JSXOpeningFragment", {
        aliases: [
            "JSX",
            "Immutable"
        ]
    });
    (0, _utils.default)("JSXClosingFragment", {
        aliases: [
            "JSX",
            "Immutable"
        ]
    });
    return jsx;
}

var misc = {};

var placeholders = {};

function _createForOfIteratorHelperLoose$f(o, allowArrayLike) {
    var it = typeof Symbol !== "undefined" && o[Symbol.iterator] || o["@@iterator"];
    if (it) return (it = it.call(o)).next.bind(it);
    if (Array.isArray(o) || (it = unsupportedIterableToArray(o)) || allowArrayLike && o && typeof o.length === "number") {
        if (it) o = it;
        var i = 0;
        return function() {
            if (i >= o.length) return {
                done: true
            };
            return {
                done: false,
                value: o[i++]
            };
        };
    }
    throw new TypeError("Invalid attempt to iterate non-iterable instance.\\nIn order to be iterable, non-array objects must have a [Symbol.iterator]() method.");
}
var hasRequiredPlaceholders;
function requirePlaceholders() {
    if (hasRequiredPlaceholders) return placeholders;
    hasRequiredPlaceholders = 1;
    Object.defineProperty(placeholders, "__esModule", {
        value: true
    });
    placeholders.PLACEHOLDERS_FLIPPED_ALIAS = placeholders.PLACEHOLDERS_ALIAS = placeholders.PLACEHOLDERS = void 0;
    var _utils = requireUtils();
    var PLACEHOLDERS = [
        "Identifier",
        "StringLiteral",
        "Expression",
        "Statement",
        "Declaration",
        "BlockStatement",
        "ClassBody",
        "Pattern"
    ];
    placeholders.PLACEHOLDERS = PLACEHOLDERS;
    var PLACEHOLDERS_ALIAS = {
        Declaration: [
            "Statement"
        ],
        Pattern: [
            "PatternLike",
            "LVal"
        ]
    };
    placeholders.PLACEHOLDERS_ALIAS = PLACEHOLDERS_ALIAS;
    for(var _iterator = _createForOfIteratorHelperLoose$f(PLACEHOLDERS), _step; !(_step = _iterator()).done;){
        var type = _step.value;
        var alias = _utils.ALIAS_KEYS[type];
        if (alias != null && alias.length) PLACEHOLDERS_ALIAS[type] = alias;
    }
    var PLACEHOLDERS_FLIPPED_ALIAS = {};
    placeholders.PLACEHOLDERS_FLIPPED_ALIAS = PLACEHOLDERS_FLIPPED_ALIAS;
    Object.keys(PLACEHOLDERS_ALIAS).forEach(function(type) {
        PLACEHOLDERS_ALIAS[type].forEach(function(alias) {
            if (!Object.hasOwnProperty.call(PLACEHOLDERS_FLIPPED_ALIAS, alias)) {
                PLACEHOLDERS_FLIPPED_ALIAS[alias] = [];
            }
            PLACEHOLDERS_FLIPPED_ALIAS[alias].push(type);
        });
    });
    return placeholders;
}

var hasRequiredMisc;
function requireMisc() {
    if (hasRequiredMisc) return misc;
    hasRequiredMisc = 1;
    var _utils = requireUtils();
    var _placeholders = requirePlaceholders();
    {
        (0, _utils.default)("Noop", {
            visitor: []
        });
    }
    (0, _utils.default)("Placeholder", {
        visitor: [],
        builder: [
            "expectedNode",
            "name"
        ],
        fields: {
            name: {
                validate: (0, _utils.assertNodeType)("Identifier")
            },
            expectedNode: {
                validate: (_utils.assertOneOf).apply(this, _placeholders.PLACEHOLDERS)
            }
        }
    });
    (0, _utils.default)("V8IntrinsicIdentifier", {
        builder: [
            "name"
        ],
        fields: {
            name: {
                validate: (0, _utils.assertValueType)("string")
            }
        }
    });
    return misc;
}

var experimental = {};

var hasRequiredExperimental;
function requireExperimental() {
    if (hasRequiredExperimental) return experimental;
    hasRequiredExperimental = 1;
    var _utils = requireUtils();
    (0, _utils.default)("ArgumentPlaceholder", {});
    (0, _utils.default)("BindExpression", {
        visitor: [
            "object",
            "callee"
        ],
        aliases: [
            "Expression"
        ],
        fields: !process.env.BABEL_TYPES_8_BREAKING ? {
            object: {
                validate: Object.assign(function() {}, {
                    oneOfNodeTypes: [
                        "Expression"
                    ]
                })
            },
            callee: {
                validate: Object.assign(function() {}, {
                    oneOfNodeTypes: [
                        "Expression"
                    ]
                })
            }
        } : {
            object: {
                validate: (0, _utils.assertNodeType)("Expression")
            },
            callee: {
                validate: (0, _utils.assertNodeType)("Expression")
            }
        }
    });
    (0, _utils.default)("ImportAttribute", {
        visitor: [
            "key",
            "value"
        ],
        fields: {
            key: {
                validate: (0, _utils.assertNodeType)("Identifier", "StringLiteral")
            },
            value: {
                validate: (0, _utils.assertNodeType)("StringLiteral")
            }
        }
    });
    (0, _utils.default)("Decorator", {
        visitor: [
            "expression"
        ],
        fields: {
            expression: {
                validate: (0, _utils.assertNodeType)("Expression")
            }
        }
    });
    (0, _utils.default)("DoExpression", {
        visitor: [
            "body"
        ],
        builder: [
            "body",
            "async"
        ],
        aliases: [
            "Expression"
        ],
        fields: {
            body: {
                validate: (0, _utils.assertNodeType)("BlockStatement")
            },
            async: {
                validate: (0, _utils.assertValueType)("boolean"),
                default: false
            }
        }
    });
    (0, _utils.default)("ExportDefaultSpecifier", {
        visitor: [
            "exported"
        ],
        aliases: [
            "ModuleSpecifier"
        ],
        fields: {
            exported: {
                validate: (0, _utils.assertNodeType)("Identifier")
            }
        }
    });
    (0, _utils.default)("RecordExpression", {
        visitor: [
            "properties"
        ],
        aliases: [
            "Expression"
        ],
        fields: {
            properties: {
                validate: (0, _utils.chain)((0, _utils.assertValueType)("array"), (0, _utils.assertEach)((0, _utils.assertNodeType)("ObjectProperty", "SpreadElement")))
            }
        }
    });
    (0, _utils.default)("TupleExpression", {
        fields: {
            elements: {
                validate: (0, _utils.chain)((0, _utils.assertValueType)("array"), (0, _utils.assertEach)((0, _utils.assertNodeType)("Expression", "SpreadElement"))),
                default: []
            }
        },
        visitor: [
            "elements"
        ],
        aliases: [
            "Expression"
        ]
    });
    (0, _utils.default)("DecimalLiteral", {
        builder: [
            "value"
        ],
        fields: {
            value: {
                validate: (0, _utils.assertValueType)("string")
            }
        },
        aliases: [
            "Expression",
            "Pureish",
            "Literal",
            "Immutable"
        ]
    });
    (0, _utils.default)("StaticBlock", {
        visitor: [
            "body"
        ],
        fields: {
            body: {
                validate: (0, _utils.chain)((0, _utils.assertValueType)("array"), (0, _utils.assertEach)((0, _utils.assertNodeType)("Statement")))
            }
        },
        aliases: [
            "Scopable",
            "BlockParent"
        ]
    });
    (0, _utils.default)("ModuleExpression", {
        visitor: [
            "body"
        ],
        fields: {
            body: {
                validate: (0, _utils.assertNodeType)("Program")
            }
        },
        aliases: [
            "Expression"
        ]
    });
    (0, _utils.default)("TopicReference", {
        aliases: [
            "Expression"
        ]
    });
    (0, _utils.default)("PipelineTopicExpression", {
        builder: [
            "expression"
        ],
        visitor: [
            "expression"
        ],
        fields: {
            expression: {
                validate: (0, _utils.assertNodeType)("Expression")
            }
        },
        aliases: [
            "Expression"
        ]
    });
    (0, _utils.default)("PipelineBareFunction", {
        builder: [
            "callee"
        ],
        visitor: [
            "callee"
        ],
        fields: {
            callee: {
                validate: (0, _utils.assertNodeType)("Expression")
            }
        },
        aliases: [
            "Expression"
        ]
    });
    (0, _utils.default)("PipelinePrimaryTopicReference", {
        aliases: [
            "Expression"
        ]
    });
    return experimental;
}

var typescript = {};

function _createForOfIteratorHelperLoose$e(o, allowArrayLike) {
    var it = typeof Symbol !== "undefined" && o[Symbol.iterator] || o["@@iterator"];
    if (it) return (it = it.call(o)).next.bind(it);
    if (Array.isArray(o) || (it = unsupportedIterableToArray(o)) || allowArrayLike && o && typeof o.length === "number") {
        if (it) o = it;
        var i = 0;
        return function() {
            if (i >= o.length) return {
                done: true
            };
            return {
                done: false,
                value: o[i++]
            };
        };
    }
    throw new TypeError("Invalid attempt to iterate non-iterable instance.\\nIn order to be iterable, non-array objects must have a [Symbol.iterator]() method.");
}
var hasRequiredTypescript;
function requireTypescript() {
    if (hasRequiredTypescript) return typescript;
    hasRequiredTypescript = 1;
    var _utils = requireUtils();
    var _core = requireCore();
    var _is = requireIs();
    var bool = (0, _utils.assertValueType)("boolean");
    var tSFunctionTypeAnnotationCommon = {
        returnType: {
            validate: (0, _utils.assertNodeType)("TSTypeAnnotation", "Noop"),
            optional: true
        },
        typeParameters: {
            validate: (0, _utils.assertNodeType)("TSTypeParameterDeclaration", "Noop"),
            optional: true
        }
    };
    (0, _utils.default)("TSParameterProperty", {
        aliases: [
            "LVal"
        ],
        visitor: [
            "parameter"
        ],
        fields: {
            accessibility: {
                validate: (0, _utils.assertOneOf)("public", "private", "protected"),
                optional: true
            },
            readonly: {
                validate: (0, _utils.assertValueType)("boolean"),
                optional: true
            },
            parameter: {
                validate: (0, _utils.assertNodeType)("Identifier", "AssignmentPattern")
            },
            override: {
                validate: (0, _utils.assertValueType)("boolean"),
                optional: true
            },
            decorators: {
                validate: (0, _utils.chain)((0, _utils.assertValueType)("array"), (0, _utils.assertEach)((0, _utils.assertNodeType)("Decorator"))),
                optional: true
            }
        }
    });
    (0, _utils.default)("TSDeclareFunction", {
        aliases: [
            "Statement",
            "Declaration"
        ],
        visitor: [
            "id",
            "typeParameters",
            "params",
            "returnType"
        ],
        fields: Object.assign({}, _core.functionDeclarationCommon, tSFunctionTypeAnnotationCommon)
    });
    (0, _utils.default)("TSDeclareMethod", {
        visitor: [
            "decorators",
            "key",
            "typeParameters",
            "params",
            "returnType"
        ],
        fields: Object.assign({}, _core.classMethodOrDeclareMethodCommon, tSFunctionTypeAnnotationCommon)
    });
    (0, _utils.default)("TSQualifiedName", {
        aliases: [
            "TSEntityName"
        ],
        visitor: [
            "left",
            "right"
        ],
        fields: {
            left: (0, _utils.validateType)("TSEntityName"),
            right: (0, _utils.validateType)("Identifier")
        }
    });
    var signatureDeclarationCommon = {
        typeParameters: (0, _utils.validateOptionalType)("TSTypeParameterDeclaration"),
        parameters: (0, _utils.validateArrayOfType)([
            "Identifier",
            "RestElement"
        ]),
        typeAnnotation: (0, _utils.validateOptionalType)("TSTypeAnnotation")
    };
    var callConstructSignatureDeclaration = {
        aliases: [
            "TSTypeElement"
        ],
        visitor: [
            "typeParameters",
            "parameters",
            "typeAnnotation"
        ],
        fields: signatureDeclarationCommon
    };
    (0, _utils.default)("TSCallSignatureDeclaration", callConstructSignatureDeclaration);
    (0, _utils.default)("TSConstructSignatureDeclaration", callConstructSignatureDeclaration);
    var namedTypeElementCommon = {
        key: (0, _utils.validateType)("Expression"),
        computed: (0, _utils.validate)(bool),
        optional: (0, _utils.validateOptional)(bool)
    };
    (0, _utils.default)("TSPropertySignature", {
        aliases: [
            "TSTypeElement"
        ],
        visitor: [
            "key",
            "typeAnnotation",
            "initializer"
        ],
        fields: Object.assign({}, namedTypeElementCommon, {
            readonly: (0, _utils.validateOptional)(bool),
            typeAnnotation: (0, _utils.validateOptionalType)("TSTypeAnnotation"),
            initializer: (0, _utils.validateOptionalType)("Expression"),
            kind: {
                validate: (0, _utils.assertOneOf)("get", "set")
            }
        })
    });
    (0, _utils.default)("TSMethodSignature", {
        aliases: [
            "TSTypeElement"
        ],
        visitor: [
            "key",
            "typeParameters",
            "parameters",
            "typeAnnotation"
        ],
        fields: Object.assign({}, signatureDeclarationCommon, namedTypeElementCommon, {
            kind: {
                validate: (0, _utils.assertOneOf)("method", "get", "set")
            }
        })
    });
    (0, _utils.default)("TSIndexSignature", {
        aliases: [
            "TSTypeElement"
        ],
        visitor: [
            "parameters",
            "typeAnnotation"
        ],
        fields: {
            readonly: (0, _utils.validateOptional)(bool),
            static: (0, _utils.validateOptional)(bool),
            parameters: (0, _utils.validateArrayOfType)("Identifier"),
            typeAnnotation: (0, _utils.validateOptionalType)("TSTypeAnnotation")
        }
    });
    var tsKeywordTypes = [
        "TSAnyKeyword",
        "TSBooleanKeyword",
        "TSBigIntKeyword",
        "TSIntrinsicKeyword",
        "TSNeverKeyword",
        "TSNullKeyword",
        "TSNumberKeyword",
        "TSObjectKeyword",
        "TSStringKeyword",
        "TSSymbolKeyword",
        "TSUndefinedKeyword",
        "TSUnknownKeyword",
        "TSVoidKeyword"
    ];
    for(var _iterator = _createForOfIteratorHelperLoose$e(tsKeywordTypes), _step; !(_step = _iterator()).done;){
        var type = _step.value;
        (0, _utils.default)(type, {
            aliases: [
                "TSType",
                "TSBaseType"
            ],
            visitor: [],
            fields: {}
        });
    }
    (0, _utils.default)("TSThisType", {
        aliases: [
            "TSType",
            "TSBaseType"
        ],
        visitor: [],
        fields: {}
    });
    var fnOrCtrBase = {
        aliases: [
            "TSType"
        ],
        visitor: [
            "typeParameters",
            "parameters",
            "typeAnnotation"
        ]
    };
    (0, _utils.default)("TSFunctionType", Object.assign({}, fnOrCtrBase, {
        fields: signatureDeclarationCommon
    }));
    (0, _utils.default)("TSConstructorType", Object.assign({}, fnOrCtrBase, {
        fields: Object.assign({}, signatureDeclarationCommon, {
            abstract: (0, _utils.validateOptional)(bool)
        })
    }));
    (0, _utils.default)("TSTypeReference", {
        aliases: [
            "TSType"
        ],
        visitor: [
            "typeName",
            "typeParameters"
        ],
        fields: {
            typeName: (0, _utils.validateType)("TSEntityName"),
            typeParameters: (0, _utils.validateOptionalType)("TSTypeParameterInstantiation")
        }
    });
    (0, _utils.default)("TSTypePredicate", {
        aliases: [
            "TSType"
        ],
        visitor: [
            "parameterName",
            "typeAnnotation"
        ],
        builder: [
            "parameterName",
            "typeAnnotation",
            "asserts"
        ],
        fields: {
            parameterName: (0, _utils.validateType)([
                "Identifier",
                "TSThisType"
            ]),
            typeAnnotation: (0, _utils.validateOptionalType)("TSTypeAnnotation"),
            asserts: (0, _utils.validateOptional)(bool)
        }
    });
    (0, _utils.default)("TSTypeQuery", {
        aliases: [
            "TSType"
        ],
        visitor: [
            "exprName"
        ],
        fields: {
            exprName: (0, _utils.validateType)([
                "TSEntityName",
                "TSImportType"
            ])
        }
    });
    (0, _utils.default)("TSTypeLiteral", {
        aliases: [
            "TSType"
        ],
        visitor: [
            "members"
        ],
        fields: {
            members: (0, _utils.validateArrayOfType)("TSTypeElement")
        }
    });
    (0, _utils.default)("TSArrayType", {
        aliases: [
            "TSType"
        ],
        visitor: [
            "elementType"
        ],
        fields: {
            elementType: (0, _utils.validateType)("TSType")
        }
    });
    (0, _utils.default)("TSTupleType", {
        aliases: [
            "TSType"
        ],
        visitor: [
            "elementTypes"
        ],
        fields: {
            elementTypes: (0, _utils.validateArrayOfType)([
                "TSType",
                "TSNamedTupleMember"
            ])
        }
    });
    (0, _utils.default)("TSOptionalType", {
        aliases: [
            "TSType"
        ],
        visitor: [
            "typeAnnotation"
        ],
        fields: {
            typeAnnotation: (0, _utils.validateType)("TSType")
        }
    });
    (0, _utils.default)("TSRestType", {
        aliases: [
            "TSType"
        ],
        visitor: [
            "typeAnnotation"
        ],
        fields: {
            typeAnnotation: (0, _utils.validateType)("TSType")
        }
    });
    (0, _utils.default)("TSNamedTupleMember", {
        visitor: [
            "label",
            "elementType"
        ],
        builder: [
            "label",
            "elementType",
            "optional"
        ],
        fields: {
            label: (0, _utils.validateType)("Identifier"),
            optional: {
                validate: bool,
                default: false
            },
            elementType: (0, _utils.validateType)("TSType")
        }
    });
    var unionOrIntersection = {
        aliases: [
            "TSType"
        ],
        visitor: [
            "types"
        ],
        fields: {
            types: (0, _utils.validateArrayOfType)("TSType")
        }
    };
    (0, _utils.default)("TSUnionType", unionOrIntersection);
    (0, _utils.default)("TSIntersectionType", unionOrIntersection);
    (0, _utils.default)("TSConditionalType", {
        aliases: [
            "TSType"
        ],
        visitor: [
            "checkType",
            "extendsType",
            "trueType",
            "falseType"
        ],
        fields: {
            checkType: (0, _utils.validateType)("TSType"),
            extendsType: (0, _utils.validateType)("TSType"),
            trueType: (0, _utils.validateType)("TSType"),
            falseType: (0, _utils.validateType)("TSType")
        }
    });
    (0, _utils.default)("TSInferType", {
        aliases: [
            "TSType"
        ],
        visitor: [
            "typeParameter"
        ],
        fields: {
            typeParameter: (0, _utils.validateType)("TSTypeParameter")
        }
    });
    (0, _utils.default)("TSParenthesizedType", {
        aliases: [
            "TSType"
        ],
        visitor: [
            "typeAnnotation"
        ],
        fields: {
            typeAnnotation: (0, _utils.validateType)("TSType")
        }
    });
    (0, _utils.default)("TSTypeOperator", {
        aliases: [
            "TSType"
        ],
        visitor: [
            "typeAnnotation"
        ],
        fields: {
            operator: (0, _utils.validate)((0, _utils.assertValueType)("string")),
            typeAnnotation: (0, _utils.validateType)("TSType")
        }
    });
    (0, _utils.default)("TSIndexedAccessType", {
        aliases: [
            "TSType"
        ],
        visitor: [
            "objectType",
            "indexType"
        ],
        fields: {
            objectType: (0, _utils.validateType)("TSType"),
            indexType: (0, _utils.validateType)("TSType")
        }
    });
    (0, _utils.default)("TSMappedType", {
        aliases: [
            "TSType"
        ],
        visitor: [
            "typeParameter",
            "typeAnnotation",
            "nameType"
        ],
        fields: {
            readonly: (0, _utils.validateOptional)(bool),
            typeParameter: (0, _utils.validateType)("TSTypeParameter"),
            optional: (0, _utils.validateOptional)(bool),
            typeAnnotation: (0, _utils.validateOptionalType)("TSType"),
            nameType: (0, _utils.validateOptionalType)("TSType")
        }
    });
    (0, _utils.default)("TSLiteralType", {
        aliases: [
            "TSType",
            "TSBaseType"
        ],
        visitor: [
            "literal"
        ],
        fields: {
            literal: {
                validate: function() {
                    var validator = function validator(parent, key, node) {
                        if ((0, _is.default)("UnaryExpression", node)) {
                            unaryOperator(node, "operator", node.operator);
                            unaryExpression(node, "argument", node.argument);
                        } else {
                            literal(parent, key, node);
                        }
                    };
                    var unaryExpression = (0, _utils.assertNodeType)("NumericLiteral", "BigIntLiteral");
                    var unaryOperator = (0, _utils.assertOneOf)("-");
                    var literal = (0, _utils.assertNodeType)("NumericLiteral", "StringLiteral", "BooleanLiteral", "BigIntLiteral");
                    validator.oneOfNodeTypes = [
                        "NumericLiteral",
                        "StringLiteral",
                        "BooleanLiteral",
                        "BigIntLiteral",
                        "UnaryExpression"
                    ];
                    return validator;
                }()
            }
        }
    });
    (0, _utils.default)("TSExpressionWithTypeArguments", {
        aliases: [
            "TSType"
        ],
        visitor: [
            "expression",
            "typeParameters"
        ],
        fields: {
            expression: (0, _utils.validateType)("TSEntityName"),
            typeParameters: (0, _utils.validateOptionalType)("TSTypeParameterInstantiation")
        }
    });
    (0, _utils.default)("TSInterfaceDeclaration", {
        aliases: [
            "Statement",
            "Declaration"
        ],
        visitor: [
            "id",
            "typeParameters",
            "extends",
            "body"
        ],
        fields: {
            declare: (0, _utils.validateOptional)(bool),
            id: (0, _utils.validateType)("Identifier"),
            typeParameters: (0, _utils.validateOptionalType)("TSTypeParameterDeclaration"),
            extends: (0, _utils.validateOptional)((0, _utils.arrayOfType)("TSExpressionWithTypeArguments")),
            body: (0, _utils.validateType)("TSInterfaceBody")
        }
    });
    (0, _utils.default)("TSInterfaceBody", {
        visitor: [
            "body"
        ],
        fields: {
            body: (0, _utils.validateArrayOfType)("TSTypeElement")
        }
    });
    (0, _utils.default)("TSTypeAliasDeclaration", {
        aliases: [
            "Statement",
            "Declaration"
        ],
        visitor: [
            "id",
            "typeParameters",
            "typeAnnotation"
        ],
        fields: {
            declare: (0, _utils.validateOptional)(bool),
            id: (0, _utils.validateType)("Identifier"),
            typeParameters: (0, _utils.validateOptionalType)("TSTypeParameterDeclaration"),
            typeAnnotation: (0, _utils.validateType)("TSType")
        }
    });
    (0, _utils.default)("TSAsExpression", {
        aliases: [
            "Expression"
        ],
        visitor: [
            "expression",
            "typeAnnotation"
        ],
        fields: {
            expression: (0, _utils.validateType)("Expression"),
            typeAnnotation: (0, _utils.validateType)("TSType")
        }
    });
    (0, _utils.default)("TSTypeAssertion", {
        aliases: [
            "Expression"
        ],
        visitor: [
            "typeAnnotation",
            "expression"
        ],
        fields: {
            typeAnnotation: (0, _utils.validateType)("TSType"),
            expression: (0, _utils.validateType)("Expression")
        }
    });
    (0, _utils.default)("TSEnumDeclaration", {
        aliases: [
            "Statement",
            "Declaration"
        ],
        visitor: [
            "id",
            "members"
        ],
        fields: {
            declare: (0, _utils.validateOptional)(bool),
            const: (0, _utils.validateOptional)(bool),
            id: (0, _utils.validateType)("Identifier"),
            members: (0, _utils.validateArrayOfType)("TSEnumMember"),
            initializer: (0, _utils.validateOptionalType)("Expression")
        }
    });
    (0, _utils.default)("TSEnumMember", {
        visitor: [
            "id",
            "initializer"
        ],
        fields: {
            id: (0, _utils.validateType)([
                "Identifier",
                "StringLiteral"
            ]),
            initializer: (0, _utils.validateOptionalType)("Expression")
        }
    });
    (0, _utils.default)("TSModuleDeclaration", {
        aliases: [
            "Statement",
            "Declaration"
        ],
        visitor: [
            "id",
            "body"
        ],
        fields: {
            declare: (0, _utils.validateOptional)(bool),
            global: (0, _utils.validateOptional)(bool),
            id: (0, _utils.validateType)([
                "Identifier",
                "StringLiteral"
            ]),
            body: (0, _utils.validateType)([
                "TSModuleBlock",
                "TSModuleDeclaration"
            ])
        }
    });
    (0, _utils.default)("TSModuleBlock", {
        aliases: [
            "Scopable",
            "Block",
            "BlockParent"
        ],
        visitor: [
            "body"
        ],
        fields: {
            body: (0, _utils.validateArrayOfType)("Statement")
        }
    });
    (0, _utils.default)("TSImportType", {
        aliases: [
            "TSType"
        ],
        visitor: [
            "argument",
            "qualifier",
            "typeParameters"
        ],
        fields: {
            argument: (0, _utils.validateType)("StringLiteral"),
            qualifier: (0, _utils.validateOptionalType)("TSEntityName"),
            typeParameters: (0, _utils.validateOptionalType)("TSTypeParameterInstantiation")
        }
    });
    (0, _utils.default)("TSImportEqualsDeclaration", {
        aliases: [
            "Statement"
        ],
        visitor: [
            "id",
            "moduleReference"
        ],
        fields: {
            isExport: (0, _utils.validate)(bool),
            id: (0, _utils.validateType)("Identifier"),
            moduleReference: (0, _utils.validateType)([
                "TSEntityName",
                "TSExternalModuleReference"
            ]),
            importKind: {
                validate: (0, _utils.assertOneOf)("type", "value"),
                optional: true
            }
        }
    });
    (0, _utils.default)("TSExternalModuleReference", {
        visitor: [
            "expression"
        ],
        fields: {
            expression: (0, _utils.validateType)("StringLiteral")
        }
    });
    (0, _utils.default)("TSNonNullExpression", {
        aliases: [
            "Expression"
        ],
        visitor: [
            "expression"
        ],
        fields: {
            expression: (0, _utils.validateType)("Expression")
        }
    });
    (0, _utils.default)("TSExportAssignment", {
        aliases: [
            "Statement"
        ],
        visitor: [
            "expression"
        ],
        fields: {
            expression: (0, _utils.validateType)("Expression")
        }
    });
    (0, _utils.default)("TSNamespaceExportDeclaration", {
        aliases: [
            "Statement"
        ],
        visitor: [
            "id"
        ],
        fields: {
            id: (0, _utils.validateType)("Identifier")
        }
    });
    (0, _utils.default)("TSTypeAnnotation", {
        visitor: [
            "typeAnnotation"
        ],
        fields: {
            typeAnnotation: {
                validate: (0, _utils.assertNodeType)("TSType")
            }
        }
    });
    (0, _utils.default)("TSTypeParameterInstantiation", {
        visitor: [
            "params"
        ],
        fields: {
            params: {
                validate: (0, _utils.chain)((0, _utils.assertValueType)("array"), (0, _utils.assertEach)((0, _utils.assertNodeType)("TSType")))
            }
        }
    });
    (0, _utils.default)("TSTypeParameterDeclaration", {
        visitor: [
            "params"
        ],
        fields: {
            params: {
                validate: (0, _utils.chain)((0, _utils.assertValueType)("array"), (0, _utils.assertEach)((0, _utils.assertNodeType)("TSTypeParameter")))
            }
        }
    });
    (0, _utils.default)("TSTypeParameter", {
        builder: [
            "constraint",
            "default",
            "name"
        ],
        visitor: [
            "constraint",
            "default"
        ],
        fields: {
            name: {
                validate: (0, _utils.assertValueType)("string")
            },
            constraint: {
                validate: (0, _utils.assertNodeType)("TSType"),
                optional: true
            },
            default: {
                validate: (0, _utils.assertNodeType)("TSType"),
                optional: true
            }
        }
    });
    return typescript;
}

var hasRequiredDefinitions;
function requireDefinitions() {
    if (hasRequiredDefinitions) return definitions;
    hasRequiredDefinitions = 1;
    (function(exports) {
        Object.defineProperty(exports, "__esModule", {
            value: true
        });
        Object.defineProperty(exports, "VISITOR_KEYS", {
            enumerable: true,
            get: function get() {
                return _utils.VISITOR_KEYS;
            }
        });
        Object.defineProperty(exports, "ALIAS_KEYS", {
            enumerable: true,
            get: function get() {
                return _utils.ALIAS_KEYS;
            }
        });
        Object.defineProperty(exports, "FLIPPED_ALIAS_KEYS", {
            enumerable: true,
            get: function get() {
                return _utils.FLIPPED_ALIAS_KEYS;
            }
        });
        Object.defineProperty(exports, "NODE_FIELDS", {
            enumerable: true,
            get: function get() {
                return _utils.NODE_FIELDS;
            }
        });
        Object.defineProperty(exports, "BUILDER_KEYS", {
            enumerable: true,
            get: function get() {
                return _utils.BUILDER_KEYS;
            }
        });
        Object.defineProperty(exports, "DEPRECATED_KEYS", {
            enumerable: true,
            get: function get() {
                return _utils.DEPRECATED_KEYS;
            }
        });
        Object.defineProperty(exports, "NODE_PARENT_VALIDATIONS", {
            enumerable: true,
            get: function get() {
                return _utils.NODE_PARENT_VALIDATIONS;
            }
        });
        Object.defineProperty(exports, "PLACEHOLDERS", {
            enumerable: true,
            get: function get() {
                return _placeholders.PLACEHOLDERS;
            }
        });
        Object.defineProperty(exports, "PLACEHOLDERS_ALIAS", {
            enumerable: true,
            get: function get() {
                return _placeholders.PLACEHOLDERS_ALIAS;
            }
        });
        Object.defineProperty(exports, "PLACEHOLDERS_FLIPPED_ALIAS", {
            enumerable: true,
            get: function get() {
                return _placeholders.PLACEHOLDERS_FLIPPED_ALIAS;
            }
        });
        exports.TYPES = void 0;
        var _toFastProperties = requireToFastProperties();
        requireCore();
        requireFlow();
        requireJsx();
        requireMisc();
        requireExperimental();
        requireTypescript();
        var _utils = requireUtils();
        var _placeholders = requirePlaceholders();
        _toFastProperties(_utils.VISITOR_KEYS);
        _toFastProperties(_utils.ALIAS_KEYS);
        _toFastProperties(_utils.FLIPPED_ALIAS_KEYS);
        _toFastProperties(_utils.NODE_FIELDS);
        _toFastProperties(_utils.BUILDER_KEYS);
        _toFastProperties(_utils.DEPRECATED_KEYS);
        _toFastProperties(_placeholders.PLACEHOLDERS_ALIAS);
        _toFastProperties(_placeholders.PLACEHOLDERS_FLIPPED_ALIAS);
        var TYPES = Object.keys(_utils.VISITOR_KEYS).concat(Object.keys(_utils.FLIPPED_ALIAS_KEYS)).concat(Object.keys(_utils.DEPRECATED_KEYS));
        exports.TYPES = TYPES;
    })(definitions);
    return definitions;
}

function _createForOfIteratorHelperLoose$d(o, allowArrayLike) {
    var it = typeof Symbol !== "undefined" && o[Symbol.iterator] || o["@@iterator"];
    if (it) return (it = it.call(o)).next.bind(it);
    if (Array.isArray(o) || (it = unsupportedIterableToArray(o)) || allowArrayLike && o && typeof o.length === "number") {
        if (it) o = it;
        var i = 0;
        return function() {
            if (i >= o.length) return {
                done: true
            };
            return {
                done: false,
                value: o[i++]
            };
        };
    }
    throw new TypeError("Invalid attempt to iterate non-iterable instance.\\nIn order to be iterable, non-array objects must have a [Symbol.iterator]() method.");
}
Object.defineProperty(builder$1, "__esModule", {
    value: true
});
builder$1.default = builder;
var _definitions$6 = requireDefinitions();
var _validate = requireValidate();
function builder(type) {
    for(var _len = arguments.length, args = new Array(_len > 1 ? _len - 1 : 0), _key = 1; _key < _len; _key++){
        args[_key - 1] = arguments[_key];
    }
    var keys = _definitions$6.BUILDER_KEYS[type];
    var countArgs = args.length;
    if (countArgs > keys.length) {
        throw new Error(type + ": Too many arguments passed. Received " + countArgs + " but can receive no more than " + keys.length);
    }
    var node = {
        type: type
    };
    var i = 0;
    keys.forEach(function(key) {
        var field = _definitions$6.NODE_FIELDS[type][key];
        var arg;
        if (i < countArgs) arg = args[i];
        if (arg === undefined) {
            arg = Array.isArray(field.default) ? [] : field.default;
        }
        node[key] = arg;
        i++;
    });
    for(var _iterator = _createForOfIteratorHelperLoose$d(Object.keys(node)), _step; !(_step = _iterator()).done;){
        var key = _step.value;
        (0, _validate.default)(node, key, node[key]);
    }
    return node;
}

function _arrayLikeToArray$1(arr, len) {
    if (len == null || len > arr.length) len = arr.length;
    for(var i = 0, arr2 = new Array(len); i < len; i++)arr2[i] = arr[i];
    return arr2;
}
function _arrayWithoutHoles$1(arr) {
    if (Array.isArray(arr)) return _arrayLikeToArray$1(arr);
}
function _iterableToArray$1(iter) {
    if (typeof Symbol !== "undefined" && iter[Symbol.iterator] != null || iter["@@iterator"] != null) return Array.from(iter);
}
function _nonIterableSpread$1() {
    throw new TypeError("Invalid attempt to spread non-iterable instance.\\nIn order to be iterable, non-array objects must have a [Symbol.iterator]() method.");
}
function _toConsumableArray$1(arr) {
    return _arrayWithoutHoles$1(arr) || _iterableToArray$1(arr) || _unsupportedIterableToArray$1(arr) || _nonIterableSpread$1();
}
function _unsupportedIterableToArray$1(o, minLen) {
    if (!o) return;
    if (typeof o === "string") return _arrayLikeToArray$1(o, minLen);
    var n = Object.prototype.toString.call(o).slice(8, -1);
    if (n === "Object" && o.constructor) n = o.constructor.name;
    if (n === "Map" || n === "Set") return Array.from(n);
    if (n === "Arguments" || /^(?:Ui|I)nt(?:8|16|32)(?:Clamped)?Array$/.test(n)) return _arrayLikeToArray$1(o, minLen);
}
Object.defineProperty(generated$3, "__esModule", {
    value: true
});
generated$3.arrayExpression = arrayExpression;
generated$3.assignmentExpression = assignmentExpression;
generated$3.binaryExpression = binaryExpression;
generated$3.interpreterDirective = interpreterDirective;
generated$3.directive = directive;
generated$3.directiveLiteral = directiveLiteral;
generated$3.blockStatement = blockStatement;
generated$3.breakStatement = breakStatement;
generated$3.callExpression = callExpression;
generated$3.catchClause = catchClause;
generated$3.conditionalExpression = conditionalExpression;
generated$3.continueStatement = continueStatement;
generated$3.debuggerStatement = debuggerStatement;
generated$3.doWhileStatement = doWhileStatement;
generated$3.emptyStatement = emptyStatement;
generated$3.expressionStatement = expressionStatement;
generated$3.file = file;
generated$3.forInStatement = forInStatement;
generated$3.forStatement = forStatement;
generated$3.functionDeclaration = functionDeclaration;
generated$3.functionExpression = functionExpression;
generated$3.identifier = identifier;
generated$3.ifStatement = ifStatement;
generated$3.labeledStatement = labeledStatement;
generated$3.stringLiteral = stringLiteral;
generated$3.numericLiteral = numericLiteral;
generated$3.nullLiteral = nullLiteral;
generated$3.booleanLiteral = booleanLiteral;
generated$3.regExpLiteral = regExpLiteral;
generated$3.logicalExpression = logicalExpression;
generated$3.memberExpression = memberExpression;
generated$3.newExpression = newExpression;
generated$3.program = program;
generated$3.objectExpression = objectExpression;
generated$3.objectMethod = objectMethod;
generated$3.objectProperty = objectProperty;
generated$3.restElement = restElement;
generated$3.returnStatement = returnStatement;
generated$3.sequenceExpression = sequenceExpression;
generated$3.parenthesizedExpression = parenthesizedExpression;
generated$3.switchCase = switchCase;
generated$3.switchStatement = switchStatement;
generated$3.thisExpression = thisExpression;
generated$3.throwStatement = throwStatement;
generated$3.tryStatement = tryStatement;
generated$3.unaryExpression = unaryExpression;
generated$3.updateExpression = updateExpression;
generated$3.variableDeclaration = variableDeclaration;
generated$3.variableDeclarator = variableDeclarator;
generated$3.whileStatement = whileStatement;
generated$3.withStatement = withStatement;
generated$3.assignmentPattern = assignmentPattern;
generated$3.arrayPattern = arrayPattern;
generated$3.arrowFunctionExpression = arrowFunctionExpression;
generated$3.classBody = classBody;
generated$3.classExpression = classExpression;
generated$3.classDeclaration = classDeclaration;
generated$3.exportAllDeclaration = exportAllDeclaration;
generated$3.exportDefaultDeclaration = exportDefaultDeclaration;
generated$3.exportNamedDeclaration = exportNamedDeclaration;
generated$3.exportSpecifier = exportSpecifier;
generated$3.forOfStatement = forOfStatement;
generated$3.importDeclaration = importDeclaration;
generated$3.importDefaultSpecifier = importDefaultSpecifier;
generated$3.importNamespaceSpecifier = importNamespaceSpecifier;
generated$3.importSpecifier = importSpecifier;
generated$3.metaProperty = metaProperty;
generated$3.classMethod = classMethod;
generated$3.objectPattern = objectPattern;
generated$3.spreadElement = spreadElement;
generated$3.super = _super;
generated$3.taggedTemplateExpression = taggedTemplateExpression;
generated$3.templateElement = templateElement;
generated$3.templateLiteral = templateLiteral;
generated$3.yieldExpression = yieldExpression;
generated$3.awaitExpression = awaitExpression;
generated$3.import = _import;
generated$3.bigIntLiteral = bigIntLiteral;
generated$3.exportNamespaceSpecifier = exportNamespaceSpecifier;
generated$3.optionalMemberExpression = optionalMemberExpression;
generated$3.optionalCallExpression = optionalCallExpression;
generated$3.classProperty = classProperty;
generated$3.classPrivateProperty = classPrivateProperty;
generated$3.classPrivateMethod = classPrivateMethod;
generated$3.privateName = privateName;
generated$3.anyTypeAnnotation = anyTypeAnnotation;
generated$3.arrayTypeAnnotation = arrayTypeAnnotation;
generated$3.booleanTypeAnnotation = booleanTypeAnnotation;
generated$3.booleanLiteralTypeAnnotation = booleanLiteralTypeAnnotation;
generated$3.nullLiteralTypeAnnotation = nullLiteralTypeAnnotation;
generated$3.classImplements = classImplements;
generated$3.declareClass = declareClass;
generated$3.declareFunction = declareFunction;
generated$3.declareInterface = declareInterface;
generated$3.declareModule = declareModule;
generated$3.declareModuleExports = declareModuleExports;
generated$3.declareTypeAlias = declareTypeAlias;
generated$3.declareOpaqueType = declareOpaqueType;
generated$3.declareVariable = declareVariable;
generated$3.declareExportDeclaration = declareExportDeclaration;
generated$3.declareExportAllDeclaration = declareExportAllDeclaration;
generated$3.declaredPredicate = declaredPredicate;
generated$3.existsTypeAnnotation = existsTypeAnnotation;
generated$3.functionTypeAnnotation = functionTypeAnnotation;
generated$3.functionTypeParam = functionTypeParam;
generated$3.genericTypeAnnotation = genericTypeAnnotation;
generated$3.inferredPredicate = inferredPredicate;
generated$3.interfaceExtends = interfaceExtends;
generated$3.interfaceDeclaration = interfaceDeclaration;
generated$3.interfaceTypeAnnotation = interfaceTypeAnnotation;
generated$3.intersectionTypeAnnotation = intersectionTypeAnnotation;
generated$3.mixedTypeAnnotation = mixedTypeAnnotation;
generated$3.emptyTypeAnnotation = emptyTypeAnnotation;
generated$3.nullableTypeAnnotation = nullableTypeAnnotation;
generated$3.numberLiteralTypeAnnotation = numberLiteralTypeAnnotation;
generated$3.numberTypeAnnotation = numberTypeAnnotation;
generated$3.objectTypeAnnotation = objectTypeAnnotation;
generated$3.objectTypeInternalSlot = objectTypeInternalSlot;
generated$3.objectTypeCallProperty = objectTypeCallProperty;
generated$3.objectTypeIndexer = objectTypeIndexer;
generated$3.objectTypeProperty = objectTypeProperty;
generated$3.objectTypeSpreadProperty = objectTypeSpreadProperty;
generated$3.opaqueType = opaqueType;
generated$3.qualifiedTypeIdentifier = qualifiedTypeIdentifier;
generated$3.stringLiteralTypeAnnotation = stringLiteralTypeAnnotation;
generated$3.stringTypeAnnotation = stringTypeAnnotation;
generated$3.symbolTypeAnnotation = symbolTypeAnnotation;
generated$3.thisTypeAnnotation = thisTypeAnnotation;
generated$3.tupleTypeAnnotation = tupleTypeAnnotation;
generated$3.typeofTypeAnnotation = typeofTypeAnnotation;
generated$3.typeAlias = typeAlias;
generated$3.typeAnnotation = typeAnnotation;
generated$3.typeCastExpression = typeCastExpression;
generated$3.typeParameter = typeParameter;
generated$3.typeParameterDeclaration = typeParameterDeclaration;
generated$3.typeParameterInstantiation = typeParameterInstantiation;
generated$3.unionTypeAnnotation = unionTypeAnnotation;
generated$3.variance = variance;
generated$3.voidTypeAnnotation = voidTypeAnnotation;
generated$3.enumDeclaration = enumDeclaration;
generated$3.enumBooleanBody = enumBooleanBody;
generated$3.enumNumberBody = enumNumberBody;
generated$3.enumStringBody = enumStringBody;
generated$3.enumSymbolBody = enumSymbolBody;
generated$3.enumBooleanMember = enumBooleanMember;
generated$3.enumNumberMember = enumNumberMember;
generated$3.enumStringMember = enumStringMember;
generated$3.enumDefaultedMember = enumDefaultedMember;
generated$3.indexedAccessType = indexedAccessType;
generated$3.optionalIndexedAccessType = optionalIndexedAccessType;
generated$3.jSXAttribute = generated$3.jsxAttribute = jsxAttribute;
generated$3.jSXClosingElement = generated$3.jsxClosingElement = jsxClosingElement;
generated$3.jSXElement = generated$3.jsxElement = jsxElement;
generated$3.jSXEmptyExpression = generated$3.jsxEmptyExpression = jsxEmptyExpression;
generated$3.jSXExpressionContainer = generated$3.jsxExpressionContainer = jsxExpressionContainer;
generated$3.jSXSpreadChild = generated$3.jsxSpreadChild = jsxSpreadChild;
generated$3.jSXIdentifier = generated$3.jsxIdentifier = jsxIdentifier;
generated$3.jSXMemberExpression = generated$3.jsxMemberExpression = jsxMemberExpression;
generated$3.jSXNamespacedName = generated$3.jsxNamespacedName = jsxNamespacedName;
generated$3.jSXOpeningElement = generated$3.jsxOpeningElement = jsxOpeningElement;
generated$3.jSXSpreadAttribute = generated$3.jsxSpreadAttribute = jsxSpreadAttribute;
generated$3.jSXText = generated$3.jsxText = jsxText;
generated$3.jSXFragment = generated$3.jsxFragment = jsxFragment;
generated$3.jSXOpeningFragment = generated$3.jsxOpeningFragment = jsxOpeningFragment;
generated$3.jSXClosingFragment = generated$3.jsxClosingFragment = jsxClosingFragment;
generated$3.noop = noop;
generated$3.placeholder = placeholder;
generated$3.v8IntrinsicIdentifier = v8IntrinsicIdentifier;
generated$3.argumentPlaceholder = argumentPlaceholder;
generated$3.bindExpression = bindExpression;
generated$3.importAttribute = importAttribute;
generated$3.decorator = decorator;
generated$3.doExpression = doExpression;
generated$3.exportDefaultSpecifier = exportDefaultSpecifier;
generated$3.recordExpression = recordExpression;
generated$3.tupleExpression = tupleExpression;
generated$3.decimalLiteral = decimalLiteral;
generated$3.staticBlock = staticBlock;
generated$3.moduleExpression = moduleExpression;
generated$3.topicReference = topicReference;
generated$3.pipelineTopicExpression = pipelineTopicExpression;
generated$3.pipelineBareFunction = pipelineBareFunction;
generated$3.pipelinePrimaryTopicReference = pipelinePrimaryTopicReference;
generated$3.tSParameterProperty = generated$3.tsParameterProperty = tsParameterProperty;
generated$3.tSDeclareFunction = generated$3.tsDeclareFunction = tsDeclareFunction;
generated$3.tSDeclareMethod = generated$3.tsDeclareMethod = tsDeclareMethod;
generated$3.tSQualifiedName = generated$3.tsQualifiedName = tsQualifiedName;
generated$3.tSCallSignatureDeclaration = generated$3.tsCallSignatureDeclaration = tsCallSignatureDeclaration;
generated$3.tSConstructSignatureDeclaration = generated$3.tsConstructSignatureDeclaration = tsConstructSignatureDeclaration;
generated$3.tSPropertySignature = generated$3.tsPropertySignature = tsPropertySignature;
generated$3.tSMethodSignature = generated$3.tsMethodSignature = tsMethodSignature;
generated$3.tSIndexSignature = generated$3.tsIndexSignature = tsIndexSignature;
generated$3.tSAnyKeyword = generated$3.tsAnyKeyword = tsAnyKeyword;
generated$3.tSBooleanKeyword = generated$3.tsBooleanKeyword = tsBooleanKeyword;
generated$3.tSBigIntKeyword = generated$3.tsBigIntKeyword = tsBigIntKeyword;
generated$3.tSIntrinsicKeyword = generated$3.tsIntrinsicKeyword = tsIntrinsicKeyword;
generated$3.tSNeverKeyword = generated$3.tsNeverKeyword = tsNeverKeyword;
generated$3.tSNullKeyword = generated$3.tsNullKeyword = tsNullKeyword;
generated$3.tSNumberKeyword = generated$3.tsNumberKeyword = tsNumberKeyword;
generated$3.tSObjectKeyword = generated$3.tsObjectKeyword = tsObjectKeyword;
generated$3.tSStringKeyword = generated$3.tsStringKeyword = tsStringKeyword;
generated$3.tSSymbolKeyword = generated$3.tsSymbolKeyword = tsSymbolKeyword;
generated$3.tSUndefinedKeyword = generated$3.tsUndefinedKeyword = tsUndefinedKeyword;
generated$3.tSUnknownKeyword = generated$3.tsUnknownKeyword = tsUnknownKeyword;
generated$3.tSVoidKeyword = generated$3.tsVoidKeyword = tsVoidKeyword;
generated$3.tSThisType = generated$3.tsThisType = tsThisType;
generated$3.tSFunctionType = generated$3.tsFunctionType = tsFunctionType;
generated$3.tSConstructorType = generated$3.tsConstructorType = tsConstructorType;
generated$3.tSTypeReference = generated$3.tsTypeReference = tsTypeReference;
generated$3.tSTypePredicate = generated$3.tsTypePredicate = tsTypePredicate;
generated$3.tSTypeQuery = generated$3.tsTypeQuery = tsTypeQuery;
generated$3.tSTypeLiteral = generated$3.tsTypeLiteral = tsTypeLiteral;
generated$3.tSArrayType = generated$3.tsArrayType = tsArrayType;
generated$3.tSTupleType = generated$3.tsTupleType = tsTupleType;
generated$3.tSOptionalType = generated$3.tsOptionalType = tsOptionalType;
generated$3.tSRestType = generated$3.tsRestType = tsRestType;
generated$3.tSNamedTupleMember = generated$3.tsNamedTupleMember = tsNamedTupleMember;
generated$3.tSUnionType = generated$3.tsUnionType = tsUnionType;
generated$3.tSIntersectionType = generated$3.tsIntersectionType = tsIntersectionType;
generated$3.tSConditionalType = generated$3.tsConditionalType = tsConditionalType;
generated$3.tSInferType = generated$3.tsInferType = tsInferType;
generated$3.tSParenthesizedType = generated$3.tsParenthesizedType = tsParenthesizedType;
generated$3.tSTypeOperator = generated$3.tsTypeOperator = tsTypeOperator;
generated$3.tSIndexedAccessType = generated$3.tsIndexedAccessType = tsIndexedAccessType;
generated$3.tSMappedType = generated$3.tsMappedType = tsMappedType;
generated$3.tSLiteralType = generated$3.tsLiteralType = tsLiteralType;
generated$3.tSExpressionWithTypeArguments = generated$3.tsExpressionWithTypeArguments = tsExpressionWithTypeArguments;
generated$3.tSInterfaceDeclaration = generated$3.tsInterfaceDeclaration = tsInterfaceDeclaration;
generated$3.tSInterfaceBody = generated$3.tsInterfaceBody = tsInterfaceBody;
generated$3.tSTypeAliasDeclaration = generated$3.tsTypeAliasDeclaration = tsTypeAliasDeclaration;
generated$3.tSAsExpression = generated$3.tsAsExpression = tsAsExpression;
generated$3.tSTypeAssertion = generated$3.tsTypeAssertion = tsTypeAssertion;
generated$3.tSEnumDeclaration = generated$3.tsEnumDeclaration = tsEnumDeclaration;
generated$3.tSEnumMember = generated$3.tsEnumMember = tsEnumMember;
generated$3.tSModuleDeclaration = generated$3.tsModuleDeclaration = tsModuleDeclaration;
generated$3.tSModuleBlock = generated$3.tsModuleBlock = tsModuleBlock;
generated$3.tSImportType = generated$3.tsImportType = tsImportType;
generated$3.tSImportEqualsDeclaration = generated$3.tsImportEqualsDeclaration = tsImportEqualsDeclaration;
generated$3.tSExternalModuleReference = generated$3.tsExternalModuleReference = tsExternalModuleReference;
generated$3.tSNonNullExpression = generated$3.tsNonNullExpression = tsNonNullExpression;
generated$3.tSExportAssignment = generated$3.tsExportAssignment = tsExportAssignment;
generated$3.tSNamespaceExportDeclaration = generated$3.tsNamespaceExportDeclaration = tsNamespaceExportDeclaration;
generated$3.tSTypeAnnotation = generated$3.tsTypeAnnotation = tsTypeAnnotation;
generated$3.tSTypeParameterInstantiation = generated$3.tsTypeParameterInstantiation = tsTypeParameterInstantiation;
generated$3.tSTypeParameterDeclaration = generated$3.tsTypeParameterDeclaration = tsTypeParameterDeclaration;
generated$3.tSTypeParameter = generated$3.tsTypeParameter = tsTypeParameter;
generated$3.numberLiteral = NumberLiteral;
generated$3.regexLiteral = RegexLiteral;
generated$3.restProperty = RestProperty;
generated$3.spreadProperty = SpreadProperty;
var _builder = builder$1;
function arrayExpression(elements) {
    return (_builder.default).apply(this, [
        "ArrayExpression"
    ].concat(Array.prototype.slice.call(arguments)));
}
function assignmentExpression(operator, left, right) {
    return (_builder.default).apply(this, [
        "AssignmentExpression"
    ].concat(Array.prototype.slice.call(arguments)));
}
function binaryExpression(operator, left, right) {
    return (_builder.default).apply(this, [
        "BinaryExpression"
    ].concat(Array.prototype.slice.call(arguments)));
}
function interpreterDirective(value) {
    return (_builder.default).apply(this, [
        "InterpreterDirective"
    ].concat(Array.prototype.slice.call(arguments)));
}
function directive(value) {
    return (_builder.default).apply(this, [
        "Directive"
    ].concat(Array.prototype.slice.call(arguments)));
}
function directiveLiteral(value) {
    return (_builder.default).apply(this, [
        "DirectiveLiteral"
    ].concat(Array.prototype.slice.call(arguments)));
}
function blockStatement(body, directives) {
    return (_builder.default).apply(this, [
        "BlockStatement"
    ].concat(Array.prototype.slice.call(arguments)));
}
function breakStatement(label) {
    return (_builder.default).apply(this, [
        "BreakStatement"
    ].concat(Array.prototype.slice.call(arguments)));
}
function callExpression(callee, _arguments) {
    return (_builder.default).apply(this, [
        "CallExpression"
    ].concat(Array.prototype.slice.call(arguments)));
}
function catchClause(param, body) {
    return (_builder.default).apply(this, [
        "CatchClause"
    ].concat(Array.prototype.slice.call(arguments)));
}
function conditionalExpression(test, consequent, alternate) {
    return (_builder.default).apply(this, [
        "ConditionalExpression"
    ].concat(Array.prototype.slice.call(arguments)));
}
function continueStatement(label) {
    return (_builder.default).apply(this, [
        "ContinueStatement"
    ].concat(Array.prototype.slice.call(arguments)));
}
function debuggerStatement() {
    return (_builder.default).apply(this, [
        "DebuggerStatement"
    ].concat(Array.prototype.slice.call(arguments)));
}
function doWhileStatement(test, body) {
    return (_builder.default).apply(this, [
        "DoWhileStatement"
    ].concat(Array.prototype.slice.call(arguments)));
}
function emptyStatement() {
    return (_builder.default).apply(this, [
        "EmptyStatement"
    ].concat(Array.prototype.slice.call(arguments)));
}
function expressionStatement(expression) {
    return (_builder.default).apply(this, [
        "ExpressionStatement"
    ].concat(Array.prototype.slice.call(arguments)));
}
function file(program, comments, tokens) {
    return (_builder.default).apply(this, [
        "File"
    ].concat(Array.prototype.slice.call(arguments)));
}
function forInStatement(left, right, body) {
    return (_builder.default).apply(this, [
        "ForInStatement"
    ].concat(Array.prototype.slice.call(arguments)));
}
function forStatement(init, test, update, body) {
    return (_builder.default).apply(this, [
        "ForStatement"
    ].concat(Array.prototype.slice.call(arguments)));
}
function functionDeclaration(id, params, body, generator, async) {
    return (_builder.default).apply(this, [
        "FunctionDeclaration"
    ].concat(Array.prototype.slice.call(arguments)));
}
function functionExpression(id, params, body, generator, async) {
    return (_builder.default).apply(this, [
        "FunctionExpression"
    ].concat(Array.prototype.slice.call(arguments)));
}
function identifier(name) {
    return (_builder.default).apply(this, [
        "Identifier"
    ].concat(Array.prototype.slice.call(arguments)));
}
function ifStatement(test, consequent, alternate) {
    return (_builder.default).apply(this, [
        "IfStatement"
    ].concat(Array.prototype.slice.call(arguments)));
}
function labeledStatement(label, body) {
    return (_builder.default).apply(this, [
        "LabeledStatement"
    ].concat(Array.prototype.slice.call(arguments)));
}
function stringLiteral(value) {
    return (_builder.default).apply(this, [
        "StringLiteral"
    ].concat(Array.prototype.slice.call(arguments)));
}
function numericLiteral(value) {
    return (_builder.default).apply(this, [
        "NumericLiteral"
    ].concat(Array.prototype.slice.call(arguments)));
}
function nullLiteral() {
    return (_builder.default).apply(this, [
        "NullLiteral"
    ].concat(Array.prototype.slice.call(arguments)));
}
function booleanLiteral(value) {
    return (_builder.default).apply(this, [
        "BooleanLiteral"
    ].concat(Array.prototype.slice.call(arguments)));
}
function regExpLiteral(pattern, flags) {
    return (_builder.default).apply(this, [
        "RegExpLiteral"
    ].concat(Array.prototype.slice.call(arguments)));
}
function logicalExpression(operator, left, right) {
    return (_builder.default).apply(this, [
        "LogicalExpression"
    ].concat(Array.prototype.slice.call(arguments)));
}
function memberExpression(object, property, computed, optional) {
    return (_builder.default).apply(this, [
        "MemberExpression"
    ].concat(Array.prototype.slice.call(arguments)));
}
function newExpression(callee, _arguments) {
    return (_builder.default).apply(this, [
        "NewExpression"
    ].concat(Array.prototype.slice.call(arguments)));
}
function program(body, directives, sourceType, interpreter) {
    return (_builder.default).apply(this, [
        "Program"
    ].concat(Array.prototype.slice.call(arguments)));
}
function objectExpression(properties) {
    return (_builder.default).apply(this, [
        "ObjectExpression"
    ].concat(Array.prototype.slice.call(arguments)));
}
function objectMethod(kind, key, params, body, computed, generator, async) {
    return (_builder.default).apply(this, [
        "ObjectMethod"
    ].concat(Array.prototype.slice.call(arguments)));
}
function objectProperty(key, value, computed, shorthand, decorators) {
    return (_builder.default).apply(this, [
        "ObjectProperty"
    ].concat(Array.prototype.slice.call(arguments)));
}
function restElement(argument) {
    return (_builder.default).apply(this, [
        "RestElement"
    ].concat(Array.prototype.slice.call(arguments)));
}
function returnStatement(argument) {
    return (_builder.default).apply(this, [
        "ReturnStatement"
    ].concat(Array.prototype.slice.call(arguments)));
}
function sequenceExpression(expressions) {
    return (_builder.default).apply(this, [
        "SequenceExpression"
    ].concat(Array.prototype.slice.call(arguments)));
}
function parenthesizedExpression(expression) {
    return (_builder.default).apply(this, [
        "ParenthesizedExpression"
    ].concat(Array.prototype.slice.call(arguments)));
}
function switchCase(test, consequent) {
    return (_builder.default).apply(this, [
        "SwitchCase"
    ].concat(Array.prototype.slice.call(arguments)));
}
function switchStatement(discriminant, cases) {
    return (_builder.default).apply(this, [
        "SwitchStatement"
    ].concat(Array.prototype.slice.call(arguments)));
}
function thisExpression() {
    return (_builder.default).apply(this, [
        "ThisExpression"
    ].concat(Array.prototype.slice.call(arguments)));
}
function throwStatement(argument) {
    return (_builder.default).apply(this, [
        "ThrowStatement"
    ].concat(Array.prototype.slice.call(arguments)));
}
function tryStatement(block, handler, finalizer) {
    return (_builder.default).apply(this, [
        "TryStatement"
    ].concat(Array.prototype.slice.call(arguments)));
}
function unaryExpression(operator, argument, prefix) {
    return (_builder.default).apply(this, [
        "UnaryExpression"
    ].concat(Array.prototype.slice.call(arguments)));
}
function updateExpression(operator, argument, prefix) {
    return (_builder.default).apply(this, [
        "UpdateExpression"
    ].concat(Array.prototype.slice.call(arguments)));
}
function variableDeclaration(kind, declarations) {
    return (_builder.default).apply(this, [
        "VariableDeclaration"
    ].concat(Array.prototype.slice.call(arguments)));
}
function variableDeclarator(id, init) {
    return (_builder.default).apply(this, [
        "VariableDeclarator"
    ].concat(Array.prototype.slice.call(arguments)));
}
function whileStatement(test, body) {
    return (_builder.default).apply(this, [
        "WhileStatement"
    ].concat(Array.prototype.slice.call(arguments)));
}
function withStatement(object, body) {
    return (_builder.default).apply(this, [
        "WithStatement"
    ].concat(Array.prototype.slice.call(arguments)));
}
function assignmentPattern(left, right) {
    return (_builder.default).apply(this, [
        "AssignmentPattern"
    ].concat(Array.prototype.slice.call(arguments)));
}
function arrayPattern(elements) {
    return (_builder.default).apply(this, [
        "ArrayPattern"
    ].concat(Array.prototype.slice.call(arguments)));
}
function arrowFunctionExpression(params, body, async) {
    return (_builder.default).apply(this, [
        "ArrowFunctionExpression"
    ].concat(Array.prototype.slice.call(arguments)));
}
function classBody(body) {
    return (_builder.default).apply(this, [
        "ClassBody"
    ].concat(Array.prototype.slice.call(arguments)));
}
function classExpression(id, superClass, body, decorators) {
    return (_builder.default).apply(this, [
        "ClassExpression"
    ].concat(Array.prototype.slice.call(arguments)));
}
function classDeclaration(id, superClass, body, decorators) {
    return (_builder.default).apply(this, [
        "ClassDeclaration"
    ].concat(Array.prototype.slice.call(arguments)));
}
function exportAllDeclaration(source) {
    return (_builder.default).apply(this, [
        "ExportAllDeclaration"
    ].concat(Array.prototype.slice.call(arguments)));
}
function exportDefaultDeclaration(declaration) {
    return (_builder.default).apply(this, [
        "ExportDefaultDeclaration"
    ].concat(Array.prototype.slice.call(arguments)));
}
function exportNamedDeclaration(declaration, specifiers, source) {
    return (_builder.default).apply(this, [
        "ExportNamedDeclaration"
    ].concat(Array.prototype.slice.call(arguments)));
}
function exportSpecifier(local, exported) {
    return (_builder.default).apply(this, [
        "ExportSpecifier"
    ].concat(Array.prototype.slice.call(arguments)));
}
function forOfStatement(left, right, body, _await) {
    return (_builder.default).apply(this, [
        "ForOfStatement"
    ].concat(Array.prototype.slice.call(arguments)));
}
function importDeclaration(specifiers, source) {
    return (_builder.default).apply(this, [
        "ImportDeclaration"
    ].concat(Array.prototype.slice.call(arguments)));
}
function importDefaultSpecifier(local) {
    return (_builder.default).apply(this, [
        "ImportDefaultSpecifier"
    ].concat(Array.prototype.slice.call(arguments)));
}
function importNamespaceSpecifier(local) {
    return (_builder.default).apply(this, [
        "ImportNamespaceSpecifier"
    ].concat(Array.prototype.slice.call(arguments)));
}
function importSpecifier(local, imported) {
    return (_builder.default).apply(this, [
        "ImportSpecifier"
    ].concat(Array.prototype.slice.call(arguments)));
}
function metaProperty(meta, property) {
    return (_builder.default).apply(this, [
        "MetaProperty"
    ].concat(Array.prototype.slice.call(arguments)));
}
function classMethod(kind, key, params, body, computed, _static, generator, async) {
    return (_builder.default).apply(this, [
        "ClassMethod"
    ].concat(Array.prototype.slice.call(arguments)));
}
function objectPattern(properties) {
    return (_builder.default).apply(this, [
        "ObjectPattern"
    ].concat(Array.prototype.slice.call(arguments)));
}
function spreadElement(argument) {
    return (_builder.default).apply(this, [
        "SpreadElement"
    ].concat(Array.prototype.slice.call(arguments)));
}
function _super() {
    return (_builder.default).apply(this, [
        "Super"
    ].concat(Array.prototype.slice.call(arguments)));
}
function taggedTemplateExpression(tag, quasi) {
    return (_builder.default).apply(this, [
        "TaggedTemplateExpression"
    ].concat(Array.prototype.slice.call(arguments)));
}
function templateElement(value, tail) {
    return (_builder.default).apply(this, [
        "TemplateElement"
    ].concat(Array.prototype.slice.call(arguments)));
}
function templateLiteral(quasis, expressions) {
    return (_builder.default).apply(this, [
        "TemplateLiteral"
    ].concat(Array.prototype.slice.call(arguments)));
}
function yieldExpression(argument, delegate) {
    return (_builder.default).apply(this, [
        "YieldExpression"
    ].concat(Array.prototype.slice.call(arguments)));
}
function awaitExpression(argument) {
    return (_builder.default).apply(this, [
        "AwaitExpression"
    ].concat(Array.prototype.slice.call(arguments)));
}
function _import() {
    return (_builder.default).apply(this, [
        "Import"
    ].concat(Array.prototype.slice.call(arguments)));
}
function bigIntLiteral(value) {
    return (_builder.default).apply(this, [
        "BigIntLiteral"
    ].concat(Array.prototype.slice.call(arguments)));
}
function exportNamespaceSpecifier(exported) {
    return (_builder.default).apply(this, [
        "ExportNamespaceSpecifier"
    ].concat(Array.prototype.slice.call(arguments)));
}
function optionalMemberExpression(object, property, computed, optional) {
    return (_builder.default).apply(this, [
        "OptionalMemberExpression"
    ].concat(Array.prototype.slice.call(arguments)));
}
function optionalCallExpression(callee, _arguments, optional) {
    return (_builder.default).apply(this, [
        "OptionalCallExpression"
    ].concat(Array.prototype.slice.call(arguments)));
}
function classProperty(key, value, typeAnnotation, decorators, computed, _static) {
    return (_builder.default).apply(this, [
        "ClassProperty"
    ].concat(Array.prototype.slice.call(arguments)));
}
function classPrivateProperty(key, value, decorators, _static) {
    return (_builder.default).apply(this, [
        "ClassPrivateProperty"
    ].concat(Array.prototype.slice.call(arguments)));
}
function classPrivateMethod(kind, key, params, body, _static) {
    return (_builder.default).apply(this, [
        "ClassPrivateMethod"
    ].concat(Array.prototype.slice.call(arguments)));
}
function privateName(id) {
    return (_builder.default).apply(this, [
        "PrivateName"
    ].concat(Array.prototype.slice.call(arguments)));
}
function anyTypeAnnotation() {
    return (_builder.default).apply(this, [
        "AnyTypeAnnotation"
    ].concat(Array.prototype.slice.call(arguments)));
}
function arrayTypeAnnotation(elementType) {
    return (_builder.default).apply(this, [
        "ArrayTypeAnnotation"
    ].concat(Array.prototype.slice.call(arguments)));
}
function booleanTypeAnnotation() {
    return (_builder.default).apply(this, [
        "BooleanTypeAnnotation"
    ].concat(Array.prototype.slice.call(arguments)));
}
function booleanLiteralTypeAnnotation(value) {
    return (_builder.default).apply(this, [
        "BooleanLiteralTypeAnnotation"
    ].concat(Array.prototype.slice.call(arguments)));
}
function nullLiteralTypeAnnotation() {
    return (_builder.default).apply(this, [
        "NullLiteralTypeAnnotation"
    ].concat(Array.prototype.slice.call(arguments)));
}
function classImplements(id, typeParameters) {
    return (_builder.default).apply(this, [
        "ClassImplements"
    ].concat(Array.prototype.slice.call(arguments)));
}
function declareClass(id, typeParameters, _extends, body) {
    return (_builder.default).apply(this, [
        "DeclareClass"
    ].concat(Array.prototype.slice.call(arguments)));
}
function declareFunction(id) {
    return (_builder.default).apply(this, [
        "DeclareFunction"
    ].concat(Array.prototype.slice.call(arguments)));
}
function declareInterface(id, typeParameters, _extends, body) {
    return (_builder.default).apply(this, [
        "DeclareInterface"
    ].concat(Array.prototype.slice.call(arguments)));
}
function declareModule(id, body, kind) {
    return (_builder.default).apply(this, [
        "DeclareModule"
    ].concat(Array.prototype.slice.call(arguments)));
}
function declareModuleExports(typeAnnotation) {
    return (_builder.default).apply(this, [
        "DeclareModuleExports"
    ].concat(Array.prototype.slice.call(arguments)));
}
function declareTypeAlias(id, typeParameters, right) {
    return (_builder.default).apply(this, [
        "DeclareTypeAlias"
    ].concat(Array.prototype.slice.call(arguments)));
}
function declareOpaqueType(id, typeParameters, supertype) {
    return (_builder.default).apply(this, [
        "DeclareOpaqueType"
    ].concat(Array.prototype.slice.call(arguments)));
}
function declareVariable(id) {
    return (_builder.default).apply(this, [
        "DeclareVariable"
    ].concat(Array.prototype.slice.call(arguments)));
}
function declareExportDeclaration(declaration, specifiers, source) {
    return (_builder.default).apply(this, [
        "DeclareExportDeclaration"
    ].concat(Array.prototype.slice.call(arguments)));
}
function declareExportAllDeclaration(source) {
    return (_builder.default).apply(this, [
        "DeclareExportAllDeclaration"
    ].concat(Array.prototype.slice.call(arguments)));
}
function declaredPredicate(value) {
    return (_builder.default).apply(this, [
        "DeclaredPredicate"
    ].concat(Array.prototype.slice.call(arguments)));
}
function existsTypeAnnotation() {
    return (_builder.default).apply(this, [
        "ExistsTypeAnnotation"
    ].concat(Array.prototype.slice.call(arguments)));
}
function functionTypeAnnotation(typeParameters, params, rest, returnType) {
    return (_builder.default).apply(this, [
        "FunctionTypeAnnotation"
    ].concat(Array.prototype.slice.call(arguments)));
}
function functionTypeParam(name, typeAnnotation) {
    return (_builder.default).apply(this, [
        "FunctionTypeParam"
    ].concat(Array.prototype.slice.call(arguments)));
}
function genericTypeAnnotation(id, typeParameters) {
    return (_builder.default).apply(this, [
        "GenericTypeAnnotation"
    ].concat(Array.prototype.slice.call(arguments)));
}
function inferredPredicate() {
    return (_builder.default).apply(this, [
        "InferredPredicate"
    ].concat(Array.prototype.slice.call(arguments)));
}
function interfaceExtends(id, typeParameters) {
    return (_builder.default).apply(this, [
        "InterfaceExtends"
    ].concat(Array.prototype.slice.call(arguments)));
}
function interfaceDeclaration(id, typeParameters, _extends, body) {
    return (_builder.default).apply(this, [
        "InterfaceDeclaration"
    ].concat(Array.prototype.slice.call(arguments)));
}
function interfaceTypeAnnotation(_extends, body) {
    return (_builder.default).apply(this, [
        "InterfaceTypeAnnotation"
    ].concat(Array.prototype.slice.call(arguments)));
}
function intersectionTypeAnnotation(types) {
    return (_builder.default).apply(this, [
        "IntersectionTypeAnnotation"
    ].concat(Array.prototype.slice.call(arguments)));
}
function mixedTypeAnnotation() {
    return (_builder.default).apply(this, [
        "MixedTypeAnnotation"
    ].concat(Array.prototype.slice.call(arguments)));
}
function emptyTypeAnnotation() {
    return (_builder.default).apply(this, [
        "EmptyTypeAnnotation"
    ].concat(Array.prototype.slice.call(arguments)));
}
function nullableTypeAnnotation(typeAnnotation) {
    return (_builder.default).apply(this, [
        "NullableTypeAnnotation"
    ].concat(Array.prototype.slice.call(arguments)));
}
function numberLiteralTypeAnnotation(value) {
    return (_builder.default).apply(this, [
        "NumberLiteralTypeAnnotation"
    ].concat(Array.prototype.slice.call(arguments)));
}
function numberTypeAnnotation() {
    return (_builder.default).apply(this, [
        "NumberTypeAnnotation"
    ].concat(Array.prototype.slice.call(arguments)));
}
function objectTypeAnnotation(properties, indexers, callProperties, internalSlots, exact) {
    return (_builder.default).apply(this, [
        "ObjectTypeAnnotation"
    ].concat(Array.prototype.slice.call(arguments)));
}
function objectTypeInternalSlot(id, value, optional, _static, method) {
    return (_builder.default).apply(this, [
        "ObjectTypeInternalSlot"
    ].concat(Array.prototype.slice.call(arguments)));
}
function objectTypeCallProperty(value) {
    return (_builder.default).apply(this, [
        "ObjectTypeCallProperty"
    ].concat(Array.prototype.slice.call(arguments)));
}
function objectTypeIndexer(id, key, value, variance) {
    return (_builder.default).apply(this, [
        "ObjectTypeIndexer"
    ].concat(Array.prototype.slice.call(arguments)));
}
function objectTypeProperty(key, value, variance) {
    return (_builder.default).apply(this, [
        "ObjectTypeProperty"
    ].concat(Array.prototype.slice.call(arguments)));
}
function objectTypeSpreadProperty(argument) {
    return (_builder.default).apply(this, [
        "ObjectTypeSpreadProperty"
    ].concat(Array.prototype.slice.call(arguments)));
}
function opaqueType(id, typeParameters, supertype, impltype) {
    return (_builder.default).apply(this, [
        "OpaqueType"
    ].concat(Array.prototype.slice.call(arguments)));
}
function qualifiedTypeIdentifier(id, qualification) {
    return (_builder.default).apply(this, [
        "QualifiedTypeIdentifier"
    ].concat(Array.prototype.slice.call(arguments)));
}
function stringLiteralTypeAnnotation(value) {
    return (_builder.default).apply(this, [
        "StringLiteralTypeAnnotation"
    ].concat(Array.prototype.slice.call(arguments)));
}
function stringTypeAnnotation() {
    return (_builder.default).apply(this, [
        "StringTypeAnnotation"
    ].concat(Array.prototype.slice.call(arguments)));
}
function symbolTypeAnnotation() {
    return (_builder.default).apply(this, [
        "SymbolTypeAnnotation"
    ].concat(Array.prototype.slice.call(arguments)));
}
function thisTypeAnnotation() {
    return (_builder.default).apply(this, [
        "ThisTypeAnnotation"
    ].concat(Array.prototype.slice.call(arguments)));
}
function tupleTypeAnnotation(types) {
    return (_builder.default).apply(this, [
        "TupleTypeAnnotation"
    ].concat(Array.prototype.slice.call(arguments)));
}
function typeofTypeAnnotation(argument) {
    return (_builder.default).apply(this, [
        "TypeofTypeAnnotation"
    ].concat(Array.prototype.slice.call(arguments)));
}
function typeAlias(id, typeParameters, right) {
    return (_builder.default).apply(this, [
        "TypeAlias"
    ].concat(Array.prototype.slice.call(arguments)));
}
function typeAnnotation(typeAnnotation) {
    return (_builder.default).apply(this, [
        "TypeAnnotation"
    ].concat(Array.prototype.slice.call(arguments)));
}
function typeCastExpression(expression, typeAnnotation) {
    return (_builder.default).apply(this, [
        "TypeCastExpression"
    ].concat(Array.prototype.slice.call(arguments)));
}
function typeParameter(bound, _default, variance) {
    return (_builder.default).apply(this, [
        "TypeParameter"
    ].concat(Array.prototype.slice.call(arguments)));
}
function typeParameterDeclaration(params) {
    return (_builder.default).apply(this, [
        "TypeParameterDeclaration"
    ].concat(Array.prototype.slice.call(arguments)));
}
function typeParameterInstantiation(params) {
    return (_builder.default).apply(this, [
        "TypeParameterInstantiation"
    ].concat(Array.prototype.slice.call(arguments)));
}
function unionTypeAnnotation(types) {
    return (_builder.default).apply(this, [
        "UnionTypeAnnotation"
    ].concat(Array.prototype.slice.call(arguments)));
}
function variance(kind) {
    return (_builder.default).apply(this, [
        "Variance"
    ].concat(Array.prototype.slice.call(arguments)));
}
function voidTypeAnnotation() {
    return (_builder.default).apply(this, [
        "VoidTypeAnnotation"
    ].concat(Array.prototype.slice.call(arguments)));
}
function enumDeclaration(id, body) {
    return (_builder.default).apply(this, [
        "EnumDeclaration"
    ].concat(Array.prototype.slice.call(arguments)));
}
function enumBooleanBody(members) {
    return (_builder.default).apply(this, [
        "EnumBooleanBody"
    ].concat(Array.prototype.slice.call(arguments)));
}
function enumNumberBody(members) {
    return (_builder.default).apply(this, [
        "EnumNumberBody"
    ].concat(Array.prototype.slice.call(arguments)));
}
function enumStringBody(members) {
    return (_builder.default).apply(this, [
        "EnumStringBody"
    ].concat(Array.prototype.slice.call(arguments)));
}
function enumSymbolBody(members) {
    return (_builder.default).apply(this, [
        "EnumSymbolBody"
    ].concat(Array.prototype.slice.call(arguments)));
}
function enumBooleanMember(id) {
    return (_builder.default).apply(this, [
        "EnumBooleanMember"
    ].concat(Array.prototype.slice.call(arguments)));
}
function enumNumberMember(id, init) {
    return (_builder.default).apply(this, [
        "EnumNumberMember"
    ].concat(Array.prototype.slice.call(arguments)));
}
function enumStringMember(id, init) {
    return (_builder.default).apply(this, [
        "EnumStringMember"
    ].concat(Array.prototype.slice.call(arguments)));
}
function enumDefaultedMember(id) {
    return (_builder.default).apply(this, [
        "EnumDefaultedMember"
    ].concat(Array.prototype.slice.call(arguments)));
}
function indexedAccessType(objectType, indexType) {
    return (_builder.default).apply(this, [
        "IndexedAccessType"
    ].concat(Array.prototype.slice.call(arguments)));
}
function optionalIndexedAccessType(objectType, indexType) {
    return (_builder.default).apply(this, [
        "OptionalIndexedAccessType"
    ].concat(Array.prototype.slice.call(arguments)));
}
function jsxAttribute(name, value) {
    return (_builder.default).apply(this, [
        "JSXAttribute"
    ].concat(Array.prototype.slice.call(arguments)));
}
function jsxClosingElement(name) {
    return (_builder.default).apply(this, [
        "JSXClosingElement"
    ].concat(Array.prototype.slice.call(arguments)));
}
function jsxElement(openingElement, closingElement, children, selfClosing) {
    return (_builder.default).apply(this, [
        "JSXElement"
    ].concat(Array.prototype.slice.call(arguments)));
}
function jsxEmptyExpression() {
    return (_builder.default).apply(this, [
        "JSXEmptyExpression"
    ].concat(Array.prototype.slice.call(arguments)));
}
function jsxExpressionContainer(expression) {
    return (_builder.default).apply(this, [
        "JSXExpressionContainer"
    ].concat(Array.prototype.slice.call(arguments)));
}
function jsxSpreadChild(expression) {
    return (_builder.default).apply(this, [
        "JSXSpreadChild"
    ].concat(Array.prototype.slice.call(arguments)));
}
function jsxIdentifier(name) {
    return (_builder.default).apply(this, [
        "JSXIdentifier"
    ].concat(Array.prototype.slice.call(arguments)));
}
function jsxMemberExpression(object, property) {
    return (_builder.default).apply(this, [
        "JSXMemberExpression"
    ].concat(Array.prototype.slice.call(arguments)));
}
function jsxNamespacedName(namespace, name) {
    return (_builder.default).apply(this, [
        "JSXNamespacedName"
    ].concat(Array.prototype.slice.call(arguments)));
}
function jsxOpeningElement(name, attributes, selfClosing) {
    return (_builder.default).apply(this, [
        "JSXOpeningElement"
    ].concat(Array.prototype.slice.call(arguments)));
}
function jsxSpreadAttribute(argument) {
    return (_builder.default).apply(this, [
        "JSXSpreadAttribute"
    ].concat(Array.prototype.slice.call(arguments)));
}
function jsxText(value) {
    return (_builder.default).apply(this, [
        "JSXText"
    ].concat(Array.prototype.slice.call(arguments)));
}
function jsxFragment(openingFragment, closingFragment, children) {
    return (_builder.default).apply(this, [
        "JSXFragment"
    ].concat(Array.prototype.slice.call(arguments)));
}
function jsxOpeningFragment() {
    return (_builder.default).apply(this, [
        "JSXOpeningFragment"
    ].concat(Array.prototype.slice.call(arguments)));
}
function jsxClosingFragment() {
    return (_builder.default).apply(this, [
        "JSXClosingFragment"
    ].concat(Array.prototype.slice.call(arguments)));
}
function noop() {
    return (_builder.default).apply(this, [
        "Noop"
    ].concat(Array.prototype.slice.call(arguments)));
}
function placeholder(expectedNode, name) {
    return (_builder.default).apply(this, [
        "Placeholder"
    ].concat(Array.prototype.slice.call(arguments)));
}
function v8IntrinsicIdentifier(name) {
    return (_builder.default).apply(this, [
        "V8IntrinsicIdentifier"
    ].concat(Array.prototype.slice.call(arguments)));
}
function argumentPlaceholder() {
    return (_builder.default).apply(this, [
        "ArgumentPlaceholder"
    ].concat(Array.prototype.slice.call(arguments)));
}
function bindExpression(object, callee) {
    return (_builder.default).apply(this, [
        "BindExpression"
    ].concat(Array.prototype.slice.call(arguments)));
}
function importAttribute(key, value) {
    return (_builder.default).apply(this, [
        "ImportAttribute"
    ].concat(Array.prototype.slice.call(arguments)));
}
function decorator(expression) {
    return (_builder.default).apply(this, [
        "Decorator"
    ].concat(Array.prototype.slice.call(arguments)));
}
function doExpression(body, async) {
    return (_builder.default).apply(this, [
        "DoExpression"
    ].concat(Array.prototype.slice.call(arguments)));
}
function exportDefaultSpecifier(exported) {
    return (_builder.default).apply(this, [
        "ExportDefaultSpecifier"
    ].concat(Array.prototype.slice.call(arguments)));
}
function recordExpression(properties) {
    return (_builder.default).apply(this, [
        "RecordExpression"
    ].concat(Array.prototype.slice.call(arguments)));
}
function tupleExpression(elements) {
    return (_builder.default).apply(this, [
        "TupleExpression"
    ].concat(Array.prototype.slice.call(arguments)));
}
function decimalLiteral(value) {
    return (_builder.default).apply(this, [
        "DecimalLiteral"
    ].concat(Array.prototype.slice.call(arguments)));
}
function staticBlock(body) {
    return (_builder.default).apply(this, [
        "StaticBlock"
    ].concat(Array.prototype.slice.call(arguments)));
}
function moduleExpression(body) {
    return (_builder.default).apply(this, [
        "ModuleExpression"
    ].concat(Array.prototype.slice.call(arguments)));
}
function topicReference() {
    return (_builder.default).apply(this, [
        "TopicReference"
    ].concat(Array.prototype.slice.call(arguments)));
}
function pipelineTopicExpression(expression) {
    return (_builder.default).apply(this, [
        "PipelineTopicExpression"
    ].concat(Array.prototype.slice.call(arguments)));
}
function pipelineBareFunction(callee) {
    return (_builder.default).apply(this, [
        "PipelineBareFunction"
    ].concat(Array.prototype.slice.call(arguments)));
}
function pipelinePrimaryTopicReference() {
    return (_builder.default).apply(this, [
        "PipelinePrimaryTopicReference"
    ].concat(Array.prototype.slice.call(arguments)));
}
function tsParameterProperty(parameter) {
    return (_builder.default).apply(this, [
        "TSParameterProperty"
    ].concat(Array.prototype.slice.call(arguments)));
}
function tsDeclareFunction(id, typeParameters, params, returnType) {
    return (_builder.default).apply(this, [
        "TSDeclareFunction"
    ].concat(Array.prototype.slice.call(arguments)));
}
function tsDeclareMethod(decorators, key, typeParameters, params, returnType) {
    return (_builder.default).apply(this, [
        "TSDeclareMethod"
    ].concat(Array.prototype.slice.call(arguments)));
}
function tsQualifiedName(left, right) {
    return (_builder.default).apply(this, [
        "TSQualifiedName"
    ].concat(Array.prototype.slice.call(arguments)));
}
function tsCallSignatureDeclaration(typeParameters, parameters, typeAnnotation) {
    return (_builder.default).apply(this, [
        "TSCallSignatureDeclaration"
    ].concat(Array.prototype.slice.call(arguments)));
}
function tsConstructSignatureDeclaration(typeParameters, parameters, typeAnnotation) {
    return (_builder.default).apply(this, [
        "TSConstructSignatureDeclaration"
    ].concat(Array.prototype.slice.call(arguments)));
}
function tsPropertySignature(key, typeAnnotation, initializer) {
    return (_builder.default).apply(this, [
        "TSPropertySignature"
    ].concat(Array.prototype.slice.call(arguments)));
}
function tsMethodSignature(key, typeParameters, parameters, typeAnnotation) {
    return (_builder.default).apply(this, [
        "TSMethodSignature"
    ].concat(Array.prototype.slice.call(arguments)));
}
function tsIndexSignature(parameters, typeAnnotation) {
    return (_builder.default).apply(this, [
        "TSIndexSignature"
    ].concat(Array.prototype.slice.call(arguments)));
}
function tsAnyKeyword() {
    return (_builder.default).apply(this, [
        "TSAnyKeyword"
    ].concat(Array.prototype.slice.call(arguments)));
}
function tsBooleanKeyword() {
    return (_builder.default).apply(this, [
        "TSBooleanKeyword"
    ].concat(Array.prototype.slice.call(arguments)));
}
function tsBigIntKeyword() {
    return (_builder.default).apply(this, [
        "TSBigIntKeyword"
    ].concat(Array.prototype.slice.call(arguments)));
}
function tsIntrinsicKeyword() {
    return (_builder.default).apply(this, [
        "TSIntrinsicKeyword"
    ].concat(Array.prototype.slice.call(arguments)));
}
function tsNeverKeyword() {
    return (_builder.default).apply(this, [
        "TSNeverKeyword"
    ].concat(Array.prototype.slice.call(arguments)));
}
function tsNullKeyword() {
    return (_builder.default).apply(this, [
        "TSNullKeyword"
    ].concat(Array.prototype.slice.call(arguments)));
}
function tsNumberKeyword() {
    return (_builder.default).apply(this, [
        "TSNumberKeyword"
    ].concat(Array.prototype.slice.call(arguments)));
}
function tsObjectKeyword() {
    return (_builder.default).apply(this, [
        "TSObjectKeyword"
    ].concat(Array.prototype.slice.call(arguments)));
}
function tsStringKeyword() {
    return (_builder.default).apply(this, [
        "TSStringKeyword"
    ].concat(Array.prototype.slice.call(arguments)));
}
function tsSymbolKeyword() {
    return (_builder.default).apply(this, [
        "TSSymbolKeyword"
    ].concat(Array.prototype.slice.call(arguments)));
}
function tsUndefinedKeyword() {
    return (_builder.default).apply(this, [
        "TSUndefinedKeyword"
    ].concat(Array.prototype.slice.call(arguments)));
}
function tsUnknownKeyword() {
    return (_builder.default).apply(this, [
        "TSUnknownKeyword"
    ].concat(Array.prototype.slice.call(arguments)));
}
function tsVoidKeyword() {
    return (_builder.default).apply(this, [
        "TSVoidKeyword"
    ].concat(Array.prototype.slice.call(arguments)));
}
function tsThisType() {
    return (_builder.default).apply(this, [
        "TSThisType"
    ].concat(Array.prototype.slice.call(arguments)));
}
function tsFunctionType(typeParameters, parameters, typeAnnotation) {
    return (_builder.default).apply(this, [
        "TSFunctionType"
    ].concat(Array.prototype.slice.call(arguments)));
}
function tsConstructorType(typeParameters, parameters, typeAnnotation) {
    return (_builder.default).apply(this, [
        "TSConstructorType"
    ].concat(Array.prototype.slice.call(arguments)));
}
function tsTypeReference(typeName, typeParameters) {
    return (_builder.default).apply(this, [
        "TSTypeReference"
    ].concat(Array.prototype.slice.call(arguments)));
}
function tsTypePredicate(parameterName, typeAnnotation, asserts) {
    return (_builder.default).apply(this, [
        "TSTypePredicate"
    ].concat(Array.prototype.slice.call(arguments)));
}
function tsTypeQuery(exprName) {
    return (_builder.default).apply(this, [
        "TSTypeQuery"
    ].concat(Array.prototype.slice.call(arguments)));
}
function tsTypeLiteral(members) {
    return (_builder.default).apply(this, [
        "TSTypeLiteral"
    ].concat(Array.prototype.slice.call(arguments)));
}
function tsArrayType(elementType) {
    return (_builder.default).apply(this, [
        "TSArrayType"
    ].concat(Array.prototype.slice.call(arguments)));
}
function tsTupleType(elementTypes) {
    return (_builder.default).apply(this, [
        "TSTupleType"
    ].concat(Array.prototype.slice.call(arguments)));
}
function tsOptionalType(typeAnnotation) {
    return (_builder.default).apply(this, [
        "TSOptionalType"
    ].concat(Array.prototype.slice.call(arguments)));
}
function tsRestType(typeAnnotation) {
    return (_builder.default).apply(this, [
        "TSRestType"
    ].concat(Array.prototype.slice.call(arguments)));
}
function tsNamedTupleMember(label, elementType, optional) {
    return (_builder.default).apply(this, [
        "TSNamedTupleMember"
    ].concat(Array.prototype.slice.call(arguments)));
}
function tsUnionType(types) {
    return (_builder.default).apply(this, [
        "TSUnionType"
    ].concat(Array.prototype.slice.call(arguments)));
}
function tsIntersectionType(types) {
    return (_builder.default).apply(this, [
        "TSIntersectionType"
    ].concat(Array.prototype.slice.call(arguments)));
}
function tsConditionalType(checkType, extendsType, trueType, falseType) {
    return (_builder.default).apply(this, [
        "TSConditionalType"
    ].concat(Array.prototype.slice.call(arguments)));
}
function tsInferType(typeParameter) {
    return (_builder.default).apply(this, [
        "TSInferType"
    ].concat(Array.prototype.slice.call(arguments)));
}
function tsParenthesizedType(typeAnnotation) {
    return (_builder.default).apply(this, [
        "TSParenthesizedType"
    ].concat(Array.prototype.slice.call(arguments)));
}
function tsTypeOperator(typeAnnotation) {
    return (_builder.default).apply(this, [
        "TSTypeOperator"
    ].concat(Array.prototype.slice.call(arguments)));
}
function tsIndexedAccessType(objectType, indexType) {
    return (_builder.default).apply(this, [
        "TSIndexedAccessType"
    ].concat(Array.prototype.slice.call(arguments)));
}
function tsMappedType(typeParameter, typeAnnotation, nameType) {
    return (_builder.default).apply(this, [
        "TSMappedType"
    ].concat(Array.prototype.slice.call(arguments)));
}
function tsLiteralType(literal) {
    return (_builder.default).apply(this, [
        "TSLiteralType"
    ].concat(Array.prototype.slice.call(arguments)));
}
function tsExpressionWithTypeArguments(expression, typeParameters) {
    return (_builder.default).apply(this, [
        "TSExpressionWithTypeArguments"
    ].concat(Array.prototype.slice.call(arguments)));
}
function tsInterfaceDeclaration(id, typeParameters, _extends, body) {
    return (_builder.default).apply(this, [
        "TSInterfaceDeclaration"
    ].concat(Array.prototype.slice.call(arguments)));
}
function tsInterfaceBody(body) {
    return (_builder.default).apply(this, [
        "TSInterfaceBody"
    ].concat(Array.prototype.slice.call(arguments)));
}
function tsTypeAliasDeclaration(id, typeParameters, typeAnnotation) {
    return (_builder.default).apply(this, [
        "TSTypeAliasDeclaration"
    ].concat(Array.prototype.slice.call(arguments)));
}
function tsAsExpression(expression, typeAnnotation) {
    return (_builder.default).apply(this, [
        "TSAsExpression"
    ].concat(Array.prototype.slice.call(arguments)));
}
function tsTypeAssertion(typeAnnotation, expression) {
    return (_builder.default).apply(this, [
        "TSTypeAssertion"
    ].concat(Array.prototype.slice.call(arguments)));
}
function tsEnumDeclaration(id, members) {
    return (_builder.default).apply(this, [
        "TSEnumDeclaration"
    ].concat(Array.prototype.slice.call(arguments)));
}
function tsEnumMember(id, initializer) {
    return (_builder.default).apply(this, [
        "TSEnumMember"
    ].concat(Array.prototype.slice.call(arguments)));
}
function tsModuleDeclaration(id, body) {
    return (_builder.default).apply(this, [
        "TSModuleDeclaration"
    ].concat(Array.prototype.slice.call(arguments)));
}
function tsModuleBlock(body) {
    return (_builder.default).apply(this, [
        "TSModuleBlock"
    ].concat(Array.prototype.slice.call(arguments)));
}
function tsImportType(argument, qualifier, typeParameters) {
    return (_builder.default).apply(this, [
        "TSImportType"
    ].concat(Array.prototype.slice.call(arguments)));
}
function tsImportEqualsDeclaration(id, moduleReference) {
    return (_builder.default).apply(this, [
        "TSImportEqualsDeclaration"
    ].concat(Array.prototype.slice.call(arguments)));
}
function tsExternalModuleReference(expression) {
    return (_builder.default).apply(this, [
        "TSExternalModuleReference"
    ].concat(Array.prototype.slice.call(arguments)));
}
function tsNonNullExpression(expression) {
    return (_builder.default).apply(this, [
        "TSNonNullExpression"
    ].concat(Array.prototype.slice.call(arguments)));
}
function tsExportAssignment(expression) {
    return (_builder.default).apply(this, [
        "TSExportAssignment"
    ].concat(Array.prototype.slice.call(arguments)));
}
function tsNamespaceExportDeclaration(id) {
    return (_builder.default).apply(this, [
        "TSNamespaceExportDeclaration"
    ].concat(Array.prototype.slice.call(arguments)));
}
function tsTypeAnnotation(typeAnnotation) {
    return (_builder.default).apply(this, [
        "TSTypeAnnotation"
    ].concat(Array.prototype.slice.call(arguments)));
}
function tsTypeParameterInstantiation(params) {
    return (_builder.default).apply(this, [
        "TSTypeParameterInstantiation"
    ].concat(Array.prototype.slice.call(arguments)));
}
function tsTypeParameterDeclaration(params) {
    return (_builder.default).apply(this, [
        "TSTypeParameterDeclaration"
    ].concat(Array.prototype.slice.call(arguments)));
}
function tsTypeParameter(constraint, _default, name) {
    return (_builder.default).apply(this, [
        "TSTypeParameter"
    ].concat(Array.prototype.slice.call(arguments)));
}
function NumberLiteral() {
    for(var _len = arguments.length, args = new Array(_len), _key = 0; _key < _len; _key++){
        args[_key] = arguments[_key];
    }
    console.trace("The node type NumberLiteral has been renamed to NumericLiteral");
    return (_builder.default).apply(this, [
        "NumberLiteral"
    ].concat(_toConsumableArray$1(args)));
}
function RegexLiteral() {
    for(var _len = arguments.length, args = new Array(_len), _key = 0; _key < _len; _key++){
        args[_key] = arguments[_key];
    }
    console.trace("The node type RegexLiteral has been renamed to RegExpLiteral");
    return (_builder.default).apply(this, [
        "RegexLiteral"
    ].concat(_toConsumableArray$1(args)));
}
function RestProperty() {
    for(var _len = arguments.length, args = new Array(_len), _key = 0; _key < _len; _key++){
        args[_key] = arguments[_key];
    }
    console.trace("The node type RestProperty has been renamed to RestElement");
    return (_builder.default).apply(this, [
        "RestProperty"
    ].concat(_toConsumableArray$1(args)));
}
function SpreadProperty() {
    for(var _len = arguments.length, args = new Array(_len), _key = 0; _key < _len; _key++){
        args[_key] = arguments[_key];
    }
    console.trace("The node type SpreadProperty has been renamed to SpreadElement");
    return (_builder.default).apply(this, [
        "SpreadProperty"
    ].concat(_toConsumableArray$1(args)));
}

Object.defineProperty(cleanJSXElementLiteralChild$1, "__esModule", {
    value: true
});
cleanJSXElementLiteralChild$1.default = cleanJSXElementLiteralChild;
var _generated$n = generated$3;
function cleanJSXElementLiteralChild(child, args) {
    var lines = child.value.split(/\r\n|\n|\r/);
    var lastNonEmptyLine = 0;
    for(var i = 0; i < lines.length; i++){
        if (lines[i].match(/[^ \t]/)) {
            lastNonEmptyLine = i;
        }
    }
    var str = "";
    for(var i1 = 0; i1 < lines.length; i1++){
        var line = lines[i1];
        var isFirstLine = i1 === 0;
        var isLastLine = i1 === lines.length - 1;
        var isLastNonEmptyLine = i1 === lastNonEmptyLine;
        var trimmedLine = line.replace(/\t/g, " ");
        if (!isFirstLine) {
            trimmedLine = trimmedLine.replace(/^[ ]+/, "");
        }
        if (!isLastLine) {
            trimmedLine = trimmedLine.replace(/[ ]+$/, "");
        }
        if (trimmedLine) {
            if (!isLastNonEmptyLine) {
                trimmedLine += " ";
            }
            str += trimmedLine;
        }
    }
    if (str) args.push((0, _generated$n.stringLiteral)(str));
}

Object.defineProperty(buildChildren$1, "__esModule", {
    value: true
});
buildChildren$1.default = buildChildren;
var _generated$m = generated$4;
var _cleanJSXElementLiteralChild = cleanJSXElementLiteralChild$1;
function buildChildren(node) {
    var elements = [];
    for(var i = 0; i < node.children.length; i++){
        var child = node.children[i];
        if ((0, _generated$m.isJSXText)(child)) {
            (0, _cleanJSXElementLiteralChild.default)(child, elements);
            continue;
        }
        if ((0, _generated$m.isJSXExpressionContainer)(child)) child = child.expression;
        if ((0, _generated$m.isJSXEmptyExpression)(child)) continue;
        elements.push(child);
    }
    return elements;
}

var assertNode$1 = {};

var isNode$1 = {};

Object.defineProperty(isNode$1, "__esModule", {
    value: true
});
isNode$1.default = isNode;
var _definitions$5 = requireDefinitions();
function isNode(node) {
    return !!(node && _definitions$5.VISITOR_KEYS[node.type]);
}

Object.defineProperty(assertNode$1, "__esModule", {
    value: true
});
assertNode$1.default = assertNode;
var _isNode = isNode$1;
function assertNode(node) {
    if (!(0, _isNode.default)(node)) {
        var _node$type;
        var type = (_node$type = node == null ? void 0 : node.type) != null ? _node$type : JSON.stringify(node);
        throw new TypeError('Not a valid node of type "' + type + '"');
    }
}

var generated$2 = {};

Object.defineProperty(generated$2, "__esModule", {
    value: true
});
generated$2.assertArrayExpression = assertArrayExpression;
generated$2.assertAssignmentExpression = assertAssignmentExpression;
generated$2.assertBinaryExpression = assertBinaryExpression;
generated$2.assertInterpreterDirective = assertInterpreterDirective;
generated$2.assertDirective = assertDirective;
generated$2.assertDirectiveLiteral = assertDirectiveLiteral;
generated$2.assertBlockStatement = assertBlockStatement;
generated$2.assertBreakStatement = assertBreakStatement;
generated$2.assertCallExpression = assertCallExpression;
generated$2.assertCatchClause = assertCatchClause;
generated$2.assertConditionalExpression = assertConditionalExpression;
generated$2.assertContinueStatement = assertContinueStatement;
generated$2.assertDebuggerStatement = assertDebuggerStatement;
generated$2.assertDoWhileStatement = assertDoWhileStatement;
generated$2.assertEmptyStatement = assertEmptyStatement;
generated$2.assertExpressionStatement = assertExpressionStatement;
generated$2.assertFile = assertFile;
generated$2.assertForInStatement = assertForInStatement;
generated$2.assertForStatement = assertForStatement;
generated$2.assertFunctionDeclaration = assertFunctionDeclaration;
generated$2.assertFunctionExpression = assertFunctionExpression;
generated$2.assertIdentifier = assertIdentifier;
generated$2.assertIfStatement = assertIfStatement;
generated$2.assertLabeledStatement = assertLabeledStatement;
generated$2.assertStringLiteral = assertStringLiteral;
generated$2.assertNumericLiteral = assertNumericLiteral;
generated$2.assertNullLiteral = assertNullLiteral;
generated$2.assertBooleanLiteral = assertBooleanLiteral;
generated$2.assertRegExpLiteral = assertRegExpLiteral;
generated$2.assertLogicalExpression = assertLogicalExpression;
generated$2.assertMemberExpression = assertMemberExpression;
generated$2.assertNewExpression = assertNewExpression;
generated$2.assertProgram = assertProgram;
generated$2.assertObjectExpression = assertObjectExpression;
generated$2.assertObjectMethod = assertObjectMethod;
generated$2.assertObjectProperty = assertObjectProperty;
generated$2.assertRestElement = assertRestElement;
generated$2.assertReturnStatement = assertReturnStatement;
generated$2.assertSequenceExpression = assertSequenceExpression;
generated$2.assertParenthesizedExpression = assertParenthesizedExpression;
generated$2.assertSwitchCase = assertSwitchCase;
generated$2.assertSwitchStatement = assertSwitchStatement;
generated$2.assertThisExpression = assertThisExpression;
generated$2.assertThrowStatement = assertThrowStatement;
generated$2.assertTryStatement = assertTryStatement;
generated$2.assertUnaryExpression = assertUnaryExpression;
generated$2.assertUpdateExpression = assertUpdateExpression;
generated$2.assertVariableDeclaration = assertVariableDeclaration;
generated$2.assertVariableDeclarator = assertVariableDeclarator;
generated$2.assertWhileStatement = assertWhileStatement;
generated$2.assertWithStatement = assertWithStatement;
generated$2.assertAssignmentPattern = assertAssignmentPattern;
generated$2.assertArrayPattern = assertArrayPattern;
generated$2.assertArrowFunctionExpression = assertArrowFunctionExpression;
generated$2.assertClassBody = assertClassBody;
generated$2.assertClassExpression = assertClassExpression;
generated$2.assertClassDeclaration = assertClassDeclaration;
generated$2.assertExportAllDeclaration = assertExportAllDeclaration;
generated$2.assertExportDefaultDeclaration = assertExportDefaultDeclaration;
generated$2.assertExportNamedDeclaration = assertExportNamedDeclaration;
generated$2.assertExportSpecifier = assertExportSpecifier;
generated$2.assertForOfStatement = assertForOfStatement;
generated$2.assertImportDeclaration = assertImportDeclaration;
generated$2.assertImportDefaultSpecifier = assertImportDefaultSpecifier;
generated$2.assertImportNamespaceSpecifier = assertImportNamespaceSpecifier;
generated$2.assertImportSpecifier = assertImportSpecifier;
generated$2.assertMetaProperty = assertMetaProperty;
generated$2.assertClassMethod = assertClassMethod;
generated$2.assertObjectPattern = assertObjectPattern;
generated$2.assertSpreadElement = assertSpreadElement;
generated$2.assertSuper = assertSuper;
generated$2.assertTaggedTemplateExpression = assertTaggedTemplateExpression;
generated$2.assertTemplateElement = assertTemplateElement;
generated$2.assertTemplateLiteral = assertTemplateLiteral;
generated$2.assertYieldExpression = assertYieldExpression;
generated$2.assertAwaitExpression = assertAwaitExpression;
generated$2.assertImport = assertImport;
generated$2.assertBigIntLiteral = assertBigIntLiteral;
generated$2.assertExportNamespaceSpecifier = assertExportNamespaceSpecifier;
generated$2.assertOptionalMemberExpression = assertOptionalMemberExpression;
generated$2.assertOptionalCallExpression = assertOptionalCallExpression;
generated$2.assertClassProperty = assertClassProperty;
generated$2.assertClassPrivateProperty = assertClassPrivateProperty;
generated$2.assertClassPrivateMethod = assertClassPrivateMethod;
generated$2.assertPrivateName = assertPrivateName;
generated$2.assertAnyTypeAnnotation = assertAnyTypeAnnotation;
generated$2.assertArrayTypeAnnotation = assertArrayTypeAnnotation;
generated$2.assertBooleanTypeAnnotation = assertBooleanTypeAnnotation;
generated$2.assertBooleanLiteralTypeAnnotation = assertBooleanLiteralTypeAnnotation;
generated$2.assertNullLiteralTypeAnnotation = assertNullLiteralTypeAnnotation;
generated$2.assertClassImplements = assertClassImplements;
generated$2.assertDeclareClass = assertDeclareClass;
generated$2.assertDeclareFunction = assertDeclareFunction;
generated$2.assertDeclareInterface = assertDeclareInterface;
generated$2.assertDeclareModule = assertDeclareModule;
generated$2.assertDeclareModuleExports = assertDeclareModuleExports;
generated$2.assertDeclareTypeAlias = assertDeclareTypeAlias;
generated$2.assertDeclareOpaqueType = assertDeclareOpaqueType;
generated$2.assertDeclareVariable = assertDeclareVariable;
generated$2.assertDeclareExportDeclaration = assertDeclareExportDeclaration;
generated$2.assertDeclareExportAllDeclaration = assertDeclareExportAllDeclaration;
generated$2.assertDeclaredPredicate = assertDeclaredPredicate;
generated$2.assertExistsTypeAnnotation = assertExistsTypeAnnotation;
generated$2.assertFunctionTypeAnnotation = assertFunctionTypeAnnotation;
generated$2.assertFunctionTypeParam = assertFunctionTypeParam;
generated$2.assertGenericTypeAnnotation = assertGenericTypeAnnotation;
generated$2.assertInferredPredicate = assertInferredPredicate;
generated$2.assertInterfaceExtends = assertInterfaceExtends;
generated$2.assertInterfaceDeclaration = assertInterfaceDeclaration;
generated$2.assertInterfaceTypeAnnotation = assertInterfaceTypeAnnotation;
generated$2.assertIntersectionTypeAnnotation = assertIntersectionTypeAnnotation;
generated$2.assertMixedTypeAnnotation = assertMixedTypeAnnotation;
generated$2.assertEmptyTypeAnnotation = assertEmptyTypeAnnotation;
generated$2.assertNullableTypeAnnotation = assertNullableTypeAnnotation;
generated$2.assertNumberLiteralTypeAnnotation = assertNumberLiteralTypeAnnotation;
generated$2.assertNumberTypeAnnotation = assertNumberTypeAnnotation;
generated$2.assertObjectTypeAnnotation = assertObjectTypeAnnotation;
generated$2.assertObjectTypeInternalSlot = assertObjectTypeInternalSlot;
generated$2.assertObjectTypeCallProperty = assertObjectTypeCallProperty;
generated$2.assertObjectTypeIndexer = assertObjectTypeIndexer;
generated$2.assertObjectTypeProperty = assertObjectTypeProperty;
generated$2.assertObjectTypeSpreadProperty = assertObjectTypeSpreadProperty;
generated$2.assertOpaqueType = assertOpaqueType;
generated$2.assertQualifiedTypeIdentifier = assertQualifiedTypeIdentifier;
generated$2.assertStringLiteralTypeAnnotation = assertStringLiteralTypeAnnotation;
generated$2.assertStringTypeAnnotation = assertStringTypeAnnotation;
generated$2.assertSymbolTypeAnnotation = assertSymbolTypeAnnotation;
generated$2.assertThisTypeAnnotation = assertThisTypeAnnotation;
generated$2.assertTupleTypeAnnotation = assertTupleTypeAnnotation;
generated$2.assertTypeofTypeAnnotation = assertTypeofTypeAnnotation;
generated$2.assertTypeAlias = assertTypeAlias;
generated$2.assertTypeAnnotation = assertTypeAnnotation;
generated$2.assertTypeCastExpression = assertTypeCastExpression;
generated$2.assertTypeParameter = assertTypeParameter;
generated$2.assertTypeParameterDeclaration = assertTypeParameterDeclaration;
generated$2.assertTypeParameterInstantiation = assertTypeParameterInstantiation;
generated$2.assertUnionTypeAnnotation = assertUnionTypeAnnotation;
generated$2.assertVariance = assertVariance;
generated$2.assertVoidTypeAnnotation = assertVoidTypeAnnotation;
generated$2.assertEnumDeclaration = assertEnumDeclaration;
generated$2.assertEnumBooleanBody = assertEnumBooleanBody;
generated$2.assertEnumNumberBody = assertEnumNumberBody;
generated$2.assertEnumStringBody = assertEnumStringBody;
generated$2.assertEnumSymbolBody = assertEnumSymbolBody;
generated$2.assertEnumBooleanMember = assertEnumBooleanMember;
generated$2.assertEnumNumberMember = assertEnumNumberMember;
generated$2.assertEnumStringMember = assertEnumStringMember;
generated$2.assertEnumDefaultedMember = assertEnumDefaultedMember;
generated$2.assertIndexedAccessType = assertIndexedAccessType;
generated$2.assertOptionalIndexedAccessType = assertOptionalIndexedAccessType;
generated$2.assertJSXAttribute = assertJSXAttribute;
generated$2.assertJSXClosingElement = assertJSXClosingElement;
generated$2.assertJSXElement = assertJSXElement;
generated$2.assertJSXEmptyExpression = assertJSXEmptyExpression;
generated$2.assertJSXExpressionContainer = assertJSXExpressionContainer;
generated$2.assertJSXSpreadChild = assertJSXSpreadChild;
generated$2.assertJSXIdentifier = assertJSXIdentifier;
generated$2.assertJSXMemberExpression = assertJSXMemberExpression;
generated$2.assertJSXNamespacedName = assertJSXNamespacedName;
generated$2.assertJSXOpeningElement = assertJSXOpeningElement;
generated$2.assertJSXSpreadAttribute = assertJSXSpreadAttribute;
generated$2.assertJSXText = assertJSXText;
generated$2.assertJSXFragment = assertJSXFragment;
generated$2.assertJSXOpeningFragment = assertJSXOpeningFragment;
generated$2.assertJSXClosingFragment = assertJSXClosingFragment;
generated$2.assertNoop = assertNoop;
generated$2.assertPlaceholder = assertPlaceholder;
generated$2.assertV8IntrinsicIdentifier = assertV8IntrinsicIdentifier;
generated$2.assertArgumentPlaceholder = assertArgumentPlaceholder;
generated$2.assertBindExpression = assertBindExpression;
generated$2.assertImportAttribute = assertImportAttribute;
generated$2.assertDecorator = assertDecorator;
generated$2.assertDoExpression = assertDoExpression;
generated$2.assertExportDefaultSpecifier = assertExportDefaultSpecifier;
generated$2.assertRecordExpression = assertRecordExpression;
generated$2.assertTupleExpression = assertTupleExpression;
generated$2.assertDecimalLiteral = assertDecimalLiteral;
generated$2.assertStaticBlock = assertStaticBlock;
generated$2.assertModuleExpression = assertModuleExpression;
generated$2.assertTopicReference = assertTopicReference;
generated$2.assertPipelineTopicExpression = assertPipelineTopicExpression;
generated$2.assertPipelineBareFunction = assertPipelineBareFunction;
generated$2.assertPipelinePrimaryTopicReference = assertPipelinePrimaryTopicReference;
generated$2.assertTSParameterProperty = assertTSParameterProperty;
generated$2.assertTSDeclareFunction = assertTSDeclareFunction;
generated$2.assertTSDeclareMethod = assertTSDeclareMethod;
generated$2.assertTSQualifiedName = assertTSQualifiedName;
generated$2.assertTSCallSignatureDeclaration = assertTSCallSignatureDeclaration;
generated$2.assertTSConstructSignatureDeclaration = assertTSConstructSignatureDeclaration;
generated$2.assertTSPropertySignature = assertTSPropertySignature;
generated$2.assertTSMethodSignature = assertTSMethodSignature;
generated$2.assertTSIndexSignature = assertTSIndexSignature;
generated$2.assertTSAnyKeyword = assertTSAnyKeyword;
generated$2.assertTSBooleanKeyword = assertTSBooleanKeyword;
generated$2.assertTSBigIntKeyword = assertTSBigIntKeyword;
generated$2.assertTSIntrinsicKeyword = assertTSIntrinsicKeyword;
generated$2.assertTSNeverKeyword = assertTSNeverKeyword;
generated$2.assertTSNullKeyword = assertTSNullKeyword;
generated$2.assertTSNumberKeyword = assertTSNumberKeyword;
generated$2.assertTSObjectKeyword = assertTSObjectKeyword;
generated$2.assertTSStringKeyword = assertTSStringKeyword;
generated$2.assertTSSymbolKeyword = assertTSSymbolKeyword;
generated$2.assertTSUndefinedKeyword = assertTSUndefinedKeyword;
generated$2.assertTSUnknownKeyword = assertTSUnknownKeyword;
generated$2.assertTSVoidKeyword = assertTSVoidKeyword;
generated$2.assertTSThisType = assertTSThisType;
generated$2.assertTSFunctionType = assertTSFunctionType;
generated$2.assertTSConstructorType = assertTSConstructorType;
generated$2.assertTSTypeReference = assertTSTypeReference;
generated$2.assertTSTypePredicate = assertTSTypePredicate;
generated$2.assertTSTypeQuery = assertTSTypeQuery;
generated$2.assertTSTypeLiteral = assertTSTypeLiteral;
generated$2.assertTSArrayType = assertTSArrayType;
generated$2.assertTSTupleType = assertTSTupleType;
generated$2.assertTSOptionalType = assertTSOptionalType;
generated$2.assertTSRestType = assertTSRestType;
generated$2.assertTSNamedTupleMember = assertTSNamedTupleMember;
generated$2.assertTSUnionType = assertTSUnionType;
generated$2.assertTSIntersectionType = assertTSIntersectionType;
generated$2.assertTSConditionalType = assertTSConditionalType;
generated$2.assertTSInferType = assertTSInferType;
generated$2.assertTSParenthesizedType = assertTSParenthesizedType;
generated$2.assertTSTypeOperator = assertTSTypeOperator;
generated$2.assertTSIndexedAccessType = assertTSIndexedAccessType;
generated$2.assertTSMappedType = assertTSMappedType;
generated$2.assertTSLiteralType = assertTSLiteralType;
generated$2.assertTSExpressionWithTypeArguments = assertTSExpressionWithTypeArguments;
generated$2.assertTSInterfaceDeclaration = assertTSInterfaceDeclaration;
generated$2.assertTSInterfaceBody = assertTSInterfaceBody;
generated$2.assertTSTypeAliasDeclaration = assertTSTypeAliasDeclaration;
generated$2.assertTSAsExpression = assertTSAsExpression;
generated$2.assertTSTypeAssertion = assertTSTypeAssertion;
generated$2.assertTSEnumDeclaration = assertTSEnumDeclaration;
generated$2.assertTSEnumMember = assertTSEnumMember;
generated$2.assertTSModuleDeclaration = assertTSModuleDeclaration;
generated$2.assertTSModuleBlock = assertTSModuleBlock;
generated$2.assertTSImportType = assertTSImportType;
generated$2.assertTSImportEqualsDeclaration = assertTSImportEqualsDeclaration;
generated$2.assertTSExternalModuleReference = assertTSExternalModuleReference;
generated$2.assertTSNonNullExpression = assertTSNonNullExpression;
generated$2.assertTSExportAssignment = assertTSExportAssignment;
generated$2.assertTSNamespaceExportDeclaration = assertTSNamespaceExportDeclaration;
generated$2.assertTSTypeAnnotation = assertTSTypeAnnotation;
generated$2.assertTSTypeParameterInstantiation = assertTSTypeParameterInstantiation;
generated$2.assertTSTypeParameterDeclaration = assertTSTypeParameterDeclaration;
generated$2.assertTSTypeParameter = assertTSTypeParameter;
generated$2.assertExpression = assertExpression;
generated$2.assertBinary = assertBinary;
generated$2.assertScopable = assertScopable;
generated$2.assertBlockParent = assertBlockParent;
generated$2.assertBlock = assertBlock;
generated$2.assertStatement = assertStatement;
generated$2.assertTerminatorless = assertTerminatorless;
generated$2.assertCompletionStatement = assertCompletionStatement;
generated$2.assertConditional = assertConditional;
generated$2.assertLoop = assertLoop;
generated$2.assertWhile = assertWhile;
generated$2.assertExpressionWrapper = assertExpressionWrapper;
generated$2.assertFor = assertFor;
generated$2.assertForXStatement = assertForXStatement;
generated$2.assertFunction = assertFunction;
generated$2.assertFunctionParent = assertFunctionParent;
generated$2.assertPureish = assertPureish;
generated$2.assertDeclaration = assertDeclaration;
generated$2.assertPatternLike = assertPatternLike;
generated$2.assertLVal = assertLVal;
generated$2.assertTSEntityName = assertTSEntityName;
generated$2.assertLiteral = assertLiteral;
generated$2.assertImmutable = assertImmutable;
generated$2.assertUserWhitespacable = assertUserWhitespacable;
generated$2.assertMethod = assertMethod;
generated$2.assertObjectMember = assertObjectMember;
generated$2.assertProperty = assertProperty;
generated$2.assertUnaryLike = assertUnaryLike;
generated$2.assertPattern = assertPattern;
generated$2.assertClass = assertClass;
generated$2.assertModuleDeclaration = assertModuleDeclaration;
generated$2.assertExportDeclaration = assertExportDeclaration;
generated$2.assertModuleSpecifier = assertModuleSpecifier;
generated$2.assertPrivate = assertPrivate;
generated$2.assertFlow = assertFlow;
generated$2.assertFlowType = assertFlowType;
generated$2.assertFlowBaseAnnotation = assertFlowBaseAnnotation;
generated$2.assertFlowDeclaration = assertFlowDeclaration;
generated$2.assertFlowPredicate = assertFlowPredicate;
generated$2.assertEnumBody = assertEnumBody;
generated$2.assertEnumMember = assertEnumMember;
generated$2.assertJSX = assertJSX;
generated$2.assertTSTypeElement = assertTSTypeElement;
generated$2.assertTSType = assertTSType;
generated$2.assertTSBaseType = assertTSBaseType;
generated$2.assertNumberLiteral = assertNumberLiteral;
generated$2.assertRegexLiteral = assertRegexLiteral;
generated$2.assertRestProperty = assertRestProperty;
generated$2.assertSpreadProperty = assertSpreadProperty;
var _is = requireIs();
function assert(type, node, opts) {
    if (!(0, _is.default)(type, node, opts)) {
        throw new Error('Expected type "' + type + '" with option ' + JSON.stringify(opts) + ", " + ('but instead got "' + node.type + '".'));
    }
}
function assertArrayExpression(node, opts) {
    assert("ArrayExpression", node, opts);
}
function assertAssignmentExpression(node, opts) {
    assert("AssignmentExpression", node, opts);
}
function assertBinaryExpression(node, opts) {
    assert("BinaryExpression", node, opts);
}
function assertInterpreterDirective(node, opts) {
    assert("InterpreterDirective", node, opts);
}
function assertDirective(node, opts) {
    assert("Directive", node, opts);
}
function assertDirectiveLiteral(node, opts) {
    assert("DirectiveLiteral", node, opts);
}
function assertBlockStatement(node, opts) {
    assert("BlockStatement", node, opts);
}
function assertBreakStatement(node, opts) {
    assert("BreakStatement", node, opts);
}
function assertCallExpression(node, opts) {
    assert("CallExpression", node, opts);
}
function assertCatchClause(node, opts) {
    assert("CatchClause", node, opts);
}
function assertConditionalExpression(node, opts) {
    assert("ConditionalExpression", node, opts);
}
function assertContinueStatement(node, opts) {
    assert("ContinueStatement", node, opts);
}
function assertDebuggerStatement(node, opts) {
    assert("DebuggerStatement", node, opts);
}
function assertDoWhileStatement(node, opts) {
    assert("DoWhileStatement", node, opts);
}
function assertEmptyStatement(node, opts) {
    assert("EmptyStatement", node, opts);
}
function assertExpressionStatement(node, opts) {
    assert("ExpressionStatement", node, opts);
}
function assertFile(node, opts) {
    assert("File", node, opts);
}
function assertForInStatement(node, opts) {
    assert("ForInStatement", node, opts);
}
function assertForStatement(node, opts) {
    assert("ForStatement", node, opts);
}
function assertFunctionDeclaration(node, opts) {
    assert("FunctionDeclaration", node, opts);
}
function assertFunctionExpression(node, opts) {
    assert("FunctionExpression", node, opts);
}
function assertIdentifier(node, opts) {
    assert("Identifier", node, opts);
}
function assertIfStatement(node, opts) {
    assert("IfStatement", node, opts);
}
function assertLabeledStatement(node, opts) {
    assert("LabeledStatement", node, opts);
}
function assertStringLiteral(node, opts) {
    assert("StringLiteral", node, opts);
}
function assertNumericLiteral(node, opts) {
    assert("NumericLiteral", node, opts);
}
function assertNullLiteral(node, opts) {
    assert("NullLiteral", node, opts);
}
function assertBooleanLiteral(node, opts) {
    assert("BooleanLiteral", node, opts);
}
function assertRegExpLiteral(node, opts) {
    assert("RegExpLiteral", node, opts);
}
function assertLogicalExpression(node, opts) {
    assert("LogicalExpression", node, opts);
}
function assertMemberExpression(node, opts) {
    assert("MemberExpression", node, opts);
}
function assertNewExpression(node, opts) {
    assert("NewExpression", node, opts);
}
function assertProgram(node, opts) {
    assert("Program", node, opts);
}
function assertObjectExpression(node, opts) {
    assert("ObjectExpression", node, opts);
}
function assertObjectMethod(node, opts) {
    assert("ObjectMethod", node, opts);
}
function assertObjectProperty(node, opts) {
    assert("ObjectProperty", node, opts);
}
function assertRestElement(node, opts) {
    assert("RestElement", node, opts);
}
function assertReturnStatement(node, opts) {
    assert("ReturnStatement", node, opts);
}
function assertSequenceExpression(node, opts) {
    assert("SequenceExpression", node, opts);
}
function assertParenthesizedExpression(node, opts) {
    assert("ParenthesizedExpression", node, opts);
}
function assertSwitchCase(node, opts) {
    assert("SwitchCase", node, opts);
}
function assertSwitchStatement(node, opts) {
    assert("SwitchStatement", node, opts);
}
function assertThisExpression(node, opts) {
    assert("ThisExpression", node, opts);
}
function assertThrowStatement(node, opts) {
    assert("ThrowStatement", node, opts);
}
function assertTryStatement(node, opts) {
    assert("TryStatement", node, opts);
}
function assertUnaryExpression(node, opts) {
    assert("UnaryExpression", node, opts);
}
function assertUpdateExpression(node, opts) {
    assert("UpdateExpression", node, opts);
}
function assertVariableDeclaration(node, opts) {
    assert("VariableDeclaration", node, opts);
}
function assertVariableDeclarator(node, opts) {
    assert("VariableDeclarator", node, opts);
}
function assertWhileStatement(node, opts) {
    assert("WhileStatement", node, opts);
}
function assertWithStatement(node, opts) {
    assert("WithStatement", node, opts);
}
function assertAssignmentPattern(node, opts) {
    assert("AssignmentPattern", node, opts);
}
function assertArrayPattern(node, opts) {
    assert("ArrayPattern", node, opts);
}
function assertArrowFunctionExpression(node, opts) {
    assert("ArrowFunctionExpression", node, opts);
}
function assertClassBody(node, opts) {
    assert("ClassBody", node, opts);
}
function assertClassExpression(node, opts) {
    assert("ClassExpression", node, opts);
}
function assertClassDeclaration(node, opts) {
    assert("ClassDeclaration", node, opts);
}
function assertExportAllDeclaration(node, opts) {
    assert("ExportAllDeclaration", node, opts);
}
function assertExportDefaultDeclaration(node, opts) {
    assert("ExportDefaultDeclaration", node, opts);
}
function assertExportNamedDeclaration(node, opts) {
    assert("ExportNamedDeclaration", node, opts);
}
function assertExportSpecifier(node, opts) {
    assert("ExportSpecifier", node, opts);
}
function assertForOfStatement(node, opts) {
    assert("ForOfStatement", node, opts);
}
function assertImportDeclaration(node, opts) {
    assert("ImportDeclaration", node, opts);
}
function assertImportDefaultSpecifier(node, opts) {
    assert("ImportDefaultSpecifier", node, opts);
}
function assertImportNamespaceSpecifier(node, opts) {
    assert("ImportNamespaceSpecifier", node, opts);
}
function assertImportSpecifier(node, opts) {
    assert("ImportSpecifier", node, opts);
}
function assertMetaProperty(node, opts) {
    assert("MetaProperty", node, opts);
}
function assertClassMethod(node, opts) {
    assert("ClassMethod", node, opts);
}
function assertObjectPattern(node, opts) {
    assert("ObjectPattern", node, opts);
}
function assertSpreadElement(node, opts) {
    assert("SpreadElement", node, opts);
}
function assertSuper(node, opts) {
    assert("Super", node, opts);
}
function assertTaggedTemplateExpression(node, opts) {
    assert("TaggedTemplateExpression", node, opts);
}
function assertTemplateElement(node, opts) {
    assert("TemplateElement", node, opts);
}
function assertTemplateLiteral(node, opts) {
    assert("TemplateLiteral", node, opts);
}
function assertYieldExpression(node, opts) {
    assert("YieldExpression", node, opts);
}
function assertAwaitExpression(node, opts) {
    assert("AwaitExpression", node, opts);
}
function assertImport(node, opts) {
    assert("Import", node, opts);
}
function assertBigIntLiteral(node, opts) {
    assert("BigIntLiteral", node, opts);
}
function assertExportNamespaceSpecifier(node, opts) {
    assert("ExportNamespaceSpecifier", node, opts);
}
function assertOptionalMemberExpression(node, opts) {
    assert("OptionalMemberExpression", node, opts);
}
function assertOptionalCallExpression(node, opts) {
    assert("OptionalCallExpression", node, opts);
}
function assertClassProperty(node, opts) {
    assert("ClassProperty", node, opts);
}
function assertClassPrivateProperty(node, opts) {
    assert("ClassPrivateProperty", node, opts);
}
function assertClassPrivateMethod(node, opts) {
    assert("ClassPrivateMethod", node, opts);
}
function assertPrivateName(node, opts) {
    assert("PrivateName", node, opts);
}
function assertAnyTypeAnnotation(node, opts) {
    assert("AnyTypeAnnotation", node, opts);
}
function assertArrayTypeAnnotation(node, opts) {
    assert("ArrayTypeAnnotation", node, opts);
}
function assertBooleanTypeAnnotation(node, opts) {
    assert("BooleanTypeAnnotation", node, opts);
}
function assertBooleanLiteralTypeAnnotation(node, opts) {
    assert("BooleanLiteralTypeAnnotation", node, opts);
}
function assertNullLiteralTypeAnnotation(node, opts) {
    assert("NullLiteralTypeAnnotation", node, opts);
}
function assertClassImplements(node, opts) {
    assert("ClassImplements", node, opts);
}
function assertDeclareClass(node, opts) {
    assert("DeclareClass", node, opts);
}
function assertDeclareFunction(node, opts) {
    assert("DeclareFunction", node, opts);
}
function assertDeclareInterface(node, opts) {
    assert("DeclareInterface", node, opts);
}
function assertDeclareModule(node, opts) {
    assert("DeclareModule", node, opts);
}
function assertDeclareModuleExports(node, opts) {
    assert("DeclareModuleExports", node, opts);
}
function assertDeclareTypeAlias(node, opts) {
    assert("DeclareTypeAlias", node, opts);
}
function assertDeclareOpaqueType(node, opts) {
    assert("DeclareOpaqueType", node, opts);
}
function assertDeclareVariable(node, opts) {
    assert("DeclareVariable", node, opts);
}
function assertDeclareExportDeclaration(node, opts) {
    assert("DeclareExportDeclaration", node, opts);
}
function assertDeclareExportAllDeclaration(node, opts) {
    assert("DeclareExportAllDeclaration", node, opts);
}
function assertDeclaredPredicate(node, opts) {
    assert("DeclaredPredicate", node, opts);
}
function assertExistsTypeAnnotation(node, opts) {
    assert("ExistsTypeAnnotation", node, opts);
}
function assertFunctionTypeAnnotation(node, opts) {
    assert("FunctionTypeAnnotation", node, opts);
}
function assertFunctionTypeParam(node, opts) {
    assert("FunctionTypeParam", node, opts);
}
function assertGenericTypeAnnotation(node, opts) {
    assert("GenericTypeAnnotation", node, opts);
}
function assertInferredPredicate(node, opts) {
    assert("InferredPredicate", node, opts);
}
function assertInterfaceExtends(node, opts) {
    assert("InterfaceExtends", node, opts);
}
function assertInterfaceDeclaration(node, opts) {
    assert("InterfaceDeclaration", node, opts);
}
function assertInterfaceTypeAnnotation(node, opts) {
    assert("InterfaceTypeAnnotation", node, opts);
}
function assertIntersectionTypeAnnotation(node, opts) {
    assert("IntersectionTypeAnnotation", node, opts);
}
function assertMixedTypeAnnotation(node, opts) {
    assert("MixedTypeAnnotation", node, opts);
}
function assertEmptyTypeAnnotation(node, opts) {
    assert("EmptyTypeAnnotation", node, opts);
}
function assertNullableTypeAnnotation(node, opts) {
    assert("NullableTypeAnnotation", node, opts);
}
function assertNumberLiteralTypeAnnotation(node, opts) {
    assert("NumberLiteralTypeAnnotation", node, opts);
}
function assertNumberTypeAnnotation(node, opts) {
    assert("NumberTypeAnnotation", node, opts);
}
function assertObjectTypeAnnotation(node, opts) {
    assert("ObjectTypeAnnotation", node, opts);
}
function assertObjectTypeInternalSlot(node, opts) {
    assert("ObjectTypeInternalSlot", node, opts);
}
function assertObjectTypeCallProperty(node, opts) {
    assert("ObjectTypeCallProperty", node, opts);
}
function assertObjectTypeIndexer(node, opts) {
    assert("ObjectTypeIndexer", node, opts);
}
function assertObjectTypeProperty(node, opts) {
    assert("ObjectTypeProperty", node, opts);
}
function assertObjectTypeSpreadProperty(node, opts) {
    assert("ObjectTypeSpreadProperty", node, opts);
}
function assertOpaqueType(node, opts) {
    assert("OpaqueType", node, opts);
}
function assertQualifiedTypeIdentifier(node, opts) {
    assert("QualifiedTypeIdentifier", node, opts);
}
function assertStringLiteralTypeAnnotation(node, opts) {
    assert("StringLiteralTypeAnnotation", node, opts);
}
function assertStringTypeAnnotation(node, opts) {
    assert("StringTypeAnnotation", node, opts);
}
function assertSymbolTypeAnnotation(node, opts) {
    assert("SymbolTypeAnnotation", node, opts);
}
function assertThisTypeAnnotation(node, opts) {
    assert("ThisTypeAnnotation", node, opts);
}
function assertTupleTypeAnnotation(node, opts) {
    assert("TupleTypeAnnotation", node, opts);
}
function assertTypeofTypeAnnotation(node, opts) {
    assert("TypeofTypeAnnotation", node, opts);
}
function assertTypeAlias(node, opts) {
    assert("TypeAlias", node, opts);
}
function assertTypeAnnotation(node, opts) {
    assert("TypeAnnotation", node, opts);
}
function assertTypeCastExpression(node, opts) {
    assert("TypeCastExpression", node, opts);
}
function assertTypeParameter(node, opts) {
    assert("TypeParameter", node, opts);
}
function assertTypeParameterDeclaration(node, opts) {
    assert("TypeParameterDeclaration", node, opts);
}
function assertTypeParameterInstantiation(node, opts) {
    assert("TypeParameterInstantiation", node, opts);
}
function assertUnionTypeAnnotation(node, opts) {
    assert("UnionTypeAnnotation", node, opts);
}
function assertVariance(node, opts) {
    assert("Variance", node, opts);
}
function assertVoidTypeAnnotation(node, opts) {
    assert("VoidTypeAnnotation", node, opts);
}
function assertEnumDeclaration(node, opts) {
    assert("EnumDeclaration", node, opts);
}
function assertEnumBooleanBody(node, opts) {
    assert("EnumBooleanBody", node, opts);
}
function assertEnumNumberBody(node, opts) {
    assert("EnumNumberBody", node, opts);
}
function assertEnumStringBody(node, opts) {
    assert("EnumStringBody", node, opts);
}
function assertEnumSymbolBody(node, opts) {
    assert("EnumSymbolBody", node, opts);
}
function assertEnumBooleanMember(node, opts) {
    assert("EnumBooleanMember", node, opts);
}
function assertEnumNumberMember(node, opts) {
    assert("EnumNumberMember", node, opts);
}
function assertEnumStringMember(node, opts) {
    assert("EnumStringMember", node, opts);
}
function assertEnumDefaultedMember(node, opts) {
    assert("EnumDefaultedMember", node, opts);
}
function assertIndexedAccessType(node, opts) {
    assert("IndexedAccessType", node, opts);
}
function assertOptionalIndexedAccessType(node, opts) {
    assert("OptionalIndexedAccessType", node, opts);
}
function assertJSXAttribute(node, opts) {
    assert("JSXAttribute", node, opts);
}
function assertJSXClosingElement(node, opts) {
    assert("JSXClosingElement", node, opts);
}
function assertJSXElement(node, opts) {
    assert("JSXElement", node, opts);
}
function assertJSXEmptyExpression(node, opts) {
    assert("JSXEmptyExpression", node, opts);
}
function assertJSXExpressionContainer(node, opts) {
    assert("JSXExpressionContainer", node, opts);
}
function assertJSXSpreadChild(node, opts) {
    assert("JSXSpreadChild", node, opts);
}
function assertJSXIdentifier(node, opts) {
    assert("JSXIdentifier", node, opts);
}
function assertJSXMemberExpression(node, opts) {
    assert("JSXMemberExpression", node, opts);
}
function assertJSXNamespacedName(node, opts) {
    assert("JSXNamespacedName", node, opts);
}
function assertJSXOpeningElement(node, opts) {
    assert("JSXOpeningElement", node, opts);
}
function assertJSXSpreadAttribute(node, opts) {
    assert("JSXSpreadAttribute", node, opts);
}
function assertJSXText(node, opts) {
    assert("JSXText", node, opts);
}
function assertJSXFragment(node, opts) {
    assert("JSXFragment", node, opts);
}
function assertJSXOpeningFragment(node, opts) {
    assert("JSXOpeningFragment", node, opts);
}
function assertJSXClosingFragment(node, opts) {
    assert("JSXClosingFragment", node, opts);
}
function assertNoop(node, opts) {
    assert("Noop", node, opts);
}
function assertPlaceholder(node, opts) {
    assert("Placeholder", node, opts);
}
function assertV8IntrinsicIdentifier(node, opts) {
    assert("V8IntrinsicIdentifier", node, opts);
}
function assertArgumentPlaceholder(node, opts) {
    assert("ArgumentPlaceholder", node, opts);
}
function assertBindExpression(node, opts) {
    assert("BindExpression", node, opts);
}
function assertImportAttribute(node, opts) {
    assert("ImportAttribute", node, opts);
}
function assertDecorator(node, opts) {
    assert("Decorator", node, opts);
}
function assertDoExpression(node, opts) {
    assert("DoExpression", node, opts);
}
function assertExportDefaultSpecifier(node, opts) {
    assert("ExportDefaultSpecifier", node, opts);
}
function assertRecordExpression(node, opts) {
    assert("RecordExpression", node, opts);
}
function assertTupleExpression(node, opts) {
    assert("TupleExpression", node, opts);
}
function assertDecimalLiteral(node, opts) {
    assert("DecimalLiteral", node, opts);
}
function assertStaticBlock(node, opts) {
    assert("StaticBlock", node, opts);
}
function assertModuleExpression(node, opts) {
    assert("ModuleExpression", node, opts);
}
function assertTopicReference(node, opts) {
    assert("TopicReference", node, opts);
}
function assertPipelineTopicExpression(node, opts) {
    assert("PipelineTopicExpression", node, opts);
}
function assertPipelineBareFunction(node, opts) {
    assert("PipelineBareFunction", node, opts);
}
function assertPipelinePrimaryTopicReference(node, opts) {
    assert("PipelinePrimaryTopicReference", node, opts);
}
function assertTSParameterProperty(node, opts) {
    assert("TSParameterProperty", node, opts);
}
function assertTSDeclareFunction(node, opts) {
    assert("TSDeclareFunction", node, opts);
}
function assertTSDeclareMethod(node, opts) {
    assert("TSDeclareMethod", node, opts);
}
function assertTSQualifiedName(node, opts) {
    assert("TSQualifiedName", node, opts);
}
function assertTSCallSignatureDeclaration(node, opts) {
    assert("TSCallSignatureDeclaration", node, opts);
}
function assertTSConstructSignatureDeclaration(node, opts) {
    assert("TSConstructSignatureDeclaration", node, opts);
}
function assertTSPropertySignature(node, opts) {
    assert("TSPropertySignature", node, opts);
}
function assertTSMethodSignature(node, opts) {
    assert("TSMethodSignature", node, opts);
}
function assertTSIndexSignature(node, opts) {
    assert("TSIndexSignature", node, opts);
}
function assertTSAnyKeyword(node, opts) {
    assert("TSAnyKeyword", node, opts);
}
function assertTSBooleanKeyword(node, opts) {
    assert("TSBooleanKeyword", node, opts);
}
function assertTSBigIntKeyword(node, opts) {
    assert("TSBigIntKeyword", node, opts);
}
function assertTSIntrinsicKeyword(node, opts) {
    assert("TSIntrinsicKeyword", node, opts);
}
function assertTSNeverKeyword(node, opts) {
    assert("TSNeverKeyword", node, opts);
}
function assertTSNullKeyword(node, opts) {
    assert("TSNullKeyword", node, opts);
}
function assertTSNumberKeyword(node, opts) {
    assert("TSNumberKeyword", node, opts);
}
function assertTSObjectKeyword(node, opts) {
    assert("TSObjectKeyword", node, opts);
}
function assertTSStringKeyword(node, opts) {
    assert("TSStringKeyword", node, opts);
}
function assertTSSymbolKeyword(node, opts) {
    assert("TSSymbolKeyword", node, opts);
}
function assertTSUndefinedKeyword(node, opts) {
    assert("TSUndefinedKeyword", node, opts);
}
function assertTSUnknownKeyword(node, opts) {
    assert("TSUnknownKeyword", node, opts);
}
function assertTSVoidKeyword(node, opts) {
    assert("TSVoidKeyword", node, opts);
}
function assertTSThisType(node, opts) {
    assert("TSThisType", node, opts);
}
function assertTSFunctionType(node, opts) {
    assert("TSFunctionType", node, opts);
}
function assertTSConstructorType(node, opts) {
    assert("TSConstructorType", node, opts);
}
function assertTSTypeReference(node, opts) {
    assert("TSTypeReference", node, opts);
}
function assertTSTypePredicate(node, opts) {
    assert("TSTypePredicate", node, opts);
}
function assertTSTypeQuery(node, opts) {
    assert("TSTypeQuery", node, opts);
}
function assertTSTypeLiteral(node, opts) {
    assert("TSTypeLiteral", node, opts);
}
function assertTSArrayType(node, opts) {
    assert("TSArrayType", node, opts);
}
function assertTSTupleType(node, opts) {
    assert("TSTupleType", node, opts);
}
function assertTSOptionalType(node, opts) {
    assert("TSOptionalType", node, opts);
}
function assertTSRestType(node, opts) {
    assert("TSRestType", node, opts);
}
function assertTSNamedTupleMember(node, opts) {
    assert("TSNamedTupleMember", node, opts);
}
function assertTSUnionType(node, opts) {
    assert("TSUnionType", node, opts);
}
function assertTSIntersectionType(node, opts) {
    assert("TSIntersectionType", node, opts);
}
function assertTSConditionalType(node, opts) {
    assert("TSConditionalType", node, opts);
}
function assertTSInferType(node, opts) {
    assert("TSInferType", node, opts);
}
function assertTSParenthesizedType(node, opts) {
    assert("TSParenthesizedType", node, opts);
}
function assertTSTypeOperator(node, opts) {
    assert("TSTypeOperator", node, opts);
}
function assertTSIndexedAccessType(node, opts) {
    assert("TSIndexedAccessType", node, opts);
}
function assertTSMappedType(node, opts) {
    assert("TSMappedType", node, opts);
}
function assertTSLiteralType(node, opts) {
    assert("TSLiteralType", node, opts);
}
function assertTSExpressionWithTypeArguments(node, opts) {
    assert("TSExpressionWithTypeArguments", node, opts);
}
function assertTSInterfaceDeclaration(node, opts) {
    assert("TSInterfaceDeclaration", node, opts);
}
function assertTSInterfaceBody(node, opts) {
    assert("TSInterfaceBody", node, opts);
}
function assertTSTypeAliasDeclaration(node, opts) {
    assert("TSTypeAliasDeclaration", node, opts);
}
function assertTSAsExpression(node, opts) {
    assert("TSAsExpression", node, opts);
}
function assertTSTypeAssertion(node, opts) {
    assert("TSTypeAssertion", node, opts);
}
function assertTSEnumDeclaration(node, opts) {
    assert("TSEnumDeclaration", node, opts);
}
function assertTSEnumMember(node, opts) {
    assert("TSEnumMember", node, opts);
}
function assertTSModuleDeclaration(node, opts) {
    assert("TSModuleDeclaration", node, opts);
}
function assertTSModuleBlock(node, opts) {
    assert("TSModuleBlock", node, opts);
}
function assertTSImportType(node, opts) {
    assert("TSImportType", node, opts);
}
function assertTSImportEqualsDeclaration(node, opts) {
    assert("TSImportEqualsDeclaration", node, opts);
}
function assertTSExternalModuleReference(node, opts) {
    assert("TSExternalModuleReference", node, opts);
}
function assertTSNonNullExpression(node, opts) {
    assert("TSNonNullExpression", node, opts);
}
function assertTSExportAssignment(node, opts) {
    assert("TSExportAssignment", node, opts);
}
function assertTSNamespaceExportDeclaration(node, opts) {
    assert("TSNamespaceExportDeclaration", node, opts);
}
function assertTSTypeAnnotation(node, opts) {
    assert("TSTypeAnnotation", node, opts);
}
function assertTSTypeParameterInstantiation(node, opts) {
    assert("TSTypeParameterInstantiation", node, opts);
}
function assertTSTypeParameterDeclaration(node, opts) {
    assert("TSTypeParameterDeclaration", node, opts);
}
function assertTSTypeParameter(node, opts) {
    assert("TSTypeParameter", node, opts);
}
function assertExpression(node, opts) {
    assert("Expression", node, opts);
}
function assertBinary(node, opts) {
    assert("Binary", node, opts);
}
function assertScopable(node, opts) {
    assert("Scopable", node, opts);
}
function assertBlockParent(node, opts) {
    assert("BlockParent", node, opts);
}
function assertBlock(node, opts) {
    assert("Block", node, opts);
}
function assertStatement(node, opts) {
    assert("Statement", node, opts);
}
function assertTerminatorless(node, opts) {
    assert("Terminatorless", node, opts);
}
function assertCompletionStatement(node, opts) {
    assert("CompletionStatement", node, opts);
}
function assertConditional(node, opts) {
    assert("Conditional", node, opts);
}
function assertLoop(node, opts) {
    assert("Loop", node, opts);
}
function assertWhile(node, opts) {
    assert("While", node, opts);
}
function assertExpressionWrapper(node, opts) {
    assert("ExpressionWrapper", node, opts);
}
function assertFor(node, opts) {
    assert("For", node, opts);
}
function assertForXStatement(node, opts) {
    assert("ForXStatement", node, opts);
}
function assertFunction(node, opts) {
    assert("Function", node, opts);
}
function assertFunctionParent(node, opts) {
    assert("FunctionParent", node, opts);
}
function assertPureish(node, opts) {
    assert("Pureish", node, opts);
}
function assertDeclaration(node, opts) {
    assert("Declaration", node, opts);
}
function assertPatternLike(node, opts) {
    assert("PatternLike", node, opts);
}
function assertLVal(node, opts) {
    assert("LVal", node, opts);
}
function assertTSEntityName(node, opts) {
    assert("TSEntityName", node, opts);
}
function assertLiteral(node, opts) {
    assert("Literal", node, opts);
}
function assertImmutable(node, opts) {
    assert("Immutable", node, opts);
}
function assertUserWhitespacable(node, opts) {
    assert("UserWhitespacable", node, opts);
}
function assertMethod(node, opts) {
    assert("Method", node, opts);
}
function assertObjectMember(node, opts) {
    assert("ObjectMember", node, opts);
}
function assertProperty(node, opts) {
    assert("Property", node, opts);
}
function assertUnaryLike(node, opts) {
    assert("UnaryLike", node, opts);
}
function assertPattern(node, opts) {
    assert("Pattern", node, opts);
}
function assertClass(node, opts) {
    assert("Class", node, opts);
}
function assertModuleDeclaration(node, opts) {
    assert("ModuleDeclaration", node, opts);
}
function assertExportDeclaration(node, opts) {
    assert("ExportDeclaration", node, opts);
}
function assertModuleSpecifier(node, opts) {
    assert("ModuleSpecifier", node, opts);
}
function assertPrivate(node, opts) {
    assert("Private", node, opts);
}
function assertFlow(node, opts) {
    assert("Flow", node, opts);
}
function assertFlowType(node, opts) {
    assert("FlowType", node, opts);
}
function assertFlowBaseAnnotation(node, opts) {
    assert("FlowBaseAnnotation", node, opts);
}
function assertFlowDeclaration(node, opts) {
    assert("FlowDeclaration", node, opts);
}
function assertFlowPredicate(node, opts) {
    assert("FlowPredicate", node, opts);
}
function assertEnumBody(node, opts) {
    assert("EnumBody", node, opts);
}
function assertEnumMember(node, opts) {
    assert("EnumMember", node, opts);
}
function assertJSX(node, opts) {
    assert("JSX", node, opts);
}
function assertTSTypeElement(node, opts) {
    assert("TSTypeElement", node, opts);
}
function assertTSType(node, opts) {
    assert("TSType", node, opts);
}
function assertTSBaseType(node, opts) {
    assert("TSBaseType", node, opts);
}
function assertNumberLiteral(node, opts) {
    console.trace("The node type NumberLiteral has been renamed to NumericLiteral");
    assert("NumberLiteral", node, opts);
}
function assertRegexLiteral(node, opts) {
    console.trace("The node type RegexLiteral has been renamed to RegExpLiteral");
    assert("RegexLiteral", node, opts);
}
function assertRestProperty(node, opts) {
    console.trace("The node type RestProperty has been renamed to RestElement");
    assert("RestProperty", node, opts);
}
function assertSpreadProperty(node, opts) {
    console.trace("The node type SpreadProperty has been renamed to SpreadElement");
    assert("SpreadProperty", node, opts);
}

var createTypeAnnotationBasedOnTypeof$1 = {};

Object.defineProperty(createTypeAnnotationBasedOnTypeof$1, "__esModule", {
    value: true
});
createTypeAnnotationBasedOnTypeof$1.default = createTypeAnnotationBasedOnTypeof;
var _generated$l = generated$3;
function createTypeAnnotationBasedOnTypeof(type) {
    if (type === "string") {
        return (0, _generated$l.stringTypeAnnotation)();
    } else if (type === "number") {
        return (0, _generated$l.numberTypeAnnotation)();
    } else if (type === "undefined") {
        return (0, _generated$l.voidTypeAnnotation)();
    } else if (type === "boolean") {
        return (0, _generated$l.booleanTypeAnnotation)();
    } else if (type === "function") {
        return (0, _generated$l.genericTypeAnnotation)((0, _generated$l.identifier)("Function"));
    } else if (type === "object") {
        return (0, _generated$l.genericTypeAnnotation)((0, _generated$l.identifier)("Object"));
    } else if (type === "symbol") {
        return (0, _generated$l.genericTypeAnnotation)((0, _generated$l.identifier)("Symbol"));
    } else if (type === "bigint") {
        return (0, _generated$l.anyTypeAnnotation)();
    } else {
        throw new Error("Invalid typeof value: " + type);
    }
}

var createFlowUnionType$1 = {};

var removeTypeDuplicates$3 = {};

function _createForOfIteratorHelperLoose$c(o, allowArrayLike) {
    var it = typeof Symbol !== "undefined" && o[Symbol.iterator] || o["@@iterator"];
    if (it) return (it = it.call(o)).next.bind(it);
    if (Array.isArray(o) || (it = unsupportedIterableToArray(o)) || allowArrayLike && o && typeof o.length === "number") {
        if (it) o = it;
        var i = 0;
        return function() {
            if (i >= o.length) return {
                done: true
            };
            return {
                done: false,
                value: o[i++]
            };
        };
    }
    throw new TypeError("Invalid attempt to iterate non-iterable instance.\\nIn order to be iterable, non-array objects must have a [Symbol.iterator]() method.");
}
Object.defineProperty(removeTypeDuplicates$3, "__esModule", {
    value: true
});
removeTypeDuplicates$3.default = removeTypeDuplicates$2;
var _generated$k = generated$4;
function getQualifiedName(node) {
    return (0, _generated$k.isIdentifier)(node) ? node.name : node.id.name + "." + getQualifiedName(node.qualification);
}
function removeTypeDuplicates$2(nodes) {
    var generics = {};
    var bases = {};
    var typeGroups = [];
    var types = [];
    for(var i = 0; i < nodes.length; i++){
        var node = nodes[i];
        if (!node) continue;
        if (types.indexOf(node) >= 0) {
            continue;
        }
        if ((0, _generated$k.isAnyTypeAnnotation)(node)) {
            return [
                node
            ];
        }
        if ((0, _generated$k.isFlowBaseAnnotation)(node)) {
            bases[node.type] = node;
            continue;
        }
        if ((0, _generated$k.isUnionTypeAnnotation)(node)) {
            if (typeGroups.indexOf(node.types) < 0) {
                nodes = nodes.concat(node.types);
                typeGroups.push(node.types);
            }
            continue;
        }
        if ((0, _generated$k.isGenericTypeAnnotation)(node)) {
            var name = getQualifiedName(node.id);
            if (generics[name]) {
                var existing = generics[name];
                if (existing.typeParameters) {
                    if (node.typeParameters) {
                        existing.typeParameters.params = removeTypeDuplicates$2(existing.typeParameters.params.concat(node.typeParameters.params));
                    }
                } else {
                    existing = node.typeParameters;
                }
            } else {
                generics[name] = node;
            }
            continue;
        }
        types.push(node);
    }
    for(var _iterator = _createForOfIteratorHelperLoose$c(Object.keys(bases)), _step; !(_step = _iterator()).done;){
        var type = _step.value;
        types.push(bases[type]);
    }
    for(var _iterator1 = _createForOfIteratorHelperLoose$c(Object.keys(generics)), _step1; !(_step1 = _iterator1()).done;){
        var name1 = _step1.value;
        types.push(generics[name1]);
    }
    return types;
}

Object.defineProperty(createFlowUnionType$1, "__esModule", {
    value: true
});
createFlowUnionType$1.default = createFlowUnionType;
var _generated$j = generated$3;
var _removeTypeDuplicates$1 = removeTypeDuplicates$3;
function createFlowUnionType(types) {
    var flattened = (0, _removeTypeDuplicates$1.default)(types);
    if (flattened.length === 1) {
        return flattened[0];
    } else {
        return (0, _generated$j.unionTypeAnnotation)(flattened);
    }
}

var createTSUnionType$1 = {};

var removeTypeDuplicates$1 = {};

function _createForOfIteratorHelperLoose$b(o, allowArrayLike) {
    var it = typeof Symbol !== "undefined" && o[Symbol.iterator] || o["@@iterator"];
    if (it) return (it = it.call(o)).next.bind(it);
    if (Array.isArray(o) || (it = unsupportedIterableToArray(o)) || allowArrayLike && o && typeof o.length === "number") {
        if (it) o = it;
        var i = 0;
        return function() {
            if (i >= o.length) return {
                done: true
            };
            return {
                done: false,
                value: o[i++]
            };
        };
    }
    throw new TypeError("Invalid attempt to iterate non-iterable instance.\\nIn order to be iterable, non-array objects must have a [Symbol.iterator]() method.");
}
Object.defineProperty(removeTypeDuplicates$1, "__esModule", {
    value: true
});
removeTypeDuplicates$1.default = removeTypeDuplicates;
var _generated$i = generated$4;
function removeTypeDuplicates(nodes) {
    var generics = {};
    var bases = {};
    var typeGroups = [];
    var types = [];
    for(var i = 0; i < nodes.length; i++){
        var node = nodes[i];
        if (!node) continue;
        if (types.indexOf(node) >= 0) {
            continue;
        }
        if ((0, _generated$i.isTSAnyKeyword)(node)) {
            return [
                node
            ];
        }
        if ((0, _generated$i.isTSBaseType)(node)) {
            bases[node.type] = node;
            continue;
        }
        if ((0, _generated$i.isTSUnionType)(node)) {
            if (typeGroups.indexOf(node.types) < 0) {
                nodes = nodes.concat(node.types);
                typeGroups.push(node.types);
            }
            continue;
        }
        types.push(node);
    }
    for(var _iterator = _createForOfIteratorHelperLoose$b(Object.keys(bases)), _step; !(_step = _iterator()).done;){
        var type = _step.value;
        types.push(bases[type]);
    }
    for(var _iterator1 = _createForOfIteratorHelperLoose$b(Object.keys(generics)), _step1; !(_step1 = _iterator1()).done;){
        var name = _step1.value;
        types.push(generics[name]);
    }
    return types;
}

Object.defineProperty(createTSUnionType$1, "__esModule", {
    value: true
});
createTSUnionType$1.default = createTSUnionType;
var _generated$h = generated$3;
var _removeTypeDuplicates = removeTypeDuplicates$1;
function createTSUnionType(typeAnnotations) {
    var types = typeAnnotations.map(function(type) {
        return type.typeAnnotation;
    });
    var flattened = (0, _removeTypeDuplicates.default)(types);
    if (flattened.length === 1) {
        return flattened[0];
    } else {
        return (0, _generated$h.tsUnionType)(flattened);
    }
}

var uppercase = {};

(function(exports) {
    Object.defineProperty(exports, "__esModule", {
        value: true
    });
    Object.defineProperty(exports, "ArrayExpression", {
        enumerable: true,
        get: function get() {
            return _index.arrayExpression;
        }
    });
    Object.defineProperty(exports, "AssignmentExpression", {
        enumerable: true,
        get: function get() {
            return _index.assignmentExpression;
        }
    });
    Object.defineProperty(exports, "BinaryExpression", {
        enumerable: true,
        get: function get() {
            return _index.binaryExpression;
        }
    });
    Object.defineProperty(exports, "InterpreterDirective", {
        enumerable: true,
        get: function get() {
            return _index.interpreterDirective;
        }
    });
    Object.defineProperty(exports, "Directive", {
        enumerable: true,
        get: function get() {
            return _index.directive;
        }
    });
    Object.defineProperty(exports, "DirectiveLiteral", {
        enumerable: true,
        get: function get() {
            return _index.directiveLiteral;
        }
    });
    Object.defineProperty(exports, "BlockStatement", {
        enumerable: true,
        get: function get() {
            return _index.blockStatement;
        }
    });
    Object.defineProperty(exports, "BreakStatement", {
        enumerable: true,
        get: function get() {
            return _index.breakStatement;
        }
    });
    Object.defineProperty(exports, "CallExpression", {
        enumerable: true,
        get: function get() {
            return _index.callExpression;
        }
    });
    Object.defineProperty(exports, "CatchClause", {
        enumerable: true,
        get: function get() {
            return _index.catchClause;
        }
    });
    Object.defineProperty(exports, "ConditionalExpression", {
        enumerable: true,
        get: function get() {
            return _index.conditionalExpression;
        }
    });
    Object.defineProperty(exports, "ContinueStatement", {
        enumerable: true,
        get: function get() {
            return _index.continueStatement;
        }
    });
    Object.defineProperty(exports, "DebuggerStatement", {
        enumerable: true,
        get: function get() {
            return _index.debuggerStatement;
        }
    });
    Object.defineProperty(exports, "DoWhileStatement", {
        enumerable: true,
        get: function get() {
            return _index.doWhileStatement;
        }
    });
    Object.defineProperty(exports, "EmptyStatement", {
        enumerable: true,
        get: function get() {
            return _index.emptyStatement;
        }
    });
    Object.defineProperty(exports, "ExpressionStatement", {
        enumerable: true,
        get: function get() {
            return _index.expressionStatement;
        }
    });
    Object.defineProperty(exports, "File", {
        enumerable: true,
        get: function get() {
            return _index.file;
        }
    });
    Object.defineProperty(exports, "ForInStatement", {
        enumerable: true,
        get: function get() {
            return _index.forInStatement;
        }
    });
    Object.defineProperty(exports, "ForStatement", {
        enumerable: true,
        get: function get() {
            return _index.forStatement;
        }
    });
    Object.defineProperty(exports, "FunctionDeclaration", {
        enumerable: true,
        get: function get() {
            return _index.functionDeclaration;
        }
    });
    Object.defineProperty(exports, "FunctionExpression", {
        enumerable: true,
        get: function get() {
            return _index.functionExpression;
        }
    });
    Object.defineProperty(exports, "Identifier", {
        enumerable: true,
        get: function get() {
            return _index.identifier;
        }
    });
    Object.defineProperty(exports, "IfStatement", {
        enumerable: true,
        get: function get() {
            return _index.ifStatement;
        }
    });
    Object.defineProperty(exports, "LabeledStatement", {
        enumerable: true,
        get: function get() {
            return _index.labeledStatement;
        }
    });
    Object.defineProperty(exports, "StringLiteral", {
        enumerable: true,
        get: function get() {
            return _index.stringLiteral;
        }
    });
    Object.defineProperty(exports, "NumericLiteral", {
        enumerable: true,
        get: function get() {
            return _index.numericLiteral;
        }
    });
    Object.defineProperty(exports, "NullLiteral", {
        enumerable: true,
        get: function get() {
            return _index.nullLiteral;
        }
    });
    Object.defineProperty(exports, "BooleanLiteral", {
        enumerable: true,
        get: function get() {
            return _index.booleanLiteral;
        }
    });
    Object.defineProperty(exports, "RegExpLiteral", {
        enumerable: true,
        get: function get() {
            return _index.regExpLiteral;
        }
    });
    Object.defineProperty(exports, "LogicalExpression", {
        enumerable: true,
        get: function get() {
            return _index.logicalExpression;
        }
    });
    Object.defineProperty(exports, "MemberExpression", {
        enumerable: true,
        get: function get() {
            return _index.memberExpression;
        }
    });
    Object.defineProperty(exports, "NewExpression", {
        enumerable: true,
        get: function get() {
            return _index.newExpression;
        }
    });
    Object.defineProperty(exports, "Program", {
        enumerable: true,
        get: function get() {
            return _index.program;
        }
    });
    Object.defineProperty(exports, "ObjectExpression", {
        enumerable: true,
        get: function get() {
            return _index.objectExpression;
        }
    });
    Object.defineProperty(exports, "ObjectMethod", {
        enumerable: true,
        get: function get() {
            return _index.objectMethod;
        }
    });
    Object.defineProperty(exports, "ObjectProperty", {
        enumerable: true,
        get: function get() {
            return _index.objectProperty;
        }
    });
    Object.defineProperty(exports, "RestElement", {
        enumerable: true,
        get: function get() {
            return _index.restElement;
        }
    });
    Object.defineProperty(exports, "ReturnStatement", {
        enumerable: true,
        get: function get() {
            return _index.returnStatement;
        }
    });
    Object.defineProperty(exports, "SequenceExpression", {
        enumerable: true,
        get: function get() {
            return _index.sequenceExpression;
        }
    });
    Object.defineProperty(exports, "ParenthesizedExpression", {
        enumerable: true,
        get: function get() {
            return _index.parenthesizedExpression;
        }
    });
    Object.defineProperty(exports, "SwitchCase", {
        enumerable: true,
        get: function get() {
            return _index.switchCase;
        }
    });
    Object.defineProperty(exports, "SwitchStatement", {
        enumerable: true,
        get: function get() {
            return _index.switchStatement;
        }
    });
    Object.defineProperty(exports, "ThisExpression", {
        enumerable: true,
        get: function get() {
            return _index.thisExpression;
        }
    });
    Object.defineProperty(exports, "ThrowStatement", {
        enumerable: true,
        get: function get() {
            return _index.throwStatement;
        }
    });
    Object.defineProperty(exports, "TryStatement", {
        enumerable: true,
        get: function get() {
            return _index.tryStatement;
        }
    });
    Object.defineProperty(exports, "UnaryExpression", {
        enumerable: true,
        get: function get() {
            return _index.unaryExpression;
        }
    });
    Object.defineProperty(exports, "UpdateExpression", {
        enumerable: true,
        get: function get() {
            return _index.updateExpression;
        }
    });
    Object.defineProperty(exports, "VariableDeclaration", {
        enumerable: true,
        get: function get() {
            return _index.variableDeclaration;
        }
    });
    Object.defineProperty(exports, "VariableDeclarator", {
        enumerable: true,
        get: function get() {
            return _index.variableDeclarator;
        }
    });
    Object.defineProperty(exports, "WhileStatement", {
        enumerable: true,
        get: function get() {
            return _index.whileStatement;
        }
    });
    Object.defineProperty(exports, "WithStatement", {
        enumerable: true,
        get: function get() {
            return _index.withStatement;
        }
    });
    Object.defineProperty(exports, "AssignmentPattern", {
        enumerable: true,
        get: function get() {
            return _index.assignmentPattern;
        }
    });
    Object.defineProperty(exports, "ArrayPattern", {
        enumerable: true,
        get: function get() {
            return _index.arrayPattern;
        }
    });
    Object.defineProperty(exports, "ArrowFunctionExpression", {
        enumerable: true,
        get: function get() {
            return _index.arrowFunctionExpression;
        }
    });
    Object.defineProperty(exports, "ClassBody", {
        enumerable: true,
        get: function get() {
            return _index.classBody;
        }
    });
    Object.defineProperty(exports, "ClassExpression", {
        enumerable: true,
        get: function get() {
            return _index.classExpression;
        }
    });
    Object.defineProperty(exports, "ClassDeclaration", {
        enumerable: true,
        get: function get() {
            return _index.classDeclaration;
        }
    });
    Object.defineProperty(exports, "ExportAllDeclaration", {
        enumerable: true,
        get: function get() {
            return _index.exportAllDeclaration;
        }
    });
    Object.defineProperty(exports, "ExportDefaultDeclaration", {
        enumerable: true,
        get: function get() {
            return _index.exportDefaultDeclaration;
        }
    });
    Object.defineProperty(exports, "ExportNamedDeclaration", {
        enumerable: true,
        get: function get() {
            return _index.exportNamedDeclaration;
        }
    });
    Object.defineProperty(exports, "ExportSpecifier", {
        enumerable: true,
        get: function get() {
            return _index.exportSpecifier;
        }
    });
    Object.defineProperty(exports, "ForOfStatement", {
        enumerable: true,
        get: function get() {
            return _index.forOfStatement;
        }
    });
    Object.defineProperty(exports, "ImportDeclaration", {
        enumerable: true,
        get: function get() {
            return _index.importDeclaration;
        }
    });
    Object.defineProperty(exports, "ImportDefaultSpecifier", {
        enumerable: true,
        get: function get() {
            return _index.importDefaultSpecifier;
        }
    });
    Object.defineProperty(exports, "ImportNamespaceSpecifier", {
        enumerable: true,
        get: function get() {
            return _index.importNamespaceSpecifier;
        }
    });
    Object.defineProperty(exports, "ImportSpecifier", {
        enumerable: true,
        get: function get() {
            return _index.importSpecifier;
        }
    });
    Object.defineProperty(exports, "MetaProperty", {
        enumerable: true,
        get: function get() {
            return _index.metaProperty;
        }
    });
    Object.defineProperty(exports, "ClassMethod", {
        enumerable: true,
        get: function get() {
            return _index.classMethod;
        }
    });
    Object.defineProperty(exports, "ObjectPattern", {
        enumerable: true,
        get: function get() {
            return _index.objectPattern;
        }
    });
    Object.defineProperty(exports, "SpreadElement", {
        enumerable: true,
        get: function get() {
            return _index.spreadElement;
        }
    });
    Object.defineProperty(exports, "Super", {
        enumerable: true,
        get: function get() {
            return _index.super;
        }
    });
    Object.defineProperty(exports, "TaggedTemplateExpression", {
        enumerable: true,
        get: function get() {
            return _index.taggedTemplateExpression;
        }
    });
    Object.defineProperty(exports, "TemplateElement", {
        enumerable: true,
        get: function get() {
            return _index.templateElement;
        }
    });
    Object.defineProperty(exports, "TemplateLiteral", {
        enumerable: true,
        get: function get() {
            return _index.templateLiteral;
        }
    });
    Object.defineProperty(exports, "YieldExpression", {
        enumerable: true,
        get: function get() {
            return _index.yieldExpression;
        }
    });
    Object.defineProperty(exports, "AwaitExpression", {
        enumerable: true,
        get: function get() {
            return _index.awaitExpression;
        }
    });
    Object.defineProperty(exports, "Import", {
        enumerable: true,
        get: function get() {
            return _index.import;
        }
    });
    Object.defineProperty(exports, "BigIntLiteral", {
        enumerable: true,
        get: function get() {
            return _index.bigIntLiteral;
        }
    });
    Object.defineProperty(exports, "ExportNamespaceSpecifier", {
        enumerable: true,
        get: function get() {
            return _index.exportNamespaceSpecifier;
        }
    });
    Object.defineProperty(exports, "OptionalMemberExpression", {
        enumerable: true,
        get: function get() {
            return _index.optionalMemberExpression;
        }
    });
    Object.defineProperty(exports, "OptionalCallExpression", {
        enumerable: true,
        get: function get() {
            return _index.optionalCallExpression;
        }
    });
    Object.defineProperty(exports, "ClassProperty", {
        enumerable: true,
        get: function get() {
            return _index.classProperty;
        }
    });
    Object.defineProperty(exports, "ClassPrivateProperty", {
        enumerable: true,
        get: function get() {
            return _index.classPrivateProperty;
        }
    });
    Object.defineProperty(exports, "ClassPrivateMethod", {
        enumerable: true,
        get: function get() {
            return _index.classPrivateMethod;
        }
    });
    Object.defineProperty(exports, "PrivateName", {
        enumerable: true,
        get: function get() {
            return _index.privateName;
        }
    });
    Object.defineProperty(exports, "AnyTypeAnnotation", {
        enumerable: true,
        get: function get() {
            return _index.anyTypeAnnotation;
        }
    });
    Object.defineProperty(exports, "ArrayTypeAnnotation", {
        enumerable: true,
        get: function get() {
            return _index.arrayTypeAnnotation;
        }
    });
    Object.defineProperty(exports, "BooleanTypeAnnotation", {
        enumerable: true,
        get: function get() {
            return _index.booleanTypeAnnotation;
        }
    });
    Object.defineProperty(exports, "BooleanLiteralTypeAnnotation", {
        enumerable: true,
        get: function get() {
            return _index.booleanLiteralTypeAnnotation;
        }
    });
    Object.defineProperty(exports, "NullLiteralTypeAnnotation", {
        enumerable: true,
        get: function get() {
            return _index.nullLiteralTypeAnnotation;
        }
    });
    Object.defineProperty(exports, "ClassImplements", {
        enumerable: true,
        get: function get() {
            return _index.classImplements;
        }
    });
    Object.defineProperty(exports, "DeclareClass", {
        enumerable: true,
        get: function get() {
            return _index.declareClass;
        }
    });
    Object.defineProperty(exports, "DeclareFunction", {
        enumerable: true,
        get: function get() {
            return _index.declareFunction;
        }
    });
    Object.defineProperty(exports, "DeclareInterface", {
        enumerable: true,
        get: function get() {
            return _index.declareInterface;
        }
    });
    Object.defineProperty(exports, "DeclareModule", {
        enumerable: true,
        get: function get() {
            return _index.declareModule;
        }
    });
    Object.defineProperty(exports, "DeclareModuleExports", {
        enumerable: true,
        get: function get() {
            return _index.declareModuleExports;
        }
    });
    Object.defineProperty(exports, "DeclareTypeAlias", {
        enumerable: true,
        get: function get() {
            return _index.declareTypeAlias;
        }
    });
    Object.defineProperty(exports, "DeclareOpaqueType", {
        enumerable: true,
        get: function get() {
            return _index.declareOpaqueType;
        }
    });
    Object.defineProperty(exports, "DeclareVariable", {
        enumerable: true,
        get: function get() {
            return _index.declareVariable;
        }
    });
    Object.defineProperty(exports, "DeclareExportDeclaration", {
        enumerable: true,
        get: function get() {
            return _index.declareExportDeclaration;
        }
    });
    Object.defineProperty(exports, "DeclareExportAllDeclaration", {
        enumerable: true,
        get: function get() {
            return _index.declareExportAllDeclaration;
        }
    });
    Object.defineProperty(exports, "DeclaredPredicate", {
        enumerable: true,
        get: function get() {
            return _index.declaredPredicate;
        }
    });
    Object.defineProperty(exports, "ExistsTypeAnnotation", {
        enumerable: true,
        get: function get() {
            return _index.existsTypeAnnotation;
        }
    });
    Object.defineProperty(exports, "FunctionTypeAnnotation", {
        enumerable: true,
        get: function get() {
            return _index.functionTypeAnnotation;
        }
    });
    Object.defineProperty(exports, "FunctionTypeParam", {
        enumerable: true,
        get: function get() {
            return _index.functionTypeParam;
        }
    });
    Object.defineProperty(exports, "GenericTypeAnnotation", {
        enumerable: true,
        get: function get() {
            return _index.genericTypeAnnotation;
        }
    });
    Object.defineProperty(exports, "InferredPredicate", {
        enumerable: true,
        get: function get() {
            return _index.inferredPredicate;
        }
    });
    Object.defineProperty(exports, "InterfaceExtends", {
        enumerable: true,
        get: function get() {
            return _index.interfaceExtends;
        }
    });
    Object.defineProperty(exports, "InterfaceDeclaration", {
        enumerable: true,
        get: function get() {
            return _index.interfaceDeclaration;
        }
    });
    Object.defineProperty(exports, "InterfaceTypeAnnotation", {
        enumerable: true,
        get: function get() {
            return _index.interfaceTypeAnnotation;
        }
    });
    Object.defineProperty(exports, "IntersectionTypeAnnotation", {
        enumerable: true,
        get: function get() {
            return _index.intersectionTypeAnnotation;
        }
    });
    Object.defineProperty(exports, "MixedTypeAnnotation", {
        enumerable: true,
        get: function get() {
            return _index.mixedTypeAnnotation;
        }
    });
    Object.defineProperty(exports, "EmptyTypeAnnotation", {
        enumerable: true,
        get: function get() {
            return _index.emptyTypeAnnotation;
        }
    });
    Object.defineProperty(exports, "NullableTypeAnnotation", {
        enumerable: true,
        get: function get() {
            return _index.nullableTypeAnnotation;
        }
    });
    Object.defineProperty(exports, "NumberLiteralTypeAnnotation", {
        enumerable: true,
        get: function get() {
            return _index.numberLiteralTypeAnnotation;
        }
    });
    Object.defineProperty(exports, "NumberTypeAnnotation", {
        enumerable: true,
        get: function get() {
            return _index.numberTypeAnnotation;
        }
    });
    Object.defineProperty(exports, "ObjectTypeAnnotation", {
        enumerable: true,
        get: function get() {
            return _index.objectTypeAnnotation;
        }
    });
    Object.defineProperty(exports, "ObjectTypeInternalSlot", {
        enumerable: true,
        get: function get() {
            return _index.objectTypeInternalSlot;
        }
    });
    Object.defineProperty(exports, "ObjectTypeCallProperty", {
        enumerable: true,
        get: function get() {
            return _index.objectTypeCallProperty;
        }
    });
    Object.defineProperty(exports, "ObjectTypeIndexer", {
        enumerable: true,
        get: function get() {
            return _index.objectTypeIndexer;
        }
    });
    Object.defineProperty(exports, "ObjectTypeProperty", {
        enumerable: true,
        get: function get() {
            return _index.objectTypeProperty;
        }
    });
    Object.defineProperty(exports, "ObjectTypeSpreadProperty", {
        enumerable: true,
        get: function get() {
            return _index.objectTypeSpreadProperty;
        }
    });
    Object.defineProperty(exports, "OpaqueType", {
        enumerable: true,
        get: function get() {
            return _index.opaqueType;
        }
    });
    Object.defineProperty(exports, "QualifiedTypeIdentifier", {
        enumerable: true,
        get: function get() {
            return _index.qualifiedTypeIdentifier;
        }
    });
    Object.defineProperty(exports, "StringLiteralTypeAnnotation", {
        enumerable: true,
        get: function get() {
            return _index.stringLiteralTypeAnnotation;
        }
    });
    Object.defineProperty(exports, "StringTypeAnnotation", {
        enumerable: true,
        get: function get() {
            return _index.stringTypeAnnotation;
        }
    });
    Object.defineProperty(exports, "SymbolTypeAnnotation", {
        enumerable: true,
        get: function get() {
            return _index.symbolTypeAnnotation;
        }
    });
    Object.defineProperty(exports, "ThisTypeAnnotation", {
        enumerable: true,
        get: function get() {
            return _index.thisTypeAnnotation;
        }
    });
    Object.defineProperty(exports, "TupleTypeAnnotation", {
        enumerable: true,
        get: function get() {
            return _index.tupleTypeAnnotation;
        }
    });
    Object.defineProperty(exports, "TypeofTypeAnnotation", {
        enumerable: true,
        get: function get() {
            return _index.typeofTypeAnnotation;
        }
    });
    Object.defineProperty(exports, "TypeAlias", {
        enumerable: true,
        get: function get() {
            return _index.typeAlias;
        }
    });
    Object.defineProperty(exports, "TypeAnnotation", {
        enumerable: true,
        get: function get() {
            return _index.typeAnnotation;
        }
    });
    Object.defineProperty(exports, "TypeCastExpression", {
        enumerable: true,
        get: function get() {
            return _index.typeCastExpression;
        }
    });
    Object.defineProperty(exports, "TypeParameter", {
        enumerable: true,
        get: function get() {
            return _index.typeParameter;
        }
    });
    Object.defineProperty(exports, "TypeParameterDeclaration", {
        enumerable: true,
        get: function get() {
            return _index.typeParameterDeclaration;
        }
    });
    Object.defineProperty(exports, "TypeParameterInstantiation", {
        enumerable: true,
        get: function get() {
            return _index.typeParameterInstantiation;
        }
    });
    Object.defineProperty(exports, "UnionTypeAnnotation", {
        enumerable: true,
        get: function get() {
            return _index.unionTypeAnnotation;
        }
    });
    Object.defineProperty(exports, "Variance", {
        enumerable: true,
        get: function get() {
            return _index.variance;
        }
    });
    Object.defineProperty(exports, "VoidTypeAnnotation", {
        enumerable: true,
        get: function get() {
            return _index.voidTypeAnnotation;
        }
    });
    Object.defineProperty(exports, "EnumDeclaration", {
        enumerable: true,
        get: function get() {
            return _index.enumDeclaration;
        }
    });
    Object.defineProperty(exports, "EnumBooleanBody", {
        enumerable: true,
        get: function get() {
            return _index.enumBooleanBody;
        }
    });
    Object.defineProperty(exports, "EnumNumberBody", {
        enumerable: true,
        get: function get() {
            return _index.enumNumberBody;
        }
    });
    Object.defineProperty(exports, "EnumStringBody", {
        enumerable: true,
        get: function get() {
            return _index.enumStringBody;
        }
    });
    Object.defineProperty(exports, "EnumSymbolBody", {
        enumerable: true,
        get: function get() {
            return _index.enumSymbolBody;
        }
    });
    Object.defineProperty(exports, "EnumBooleanMember", {
        enumerable: true,
        get: function get() {
            return _index.enumBooleanMember;
        }
    });
    Object.defineProperty(exports, "EnumNumberMember", {
        enumerable: true,
        get: function get() {
            return _index.enumNumberMember;
        }
    });
    Object.defineProperty(exports, "EnumStringMember", {
        enumerable: true,
        get: function get() {
            return _index.enumStringMember;
        }
    });
    Object.defineProperty(exports, "EnumDefaultedMember", {
        enumerable: true,
        get: function get() {
            return _index.enumDefaultedMember;
        }
    });
    Object.defineProperty(exports, "IndexedAccessType", {
        enumerable: true,
        get: function get() {
            return _index.indexedAccessType;
        }
    });
    Object.defineProperty(exports, "OptionalIndexedAccessType", {
        enumerable: true,
        get: function get() {
            return _index.optionalIndexedAccessType;
        }
    });
    Object.defineProperty(exports, "JSXAttribute", {
        enumerable: true,
        get: function get() {
            return _index.jsxAttribute;
        }
    });
    Object.defineProperty(exports, "JSXClosingElement", {
        enumerable: true,
        get: function get() {
            return _index.jsxClosingElement;
        }
    });
    Object.defineProperty(exports, "JSXElement", {
        enumerable: true,
        get: function get() {
            return _index.jsxElement;
        }
    });
    Object.defineProperty(exports, "JSXEmptyExpression", {
        enumerable: true,
        get: function get() {
            return _index.jsxEmptyExpression;
        }
    });
    Object.defineProperty(exports, "JSXExpressionContainer", {
        enumerable: true,
        get: function get() {
            return _index.jsxExpressionContainer;
        }
    });
    Object.defineProperty(exports, "JSXSpreadChild", {
        enumerable: true,
        get: function get() {
            return _index.jsxSpreadChild;
        }
    });
    Object.defineProperty(exports, "JSXIdentifier", {
        enumerable: true,
        get: function get() {
            return _index.jsxIdentifier;
        }
    });
    Object.defineProperty(exports, "JSXMemberExpression", {
        enumerable: true,
        get: function get() {
            return _index.jsxMemberExpression;
        }
    });
    Object.defineProperty(exports, "JSXNamespacedName", {
        enumerable: true,
        get: function get() {
            return _index.jsxNamespacedName;
        }
    });
    Object.defineProperty(exports, "JSXOpeningElement", {
        enumerable: true,
        get: function get() {
            return _index.jsxOpeningElement;
        }
    });
    Object.defineProperty(exports, "JSXSpreadAttribute", {
        enumerable: true,
        get: function get() {
            return _index.jsxSpreadAttribute;
        }
    });
    Object.defineProperty(exports, "JSXText", {
        enumerable: true,
        get: function get() {
            return _index.jsxText;
        }
    });
    Object.defineProperty(exports, "JSXFragment", {
        enumerable: true,
        get: function get() {
            return _index.jsxFragment;
        }
    });
    Object.defineProperty(exports, "JSXOpeningFragment", {
        enumerable: true,
        get: function get() {
            return _index.jsxOpeningFragment;
        }
    });
    Object.defineProperty(exports, "JSXClosingFragment", {
        enumerable: true,
        get: function get() {
            return _index.jsxClosingFragment;
        }
    });
    Object.defineProperty(exports, "Noop", {
        enumerable: true,
        get: function get() {
            return _index.noop;
        }
    });
    Object.defineProperty(exports, "Placeholder", {
        enumerable: true,
        get: function get() {
            return _index.placeholder;
        }
    });
    Object.defineProperty(exports, "V8IntrinsicIdentifier", {
        enumerable: true,
        get: function get() {
            return _index.v8IntrinsicIdentifier;
        }
    });
    Object.defineProperty(exports, "ArgumentPlaceholder", {
        enumerable: true,
        get: function get() {
            return _index.argumentPlaceholder;
        }
    });
    Object.defineProperty(exports, "BindExpression", {
        enumerable: true,
        get: function get() {
            return _index.bindExpression;
        }
    });
    Object.defineProperty(exports, "ImportAttribute", {
        enumerable: true,
        get: function get() {
            return _index.importAttribute;
        }
    });
    Object.defineProperty(exports, "Decorator", {
        enumerable: true,
        get: function get() {
            return _index.decorator;
        }
    });
    Object.defineProperty(exports, "DoExpression", {
        enumerable: true,
        get: function get() {
            return _index.doExpression;
        }
    });
    Object.defineProperty(exports, "ExportDefaultSpecifier", {
        enumerable: true,
        get: function get() {
            return _index.exportDefaultSpecifier;
        }
    });
    Object.defineProperty(exports, "RecordExpression", {
        enumerable: true,
        get: function get() {
            return _index.recordExpression;
        }
    });
    Object.defineProperty(exports, "TupleExpression", {
        enumerable: true,
        get: function get() {
            return _index.tupleExpression;
        }
    });
    Object.defineProperty(exports, "DecimalLiteral", {
        enumerable: true,
        get: function get() {
            return _index.decimalLiteral;
        }
    });
    Object.defineProperty(exports, "StaticBlock", {
        enumerable: true,
        get: function get() {
            return _index.staticBlock;
        }
    });
    Object.defineProperty(exports, "ModuleExpression", {
        enumerable: true,
        get: function get() {
            return _index.moduleExpression;
        }
    });
    Object.defineProperty(exports, "TopicReference", {
        enumerable: true,
        get: function get() {
            return _index.topicReference;
        }
    });
    Object.defineProperty(exports, "PipelineTopicExpression", {
        enumerable: true,
        get: function get() {
            return _index.pipelineTopicExpression;
        }
    });
    Object.defineProperty(exports, "PipelineBareFunction", {
        enumerable: true,
        get: function get() {
            return _index.pipelineBareFunction;
        }
    });
    Object.defineProperty(exports, "PipelinePrimaryTopicReference", {
        enumerable: true,
        get: function get() {
            return _index.pipelinePrimaryTopicReference;
        }
    });
    Object.defineProperty(exports, "TSParameterProperty", {
        enumerable: true,
        get: function get() {
            return _index.tsParameterProperty;
        }
    });
    Object.defineProperty(exports, "TSDeclareFunction", {
        enumerable: true,
        get: function get() {
            return _index.tsDeclareFunction;
        }
    });
    Object.defineProperty(exports, "TSDeclareMethod", {
        enumerable: true,
        get: function get() {
            return _index.tsDeclareMethod;
        }
    });
    Object.defineProperty(exports, "TSQualifiedName", {
        enumerable: true,
        get: function get() {
            return _index.tsQualifiedName;
        }
    });
    Object.defineProperty(exports, "TSCallSignatureDeclaration", {
        enumerable: true,
        get: function get() {
            return _index.tsCallSignatureDeclaration;
        }
    });
    Object.defineProperty(exports, "TSConstructSignatureDeclaration", {
        enumerable: true,
        get: function get() {
            return _index.tsConstructSignatureDeclaration;
        }
    });
    Object.defineProperty(exports, "TSPropertySignature", {
        enumerable: true,
        get: function get() {
            return _index.tsPropertySignature;
        }
    });
    Object.defineProperty(exports, "TSMethodSignature", {
        enumerable: true,
        get: function get() {
            return _index.tsMethodSignature;
        }
    });
    Object.defineProperty(exports, "TSIndexSignature", {
        enumerable: true,
        get: function get() {
            return _index.tsIndexSignature;
        }
    });
    Object.defineProperty(exports, "TSAnyKeyword", {
        enumerable: true,
        get: function get() {
            return _index.tsAnyKeyword;
        }
    });
    Object.defineProperty(exports, "TSBooleanKeyword", {
        enumerable: true,
        get: function get() {
            return _index.tsBooleanKeyword;
        }
    });
    Object.defineProperty(exports, "TSBigIntKeyword", {
        enumerable: true,
        get: function get() {
            return _index.tsBigIntKeyword;
        }
    });
    Object.defineProperty(exports, "TSIntrinsicKeyword", {
        enumerable: true,
        get: function get() {
            return _index.tsIntrinsicKeyword;
        }
    });
    Object.defineProperty(exports, "TSNeverKeyword", {
        enumerable: true,
        get: function get() {
            return _index.tsNeverKeyword;
        }
    });
    Object.defineProperty(exports, "TSNullKeyword", {
        enumerable: true,
        get: function get() {
            return _index.tsNullKeyword;
        }
    });
    Object.defineProperty(exports, "TSNumberKeyword", {
        enumerable: true,
        get: function get() {
            return _index.tsNumberKeyword;
        }
    });
    Object.defineProperty(exports, "TSObjectKeyword", {
        enumerable: true,
        get: function get() {
            return _index.tsObjectKeyword;
        }
    });
    Object.defineProperty(exports, "TSStringKeyword", {
        enumerable: true,
        get: function get() {
            return _index.tsStringKeyword;
        }
    });
    Object.defineProperty(exports, "TSSymbolKeyword", {
        enumerable: true,
        get: function get() {
            return _index.tsSymbolKeyword;
        }
    });
    Object.defineProperty(exports, "TSUndefinedKeyword", {
        enumerable: true,
        get: function get() {
            return _index.tsUndefinedKeyword;
        }
    });
    Object.defineProperty(exports, "TSUnknownKeyword", {
        enumerable: true,
        get: function get() {
            return _index.tsUnknownKeyword;
        }
    });
    Object.defineProperty(exports, "TSVoidKeyword", {
        enumerable: true,
        get: function get() {
            return _index.tsVoidKeyword;
        }
    });
    Object.defineProperty(exports, "TSThisType", {
        enumerable: true,
        get: function get() {
            return _index.tsThisType;
        }
    });
    Object.defineProperty(exports, "TSFunctionType", {
        enumerable: true,
        get: function get() {
            return _index.tsFunctionType;
        }
    });
    Object.defineProperty(exports, "TSConstructorType", {
        enumerable: true,
        get: function get() {
            return _index.tsConstructorType;
        }
    });
    Object.defineProperty(exports, "TSTypeReference", {
        enumerable: true,
        get: function get() {
            return _index.tsTypeReference;
        }
    });
    Object.defineProperty(exports, "TSTypePredicate", {
        enumerable: true,
        get: function get() {
            return _index.tsTypePredicate;
        }
    });
    Object.defineProperty(exports, "TSTypeQuery", {
        enumerable: true,
        get: function get() {
            return _index.tsTypeQuery;
        }
    });
    Object.defineProperty(exports, "TSTypeLiteral", {
        enumerable: true,
        get: function get() {
            return _index.tsTypeLiteral;
        }
    });
    Object.defineProperty(exports, "TSArrayType", {
        enumerable: true,
        get: function get() {
            return _index.tsArrayType;
        }
    });
    Object.defineProperty(exports, "TSTupleType", {
        enumerable: true,
        get: function get() {
            return _index.tsTupleType;
        }
    });
    Object.defineProperty(exports, "TSOptionalType", {
        enumerable: true,
        get: function get() {
            return _index.tsOptionalType;
        }
    });
    Object.defineProperty(exports, "TSRestType", {
        enumerable: true,
        get: function get() {
            return _index.tsRestType;
        }
    });
    Object.defineProperty(exports, "TSNamedTupleMember", {
        enumerable: true,
        get: function get() {
            return _index.tsNamedTupleMember;
        }
    });
    Object.defineProperty(exports, "TSUnionType", {
        enumerable: true,
        get: function get() {
            return _index.tsUnionType;
        }
    });
    Object.defineProperty(exports, "TSIntersectionType", {
        enumerable: true,
        get: function get() {
            return _index.tsIntersectionType;
        }
    });
    Object.defineProperty(exports, "TSConditionalType", {
        enumerable: true,
        get: function get() {
            return _index.tsConditionalType;
        }
    });
    Object.defineProperty(exports, "TSInferType", {
        enumerable: true,
        get: function get() {
            return _index.tsInferType;
        }
    });
    Object.defineProperty(exports, "TSParenthesizedType", {
        enumerable: true,
        get: function get() {
            return _index.tsParenthesizedType;
        }
    });
    Object.defineProperty(exports, "TSTypeOperator", {
        enumerable: true,
        get: function get() {
            return _index.tsTypeOperator;
        }
    });
    Object.defineProperty(exports, "TSIndexedAccessType", {
        enumerable: true,
        get: function get() {
            return _index.tsIndexedAccessType;
        }
    });
    Object.defineProperty(exports, "TSMappedType", {
        enumerable: true,
        get: function get() {
            return _index.tsMappedType;
        }
    });
    Object.defineProperty(exports, "TSLiteralType", {
        enumerable: true,
        get: function get() {
            return _index.tsLiteralType;
        }
    });
    Object.defineProperty(exports, "TSExpressionWithTypeArguments", {
        enumerable: true,
        get: function get() {
            return _index.tsExpressionWithTypeArguments;
        }
    });
    Object.defineProperty(exports, "TSInterfaceDeclaration", {
        enumerable: true,
        get: function get() {
            return _index.tsInterfaceDeclaration;
        }
    });
    Object.defineProperty(exports, "TSInterfaceBody", {
        enumerable: true,
        get: function get() {
            return _index.tsInterfaceBody;
        }
    });
    Object.defineProperty(exports, "TSTypeAliasDeclaration", {
        enumerable: true,
        get: function get() {
            return _index.tsTypeAliasDeclaration;
        }
    });
    Object.defineProperty(exports, "TSAsExpression", {
        enumerable: true,
        get: function get() {
            return _index.tsAsExpression;
        }
    });
    Object.defineProperty(exports, "TSTypeAssertion", {
        enumerable: true,
        get: function get() {
            return _index.tsTypeAssertion;
        }
    });
    Object.defineProperty(exports, "TSEnumDeclaration", {
        enumerable: true,
        get: function get() {
            return _index.tsEnumDeclaration;
        }
    });
    Object.defineProperty(exports, "TSEnumMember", {
        enumerable: true,
        get: function get() {
            return _index.tsEnumMember;
        }
    });
    Object.defineProperty(exports, "TSModuleDeclaration", {
        enumerable: true,
        get: function get() {
            return _index.tsModuleDeclaration;
        }
    });
    Object.defineProperty(exports, "TSModuleBlock", {
        enumerable: true,
        get: function get() {
            return _index.tsModuleBlock;
        }
    });
    Object.defineProperty(exports, "TSImportType", {
        enumerable: true,
        get: function get() {
            return _index.tsImportType;
        }
    });
    Object.defineProperty(exports, "TSImportEqualsDeclaration", {
        enumerable: true,
        get: function get() {
            return _index.tsImportEqualsDeclaration;
        }
    });
    Object.defineProperty(exports, "TSExternalModuleReference", {
        enumerable: true,
        get: function get() {
            return _index.tsExternalModuleReference;
        }
    });
    Object.defineProperty(exports, "TSNonNullExpression", {
        enumerable: true,
        get: function get() {
            return _index.tsNonNullExpression;
        }
    });
    Object.defineProperty(exports, "TSExportAssignment", {
        enumerable: true,
        get: function get() {
            return _index.tsExportAssignment;
        }
    });
    Object.defineProperty(exports, "TSNamespaceExportDeclaration", {
        enumerable: true,
        get: function get() {
            return _index.tsNamespaceExportDeclaration;
        }
    });
    Object.defineProperty(exports, "TSTypeAnnotation", {
        enumerable: true,
        get: function get() {
            return _index.tsTypeAnnotation;
        }
    });
    Object.defineProperty(exports, "TSTypeParameterInstantiation", {
        enumerable: true,
        get: function get() {
            return _index.tsTypeParameterInstantiation;
        }
    });
    Object.defineProperty(exports, "TSTypeParameterDeclaration", {
        enumerable: true,
        get: function get() {
            return _index.tsTypeParameterDeclaration;
        }
    });
    Object.defineProperty(exports, "TSTypeParameter", {
        enumerable: true,
        get: function get() {
            return _index.tsTypeParameter;
        }
    });
    Object.defineProperty(exports, "NumberLiteral", {
        enumerable: true,
        get: function get() {
            return _index.numberLiteral;
        }
    });
    Object.defineProperty(exports, "RegexLiteral", {
        enumerable: true,
        get: function get() {
            return _index.regexLiteral;
        }
    });
    Object.defineProperty(exports, "RestProperty", {
        enumerable: true,
        get: function get() {
            return _index.restProperty;
        }
    });
    Object.defineProperty(exports, "SpreadProperty", {
        enumerable: true,
        get: function get() {
            return _index.spreadProperty;
        }
    });
    var _index = generated$3;
})(uppercase);

var cloneNode$1 = {};

function _createForOfIteratorHelperLoose$a(o, allowArrayLike) {
    var it = typeof Symbol !== "undefined" && o[Symbol.iterator] || o["@@iterator"];
    if (it) return (it = it.call(o)).next.bind(it);
    if (Array.isArray(o) || (it = unsupportedIterableToArray(o)) || allowArrayLike && o && typeof o.length === "number") {
        if (it) o = it;
        var i = 0;
        return function() {
            if (i >= o.length) return {
                done: true
            };
            return {
                done: false,
                value: o[i++]
            };
        };
    }
    throw new TypeError("Invalid attempt to iterate non-iterable instance.\\nIn order to be iterable, non-array objects must have a [Symbol.iterator]() method.");
}
Object.defineProperty(cloneNode$1, "__esModule", {
    value: true
});
cloneNode$1.default = cloneNode;
var _definitions$4 = requireDefinitions();
var _generated$g = generated$4;
var has = Function.call.bind(Object.prototype.hasOwnProperty);
function cloneIfNode(obj, deep, withoutLoc) {
    if (obj && typeof obj.type === "string") {
        return cloneNode(obj, deep, withoutLoc);
    }
    return obj;
}
function cloneIfNodeOrArray(obj, deep, withoutLoc) {
    if (Array.isArray(obj)) {
        return obj.map(function(node) {
            return cloneIfNode(node, deep, withoutLoc);
        });
    }
    return cloneIfNode(obj, deep, withoutLoc);
}
function cloneNode(node, deep, withoutLoc) {
    if (deep === void 0) deep = true;
    if (withoutLoc === void 0) withoutLoc = false;
    if (!node) return node;
    var type = node.type;
    var newNode = {
        type: node.type
    };
    if ((0, _generated$g.isIdentifier)(node)) {
        newNode.name = node.name;
        if (has(node, "optional") && typeof node.optional === "boolean") {
            newNode.optional = node.optional;
        }
        if (has(node, "typeAnnotation")) {
            newNode.typeAnnotation = deep ? cloneIfNodeOrArray(node.typeAnnotation, true, withoutLoc) : node.typeAnnotation;
        }
    } else if (!has(_definitions$4.NODE_FIELDS, type)) {
        throw new Error('Unknown node type: "' + type + '"');
    } else {
        for(var _iterator = _createForOfIteratorHelperLoose$a(Object.keys(_definitions$4.NODE_FIELDS[type])), _step; !(_step = _iterator()).done;){
            var field = _step.value;
            if (has(node, field)) {
                if (deep) {
                    newNode[field] = (0, _generated$g.isFile)(node) && field === "comments" ? maybeCloneComments(node.comments, deep, withoutLoc) : cloneIfNodeOrArray(node[field], true, withoutLoc);
                } else {
                    newNode[field] = node[field];
                }
            }
        }
    }
    if (has(node, "loc")) {
        if (withoutLoc) {
            newNode.loc = null;
        } else {
            newNode.loc = node.loc;
        }
    }
    if (has(node, "leadingComments")) {
        newNode.leadingComments = maybeCloneComments(node.leadingComments, deep, withoutLoc);
    }
    if (has(node, "innerComments")) {
        newNode.innerComments = maybeCloneComments(node.innerComments, deep, withoutLoc);
    }
    if (has(node, "trailingComments")) {
        newNode.trailingComments = maybeCloneComments(node.trailingComments, deep, withoutLoc);
    }
    if (has(node, "extra")) {
        newNode.extra = Object.assign({}, node.extra);
    }
    return newNode;
}
function maybeCloneComments(comments, deep, withoutLoc) {
    if (!comments || !deep) {
        return comments;
    }
    return comments.map(function(param) {
        var type = param.type, value = param.value, loc = param.loc;
        if (withoutLoc) {
            return {
                type: type,
                value: value,
                loc: null
            };
        }
        return {
            type: type,
            value: value,
            loc: loc
        };
    });
}

var clone$1 = {};

Object.defineProperty(clone$1, "__esModule", {
    value: true
});
clone$1.default = clone;
var _cloneNode$5 = cloneNode$1;
function clone(node) {
    return (0, _cloneNode$5.default)(node, false);
}

var cloneDeep$1 = {};

Object.defineProperty(cloneDeep$1, "__esModule", {
    value: true
});
cloneDeep$1.default = cloneDeep;
var _cloneNode$4 = cloneNode$1;
function cloneDeep(node) {
    return (0, _cloneNode$4.default)(node);
}

var cloneDeepWithoutLoc$1 = {};

Object.defineProperty(cloneDeepWithoutLoc$1, "__esModule", {
    value: true
});
cloneDeepWithoutLoc$1.default = cloneDeepWithoutLoc;
var _cloneNode$3 = cloneNode$1;
function cloneDeepWithoutLoc(node) {
    return (0, _cloneNode$3.default)(node, true, true);
}

var cloneWithoutLoc$1 = {};

Object.defineProperty(cloneWithoutLoc$1, "__esModule", {
    value: true
});
cloneWithoutLoc$1.default = cloneWithoutLoc;
var _cloneNode$2 = cloneNode$1;
function cloneWithoutLoc(node) {
    return (0, _cloneNode$2.default)(node, false, true);
}

var addComment$1 = {};

var addComments$1 = {};

Object.defineProperty(addComments$1, "__esModule", {
    value: true
});
addComments$1.default = addComments;
function addComments(node, type, comments) {
    if (!comments || !node) return node;
    var key = "" + type + "Comments";
    if (node[key]) {
        if (type === "leading") {
            node[key] = comments.concat(node[key]);
        } else {
            node[key] = node[key].concat(comments);
        }
    } else {
        node[key] = comments;
    }
    return node;
}

Object.defineProperty(addComment$1, "__esModule", {
    value: true
});
addComment$1.default = addComment;
var _addComments = addComments$1;
function addComment(node, type, content, line) {
    return (0, _addComments.default)(node, type, [
        {
            type: line ? "CommentLine" : "CommentBlock",
            value: content
        }
    ]);
}

var inheritInnerComments$1 = {};

var inherit$1 = {};

Object.defineProperty(inherit$1, "__esModule", {
    value: true
});
inherit$1.default = inherit;
function inherit(key, child, parent) {
    if (child && parent) {
        child[key] = Array.from(new Set([].concat(child[key], parent[key]).filter(Boolean)));
    }
}

Object.defineProperty(inheritInnerComments$1, "__esModule", {
    value: true
});
inheritInnerComments$1.default = inheritInnerComments;
var _inherit$2 = inherit$1;
function inheritInnerComments(child, parent) {
    (0, _inherit$2.default)("innerComments", child, parent);
}

var inheritLeadingComments$1 = {};

Object.defineProperty(inheritLeadingComments$1, "__esModule", {
    value: true
});
inheritLeadingComments$1.default = inheritLeadingComments;
var _inherit$1 = inherit$1;
function inheritLeadingComments(child, parent) {
    (0, _inherit$1.default)("leadingComments", child, parent);
}

var inheritsComments$1 = {};

var inheritTrailingComments$1 = {};

Object.defineProperty(inheritTrailingComments$1, "__esModule", {
    value: true
});
inheritTrailingComments$1.default = inheritTrailingComments;
var _inherit = inherit$1;
function inheritTrailingComments(child, parent) {
    (0, _inherit.default)("trailingComments", child, parent);
}

Object.defineProperty(inheritsComments$1, "__esModule", {
    value: true
});
inheritsComments$1.default = inheritsComments;
var _inheritTrailingComments = inheritTrailingComments$1;
var _inheritLeadingComments = inheritLeadingComments$1;
var _inheritInnerComments = inheritInnerComments$1;
function inheritsComments(child, parent) {
    (0, _inheritTrailingComments.default)(child, parent);
    (0, _inheritLeadingComments.default)(child, parent);
    (0, _inheritInnerComments.default)(child, parent);
    return child;
}

var removeComments$1 = {};

Object.defineProperty(removeComments$1, "__esModule", {
    value: true
});
removeComments$1.default = removeComments;
var _constants$4 = constants;
function removeComments(node) {
    _constants$4.COMMENT_KEYS.forEach(function(key) {
        node[key] = null;
    });
    return node;
}

var generated$1 = {};

Object.defineProperty(generated$1, "__esModule", {
    value: true
});
generated$1.TSBASETYPE_TYPES = generated$1.TSTYPE_TYPES = generated$1.TSTYPEELEMENT_TYPES = generated$1.JSX_TYPES = generated$1.ENUMMEMBER_TYPES = generated$1.ENUMBODY_TYPES = generated$1.FLOWPREDICATE_TYPES = generated$1.FLOWDECLARATION_TYPES = generated$1.FLOWBASEANNOTATION_TYPES = generated$1.FLOWTYPE_TYPES = generated$1.FLOW_TYPES = generated$1.PRIVATE_TYPES = generated$1.MODULESPECIFIER_TYPES = generated$1.EXPORTDECLARATION_TYPES = generated$1.MODULEDECLARATION_TYPES = generated$1.CLASS_TYPES = generated$1.PATTERN_TYPES = generated$1.UNARYLIKE_TYPES = generated$1.PROPERTY_TYPES = generated$1.OBJECTMEMBER_TYPES = generated$1.METHOD_TYPES = generated$1.USERWHITESPACABLE_TYPES = generated$1.IMMUTABLE_TYPES = generated$1.LITERAL_TYPES = generated$1.TSENTITYNAME_TYPES = generated$1.LVAL_TYPES = generated$1.PATTERNLIKE_TYPES = generated$1.DECLARATION_TYPES = generated$1.PUREISH_TYPES = generated$1.FUNCTIONPARENT_TYPES = generated$1.FUNCTION_TYPES = generated$1.FORXSTATEMENT_TYPES = generated$1.FOR_TYPES = generated$1.EXPRESSIONWRAPPER_TYPES = generated$1.WHILE_TYPES = generated$1.LOOP_TYPES = generated$1.CONDITIONAL_TYPES = generated$1.COMPLETIONSTATEMENT_TYPES = generated$1.TERMINATORLESS_TYPES = generated$1.STATEMENT_TYPES = generated$1.BLOCK_TYPES = generated$1.BLOCKPARENT_TYPES = generated$1.SCOPABLE_TYPES = generated$1.BINARY_TYPES = generated$1.EXPRESSION_TYPES = void 0;
var _definitions$3 = requireDefinitions();
var EXPRESSION_TYPES = _definitions$3.FLIPPED_ALIAS_KEYS["Expression"];
generated$1.EXPRESSION_TYPES = EXPRESSION_TYPES;
var BINARY_TYPES = _definitions$3.FLIPPED_ALIAS_KEYS["Binary"];
generated$1.BINARY_TYPES = BINARY_TYPES;
var SCOPABLE_TYPES = _definitions$3.FLIPPED_ALIAS_KEYS["Scopable"];
generated$1.SCOPABLE_TYPES = SCOPABLE_TYPES;
var BLOCKPARENT_TYPES = _definitions$3.FLIPPED_ALIAS_KEYS["BlockParent"];
generated$1.BLOCKPARENT_TYPES = BLOCKPARENT_TYPES;
var BLOCK_TYPES = _definitions$3.FLIPPED_ALIAS_KEYS["Block"];
generated$1.BLOCK_TYPES = BLOCK_TYPES;
var STATEMENT_TYPES = _definitions$3.FLIPPED_ALIAS_KEYS["Statement"];
generated$1.STATEMENT_TYPES = STATEMENT_TYPES;
var TERMINATORLESS_TYPES = _definitions$3.FLIPPED_ALIAS_KEYS["Terminatorless"];
generated$1.TERMINATORLESS_TYPES = TERMINATORLESS_TYPES;
var COMPLETIONSTATEMENT_TYPES = _definitions$3.FLIPPED_ALIAS_KEYS["CompletionStatement"];
generated$1.COMPLETIONSTATEMENT_TYPES = COMPLETIONSTATEMENT_TYPES;
var CONDITIONAL_TYPES = _definitions$3.FLIPPED_ALIAS_KEYS["Conditional"];
generated$1.CONDITIONAL_TYPES = CONDITIONAL_TYPES;
var LOOP_TYPES = _definitions$3.FLIPPED_ALIAS_KEYS["Loop"];
generated$1.LOOP_TYPES = LOOP_TYPES;
var WHILE_TYPES = _definitions$3.FLIPPED_ALIAS_KEYS["While"];
generated$1.WHILE_TYPES = WHILE_TYPES;
var EXPRESSIONWRAPPER_TYPES = _definitions$3.FLIPPED_ALIAS_KEYS["ExpressionWrapper"];
generated$1.EXPRESSIONWRAPPER_TYPES = EXPRESSIONWRAPPER_TYPES;
var FOR_TYPES = _definitions$3.FLIPPED_ALIAS_KEYS["For"];
generated$1.FOR_TYPES = FOR_TYPES;
var FORXSTATEMENT_TYPES = _definitions$3.FLIPPED_ALIAS_KEYS["ForXStatement"];
generated$1.FORXSTATEMENT_TYPES = FORXSTATEMENT_TYPES;
var FUNCTION_TYPES = _definitions$3.FLIPPED_ALIAS_KEYS["Function"];
generated$1.FUNCTION_TYPES = FUNCTION_TYPES;
var FUNCTIONPARENT_TYPES = _definitions$3.FLIPPED_ALIAS_KEYS["FunctionParent"];
generated$1.FUNCTIONPARENT_TYPES = FUNCTIONPARENT_TYPES;
var PUREISH_TYPES = _definitions$3.FLIPPED_ALIAS_KEYS["Pureish"];
generated$1.PUREISH_TYPES = PUREISH_TYPES;
var DECLARATION_TYPES = _definitions$3.FLIPPED_ALIAS_KEYS["Declaration"];
generated$1.DECLARATION_TYPES = DECLARATION_TYPES;
var PATTERNLIKE_TYPES = _definitions$3.FLIPPED_ALIAS_KEYS["PatternLike"];
generated$1.PATTERNLIKE_TYPES = PATTERNLIKE_TYPES;
var LVAL_TYPES = _definitions$3.FLIPPED_ALIAS_KEYS["LVal"];
generated$1.LVAL_TYPES = LVAL_TYPES;
var TSENTITYNAME_TYPES = _definitions$3.FLIPPED_ALIAS_KEYS["TSEntityName"];
generated$1.TSENTITYNAME_TYPES = TSENTITYNAME_TYPES;
var LITERAL_TYPES = _definitions$3.FLIPPED_ALIAS_KEYS["Literal"];
generated$1.LITERAL_TYPES = LITERAL_TYPES;
var IMMUTABLE_TYPES = _definitions$3.FLIPPED_ALIAS_KEYS["Immutable"];
generated$1.IMMUTABLE_TYPES = IMMUTABLE_TYPES;
var USERWHITESPACABLE_TYPES = _definitions$3.FLIPPED_ALIAS_KEYS["UserWhitespacable"];
generated$1.USERWHITESPACABLE_TYPES = USERWHITESPACABLE_TYPES;
var METHOD_TYPES = _definitions$3.FLIPPED_ALIAS_KEYS["Method"];
generated$1.METHOD_TYPES = METHOD_TYPES;
var OBJECTMEMBER_TYPES = _definitions$3.FLIPPED_ALIAS_KEYS["ObjectMember"];
generated$1.OBJECTMEMBER_TYPES = OBJECTMEMBER_TYPES;
var PROPERTY_TYPES = _definitions$3.FLIPPED_ALIAS_KEYS["Property"];
generated$1.PROPERTY_TYPES = PROPERTY_TYPES;
var UNARYLIKE_TYPES = _definitions$3.FLIPPED_ALIAS_KEYS["UnaryLike"];
generated$1.UNARYLIKE_TYPES = UNARYLIKE_TYPES;
var PATTERN_TYPES = _definitions$3.FLIPPED_ALIAS_KEYS["Pattern"];
generated$1.PATTERN_TYPES = PATTERN_TYPES;
var CLASS_TYPES = _definitions$3.FLIPPED_ALIAS_KEYS["Class"];
generated$1.CLASS_TYPES = CLASS_TYPES;
var MODULEDECLARATION_TYPES = _definitions$3.FLIPPED_ALIAS_KEYS["ModuleDeclaration"];
generated$1.MODULEDECLARATION_TYPES = MODULEDECLARATION_TYPES;
var EXPORTDECLARATION_TYPES = _definitions$3.FLIPPED_ALIAS_KEYS["ExportDeclaration"];
generated$1.EXPORTDECLARATION_TYPES = EXPORTDECLARATION_TYPES;
var MODULESPECIFIER_TYPES = _definitions$3.FLIPPED_ALIAS_KEYS["ModuleSpecifier"];
generated$1.MODULESPECIFIER_TYPES = MODULESPECIFIER_TYPES;
var PRIVATE_TYPES = _definitions$3.FLIPPED_ALIAS_KEYS["Private"];
generated$1.PRIVATE_TYPES = PRIVATE_TYPES;
var FLOW_TYPES = _definitions$3.FLIPPED_ALIAS_KEYS["Flow"];
generated$1.FLOW_TYPES = FLOW_TYPES;
var FLOWTYPE_TYPES = _definitions$3.FLIPPED_ALIAS_KEYS["FlowType"];
generated$1.FLOWTYPE_TYPES = FLOWTYPE_TYPES;
var FLOWBASEANNOTATION_TYPES = _definitions$3.FLIPPED_ALIAS_KEYS["FlowBaseAnnotation"];
generated$1.FLOWBASEANNOTATION_TYPES = FLOWBASEANNOTATION_TYPES;
var FLOWDECLARATION_TYPES = _definitions$3.FLIPPED_ALIAS_KEYS["FlowDeclaration"];
generated$1.FLOWDECLARATION_TYPES = FLOWDECLARATION_TYPES;
var FLOWPREDICATE_TYPES = _definitions$3.FLIPPED_ALIAS_KEYS["FlowPredicate"];
generated$1.FLOWPREDICATE_TYPES = FLOWPREDICATE_TYPES;
var ENUMBODY_TYPES = _definitions$3.FLIPPED_ALIAS_KEYS["EnumBody"];
generated$1.ENUMBODY_TYPES = ENUMBODY_TYPES;
var ENUMMEMBER_TYPES = _definitions$3.FLIPPED_ALIAS_KEYS["EnumMember"];
generated$1.ENUMMEMBER_TYPES = ENUMMEMBER_TYPES;
var JSX_TYPES = _definitions$3.FLIPPED_ALIAS_KEYS["JSX"];
generated$1.JSX_TYPES = JSX_TYPES;
var TSTYPEELEMENT_TYPES = _definitions$3.FLIPPED_ALIAS_KEYS["TSTypeElement"];
generated$1.TSTYPEELEMENT_TYPES = TSTYPEELEMENT_TYPES;
var TSTYPE_TYPES = _definitions$3.FLIPPED_ALIAS_KEYS["TSType"];
generated$1.TSTYPE_TYPES = TSTYPE_TYPES;
var TSBASETYPE_TYPES = _definitions$3.FLIPPED_ALIAS_KEYS["TSBaseType"];
generated$1.TSBASETYPE_TYPES = TSBASETYPE_TYPES;

var ensureBlock$1 = {};

var toBlock$1 = {};

Object.defineProperty(toBlock$1, "__esModule", {
    value: true
});
toBlock$1.default = toBlock;
var _generated$f = generated$4;
var _generated2$3 = generated$3;
function toBlock(node, parent) {
    if ((0, _generated$f.isBlockStatement)(node)) {
        return node;
    }
    var blockNodes = [];
    if ((0, _generated$f.isEmptyStatement)(node)) {
        blockNodes = [];
    } else {
        if (!(0, _generated$f.isStatement)(node)) {
            if ((0, _generated$f.isFunction)(parent)) {
                node = (0, _generated2$3.returnStatement)(node);
            } else {
                node = (0, _generated2$3.expressionStatement)(node);
            }
        }
        blockNodes = [
            node
        ];
    }
    return (0, _generated2$3.blockStatement)(blockNodes);
}

Object.defineProperty(ensureBlock$1, "__esModule", {
    value: true
});
ensureBlock$1.default = ensureBlock;
var _toBlock = toBlock$1;
function ensureBlock(node, key) {
    if (key === void 0) key = "body";
    return node[key] = (0, _toBlock.default)(node[key], node);
}

var toBindingIdentifierName$1 = {};

var toIdentifier$1 = {};

function _createForOfIteratorHelperLoose$9(o, allowArrayLike) {
    var it = typeof Symbol !== "undefined" && o[Symbol.iterator] || o["@@iterator"];
    if (it) return (it = it.call(o)).next.bind(it);
    if (Array.isArray(o) || (it = unsupportedIterableToArray(o)) || allowArrayLike && o && typeof o.length === "number") {
        if (it) o = it;
        var i = 0;
        return function() {
            if (i >= o.length) return {
                done: true
            };
            return {
                done: false,
                value: o[i++]
            };
        };
    }
    throw new TypeError("Invalid attempt to iterate non-iterable instance.\\nIn order to be iterable, non-array objects must have a [Symbol.iterator]() method.");
}
Object.defineProperty(toIdentifier$1, "__esModule", {
    value: true
});
toIdentifier$1.default = toIdentifier;
var _isValidIdentifier$2 = isValidIdentifier$1;
var _helperValidatorIdentifier = lib;
function toIdentifier(input) {
    input = input + "";
    var name = "";
    for(var _iterator = _createForOfIteratorHelperLoose$9(input), _step; !(_step = _iterator()).done;){
        var c = _step.value;
        name += (0, _helperValidatorIdentifier.isIdentifierChar)(c.codePointAt(0)) ? c : "-";
    }
    name = name.replace(/^[-0-9]+/, "");
    name = name.replace(/[-\s]+(.)?/g, function(match, c) {
        return c ? c.toUpperCase() : "";
    });
    if (!(0, _isValidIdentifier$2.default)(name)) {
        name = "_" + name;
    }
    return name || "_";
}

Object.defineProperty(toBindingIdentifierName$1, "__esModule", {
    value: true
});
toBindingIdentifierName$1.default = toBindingIdentifierName;
var _toIdentifier = toIdentifier$1;
function toBindingIdentifierName(name) {
    name = (0, _toIdentifier.default)(name);
    if (name === "eval" || name === "arguments") name = "_" + name;
    return name;
}

var toComputedKey$1 = {};

Object.defineProperty(toComputedKey$1, "__esModule", {
    value: true
});
toComputedKey$1.default = toComputedKey;
var _generated$e = generated$4;
var _generated2$2 = generated$3;
function toComputedKey(node, key) {
    if (key === void 0) key = node.key || node.property;
    if (!node.computed && (0, _generated$e.isIdentifier)(key)) key = (0, _generated2$2.stringLiteral)(key.name);
    return key;
}

var toExpression$1 = {};

Object.defineProperty(toExpression$1, "__esModule", {
    value: true
});
toExpression$1.default = void 0;
var _generated$d = generated$4;
var _default$3 = toExpression;
toExpression$1.default = _default$3;
function toExpression(node) {
    if ((0, _generated$d.isExpressionStatement)(node)) {
        node = node.expression;
    }
    if ((0, _generated$d.isExpression)(node)) {
        return node;
    }
    if ((0, _generated$d.isClass)(node)) {
        node.type = "ClassExpression";
    } else if ((0, _generated$d.isFunction)(node)) {
        node.type = "FunctionExpression";
    }
    if (!(0, _generated$d.isExpression)(node)) {
        throw new Error("cannot turn " + node.type + " to an expression");
    }
    return node;
}

var toKeyAlias$1 = {};

var removePropertiesDeep$1 = {};

var traverseFast$1 = {};

function _createForOfIteratorHelperLoose$8(o, allowArrayLike) {
    var it = typeof Symbol !== "undefined" && o[Symbol.iterator] || o["@@iterator"];
    if (it) return (it = it.call(o)).next.bind(it);
    if (Array.isArray(o) || (it = unsupportedIterableToArray(o)) || allowArrayLike && o && typeof o.length === "number") {
        if (it) o = it;
        var i = 0;
        return function() {
            if (i >= o.length) return {
                done: true
            };
            return {
                done: false,
                value: o[i++]
            };
        };
    }
    throw new TypeError("Invalid attempt to iterate non-iterable instance.\\nIn order to be iterable, non-array objects must have a [Symbol.iterator]() method.");
}
Object.defineProperty(traverseFast$1, "__esModule", {
    value: true
});
traverseFast$1.default = traverseFast;
var _definitions$2 = requireDefinitions();
function traverseFast(node, enter, opts) {
    if (!node) return;
    var keys = _definitions$2.VISITOR_KEYS[node.type];
    if (!keys) return;
    opts = opts || {};
    enter(node, opts);
    for(var _iterator = _createForOfIteratorHelperLoose$8(keys), _step; !(_step = _iterator()).done;){
        var key = _step.value;
        var subNode = node[key];
        if (Array.isArray(subNode)) {
            for(var _iterator1 = _createForOfIteratorHelperLoose$8(subNode), _step1; !(_step1 = _iterator1()).done;){
                var _$node = _step1.value;
                traverseFast(_$node, enter, opts);
            }
        } else {
            traverseFast(subNode, enter, opts);
        }
    }
}

var removeProperties$1 = {};

function _createForOfIteratorHelperLoose$7(o, allowArrayLike) {
    var it = typeof Symbol !== "undefined" && o[Symbol.iterator] || o["@@iterator"];
    if (it) return (it = it.call(o)).next.bind(it);
    if (Array.isArray(o) || (it = unsupportedIterableToArray(o)) || allowArrayLike && o && typeof o.length === "number") {
        if (it) o = it;
        var i = 0;
        return function() {
            if (i >= o.length) return {
                done: true
            };
            return {
                done: false,
                value: o[i++]
            };
        };
    }
    throw new TypeError("Invalid attempt to iterate non-iterable instance.\\nIn order to be iterable, non-array objects must have a [Symbol.iterator]() method.");
}
Object.defineProperty(removeProperties$1, "__esModule", {
    value: true
});
removeProperties$1.default = removeProperties;
var _constants$3 = constants;
var CLEAR_KEYS = [
    "tokens",
    "start",
    "end",
    "loc",
    "raw",
    "rawValue"
];
var CLEAR_KEYS_PLUS_COMMENTS = _constants$3.COMMENT_KEYS.concat([
    "comments"
]).concat(CLEAR_KEYS);
function removeProperties(node, opts) {
    if (opts === void 0) opts = {};
    var map = opts.preserveComments ? CLEAR_KEYS : CLEAR_KEYS_PLUS_COMMENTS;
    for(var _iterator = _createForOfIteratorHelperLoose$7(map), _step; !(_step = _iterator()).done;){
        var key = _step.value;
        if (node[key] != null) node[key] = undefined;
    }
    for(var _iterator1 = _createForOfIteratorHelperLoose$7(Object.keys(node)), _step1; !(_step1 = _iterator1()).done;){
        var key1 = _step1.value;
        if (key1[0] === "_" && node[key1] != null) node[key1] = undefined;
    }
    var symbols = Object.getOwnPropertySymbols(node);
    for(var _iterator2 = _createForOfIteratorHelperLoose$7(symbols), _step2; !(_step2 = _iterator2()).done;){
        var sym = _step2.value;
        node[sym] = null;
    }
}

Object.defineProperty(removePropertiesDeep$1, "__esModule", {
    value: true
});
removePropertiesDeep$1.default = removePropertiesDeep;
var _traverseFast = traverseFast$1;
var _removeProperties = removeProperties$1;
function removePropertiesDeep(tree, opts) {
    (0, _traverseFast.default)(tree, _removeProperties.default, opts);
    return tree;
}

Object.defineProperty(toKeyAlias$1, "__esModule", {
    value: true
});
toKeyAlias$1.default = toKeyAlias;
var _generated$c = generated$4;
var _cloneNode$1 = cloneNode$1;
var _removePropertiesDeep = removePropertiesDeep$1;
function toKeyAlias(node, key) {
    if (key === void 0) key = node.key;
    var alias;
    if (node.kind === "method") {
        return toKeyAlias.increment() + "";
    } else if ((0, _generated$c.isIdentifier)(key)) {
        alias = key.name;
    } else if ((0, _generated$c.isStringLiteral)(key)) {
        alias = JSON.stringify(key.value);
    } else {
        alias = JSON.stringify((0, _removePropertiesDeep.default)((0, _cloneNode$1.default)(key)));
    }
    if (node.computed) {
        alias = "[" + alias + "]";
    }
    if (node.static) {
        alias = "static:" + alias;
    }
    return alias;
}
toKeyAlias.uid = 0;
toKeyAlias.increment = function() {
    if (toKeyAlias.uid >= Number.MAX_SAFE_INTEGER) {
        return toKeyAlias.uid = 0;
    } else {
        return toKeyAlias.uid++;
    }
};

var toSequenceExpression$1 = {};

var gatherSequenceExpressions$1 = {};

var getBindingIdentifiers$1 = {};

Object.defineProperty(getBindingIdentifiers$1, "__esModule", {
    value: true
});
getBindingIdentifiers$1.default = getBindingIdentifiers;
var _generated$b = generated$4;
function getBindingIdentifiers(node, duplicates, outerOnly) {
    var search = [].concat(node);
    var ids = Object.create(null);
    while(search.length){
        var id = search.shift();
        if (!id) continue;
        var keys = getBindingIdentifiers.keys[id.type];
        if ((0, _generated$b.isIdentifier)(id)) {
            if (duplicates) {
                var _ids = ids[id.name] = ids[id.name] || [];
                _ids.push(id);
            } else {
                ids[id.name] = id;
            }
            continue;
        }
        if ((0, _generated$b.isExportDeclaration)(id) && !(0, _generated$b.isExportAllDeclaration)(id)) {
            if ((0, _generated$b.isDeclaration)(id.declaration)) {
                search.push(id.declaration);
            }
            continue;
        }
        if (outerOnly) {
            if ((0, _generated$b.isFunctionDeclaration)(id)) {
                search.push(id.id);
                continue;
            }
            if ((0, _generated$b.isFunctionExpression)(id)) {
                continue;
            }
        }
        if (keys) {
            for(var i = 0; i < keys.length; i++){
                var key = keys[i];
                if (id[key]) {
                    search = search.concat(id[key]);
                }
            }
        }
    }
    return ids;
}
getBindingIdentifiers.keys = {
    DeclareClass: [
        "id"
    ],
    DeclareFunction: [
        "id"
    ],
    DeclareModule: [
        "id"
    ],
    DeclareVariable: [
        "id"
    ],
    DeclareInterface: [
        "id"
    ],
    DeclareTypeAlias: [
        "id"
    ],
    DeclareOpaqueType: [
        "id"
    ],
    InterfaceDeclaration: [
        "id"
    ],
    TypeAlias: [
        "id"
    ],
    OpaqueType: [
        "id"
    ],
    CatchClause: [
        "param"
    ],
    LabeledStatement: [
        "label"
    ],
    UnaryExpression: [
        "argument"
    ],
    AssignmentExpression: [
        "left"
    ],
    ImportSpecifier: [
        "local"
    ],
    ImportNamespaceSpecifier: [
        "local"
    ],
    ImportDefaultSpecifier: [
        "local"
    ],
    ImportDeclaration: [
        "specifiers"
    ],
    ExportSpecifier: [
        "exported"
    ],
    ExportNamespaceSpecifier: [
        "exported"
    ],
    ExportDefaultSpecifier: [
        "exported"
    ],
    FunctionDeclaration: [
        "id",
        "params"
    ],
    FunctionExpression: [
        "id",
        "params"
    ],
    ArrowFunctionExpression: [
        "params"
    ],
    ObjectMethod: [
        "params"
    ],
    ClassMethod: [
        "params"
    ],
    ForInStatement: [
        "left"
    ],
    ForOfStatement: [
        "left"
    ],
    ClassDeclaration: [
        "id"
    ],
    ClassExpression: [
        "id"
    ],
    RestElement: [
        "argument"
    ],
    UpdateExpression: [
        "argument"
    ],
    ObjectProperty: [
        "value"
    ],
    AssignmentPattern: [
        "left"
    ],
    ArrayPattern: [
        "elements"
    ],
    ObjectPattern: [
        "properties"
    ],
    VariableDeclaration: [
        "declarations"
    ],
    VariableDeclarator: [
        "id"
    ]
};

function _createForOfIteratorHelperLoose$6(o, allowArrayLike) {
    var it = typeof Symbol !== "undefined" && o[Symbol.iterator] || o["@@iterator"];
    if (it) return (it = it.call(o)).next.bind(it);
    if (Array.isArray(o) || (it = unsupportedIterableToArray(o)) || allowArrayLike && o && typeof o.length === "number") {
        if (it) o = it;
        var i = 0;
        return function() {
            if (i >= o.length) return {
                done: true
            };
            return {
                done: false,
                value: o[i++]
            };
        };
    }
    throw new TypeError("Invalid attempt to iterate non-iterable instance.\\nIn order to be iterable, non-array objects must have a [Symbol.iterator]() method.");
}
Object.defineProperty(gatherSequenceExpressions$1, "__esModule", {
    value: true
});
gatherSequenceExpressions$1.default = gatherSequenceExpressions;
var _getBindingIdentifiers$2 = getBindingIdentifiers$1;
var _generated$a = generated$4;
var _generated2$1 = generated$3;
var _cloneNode = cloneNode$1;
function gatherSequenceExpressions(nodes, scope, declars) {
    var exprs = [];
    var ensureLastUndefined = true;
    for(var _iterator = _createForOfIteratorHelperLoose$6(nodes), _step; !(_step = _iterator()).done;){
        var node = _step.value;
        if (!(0, _generated$a.isEmptyStatement)(node)) {
            ensureLastUndefined = false;
        }
        if ((0, _generated$a.isExpression)(node)) {
            exprs.push(node);
        } else if ((0, _generated$a.isExpressionStatement)(node)) {
            exprs.push(node.expression);
        } else if ((0, _generated$a.isVariableDeclaration)(node)) {
            if (node.kind !== "var") return;
            for(var _iterator1 = _createForOfIteratorHelperLoose$6(node.declarations), _step1; !(_step1 = _iterator1()).done;){
                var declar = _step1.value;
                var bindings = (0, _getBindingIdentifiers$2.default)(declar);
                for(var _iterator2 = _createForOfIteratorHelperLoose$6(Object.keys(bindings)), _step2; !(_step2 = _iterator2()).done;){
                    var key = _step2.value;
                    declars.push({
                        kind: node.kind,
                        id: (0, _cloneNode.default)(bindings[key])
                    });
                }
                if (declar.init) {
                    exprs.push((0, _generated2$1.assignmentExpression)("=", declar.id, declar.init));
                }
            }
            ensureLastUndefined = true;
        } else if ((0, _generated$a.isIfStatement)(node)) {
            var consequent = node.consequent ? gatherSequenceExpressions([
                node.consequent
            ], scope, declars) : scope.buildUndefinedNode();
            var alternate = node.alternate ? gatherSequenceExpressions([
                node.alternate
            ], scope, declars) : scope.buildUndefinedNode();
            if (!consequent || !alternate) return;
            exprs.push((0, _generated2$1.conditionalExpression)(node.test, consequent, alternate));
        } else if ((0, _generated$a.isBlockStatement)(node)) {
            var body = gatherSequenceExpressions(node.body, scope, declars);
            if (!body) return;
            exprs.push(body);
        } else if ((0, _generated$a.isEmptyStatement)(node)) {
            if (nodes.indexOf(node) === 0) {
                ensureLastUndefined = true;
            }
        } else {
            return;
        }
    }
    if (ensureLastUndefined) {
        exprs.push(scope.buildUndefinedNode());
    }
    if (exprs.length === 1) {
        return exprs[0];
    } else {
        return (0, _generated2$1.sequenceExpression)(exprs);
    }
}

function _createForOfIteratorHelperLoose$5(o, allowArrayLike) {
    var it = typeof Symbol !== "undefined" && o[Symbol.iterator] || o["@@iterator"];
    if (it) return (it = it.call(o)).next.bind(it);
    if (Array.isArray(o) || (it = unsupportedIterableToArray(o)) || allowArrayLike && o && typeof o.length === "number") {
        if (it) o = it;
        var i = 0;
        return function() {
            if (i >= o.length) return {
                done: true
            };
            return {
                done: false,
                value: o[i++]
            };
        };
    }
    throw new TypeError("Invalid attempt to iterate non-iterable instance.\\nIn order to be iterable, non-array objects must have a [Symbol.iterator]() method.");
}
Object.defineProperty(toSequenceExpression$1, "__esModule", {
    value: true
});
toSequenceExpression$1.default = toSequenceExpression;
var _gatherSequenceExpressions = gatherSequenceExpressions$1;
function toSequenceExpression(nodes, scope) {
    if (!(nodes != null && nodes.length)) return;
    var declars = [];
    var result = (0, _gatherSequenceExpressions.default)(nodes, scope, declars);
    if (!result) return;
    for(var _iterator = _createForOfIteratorHelperLoose$5(declars), _step; !(_step = _iterator()).done;){
        var declar = _step.value;
        scope.push(declar);
    }
    return result;
}

var toStatement$1 = {};

Object.defineProperty(toStatement$1, "__esModule", {
    value: true
});
toStatement$1.default = void 0;
var _generated$9 = generated$4;
var _generated2 = generated$3;
var _default$2 = toStatement;
toStatement$1.default = _default$2;
function toStatement(node, ignore) {
    if ((0, _generated$9.isStatement)(node)) {
        return node;
    }
    var mustHaveId = false;
    var newType;
    if ((0, _generated$9.isClass)(node)) {
        mustHaveId = true;
        newType = "ClassDeclaration";
    } else if ((0, _generated$9.isFunction)(node)) {
        mustHaveId = true;
        newType = "FunctionDeclaration";
    } else if ((0, _generated$9.isAssignmentExpression)(node)) {
        return (0, _generated2.expressionStatement)(node);
    }
    if (mustHaveId && !node.id) {
        newType = false;
    }
    if (!newType) {
        if (ignore) {
            return false;
        } else {
            throw new Error("cannot turn " + node.type + " to a statement");
        }
    }
    node.type = newType;
    return node;
}

var valueToNode$1 = {};

function _createForOfIteratorHelperLoose$4(o, allowArrayLike) {
    var it = typeof Symbol !== "undefined" && o[Symbol.iterator] || o["@@iterator"];
    if (it) return (it = it.call(o)).next.bind(it);
    if (Array.isArray(o) || (it = unsupportedIterableToArray(o)) || allowArrayLike && o && typeof o.length === "number") {
        if (it) o = it;
        var i = 0;
        return function() {
            if (i >= o.length) return {
                done: true
            };
            return {
                done: false,
                value: o[i++]
            };
        };
    }
    throw new TypeError("Invalid attempt to iterate non-iterable instance.\\nIn order to be iterable, non-array objects must have a [Symbol.iterator]() method.");
}
Object.defineProperty(valueToNode$1, "__esModule", {
    value: true
});
valueToNode$1.default = void 0;
var _isValidIdentifier$1 = isValidIdentifier$1;
var _generated$8 = generated$3;
var _default$1 = valueToNode;
valueToNode$1.default = _default$1;
var objectToString = Function.call.bind(Object.prototype.toString);
function isRegExp(value) {
    return objectToString(value) === "[object RegExp]";
}
function isPlainObject(value) {
    if (typeof value !== "object" || value === null || Object.prototype.toString.call(value) !== "[object Object]") {
        return false;
    }
    var proto = Object.getPrototypeOf(value);
    return proto === null || Object.getPrototypeOf(proto) === null;
}
function valueToNode(value) {
    if (value === undefined) {
        return (0, _generated$8.identifier)("undefined");
    }
    if (value === true || value === false) {
        return (0, _generated$8.booleanLiteral)(value);
    }
    if (value === null) {
        return (0, _generated$8.nullLiteral)();
    }
    if (typeof value === "string") {
        return (0, _generated$8.stringLiteral)(value);
    }
    if (typeof value === "number") {
        var result;
        if (Number.isFinite(value)) {
            result = (0, _generated$8.numericLiteral)(Math.abs(value));
        } else {
            var numerator;
            if (Number.isNaN(value)) {
                numerator = (0, _generated$8.numericLiteral)(0);
            } else {
                numerator = (0, _generated$8.numericLiteral)(1);
            }
            result = (0, _generated$8.binaryExpression)("/", numerator, (0, _generated$8.numericLiteral)(0));
        }
        if (value < 0 || Object.is(value, -0)) {
            result = (0, _generated$8.unaryExpression)("-", result);
        }
        return result;
    }
    if (isRegExp(value)) {
        var pattern = value.source;
        var flags = value.toString().match(/\/([a-z]+|)$/)[1];
        return (0, _generated$8.regExpLiteral)(pattern, flags);
    }
    if (Array.isArray(value)) {
        return (0, _generated$8.arrayExpression)(value.map(valueToNode));
    }
    if (isPlainObject(value)) {
        var props = [];
        for(var _iterator = _createForOfIteratorHelperLoose$4(Object.keys(value)), _step; !(_step = _iterator()).done;){
            var key = _step.value;
            var nodeKey = void 0;
            if ((0, _isValidIdentifier$1.default)(key)) {
                nodeKey = (0, _generated$8.identifier)(key);
            } else {
                nodeKey = (0, _generated$8.stringLiteral)(key);
            }
            props.push((0, _generated$8.objectProperty)(nodeKey, valueToNode(value[key])));
        }
        return (0, _generated$8.objectExpression)(props);
    }
    throw new Error("don't know how to turn this value into a node");
}

var appendToMemberExpression$1 = {};

Object.defineProperty(appendToMemberExpression$1, "__esModule", {
    value: true
});
appendToMemberExpression$1.default = appendToMemberExpression;
var _generated$7 = generated$3;
function appendToMemberExpression(member, append, computed) {
    if (computed === void 0) computed = false;
    member.object = (0, _generated$7.memberExpression)(member.object, member.property, member.computed);
    member.property = append;
    member.computed = !!computed;
    return member;
}

var inherits$1 = {};

function _createForOfIteratorHelperLoose$3(o, allowArrayLike) {
    var it = typeof Symbol !== "undefined" && o[Symbol.iterator] || o["@@iterator"];
    if (it) return (it = it.call(o)).next.bind(it);
    if (Array.isArray(o) || (it = unsupportedIterableToArray(o)) || allowArrayLike && o && typeof o.length === "number") {
        if (it) o = it;
        var i = 0;
        return function() {
            if (i >= o.length) return {
                done: true
            };
            return {
                done: false,
                value: o[i++]
            };
        };
    }
    throw new TypeError("Invalid attempt to iterate non-iterable instance.\\nIn order to be iterable, non-array objects must have a [Symbol.iterator]() method.");
}
Object.defineProperty(inherits$1, "__esModule", {
    value: true
});
inherits$1.default = inherits;
var _constants$2 = constants;
var _inheritsComments = inheritsComments$1;
function inherits(child, parent) {
    if (!child || !parent) return child;
    for(var _iterator = _createForOfIteratorHelperLoose$3(_constants$2.INHERIT_KEYS.optional), _step; !(_step = _iterator()).done;){
        var key = _step.value;
        if (child[key] == null) {
            child[key] = parent[key];
        }
    }
    for(var _iterator1 = _createForOfIteratorHelperLoose$3(Object.keys(parent)), _step1; !(_step1 = _iterator1()).done;){
        var key1 = _step1.value;
        if (key1[0] === "_" && key1 !== "__clone") child[key1] = parent[key1];
    }
    for(var _iterator2 = _createForOfIteratorHelperLoose$3(_constants$2.INHERIT_KEYS.force), _step2; !(_step2 = _iterator2()).done;){
        var key2 = _step2.value;
        child[key2] = parent[key2];
    }
    (0, _inheritsComments.default)(child, parent);
    return child;
}

var prependToMemberExpression$1 = {};

Object.defineProperty(prependToMemberExpression$1, "__esModule", {
    value: true
});
prependToMemberExpression$1.default = prependToMemberExpression;
var _generated$6 = generated$3;
function prependToMemberExpression(member, prepend) {
    member.object = (0, _generated$6.memberExpression)(prepend, member.object);
    return member;
}

var getOuterBindingIdentifiers$1 = {};

Object.defineProperty(getOuterBindingIdentifiers$1, "__esModule", {
    value: true
});
getOuterBindingIdentifiers$1.default = void 0;
var _getBindingIdentifiers$1 = getBindingIdentifiers$1;
var _default = getOuterBindingIdentifiers;
getOuterBindingIdentifiers$1.default = _default;
function getOuterBindingIdentifiers(node, duplicates) {
    return (0, _getBindingIdentifiers$1.default)(node, duplicates, true);
}

var traverse$1 = {};

function _createForOfIteratorHelperLoose$2(o, allowArrayLike) {
    var it = typeof Symbol !== "undefined" && o[Symbol.iterator] || o["@@iterator"];
    if (it) return (it = it.call(o)).next.bind(it);
    if (Array.isArray(o) || (it = unsupportedIterableToArray(o)) || allowArrayLike && o && typeof o.length === "number") {
        if (it) o = it;
        var i = 0;
        return function() {
            if (i >= o.length) return {
                done: true
            };
            return {
                done: false,
                value: o[i++]
            };
        };
    }
    throw new TypeError("Invalid attempt to iterate non-iterable instance.\\nIn order to be iterable, non-array objects must have a [Symbol.iterator]() method.");
}
Object.defineProperty(traverse$1, "__esModule", {
    value: true
});
traverse$1.default = traverse;
var _definitions$1 = requireDefinitions();
function traverse(node, handlers, state) {
    if (typeof handlers === "function") {
        handlers = {
            enter: handlers
        };
    }
    var enter = handlers.enter, exit = handlers.exit;
    traverseSimpleImpl(node, enter, exit, state, []);
}
function traverseSimpleImpl(node, enter, exit, state, ancestors) {
    var keys = _definitions$1.VISITOR_KEYS[node.type];
    if (!keys) return;
    if (enter) enter(node, ancestors, state);
    for(var _iterator = _createForOfIteratorHelperLoose$2(keys), _step; !(_step = _iterator()).done;){
        var key = _step.value;
        var subNode = node[key];
        if (Array.isArray(subNode)) {
            for(var i = 0; i < subNode.length; i++){
                var child = subNode[i];
                if (!child) continue;
                ancestors.push({
                    node: node,
                    key: key,
                    index: i
                });
                traverseSimpleImpl(child, enter, exit, state, ancestors);
                ancestors.pop();
            }
        } else if (subNode) {
            ancestors.push({
                node: node,
                key: key
            });
            traverseSimpleImpl(subNode, enter, exit, state, ancestors);
            ancestors.pop();
        }
    }
    if (exit) exit(node, ancestors, state);
}

var isBinding$1 = {};

Object.defineProperty(isBinding$1, "__esModule", {
    value: true
});
isBinding$1.default = isBinding;
var _getBindingIdentifiers = getBindingIdentifiers$1;
function isBinding(node, parent, grandparent) {
    if (grandparent && node.type === "Identifier" && parent.type === "ObjectProperty" && grandparent.type === "ObjectExpression") {
        return false;
    }
    var keys = _getBindingIdentifiers.default.keys[parent.type];
    if (keys) {
        for(var i = 0; i < keys.length; i++){
            var key = keys[i];
            var val = parent[key];
            if (Array.isArray(val)) {
                if (val.indexOf(node) >= 0) return true;
            } else {
                if (val === node) return true;
            }
        }
    }
    return false;
}

var isBlockScoped$1 = {};

var isLet$1 = {};

Object.defineProperty(isLet$1, "__esModule", {
    value: true
});
isLet$1.default = isLet;
var _generated$5 = generated$4;
var _constants$1 = constants;
function isLet(node) {
    return (0, _generated$5.isVariableDeclaration)(node) && (node.kind !== "var" || node[_constants$1.BLOCK_SCOPED_SYMBOL]);
}

Object.defineProperty(isBlockScoped$1, "__esModule", {
    value: true
});
isBlockScoped$1.default = isBlockScoped;
var _generated$4 = generated$4;
var _isLet = isLet$1;
function isBlockScoped(node) {
    return (0, _generated$4.isFunctionDeclaration)(node) || (0, _generated$4.isClassDeclaration)(node) || (0, _isLet.default)(node);
}

var isImmutable$1 = {};

Object.defineProperty(isImmutable$1, "__esModule", {
    value: true
});
isImmutable$1.default = isImmutable;
var _isType = requireIsType();
var _generated$3 = generated$4;
function isImmutable(node) {
    if ((0, _isType.default)(node.type, "Immutable")) return true;
    if ((0, _generated$3.isIdentifier)(node)) {
        if (node.name === "undefined") {
            return true;
        } else {
            return false;
        }
    }
    return false;
}

var isNodesEquivalent$1 = {};

var _typeof$2 = function(obj) {
    "@swc/helpers - typeof";
    return obj && typeof Symbol !== "undefined" && obj.constructor === Symbol ? "symbol" : typeof obj;
};
function _createForOfIteratorHelperLoose$1(o, allowArrayLike) {
    var it = typeof Symbol !== "undefined" && o[Symbol.iterator] || o["@@iterator"];
    if (it) return (it = it.call(o)).next.bind(it);
    if (Array.isArray(o) || (it = unsupportedIterableToArray(o)) || allowArrayLike && o && typeof o.length === "number") {
        if (it) o = it;
        var i = 0;
        return function() {
            if (i >= o.length) return {
                done: true
            };
            return {
                done: false,
                value: o[i++]
            };
        };
    }
    throw new TypeError("Invalid attempt to iterate non-iterable instance.\\nIn order to be iterable, non-array objects must have a [Symbol.iterator]() method.");
}
Object.defineProperty(isNodesEquivalent$1, "__esModule", {
    value: true
});
isNodesEquivalent$1.default = isNodesEquivalent;
var _definitions = requireDefinitions();
function isNodesEquivalent(a, b) {
    if (typeof a !== "object" || typeof b !== "object" || a == null || b == null) {
        return a === b;
    }
    if (a.type !== b.type) {
        return false;
    }
    var fields = Object.keys(_definitions.NODE_FIELDS[a.type] || a.type);
    var visitorKeys = _definitions.VISITOR_KEYS[a.type];
    for(var _iterator = _createForOfIteratorHelperLoose$1(fields), _step; !(_step = _iterator()).done;){
        var field = _step.value;
        if (_typeof$2(a[field]) !== _typeof$2(b[field])) {
            return false;
        }
        if (a[field] == null && b[field] == null) {
            continue;
        } else if (a[field] == null || b[field] == null) {
            return false;
        }
        if (Array.isArray(a[field])) {
            if (!Array.isArray(b[field])) {
                return false;
            }
            if (a[field].length !== b[field].length) {
                return false;
            }
            for(var i = 0; i < a[field].length; i++){
                if (!isNodesEquivalent(a[field][i], b[field][i])) {
                    return false;
                }
            }
            continue;
        }
        if (typeof a[field] === "object" && !(visitorKeys != null && visitorKeys.includes(field))) {
            for(var _iterator1 = _createForOfIteratorHelperLoose$1(Object.keys(a[field])), _step1; !(_step1 = _iterator1()).done;){
                var key = _step1.value;
                if (a[field][key] !== b[field][key]) {
                    return false;
                }
            }
            continue;
        }
        if (!isNodesEquivalent(a[field], b[field])) {
            return false;
        }
    }
    return true;
}

var isReferenced$1 = {};

Object.defineProperty(isReferenced$1, "__esModule", {
    value: true
});
isReferenced$1.default = isReferenced;
function isReferenced(node, parent, grandparent) {
    switch(parent.type){
        case "MemberExpression":
        case "JSXMemberExpression":
        case "OptionalMemberExpression":
            if (parent.property === node) {
                return !!parent.computed;
            }
            return parent.object === node;
        case "VariableDeclarator":
            return parent.init === node;
        case "ArrowFunctionExpression":
            return parent.body === node;
        case "PrivateName":
            return false;
        case "ClassMethod":
        case "ClassPrivateMethod":
        case "ObjectMethod":
            if (parent.params.includes(node)) {
                return false;
            }
        case "ObjectProperty":
        case "ClassProperty":
        case "ClassPrivateProperty":
            if (parent.key === node) {
                return !!parent.computed;
            }
            if (parent.value === node) {
                return !grandparent || grandparent.type !== "ObjectPattern";
            }
            return true;
        case "ClassDeclaration":
        case "ClassExpression":
            return parent.superClass === node;
        case "AssignmentExpression":
            return parent.right === node;
        case "AssignmentPattern":
            return parent.right === node;
        case "LabeledStatement":
            return false;
        case "CatchClause":
            return false;
        case "RestElement":
            return false;
        case "BreakStatement":
        case "ContinueStatement":
            return false;
        case "FunctionDeclaration":
        case "FunctionExpression":
            return false;
        case "ExportNamespaceSpecifier":
        case "ExportDefaultSpecifier":
            return false;
        case "ExportSpecifier":
            if (grandparent != null && grandparent.source) {
                return false;
            }
            return parent.local === node;
        case "ImportDefaultSpecifier":
        case "ImportNamespaceSpecifier":
        case "ImportSpecifier":
            return false;
        case "JSXAttribute":
            return false;
        case "ObjectPattern":
        case "ArrayPattern":
            return false;
        case "MetaProperty":
            return false;
        case "ObjectTypeProperty":
            return parent.key !== node;
        case "TSEnumMember":
            return parent.id !== node;
        case "TSPropertySignature":
            if (parent.key === node) {
                return !!parent.computed;
            }
            return true;
    }
    return true;
}

var isScope$1 = {};

Object.defineProperty(isScope$1, "__esModule", {
    value: true
});
isScope$1.default = isScope;
var _generated$2 = generated$4;
function isScope(node, parent) {
    if ((0, _generated$2.isBlockStatement)(node) && ((0, _generated$2.isFunction)(parent) || (0, _generated$2.isCatchClause)(parent))) {
        return false;
    }
    if ((0, _generated$2.isPattern)(node) && ((0, _generated$2.isFunction)(parent) || (0, _generated$2.isCatchClause)(parent))) {
        return true;
    }
    return (0, _generated$2.isScopable)(node);
}

var isSpecifierDefault$1 = {};

Object.defineProperty(isSpecifierDefault$1, "__esModule", {
    value: true
});
isSpecifierDefault$1.default = isSpecifierDefault;
var _generated$1 = generated$4;
function isSpecifierDefault(specifier) {
    return (0, _generated$1.isImportDefaultSpecifier)(specifier) || (0, _generated$1.isIdentifier)(specifier.imported || specifier.exported, {
        name: "default"
    });
}

var isValidES3Identifier$1 = {};

Object.defineProperty(isValidES3Identifier$1, "__esModule", {
    value: true
});
isValidES3Identifier$1.default = isValidES3Identifier;
var _isValidIdentifier = isValidIdentifier$1;
var RESERVED_WORDS_ES3_ONLY = new Set([
    "abstract",
    "boolean",
    "byte",
    "char",
    "double",
    "enum",
    "final",
    "float",
    "goto",
    "implements",
    "int",
    "interface",
    "long",
    "native",
    "package",
    "private",
    "protected",
    "public",
    "short",
    "static",
    "synchronized",
    "throws",
    "transient",
    "volatile"
]);
function isValidES3Identifier(name) {
    return (0, _isValidIdentifier.default)(name) && !RESERVED_WORDS_ES3_ONLY.has(name);
}

var isVar$1 = {};

Object.defineProperty(isVar$1, "__esModule", {
    value: true
});
isVar$1.default = isVar;
var _generated = generated$4;
var _constants = constants;
function isVar(node) {
    return (0, _generated.isVariableDeclaration)(node, {
        kind: "var"
    }) && !node[_constants.BLOCK_SCOPED_SYMBOL];
}

var generated = {};

(function(exports) {
    Object.defineProperty(exports, "__esModule", {
        value: true
    });
    var _exportNames = {
        react: true,
        assertNode: true,
        createTypeAnnotationBasedOnTypeof: true,
        createUnionTypeAnnotation: true,
        createFlowUnionType: true,
        createTSUnionType: true,
        cloneNode: true,
        clone: true,
        cloneDeep: true,
        cloneDeepWithoutLoc: true,
        cloneWithoutLoc: true,
        addComment: true,
        addComments: true,
        inheritInnerComments: true,
        inheritLeadingComments: true,
        inheritsComments: true,
        inheritTrailingComments: true,
        removeComments: true,
        ensureBlock: true,
        toBindingIdentifierName: true,
        toBlock: true,
        toComputedKey: true,
        toExpression: true,
        toIdentifier: true,
        toKeyAlias: true,
        toSequenceExpression: true,
        toStatement: true,
        valueToNode: true,
        appendToMemberExpression: true,
        inherits: true,
        prependToMemberExpression: true,
        removeProperties: true,
        removePropertiesDeep: true,
        removeTypeDuplicates: true,
        getBindingIdentifiers: true,
        getOuterBindingIdentifiers: true,
        traverse: true,
        traverseFast: true,
        shallowEqual: true,
        is: true,
        isBinding: true,
        isBlockScoped: true,
        isImmutable: true,
        isLet: true,
        isNode: true,
        isNodesEquivalent: true,
        isPlaceholderType: true,
        isReferenced: true,
        isScope: true,
        isSpecifierDefault: true,
        isType: true,
        isValidES3Identifier: true,
        isValidIdentifier: true,
        isVar: true,
        matchesPattern: true,
        validate: true,
        buildMatchMemberExpression: true
    };
    Object.defineProperty(exports, "assertNode", {
        enumerable: true,
        get: function get() {
            return _assertNode.default;
        }
    });
    Object.defineProperty(exports, "createTypeAnnotationBasedOnTypeof", {
        enumerable: true,
        get: function get() {
            return _createTypeAnnotationBasedOnTypeof.default;
        }
    });
    Object.defineProperty(exports, "createUnionTypeAnnotation", {
        enumerable: true,
        get: function get() {
            return _createFlowUnionType.default;
        }
    });
    Object.defineProperty(exports, "createFlowUnionType", {
        enumerable: true,
        get: function get() {
            return _createFlowUnionType.default;
        }
    });
    Object.defineProperty(exports, "createTSUnionType", {
        enumerable: true,
        get: function get() {
            return _createTSUnionType.default;
        }
    });
    Object.defineProperty(exports, "cloneNode", {
        enumerable: true,
        get: function get() {
            return _cloneNode.default;
        }
    });
    Object.defineProperty(exports, "clone", {
        enumerable: true,
        get: function get() {
            return _clone.default;
        }
    });
    Object.defineProperty(exports, "cloneDeep", {
        enumerable: true,
        get: function get() {
            return _cloneDeep.default;
        }
    });
    Object.defineProperty(exports, "cloneDeepWithoutLoc", {
        enumerable: true,
        get: function get() {
            return _cloneDeepWithoutLoc.default;
        }
    });
    Object.defineProperty(exports, "cloneWithoutLoc", {
        enumerable: true,
        get: function get() {
            return _cloneWithoutLoc.default;
        }
    });
    Object.defineProperty(exports, "addComment", {
        enumerable: true,
        get: function get() {
            return _addComment.default;
        }
    });
    Object.defineProperty(exports, "addComments", {
        enumerable: true,
        get: function get() {
            return _addComments.default;
        }
    });
    Object.defineProperty(exports, "inheritInnerComments", {
        enumerable: true,
        get: function get() {
            return _inheritInnerComments.default;
        }
    });
    Object.defineProperty(exports, "inheritLeadingComments", {
        enumerable: true,
        get: function get() {
            return _inheritLeadingComments.default;
        }
    });
    Object.defineProperty(exports, "inheritsComments", {
        enumerable: true,
        get: function get() {
            return _inheritsComments.default;
        }
    });
    Object.defineProperty(exports, "inheritTrailingComments", {
        enumerable: true,
        get: function get() {
            return _inheritTrailingComments.default;
        }
    });
    Object.defineProperty(exports, "removeComments", {
        enumerable: true,
        get: function get() {
            return _removeComments.default;
        }
    });
    Object.defineProperty(exports, "ensureBlock", {
        enumerable: true,
        get: function get() {
            return _ensureBlock.default;
        }
    });
    Object.defineProperty(exports, "toBindingIdentifierName", {
        enumerable: true,
        get: function get() {
            return _toBindingIdentifierName.default;
        }
    });
    Object.defineProperty(exports, "toBlock", {
        enumerable: true,
        get: function get() {
            return _toBlock.default;
        }
    });
    Object.defineProperty(exports, "toComputedKey", {
        enumerable: true,
        get: function get() {
            return _toComputedKey.default;
        }
    });
    Object.defineProperty(exports, "toExpression", {
        enumerable: true,
        get: function get() {
            return _toExpression.default;
        }
    });
    Object.defineProperty(exports, "toIdentifier", {
        enumerable: true,
        get: function get() {
            return _toIdentifier.default;
        }
    });
    Object.defineProperty(exports, "toKeyAlias", {
        enumerable: true,
        get: function get() {
            return _toKeyAlias.default;
        }
    });
    Object.defineProperty(exports, "toSequenceExpression", {
        enumerable: true,
        get: function get() {
            return _toSequenceExpression.default;
        }
    });
    Object.defineProperty(exports, "toStatement", {
        enumerable: true,
        get: function get() {
            return _toStatement.default;
        }
    });
    Object.defineProperty(exports, "valueToNode", {
        enumerable: true,
        get: function get() {
            return _valueToNode.default;
        }
    });
    Object.defineProperty(exports, "appendToMemberExpression", {
        enumerable: true,
        get: function get() {
            return _appendToMemberExpression.default;
        }
    });
    Object.defineProperty(exports, "inherits", {
        enumerable: true,
        get: function get() {
            return _inherits.default;
        }
    });
    Object.defineProperty(exports, "prependToMemberExpression", {
        enumerable: true,
        get: function get() {
            return _prependToMemberExpression.default;
        }
    });
    Object.defineProperty(exports, "removeProperties", {
        enumerable: true,
        get: function get() {
            return _removeProperties.default;
        }
    });
    Object.defineProperty(exports, "removePropertiesDeep", {
        enumerable: true,
        get: function get() {
            return _removePropertiesDeep.default;
        }
    });
    Object.defineProperty(exports, "removeTypeDuplicates", {
        enumerable: true,
        get: function get() {
            return _removeTypeDuplicates.default;
        }
    });
    Object.defineProperty(exports, "getBindingIdentifiers", {
        enumerable: true,
        get: function get() {
            return _getBindingIdentifiers.default;
        }
    });
    Object.defineProperty(exports, "getOuterBindingIdentifiers", {
        enumerable: true,
        get: function get() {
            return _getOuterBindingIdentifiers.default;
        }
    });
    Object.defineProperty(exports, "traverse", {
        enumerable: true,
        get: function get() {
            return _traverse.default;
        }
    });
    Object.defineProperty(exports, "traverseFast", {
        enumerable: true,
        get: function get() {
            return _traverseFast.default;
        }
    });
    Object.defineProperty(exports, "shallowEqual", {
        enumerable: true,
        get: function get() {
            return _shallowEqual.default;
        }
    });
    Object.defineProperty(exports, "is", {
        enumerable: true,
        get: function get() {
            return _is.default;
        }
    });
    Object.defineProperty(exports, "isBinding", {
        enumerable: true,
        get: function get() {
            return _isBinding.default;
        }
    });
    Object.defineProperty(exports, "isBlockScoped", {
        enumerable: true,
        get: function get() {
            return _isBlockScoped.default;
        }
    });
    Object.defineProperty(exports, "isImmutable", {
        enumerable: true,
        get: function get() {
            return _isImmutable.default;
        }
    });
    Object.defineProperty(exports, "isLet", {
        enumerable: true,
        get: function get() {
            return _isLet.default;
        }
    });
    Object.defineProperty(exports, "isNode", {
        enumerable: true,
        get: function get() {
            return _isNode.default;
        }
    });
    Object.defineProperty(exports, "isNodesEquivalent", {
        enumerable: true,
        get: function get() {
            return _isNodesEquivalent.default;
        }
    });
    Object.defineProperty(exports, "isPlaceholderType", {
        enumerable: true,
        get: function get() {
            return _isPlaceholderType.default;
        }
    });
    Object.defineProperty(exports, "isReferenced", {
        enumerable: true,
        get: function get() {
            return _isReferenced.default;
        }
    });
    Object.defineProperty(exports, "isScope", {
        enumerable: true,
        get: function get() {
            return _isScope.default;
        }
    });
    Object.defineProperty(exports, "isSpecifierDefault", {
        enumerable: true,
        get: function get() {
            return _isSpecifierDefault.default;
        }
    });
    Object.defineProperty(exports, "isType", {
        enumerable: true,
        get: function get() {
            return _isType.default;
        }
    });
    Object.defineProperty(exports, "isValidES3Identifier", {
        enumerable: true,
        get: function get() {
            return _isValidES3Identifier.default;
        }
    });
    Object.defineProperty(exports, "isValidIdentifier", {
        enumerable: true,
        get: function get() {
            return _isValidIdentifier.default;
        }
    });
    Object.defineProperty(exports, "isVar", {
        enumerable: true,
        get: function get() {
            return _isVar.default;
        }
    });
    Object.defineProperty(exports, "matchesPattern", {
        enumerable: true,
        get: function get() {
            return _matchesPattern.default;
        }
    });
    Object.defineProperty(exports, "validate", {
        enumerable: true,
        get: function get() {
            return _validate.default;
        }
    });
    Object.defineProperty(exports, "buildMatchMemberExpression", {
        enumerable: true,
        get: function get() {
            return _buildMatchMemberExpression.default;
        }
    });
    exports.react = void 0;
    var _isReactComponent = isReactComponent$1;
    var _isCompatTag = isCompatTag$1;
    var _buildChildren = buildChildren$1;
    var _assertNode = assertNode$1;
    var _generated = generated$2;
    Object.keys(_generated).forEach(function(key) {
        if (key === "default" || key === "__esModule") return;
        if (Object.prototype.hasOwnProperty.call(_exportNames, key)) return;
        if (key in exports && exports[key] === _generated[key]) return;
        Object.defineProperty(exports, key, {
            enumerable: true,
            get: function get() {
                return _generated[key];
            }
        });
    });
    var _createTypeAnnotationBasedOnTypeof = createTypeAnnotationBasedOnTypeof$1;
    var _createFlowUnionType = createFlowUnionType$1;
    var _createTSUnionType = createTSUnionType$1;
    var _generated2 = generated$3;
    Object.keys(_generated2).forEach(function(key) {
        if (key === "default" || key === "__esModule") return;
        if (Object.prototype.hasOwnProperty.call(_exportNames, key)) return;
        if (key in exports && exports[key] === _generated2[key]) return;
        Object.defineProperty(exports, key, {
            enumerable: true,
            get: function get() {
                return _generated2[key];
            }
        });
    });
    var _uppercase = uppercase;
    Object.keys(_uppercase).forEach(function(key) {
        if (key === "default" || key === "__esModule") return;
        if (Object.prototype.hasOwnProperty.call(_exportNames, key)) return;
        if (key in exports && exports[key] === _uppercase[key]) return;
        Object.defineProperty(exports, key, {
            enumerable: true,
            get: function get() {
                return _uppercase[key];
            }
        });
    });
    var _cloneNode = cloneNode$1;
    var _clone = clone$1;
    var _cloneDeep = cloneDeep$1;
    var _cloneDeepWithoutLoc = cloneDeepWithoutLoc$1;
    var _cloneWithoutLoc = cloneWithoutLoc$1;
    var _addComment = addComment$1;
    var _addComments = addComments$1;
    var _inheritInnerComments = inheritInnerComments$1;
    var _inheritLeadingComments = inheritLeadingComments$1;
    var _inheritsComments = inheritsComments$1;
    var _inheritTrailingComments = inheritTrailingComments$1;
    var _removeComments = removeComments$1;
    var _generated3 = generated$1;
    Object.keys(_generated3).forEach(function(key) {
        if (key === "default" || key === "__esModule") return;
        if (Object.prototype.hasOwnProperty.call(_exportNames, key)) return;
        if (key in exports && exports[key] === _generated3[key]) return;
        Object.defineProperty(exports, key, {
            enumerable: true,
            get: function get() {
                return _generated3[key];
            }
        });
    });
    var _constants = constants;
    Object.keys(_constants).forEach(function(key) {
        if (key === "default" || key === "__esModule") return;
        if (Object.prototype.hasOwnProperty.call(_exportNames, key)) return;
        if (key in exports && exports[key] === _constants[key]) return;
        Object.defineProperty(exports, key, {
            enumerable: true,
            get: function get() {
                return _constants[key];
            }
        });
    });
    var _ensureBlock = ensureBlock$1;
    var _toBindingIdentifierName = toBindingIdentifierName$1;
    var _toBlock = toBlock$1;
    var _toComputedKey = toComputedKey$1;
    var _toExpression = toExpression$1;
    var _toIdentifier = toIdentifier$1;
    var _toKeyAlias = toKeyAlias$1;
    var _toSequenceExpression = toSequenceExpression$1;
    var _toStatement = toStatement$1;
    var _valueToNode = valueToNode$1;
    var _definitions = requireDefinitions();
    Object.keys(_definitions).forEach(function(key) {
        if (key === "default" || key === "__esModule") return;
        if (Object.prototype.hasOwnProperty.call(_exportNames, key)) return;
        if (key in exports && exports[key] === _definitions[key]) return;
        Object.defineProperty(exports, key, {
            enumerable: true,
            get: function get() {
                return _definitions[key];
            }
        });
    });
    var _appendToMemberExpression = appendToMemberExpression$1;
    var _inherits = inherits$1;
    var _prependToMemberExpression = prependToMemberExpression$1;
    var _removeProperties = removeProperties$1;
    var _removePropertiesDeep = removePropertiesDeep$1;
    var _removeTypeDuplicates = removeTypeDuplicates$3;
    var _getBindingIdentifiers = getBindingIdentifiers$1;
    var _getOuterBindingIdentifiers = getOuterBindingIdentifiers$1;
    var _traverse = traverse$1;
    Object.keys(_traverse).forEach(function(key) {
        if (key === "default" || key === "__esModule") return;
        if (Object.prototype.hasOwnProperty.call(_exportNames, key)) return;
        if (key in exports && exports[key] === _traverse[key]) return;
        Object.defineProperty(exports, key, {
            enumerable: true,
            get: function get() {
                return _traverse[key];
            }
        });
    });
    var _traverseFast = traverseFast$1;
    var _shallowEqual = shallowEqual$1;
    var _is = requireIs();
    var _isBinding = isBinding$1;
    var _isBlockScoped = isBlockScoped$1;
    var _isImmutable = isImmutable$1;
    var _isLet = isLet$1;
    var _isNode = isNode$1;
    var _isNodesEquivalent = isNodesEquivalent$1;
    var _isPlaceholderType = requireIsPlaceholderType();
    var _isReferenced = isReferenced$1;
    var _isScope = isScope$1;
    var _isSpecifierDefault = isSpecifierDefault$1;
    var _isType = requireIsType();
    var _isValidES3Identifier = isValidES3Identifier$1;
    var _isValidIdentifier = isValidIdentifier$1;
    var _isVar = isVar$1;
    var _matchesPattern = matchesPattern$1;
    var _validate = requireValidate();
    var _buildMatchMemberExpression = buildMatchMemberExpression$1;
    var _generated4 = generated$4;
    Object.keys(_generated4).forEach(function(key) {
        if (key === "default" || key === "__esModule") return;
        if (Object.prototype.hasOwnProperty.call(_exportNames, key)) return;
        if (key in exports && exports[key] === _generated4[key]) return;
        Object.defineProperty(exports, key, {
            enumerable: true,
            get: function get() {
                return _generated4[key];
            }
        });
    });
    var _generated5 = generated;
    Object.keys(_generated5).forEach(function(key) {
        if (key === "default" || key === "__esModule") return;
        if (Object.prototype.hasOwnProperty.call(_exportNames, key)) return;
        if (key in exports && exports[key] === _generated5[key]) return;
        Object.defineProperty(exports, key, {
            enumerable: true,
            get: function get() {
                return _generated5[key];
            }
        });
    });
    var react = {
        isReactComponent: _isReactComponent.default,
        isCompatTag: _isCompatTag.default,
        buildChildren: _buildChildren.default
    };
    exports.react = react;
})(lib$1);

function hash(str) {
    var _$hash = 5381, i = str.length;
    while(i){
        _$hash = _$hash * 33 ^ str.charCodeAt(--i);
    }
    /* JavaScript does bitwise operations (like XOR, above) on 32-bit signed
   * integers. Since we want the results to be always positive, convert the
   * signed int to an unsigned by doing an unsigned bitshift. */ return _$hash >>> 0;
}
var stringHash = hash;

var sourceMapGenerator = {};

var base64Vlq = {};

var base64$1 = {};

/*
 * Copyright 2011 Mozilla Foundation and contributors
 * Licensed under the New BSD license. See LICENSE or:
 * http://opensource.org/licenses/BSD-3-Clause
 */ var intToCharMap = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/".split("");
/**
 * Encode an integer in the range of 0 to 63 to a single base 64 digit.
 */ base64$1.encode = function encode(number) {
    if (0 <= number && number < intToCharMap.length) {
        return intToCharMap[number];
    }
    throw new TypeError("Must be between 0 and 63: " + number);
};

/*
 * Copyright 2011 Mozilla Foundation and contributors
 * Licensed under the New BSD license. See LICENSE or:
 * http://opensource.org/licenses/BSD-3-Clause
 *
 * Based on the Base 64 VLQ implementation in Closure Compiler:
 * https://code.google.com/p/closure-compiler/source/browse/trunk/src/com/google/debugging/sourcemap/Base64VLQ.java
 *
 * Copyright 2011 The Closure Compiler Authors. All rights reserved.
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials provided
 *    with the distribution.
 *  * Neither the name of Google Inc. nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */ var base64 = base64$1;
// A single base 64 digit can contain 6 bits of data. For the base 64 variable
// length quantities we use in the source map spec, the first bit is the sign,
// the next four bits are the actual value, and the 6th bit is the
// continuation bit. The continuation bit tells us whether there are more
// digits in this value following this digit.
//
//   Continuation
//   |    Sign
//   |    |
//   V    V
//   101011
var VLQ_BASE_SHIFT = 5;
// binary: 100000
var VLQ_BASE = 1 << VLQ_BASE_SHIFT;
// binary: 011111
var VLQ_BASE_MASK = VLQ_BASE - 1;
// binary: 100000
var VLQ_CONTINUATION_BIT = VLQ_BASE;
/**
 * Converts from a two-complement value to a value where the sign bit is
 * placed in the least significant bit.  For example, as decimals:
 *   1 becomes 2 (10 binary), -1 becomes 3 (11 binary)
 *   2 becomes 4 (100 binary), -2 becomes 5 (101 binary)
 */ function toVLQSigned(aValue) {
    return aValue < 0 ? (-aValue << 1) + 1 : (aValue << 1) + 0;
}
/**
 * Returns the base 64 VLQ encoded value.
 */ base64Vlq.encode = function base64VLQ_encode(aValue) {
    var encoded = "";
    var digit;
    var vlq = toVLQSigned(aValue);
    do {
        digit = vlq & VLQ_BASE_MASK;
        vlq >>>= VLQ_BASE_SHIFT;
        if (vlq > 0) {
            // There are still more digits in this value, so we must make sure the
            // continuation bit is marked.
            digit |= VLQ_CONTINUATION_BIT;
        }
        encoded += base64.encode(digit);
    }while (vlq > 0);
    return encoded;
};

var util$3 = {};

(function(exports) {
    var getArg = /*
	 * Copyright 2011 Mozilla Foundation and contributors
	 * Licensed under the New BSD license. See LICENSE or:
	 * http://opensource.org/licenses/BSD-3-Clause
	 */ /**
	 * This is a helper function for getting values from parameter/options
	 * objects.
	 *
	 * @param args The object we are extracting values from
	 * @param name The name of the property we are getting.
	 * @param defaultValue An optional value to return if the property is missing
	 * from the object. If this is not specified and the property is missing, an
	 * error will be thrown.
	 */ function getArg(aArgs, aName, aDefaultValue) {
        if (aName in aArgs) {
            return aArgs[aName];
        } else if (arguments.length === 3) {
            return aDefaultValue;
        }
        throw new Error('"' + aName + '" is a required argument.');
    };
    var urlParse = function urlParse(aUrl) {
        var match = aUrl.match(urlRegexp);
        if (!match) {
            return null;
        }
        return {
            scheme: match[1],
            auth: match[2],
            host: match[3],
            port: match[4],
            path: match[5]
        };
    };
    var urlGenerate = function urlGenerate(aParsedUrl) {
        var url = "";
        if (aParsedUrl.scheme) {
            url += aParsedUrl.scheme + ":";
        }
        url += "//";
        if (aParsedUrl.auth) {
            url += aParsedUrl.auth + "@";
        }
        if (aParsedUrl.host) {
            url += aParsedUrl.host;
        }
        if (aParsedUrl.port) {
            url += ":" + aParsedUrl.port;
        }
        if (aParsedUrl.path) {
            url += aParsedUrl.path;
        }
        return url;
    };
    var lruMemoize = /**
	 * Takes some function `f(input) -> result` and returns a memoized version of
	 * `f`.
	 *
	 * We keep at most `MAX_CACHED_INPUTS` memoized results of `f` alive. The
	 * memoization is a dumb-simple, linear least-recently-used cache.
	 */ function lruMemoize(f) {
        var cache = [];
        return function(input) {
            for(var i = 0; i < cache.length; i++){
                if (cache[i].input === input) {
                    var temp = cache[0];
                    cache[0] = cache[i];
                    cache[i] = temp;
                    return cache[0].result;
                }
            }
            var result = f(input);
            cache.unshift({
                input: input,
                result: result
            });
            if (cache.length > MAX_CACHED_INPUTS) {
                cache.pop();
            }
            return result;
        };
    };
    var join = /**
	 * Joins two paths/URLs.
	 *
	 * @param aRoot The root path or URL.
	 * @param aPath The path or URL to be joined with the root.
	 *
	 * - If aPath is a URL or a data URI, aPath is returned, unless aPath is a
	 *   scheme-relative URL: Then the scheme of aRoot, if any, is prepended
	 *   first.
	 * - Otherwise aPath is a path. If aRoot is a URL, then its path portion
	 *   is updated with the result and aRoot is returned. Otherwise the result
	 *   is returned.
	 *   - If aPath is absolute, the result is aPath.
	 *   - Otherwise the two paths are joined with a slash.
	 * - Joining for example 'http://' and 'www.example.com' is also supported.
	 */ function join(aRoot, aPath) {
        if (aRoot === "") {
            aRoot = ".";
        }
        if (aPath === "") {
            aPath = ".";
        }
        var aPathUrl = urlParse(aPath);
        var aRootUrl = urlParse(aRoot);
        if (aRootUrl) {
            aRoot = aRootUrl.path || "/";
        }
        // `join(foo, '//www.example.org')`
        if (aPathUrl && !aPathUrl.scheme) {
            if (aRootUrl) {
                aPathUrl.scheme = aRootUrl.scheme;
            }
            return urlGenerate(aPathUrl);
        }
        if (aPathUrl || aPath.match(dataUrlRegexp)) {
            return aPath;
        }
        // `join('http://', 'www.example.com')`
        if (aRootUrl && !aRootUrl.host && !aRootUrl.path) {
            aRootUrl.host = aPath;
            return urlGenerate(aRootUrl);
        }
        var joined = aPath.charAt(0) === "/" ? aPath : normalize(aRoot.replace(/\/+$/, "") + "/" + aPath);
        if (aRootUrl) {
            aRootUrl.path = joined;
            return urlGenerate(aRootUrl);
        }
        return joined;
    };
    var relative = /**
	 * Make a path relative to a URL or another path.
	 *
	 * @param aRoot The root path or URL.
	 * @param aPath The path or URL to be made relative to aRoot.
	 */ function relative(aRoot, aPath) {
        if (aRoot === "") {
            aRoot = ".";
        }
        aRoot = aRoot.replace(/\/$/, "");
        // It is possible for the path to be above the root. In this case, simply
        // checking whether the root is a prefix of the path won't work. Instead, we
        // need to remove components from the root one by one, until either we find
        // a prefix that fits, or we run out of components to remove.
        var level = 0;
        while(aPath.indexOf(aRoot + "/") !== 0){
            var index = aRoot.lastIndexOf("/");
            if (index < 0) {
                return aPath;
            }
            // If the only part of the root that is left is the scheme (i.e. http://,
            // file:///, etc.), one or more slashes (/), or simply nothing at all, we
            // have exhausted all components, so the path is not relative to the root.
            aRoot = aRoot.slice(0, index);
            if (aRoot.match(/^([^\/]+:\/)?\/*$/)) {
                return aPath;
            }
            ++level;
        }
        // Make sure we add a "../" for each component we removed from the root.
        return Array(level + 1).join("../") + aPath.substr(aRoot.length + 1);
    };
    var identity = function identity(s) {
        return s;
    };
    var toSetString = /**
	 * Because behavior goes wacky when you set `__proto__` on objects, we
	 * have to prefix all the strings in our set with an arbitrary character.
	 *
	 * See https://github.com/mozilla/source-map/pull/31 and
	 * https://github.com/mozilla/source-map/issues/30
	 *
	 * @param String aStr
	 */ function toSetString(aStr) {
        if (isProtoString(aStr)) {
            return "$" + aStr;
        }
        return aStr;
    };
    var fromSetString = function fromSetString(aStr) {
        if (isProtoString(aStr)) {
            return aStr.slice(1);
        }
        return aStr;
    };
    var isProtoString = function isProtoString(s) {
        if (!s) {
            return false;
        }
        var length = s.length;
        if (length < 9 /* "__proto__".length */ ) {
            return false;
        }
        /* eslint-disable no-multi-spaces */ if (s.charCodeAt(length - 1) !== 95 /* '_' */  || s.charCodeAt(length - 2) !== 95 /* '_' */  || s.charCodeAt(length - 3) !== 111 /* 'o' */  || s.charCodeAt(length - 4) !== 116 /* 't' */  || s.charCodeAt(length - 5) !== 111 /* 'o' */  || s.charCodeAt(length - 6) !== 114 /* 'r' */  || s.charCodeAt(length - 7) !== 112 /* 'p' */  || s.charCodeAt(length - 8) !== 95 /* '_' */  || s.charCodeAt(length - 9) !== 95 /* '_' */ ) {
            return false;
        }
        /* eslint-enable no-multi-spaces */ for(var i = length - 10; i >= 0; i--){
            if (s.charCodeAt(i) !== 36 /* '$' */ ) {
                return false;
            }
        }
        return true;
    };
    var compareByOriginalPositions = /**
	 * Comparator between two mappings where the original positions are compared.
	 *
	 * Optionally pass in `true` as `onlyCompareGenerated` to consider two
	 * mappings with the same original source/line/column, but different generated
	 * line and column the same. Useful when searching for a mapping with a
	 * stubbed out mapping.
	 */ function compareByOriginalPositions(mappingA, mappingB, onlyCompareOriginal) {
        var cmp = strcmp(mappingA.source, mappingB.source);
        if (cmp !== 0) {
            return cmp;
        }
        cmp = mappingA.originalLine - mappingB.originalLine;
        if (cmp !== 0) {
            return cmp;
        }
        cmp = mappingA.originalColumn - mappingB.originalColumn;
        if (cmp !== 0 || onlyCompareOriginal) {
            return cmp;
        }
        cmp = mappingA.generatedColumn - mappingB.generatedColumn;
        if (cmp !== 0) {
            return cmp;
        }
        cmp = mappingA.generatedLine - mappingB.generatedLine;
        if (cmp !== 0) {
            return cmp;
        }
        return strcmp(mappingA.name, mappingB.name);
    };
    var compareByGeneratedPositionsDeflated = /**
	 * Comparator between two mappings with deflated source and name indices where
	 * the generated positions are compared.
	 *
	 * Optionally pass in `true` as `onlyCompareGenerated` to consider two
	 * mappings with the same generated line and column, but different
	 * source/name/original line and column the same. Useful when searching for a
	 * mapping with a stubbed out mapping.
	 */ function compareByGeneratedPositionsDeflated(mappingA, mappingB, onlyCompareGenerated) {
        var cmp = mappingA.generatedLine - mappingB.generatedLine;
        if (cmp !== 0) {
            return cmp;
        }
        cmp = mappingA.generatedColumn - mappingB.generatedColumn;
        if (cmp !== 0 || onlyCompareGenerated) {
            return cmp;
        }
        cmp = strcmp(mappingA.source, mappingB.source);
        if (cmp !== 0) {
            return cmp;
        }
        cmp = mappingA.originalLine - mappingB.originalLine;
        if (cmp !== 0) {
            return cmp;
        }
        cmp = mappingA.originalColumn - mappingB.originalColumn;
        if (cmp !== 0) {
            return cmp;
        }
        return strcmp(mappingA.name, mappingB.name);
    };
    var strcmp = function strcmp(aStr1, aStr2) {
        if (aStr1 === aStr2) {
            return 0;
        }
        if (aStr1 === null) {
            return 1; // aStr2 !== null
        }
        if (aStr2 === null) {
            return -1; // aStr1 !== null
        }
        if (aStr1 > aStr2) {
            return 1;
        }
        return -1;
    };
    var compareByGeneratedPositionsInflated = /**
	 * Comparator between two mappings with inflated source and name strings where
	 * the generated positions are compared.
	 */ function compareByGeneratedPositionsInflated(mappingA, mappingB) {
        var cmp = mappingA.generatedLine - mappingB.generatedLine;
        if (cmp !== 0) {
            return cmp;
        }
        cmp = mappingA.generatedColumn - mappingB.generatedColumn;
        if (cmp !== 0) {
            return cmp;
        }
        cmp = strcmp(mappingA.source, mappingB.source);
        if (cmp !== 0) {
            return cmp;
        }
        cmp = mappingA.originalLine - mappingB.originalLine;
        if (cmp !== 0) {
            return cmp;
        }
        cmp = mappingA.originalColumn - mappingB.originalColumn;
        if (cmp !== 0) {
            return cmp;
        }
        return strcmp(mappingA.name, mappingB.name);
    };
    var parseSourceMapInput = /**
	 * Strip any JSON XSSI avoidance prefix from the string (as documented
	 * in the source maps specification), and then parse the string as
	 * JSON.
	 */ function parseSourceMapInput(str) {
        return JSON.parse(str.replace(/^\)]}'[^\n]*\n/, ""));
    };
    var computeSourceURL = /**
	 * Compute the URL of a source given the the source root, the source's
	 * URL, and the source map's URL.
	 */ function computeSourceURL(sourceRoot, sourceURL, sourceMapURL) {
        sourceURL = sourceURL || "";
        if (sourceRoot) {
            // This follows what Chrome does.
            if (sourceRoot[sourceRoot.length - 1] !== "/" && sourceURL[0] !== "/") {
                sourceRoot += "/";
            }
            // The spec says:
            //   Line 4: An optional source root, useful for relocating source
            //   files on a server or removing repeated values in the
            //   “sources” entry.  This value is prepended to the individual
            //   entries in the “source” field.
            sourceURL = sourceRoot + sourceURL;
        }
        // Historically, SourceMapConsumer did not take the sourceMapURL as
        // a parameter.  This mode is still somewhat supported, which is why
        // this code block is conditional.  However, it's preferable to pass
        // the source map URL to SourceMapConsumer, so that this function
        // can implement the source URL resolution algorithm as outlined in
        // the spec.  This block is basically the equivalent of:
        //    new URL(sourceURL, sourceMapURL).toString()
        // ... except it avoids using URL, which wasn't available in the
        // older releases of node still supported by this library.
        //
        // The spec says:
        //   If the sources are not absolute URLs after prepending of the
        //   “sourceRoot”, the sources are resolved relative to the
        //   SourceMap (like resolving script src in a html document).
        if (sourceMapURL) {
            var parsed = urlParse(sourceMapURL);
            if (!parsed) {
                throw new Error("sourceMapURL could not be parsed");
            }
            if (parsed.path) {
                // Strip the last path component, but keep the "/".
                var index = parsed.path.lastIndexOf("/");
                if (index >= 0) {
                    parsed.path = parsed.path.substring(0, index + 1);
                }
            }
            sourceURL = join(urlGenerate(parsed), sourceURL);
        }
        return normalize(sourceURL);
    };
    exports.getArg = getArg;
    var urlRegexp = /^(?:([\w+\-.]+):)?\/\/(?:(\w+:\w+)@)?([\w.-]*)(?::(\d+))?(.*)$/;
    var dataUrlRegexp = /^data:.+\,.+$/;
    exports.urlParse = urlParse;
    exports.urlGenerate = urlGenerate;
    var MAX_CACHED_INPUTS = 32;
    /**
	 * Normalizes a path, or the path portion of a URL:
	 *
	 * - Replaces consecutive slashes with one slash.
	 * - Removes unnecessary '.' parts.
	 * - Removes unnecessary '<dir>/..' parts.
	 *
	 * Based on code in the Node.js 'path' core module.
	 *
	 * @param aPath The path or url to normalize.
	 */ var normalize = lruMemoize(function normalize(aPath) {
        var path = aPath;
        var url = urlParse(aPath);
        if (url) {
            if (!url.path) {
                return aPath;
            }
            path = url.path;
        }
        var isAbsolute = exports.isAbsolute(path);
        // Split the path into parts between `/` characters. This is much faster than
        // using `.split(/\/+/g)`.
        var parts = [];
        var start = 0;
        var i = 0;
        while(true){
            start = i;
            i = path.indexOf("/", start);
            if (i === -1) {
                parts.push(path.slice(start));
                break;
            } else {
                parts.push(path.slice(start, i));
                while(i < path.length && path[i] === "/"){
                    i++;
                }
            }
        }
        var up = 0;
        for(i = parts.length - 1; i >= 0; i--){
            var part = parts[i];
            if (part === ".") {
                parts.splice(i, 1);
            } else if (part === "..") {
                up++;
            } else if (up > 0) {
                if (part === "") {
                    // The first part is blank if the path is absolute. Trying to go
                    // above the root is a no-op. Therefore we can remove all '..' parts
                    // directly after the root.
                    parts.splice(i + 1, up);
                    up = 0;
                } else {
                    parts.splice(i, 2);
                    up--;
                }
            }
        }
        path = parts.join("/");
        if (path === "") {
            path = isAbsolute ? "/" : ".";
        }
        if (url) {
            url.path = path;
            return urlGenerate(url);
        }
        return path;
    });
    exports.normalize = normalize;
    exports.join = join;
    exports.isAbsolute = function(aPath) {
        return aPath.charAt(0) === "/" || urlRegexp.test(aPath);
    };
    exports.relative = relative;
    var supportsNullProto = function() {
        var obj = Object.create(null);
        return !("__proto__" in obj);
    }();
    exports.toSetString = supportsNullProto ? identity : toSetString;
    exports.fromSetString = supportsNullProto ? identity : fromSetString;
    exports.compareByOriginalPositions = compareByOriginalPositions;
    exports.compareByGeneratedPositionsDeflated = compareByGeneratedPositionsDeflated;
    exports.compareByGeneratedPositionsInflated = compareByGeneratedPositionsInflated;
    exports.parseSourceMapInput = parseSourceMapInput;
    exports.computeSourceURL = computeSourceURL;
})(util$3);

var arraySet = {};

/*
 * Copyright 2011 Mozilla Foundation and contributors
 * Licensed under the New BSD license. See LICENSE or:
 * http://opensource.org/licenses/BSD-3-Clause
 */ /**
 * A data structure which is a combination of an array and a set. Adding a new
 * member is O(1), testing for membership is O(1), and finding the index of an
 * element is O(1). Removing elements from the set is not supported. Only
 * strings are supported for membership.
 */ var ArraySet$2 = /*#__PURE__*/ function() {
    function ArraySet() {
        this._array = [];
        this._set = new Map();
    }
    var _proto = ArraySet.prototype;
    /**
   * Return how many unique items are in this ArraySet. If duplicates have been
   * added, than those do not count towards the size.
   *
   * @returns Number
   */ _proto.size = function size() {
        return this._set.size;
    };
    /**
   * Add the given string to this set.
   *
   * @param String aStr
   */ _proto.add = function add(aStr, aAllowDuplicates) {
        var isDuplicate = this.has(aStr);
        var idx = this._array.length;
        if (!isDuplicate || aAllowDuplicates) {
            this._array.push(aStr);
        }
        if (!isDuplicate) {
            this._set.set(aStr, idx);
        }
    };
    /**
   * Is the given string a member of this set?
   *
   * @param String aStr
   */ _proto.has = function has(aStr) {
        return this._set.has(aStr);
    };
    /**
   * What is the index of the given string in the array?
   *
   * @param String aStr
   */ _proto.indexOf = function indexOf(aStr) {
        var idx = this._set.get(aStr);
        if (idx >= 0) {
            return idx;
        }
        throw new Error('"' + aStr + '" is not in the set.');
    };
    /**
   * What is the element at the given index?
   *
   * @param Number aIdx
   */ _proto.at = function at(aIdx) {
        if (aIdx >= 0 && aIdx < this._array.length) {
            return this._array[aIdx];
        }
        throw new Error("No element indexed by " + aIdx);
    };
    /**
   * Returns the array representation of this set (which has the proper indices
   * indicated by indexOf). Note that this is a copy of the internal array used
   * for storing the members so that no one can mess with internal state.
   */ _proto.toArray = function toArray() {
        return this._array.slice();
    };
    /**
   * Static method for creating ArraySet instances from an existing array.
   */ ArraySet.fromArray = function fromArray(aArray, aAllowDuplicates) {
        var set = new ArraySet();
        for(var i = 0, len = aArray.length; i < len; i++){
            set.add(aArray[i], aAllowDuplicates);
        }
        return set;
    };
    return ArraySet;
}();
arraySet.ArraySet = ArraySet$2;

var mappingList = {};

/*
 * Copyright 2014 Mozilla Foundation and contributors
 * Licensed under the New BSD license. See LICENSE or:
 * http://opensource.org/licenses/BSD-3-Clause
 */ var util$2 = util$3;
/**
 * Determine whether mappingB is after mappingA with respect to generated
 * position.
 */ function generatedPositionAfter(mappingA, mappingB) {
    // Optimized for most common case
    var lineA = mappingA.generatedLine;
    var lineB = mappingB.generatedLine;
    var columnA = mappingA.generatedColumn;
    var columnB = mappingB.generatedColumn;
    return lineB > lineA || lineB == lineA && columnB >= columnA || util$2.compareByGeneratedPositionsInflated(mappingA, mappingB) <= 0;
}
/**
 * A data structure to provide a sorted view of accumulated mappings in a
 * performance conscious manner. It trades a negligible overhead in general
 * case for a large speedup in case of mappings being added in order.
 */ var MappingList$1 = /*#__PURE__*/ function() {
    function MappingList() {
        this._array = [];
        this._sorted = true;
        // Serves as infimum
        this._last = {
            generatedLine: -1,
            generatedColumn: 0
        };
    }
    var _proto = MappingList.prototype;
    /**
   * Iterate through internal items. This method takes the same arguments that
   * `Array.prototype.forEach` takes.
   *
   * NOTE: The order of the mappings is NOT guaranteed.
   */ _proto.unsortedForEach = function unsortedForEach(aCallback, aThisArg) {
        this._array.forEach(aCallback, aThisArg);
    };
    /**
   * Add the given source mapping.
   *
   * @param Object aMapping
   */ _proto.add = function add(aMapping) {
        if (generatedPositionAfter(this._last, aMapping)) {
            this._last = aMapping;
            this._array.push(aMapping);
        } else {
            this._sorted = false;
            this._array.push(aMapping);
        }
    };
    /**
   * Returns the flat, sorted array of mappings. The mappings are sorted by
   * generated position.
   *
   * WARNING: This method returns internal data without copying, for
   * performance. The return value must NOT be mutated, and should be treated as
   * an immutable borrow. If you want to take ownership, you must make your own
   * copy.
   */ _proto.toArray = function toArray() {
        if (!this._sorted) {
            this._array.sort(util$2.compareByGeneratedPositionsInflated);
            this._sorted = true;
        }
        return this._array;
    };
    return MappingList;
}();
mappingList.MappingList = MappingList$1;

/*
 * Copyright 2011 Mozilla Foundation and contributors
 * Licensed under the New BSD license. See LICENSE or:
 * http://opensource.org/licenses/BSD-3-Clause
 */ var base64VLQ = base64Vlq;
var util$1 = util$3;
var ArraySet$1 = arraySet.ArraySet;
var MappingList = mappingList.MappingList;
/**
 * An instance of the SourceMapGenerator represents a source map which is
 * being built incrementally. You may pass an object with the following
 * properties:
 *
 *   - file: The filename of the generated source.
 *   - sourceRoot: A root for all relative URLs in this source map.
 */ var SourceMapGenerator$1 = /*#__PURE__*/ function() {
    function SourceMapGenerator(aArgs) {
        if (!aArgs) {
            aArgs = {};
        }
        this._file = util$1.getArg(aArgs, "file", null);
        this._sourceRoot = util$1.getArg(aArgs, "sourceRoot", null);
        this._skipValidation = util$1.getArg(aArgs, "skipValidation", false);
        this._sources = new ArraySet$1();
        this._names = new ArraySet$1();
        this._mappings = new MappingList();
        this._sourcesContents = null;
    }
    var _proto = SourceMapGenerator.prototype;
    /**
   * Add a single mapping from original source line and column to the generated
   * source's line and column for this source map being created. The mapping
   * object should have the following properties:
   *
   *   - generated: An object with the generated line and column positions.
   *   - original: An object with the original line and column positions.
   *   - source: The original source file (relative to the sourceRoot).
   *   - name: An optional original token name for this mapping.
   */ _proto.addMapping = function addMapping(aArgs) {
        var generated = util$1.getArg(aArgs, "generated");
        var original = util$1.getArg(aArgs, "original", null);
        var source = util$1.getArg(aArgs, "source", null);
        var name = util$1.getArg(aArgs, "name", null);
        if (!this._skipValidation) {
            this._validateMapping(generated, original, source, name);
        }
        if (source != null) {
            source = String(source);
            if (!this._sources.has(source)) {
                this._sources.add(source);
            }
        }
        if (name != null) {
            name = String(name);
            if (!this._names.has(name)) {
                this._names.add(name);
            }
        }
        this._mappings.add({
            generatedLine: generated.line,
            generatedColumn: generated.column,
            originalLine: original != null && original.line,
            originalColumn: original != null && original.column,
            source: source,
            name: name
        });
    };
    /**
   * Set the source content for a source file.
   */ _proto.setSourceContent = function setSourceContent(aSourceFile, aSourceContent) {
        var source = aSourceFile;
        if (this._sourceRoot != null) {
            source = util$1.relative(this._sourceRoot, source);
        }
        if (aSourceContent != null) {
            // Add the source content to the _sourcesContents map.
            // Create a new _sourcesContents map if the property is null.
            if (!this._sourcesContents) {
                this._sourcesContents = Object.create(null);
            }
            this._sourcesContents[util$1.toSetString(source)] = aSourceContent;
        } else if (this._sourcesContents) {
            // Remove the source file from the _sourcesContents map.
            // If the _sourcesContents map is empty, set the property to null.
            delete this._sourcesContents[util$1.toSetString(source)];
            if (Object.keys(this._sourcesContents).length === 0) {
                this._sourcesContents = null;
            }
        }
    };
    /**
   * Applies the mappings of a sub-source-map for a specific source file to the
   * source map being generated. Each mapping to the supplied source file is
   * rewritten using the supplied source map. Note: The resolution for the
   * resulting mappings is the minimium of this map and the supplied map.
   *
   * @param aSourceMapConsumer The source map to be applied.
   * @param aSourceFile Optional. The filename of the source file.
   *        If omitted, SourceMapConsumer's file property will be used.
   * @param aSourceMapPath Optional. The dirname of the path to the source map
   *        to be applied. If relative, it is relative to the SourceMapConsumer.
   *        This parameter is needed when the two source maps aren't in the same
   *        directory, and the source map to be applied contains relative source
   *        paths. If so, those relative source paths need to be rewritten
   *        relative to the SourceMapGenerator.
   */ _proto.applySourceMap = function applySourceMap(aSourceMapConsumer, aSourceFile, aSourceMapPath) {
        var sourceFile = aSourceFile;
        // If aSourceFile is omitted, we will use the file property of the SourceMap
        if (aSourceFile == null) {
            if (aSourceMapConsumer.file == null) {
                throw new Error("SourceMapGenerator.prototype.applySourceMap requires either an explicit source file, " + 'or the source map\'s "file" property. Both were omitted.');
            }
            sourceFile = aSourceMapConsumer.file;
        }
        var sourceRoot = this._sourceRoot;
        // Make "sourceFile" relative if an absolute Url is passed.
        if (sourceRoot != null) {
            sourceFile = util$1.relative(sourceRoot, sourceFile);
        }
        // Applying the SourceMap can add and remove items from the sources and
        // the names array.
        var newSources = this._mappings.toArray().length > 0 ? new ArraySet$1() : this._sources;
        var newNames = new ArraySet$1();
        // Find mappings for the "sourceFile"
        this._mappings.unsortedForEach(function(mapping) {
            if (mapping.source === sourceFile && mapping.originalLine != null) {
                // Check if it can be mapped by the source map, then update the mapping.
                var original = aSourceMapConsumer.originalPositionFor({
                    line: mapping.originalLine,
                    column: mapping.originalColumn
                });
                if (original.source != null) {
                    // Copy mapping
                    mapping.source = original.source;
                    if (aSourceMapPath != null) {
                        mapping.source = util$1.join(aSourceMapPath, mapping.source);
                    }
                    if (sourceRoot != null) {
                        mapping.source = util$1.relative(sourceRoot, mapping.source);
                    }
                    mapping.originalLine = original.line;
                    mapping.originalColumn = original.column;
                    if (original.name != null) {
                        mapping.name = original.name;
                    }
                }
            }
            var source = mapping.source;
            if (source != null && !newSources.has(source)) {
                newSources.add(source);
            }
            var name = mapping.name;
            if (name != null && !newNames.has(name)) {
                newNames.add(name);
            }
        }, this);
        this._sources = newSources;
        this._names = newNames;
        // Copy sourcesContents of applied map.
        aSourceMapConsumer.sources.forEach(function(srcFile) {
            var content = aSourceMapConsumer.sourceContentFor(srcFile);
            if (content != null) {
                if (aSourceMapPath != null) {
                    srcFile = util$1.join(aSourceMapPath, srcFile);
                }
                if (sourceRoot != null) {
                    srcFile = util$1.relative(sourceRoot, srcFile);
                }
                this.setSourceContent(srcFile, content);
            }
        }, this);
    };
    /**
   * A mapping can have one of the three levels of data:
   *
   *   1. Just the generated position.
   *   2. The Generated position, original position, and original source.
   *   3. Generated and original position, original source, as well as a name
   *      token.
   *
   * To maintain consistency, we validate that any new mapping being added falls
   * in to one of these categories.
   */ _proto._validateMapping = function _validateMapping(aGenerated, aOriginal, aSource, aName) {
        // When aOriginal is truthy but has empty values for .line and .column,
        // it is most likely a programmer error. In this case we throw a very
        // specific error message to try to guide them the right way.
        // For example: https://github.com/Polymer/polymer-bundler/pull/519
        if (aOriginal && typeof aOriginal.line !== "number" && typeof aOriginal.column !== "number") {
            throw new Error("original.line and original.column are not numbers -- you probably meant to omit " + "the original mapping entirely and only map the generated position. If so, pass " + "null for the original mapping instead of an object with empty or null values.");
        }
        if (aGenerated && "line" in aGenerated && "column" in aGenerated && aGenerated.line > 0 && aGenerated.column >= 0 && !aOriginal && !aSource && !aName) ; else if (aGenerated && "line" in aGenerated && "column" in aGenerated && aOriginal && "line" in aOriginal && "column" in aOriginal && aGenerated.line > 0 && aGenerated.column >= 0 && aOriginal.line > 0 && aOriginal.column >= 0 && aSource) ; else {
            throw new Error("Invalid mapping: " + JSON.stringify({
                generated: aGenerated,
                source: aSource,
                original: aOriginal,
                name: aName
            }));
        }
    };
    /**
   * Serialize the accumulated mappings in to the stream of base 64 VLQs
   * specified by the source map format.
   */ _proto._serializeMappings = function _serializeMappings() {
        var previousGeneratedColumn = 0;
        var previousGeneratedLine = 1;
        var previousOriginalColumn = 0;
        var previousOriginalLine = 0;
        var previousName = 0;
        var previousSource = 0;
        var result = "";
        var next;
        var mapping;
        var nameIdx;
        var sourceIdx;
        var mappings = this._mappings.toArray();
        for(var i = 0, len = mappings.length; i < len; i++){
            mapping = mappings[i];
            next = "";
            if (mapping.generatedLine !== previousGeneratedLine) {
                previousGeneratedColumn = 0;
                while(mapping.generatedLine !== previousGeneratedLine){
                    next += ";";
                    previousGeneratedLine++;
                }
            } else if (i > 0) {
                if (!util$1.compareByGeneratedPositionsInflated(mapping, mappings[i - 1])) {
                    continue;
                }
                next += ",";
            }
            next += base64VLQ.encode(mapping.generatedColumn - previousGeneratedColumn);
            previousGeneratedColumn = mapping.generatedColumn;
            if (mapping.source != null) {
                sourceIdx = this._sources.indexOf(mapping.source);
                next += base64VLQ.encode(sourceIdx - previousSource);
                previousSource = sourceIdx;
                // lines are stored 0-based in SourceMap spec version 3
                next += base64VLQ.encode(mapping.originalLine - 1 - previousOriginalLine);
                previousOriginalLine = mapping.originalLine - 1;
                next += base64VLQ.encode(mapping.originalColumn - previousOriginalColumn);
                previousOriginalColumn = mapping.originalColumn;
                if (mapping.name != null) {
                    nameIdx = this._names.indexOf(mapping.name);
                    next += base64VLQ.encode(nameIdx - previousName);
                    previousName = nameIdx;
                }
            }
            result += next;
        }
        return result;
    };
    _proto._generateSourcesContent = function _generateSourcesContent(aSources, aSourceRoot) {
        return aSources.map(function(source) {
            if (!this._sourcesContents) {
                return null;
            }
            if (aSourceRoot != null) {
                source = util$1.relative(aSourceRoot, source);
            }
            var key = util$1.toSetString(source);
            return Object.prototype.hasOwnProperty.call(this._sourcesContents, key) ? this._sourcesContents[key] : null;
        }, this);
    };
    /**
   * Externalize the source map.
   */ _proto.toJSON = function toJSON() {
        var map = {
            version: this._version,
            sources: this._sources.toArray(),
            names: this._names.toArray(),
            mappings: this._serializeMappings()
        };
        if (this._file != null) {
            map.file = this._file;
        }
        if (this._sourceRoot != null) {
            map.sourceRoot = this._sourceRoot;
        }
        if (this._sourcesContents) {
            map.sourcesContent = this._generateSourcesContent(map.sources, map.sourceRoot);
        }
        return map;
    };
    /**
   * Render the source map being generated to a string.
   */ _proto.toString = function toString() {
        return JSON.stringify(this.toJSON());
    };
    /**
   * Creates a new SourceMapGenerator based on a SourceMapConsumer
   *
   * @param aSourceMapConsumer The SourceMap.
   */ SourceMapGenerator.fromSourceMap = function fromSourceMap(aSourceMapConsumer) {
        var sourceRoot = aSourceMapConsumer.sourceRoot;
        var generator = new SourceMapGenerator({
            file: aSourceMapConsumer.file,
            sourceRoot: sourceRoot
        });
        aSourceMapConsumer.eachMapping(function(mapping) {
            var newMapping = {
                generated: {
                    line: mapping.generatedLine,
                    column: mapping.generatedColumn
                }
            };
            if (mapping.source != null) {
                newMapping.source = mapping.source;
                if (sourceRoot != null) {
                    newMapping.source = util$1.relative(sourceRoot, newMapping.source);
                }
                newMapping.original = {
                    line: mapping.originalLine,
                    column: mapping.originalColumn
                };
                if (mapping.name != null) {
                    newMapping.name = mapping.name;
                }
            }
            generator.addMapping(newMapping);
        });
        aSourceMapConsumer.sources.forEach(function(sourceFile) {
            var sourceRelative = sourceFile;
            if (sourceRoot !== null) {
                sourceRelative = util$1.relative(sourceRoot, sourceFile);
            }
            if (!generator._sources.has(sourceRelative)) {
                generator._sources.add(sourceRelative);
            }
            var content = aSourceMapConsumer.sourceContentFor(sourceFile);
            if (content != null) {
                generator.setSourceContent(sourceFile, content);
            }
        });
        return generator;
    };
    return SourceMapGenerator;
}();
SourceMapGenerator$1.prototype._version = 3;
sourceMapGenerator.SourceMapGenerator = SourceMapGenerator$1;

var binarySearch$1 = {};

(function(exports) {
    /*
	 * Copyright 2011 Mozilla Foundation and contributors
	 * Licensed under the New BSD license. See LICENSE or:
	 * http://opensource.org/licenses/BSD-3-Clause
	 */ exports.GREATEST_LOWER_BOUND = 1;
    exports.LEAST_UPPER_BOUND = 2;
    /**
	 * Recursive implementation of binary search.
	 *
	 * @param aLow Indices here and lower do not contain the needle.
	 * @param aHigh Indices here and higher do not contain the needle.
	 * @param aNeedle The element being searched for.
	 * @param aHaystack The non-empty array being searched.
	 * @param aCompare Function which takes two elements and returns -1, 0, or 1.
	 * @param aBias Either 'binarySearch.GREATEST_LOWER_BOUND' or
	 *     'binarySearch.LEAST_UPPER_BOUND'. Specifies whether to return the
	 *     closest element that is smaller than or greater than the one we are
	 *     searching for, respectively, if the exact element cannot be found.
	 */ function recursiveSearch(aLow, aHigh, aNeedle, aHaystack, aCompare, aBias) {
        // This function terminates when one of the following is true:
        //
        //   1. We find the exact element we are looking for.
        //
        //   2. We did not find the exact element, but we can return the index of
        //      the next-closest element.
        //
        //   3. We did not find the exact element, and there is no next-closest
        //      element than the one we are searching for, so we return -1.
        var mid = Math.floor((aHigh - aLow) / 2) + aLow;
        var cmp = aCompare(aNeedle, aHaystack[mid], true);
        if (cmp === 0) {
            // Found the element we are looking for.
            return mid;
        } else if (cmp > 0) {
            // Our needle is greater than aHaystack[mid].
            if (aHigh - mid > 1) {
                // The element is in the upper half.
                return recursiveSearch(mid, aHigh, aNeedle, aHaystack, aCompare, aBias);
            }
            // The exact needle element was not found in this haystack. Determine if
            // we are in termination case (3) or (2) and return the appropriate thing.
            if (aBias == exports.LEAST_UPPER_BOUND) {
                return aHigh < aHaystack.length ? aHigh : -1;
            }
            return mid;
        }
        // Our needle is less than aHaystack[mid].
        if (mid - aLow > 1) {
            // The element is in the lower half.
            return recursiveSearch(aLow, mid, aNeedle, aHaystack, aCompare, aBias);
        }
        // we are in termination case (3) or (2) and return the appropriate thing.
        if (aBias == exports.LEAST_UPPER_BOUND) {
            return mid;
        }
        return aLow < 0 ? -1 : aLow;
    }
    /**
	 * This is an implementation of binary search which will always try and return
	 * the index of the closest element if there is no exact hit. This is because
	 * mappings between original and generated line/col pairs are single points,
	 * and there is an implicit region between each of them, so a miss just means
	 * that you aren't on the very start of a region.
	 *
	 * @param aNeedle The element you are looking for.
	 * @param aHaystack The array that is being searched.
	 * @param aCompare A function which takes the needle and an element in the
	 *     array and returns -1, 0, or 1 depending on whether the needle is less
	 *     than, equal to, or greater than the element, respectively.
	 * @param aBias Either 'binarySearch.GREATEST_LOWER_BOUND' or
	 *     'binarySearch.LEAST_UPPER_BOUND'. Specifies whether to return the
	 *     closest element that is smaller than or greater than the one we are
	 *     searching for, respectively, if the exact element cannot be found.
	 *     Defaults to 'binarySearch.GREATEST_LOWER_BOUND'.
	 */ exports.search = function search(aNeedle, aHaystack, aCompare, aBias) {
        if (aHaystack.length === 0) {
            return -1;
        }
        var index = recursiveSearch(-1, aHaystack.length, aNeedle, aHaystack, aCompare, aBias || exports.GREATEST_LOWER_BOUND);
        if (index < 0) {
            return -1;
        }
        // We have found either the exact element, or the next-closest element than
        // the one we are searching for. However, there may be more than one such
        // element. Make sure we always return the smallest of these.
        while(index - 1 >= 0){
            if (aCompare(aHaystack[index], aHaystack[index - 1], true) !== 0) {
                break;
            }
            --index;
        }
        return index;
    };
})(binarySearch$1);

var readWasm$2 = {exports: {}};

if (typeof fetch === "function") {
    // Web version of reading a wasm file into an array buffer.
    var mappingsWasmUrl = null;
    readWasm$2.exports = function readWasm() {
        if (typeof mappingsWasmUrl !== "string") {
            throw new Error("You must provide the URL of lib/mappings.wasm by calling " + "SourceMapConsumer.initialize({ 'lib/mappings.wasm': ... }) " + "before using SourceMapConsumer");
        }
        return fetch(mappingsWasmUrl).then(function(response) {
            return response.arrayBuffer();
        });
    };
    readWasm$2.exports.initialize = function(url) {
        return mappingsWasmUrl = url;
    };
} else {
    // Node version of reading a wasm file into an array buffer.
    var fs = require$$0__default["default"];
    var path = require$$1__default["default"];
    readWasm$2.exports = function readWasm() {
        return new Promise(function(resolve, reject) {
            var wasmPath = path.join(__dirname, "mappings.wasm");
            fs.readFile(wasmPath, null, function(error, data) {
                if (error) {
                    reject(error);
                    return;
                }
                resolve(data.buffer);
            });
        });
    };
    readWasm$2.exports.initialize = function(_) {
        console.debug("SourceMapConsumer.initialize is a no-op when running in node.js");
    };
}

var readWasm$1 = readWasm$2.exports;
/**
 * Provide the JIT with a nice shape / hidden class.
 */ function Mapping() {
    this.generatedLine = 0;
    this.generatedColumn = 0;
    this.lastGeneratedColumn = null;
    this.source = null;
    this.originalLine = null;
    this.originalColumn = null;
    this.name = null;
}
var cachedWasm = null;
var wasm$1 = function wasm() {
    if (cachedWasm) {
        return cachedWasm;
    }
    var callbackStack = [];
    cachedWasm = readWasm$1().then(function(buffer) {
        return WebAssembly.instantiate(buffer, {
            env: {
                mapping_callback: function mapping_callback(generatedLine, generatedColumn, hasLastGeneratedColumn, lastGeneratedColumn, hasOriginal, source, originalLine, originalColumn, hasName, name) {
                    var mapping = new Mapping();
                    // JS uses 1-based line numbers, wasm uses 0-based.
                    mapping.generatedLine = generatedLine + 1;
                    mapping.generatedColumn = generatedColumn;
                    if (hasLastGeneratedColumn) {
                        // JS uses inclusive last generated column, wasm uses exclusive.
                        mapping.lastGeneratedColumn = lastGeneratedColumn - 1;
                    }
                    if (hasOriginal) {
                        mapping.source = source;
                        // JS uses 1-based line numbers, wasm uses 0-based.
                        mapping.originalLine = originalLine + 1;
                        mapping.originalColumn = originalColumn;
                        if (hasName) {
                            mapping.name = name;
                        }
                    }
                    callbackStack[callbackStack.length - 1](mapping);
                },
                start_all_generated_locations_for: function start_all_generated_locations_for() {
                    console.time("all_generated_locations_for");
                },
                end_all_generated_locations_for: function end_all_generated_locations_for() {
                    console.timeEnd("all_generated_locations_for");
                },
                start_compute_column_spans: function start_compute_column_spans() {
                    console.time("compute_column_spans");
                },
                end_compute_column_spans: function end_compute_column_spans() {
                    console.timeEnd("compute_column_spans");
                },
                start_generated_location_for: function start_generated_location_for() {
                    console.time("generated_location_for");
                },
                end_generated_location_for: function end_generated_location_for() {
                    console.timeEnd("generated_location_for");
                },
                start_original_location_for: function start_original_location_for() {
                    console.time("original_location_for");
                },
                end_original_location_for: function end_original_location_for() {
                    console.timeEnd("original_location_for");
                },
                start_parse_mappings: function start_parse_mappings() {
                    console.time("parse_mappings");
                },
                end_parse_mappings: function end_parse_mappings() {
                    console.timeEnd("parse_mappings");
                },
                start_sort_by_generated_location: function start_sort_by_generated_location() {
                    console.time("sort_by_generated_location");
                },
                end_sort_by_generated_location: function end_sort_by_generated_location() {
                    console.timeEnd("sort_by_generated_location");
                },
                start_sort_by_original_location: function start_sort_by_original_location() {
                    console.time("sort_by_original_location");
                },
                end_sort_by_original_location: function end_sort_by_original_location() {
                    console.timeEnd("sort_by_original_location");
                }
            }
        });
    }).then(function(Wasm) {
        return {
            exports: Wasm.instance.exports,
            withMappingCallback: function(mappingCallback, f) {
                callbackStack.push(mappingCallback);
                try {
                    f();
                } finally{
                    callbackStack.pop();
                }
            }
        };
    }).then(null, function(e) {
        cachedWasm = null;
        throw e;
    });
    return cachedWasm;
};

/* -*- Mode: js; js-indent-level: 2; -*- */ function _assertThisInitialized(self) {
    if (self === void 0) {
        throw new ReferenceError("this hasn't been initialised - super() hasn't been called");
    }
    return self;
}
function _defineProperties(target, props) {
    for(var i = 0; i < props.length; i++){
        var descriptor = props[i];
        descriptor.enumerable = descriptor.enumerable || false;
        descriptor.configurable = true;
        if ("value" in descriptor) descriptor.writable = true;
        Object.defineProperty(target, descriptor.key, descriptor);
    }
}
function _createClass(Constructor, protoProps, staticProps) {
    if (protoProps) _defineProperties(Constructor.prototype, protoProps);
    if (staticProps) _defineProperties(Constructor, staticProps);
    return Constructor;
}
function _inherits(subClass, superClass) {
    if (typeof superClass !== "function" && superClass !== null) {
        throw new TypeError("Super expression must either be null or a function");
    }
    subClass.prototype = Object.create(superClass && superClass.prototype, {
        constructor: {
            value: subClass,
            writable: true,
            configurable: true
        }
    });
    if (superClass) _setPrototypeOf(subClass, superClass);
}
function _possibleConstructorReturn(self, call) {
    if (call && (_typeof$1(call) === "object" || typeof call === "function")) {
        return call;
    }
    return _assertThisInitialized(self);
}
function _setPrototypeOf(o, p) {
    _setPrototypeOf = Object.setPrototypeOf || function _setPrototypeOf(o, p) {
        o.__proto__ = p;
        return o;
    };
    return _setPrototypeOf(o, p);
}
var _typeof$1 = function(obj) {
    "@swc/helpers - typeof";
    return obj && typeof Symbol !== "undefined" && obj.constructor === Symbol ? "symbol" : typeof obj;
};
/*
 * Copyright 2011 Mozilla Foundation and contributors
 * Licensed under the New BSD license. See LICENSE or:
 * http://opensource.org/licenses/BSD-3-Clause
 */ var util = util$3;
var binarySearch = binarySearch$1;
var ArraySet = arraySet.ArraySet;
var readWasm = readWasm$2.exports;
var wasm = wasm$1;
var INTERNAL = Symbol("smcInternal");
var SourceMapConsumer = /*#__PURE__*/ function() {
    function SourceMapConsumer(aSourceMap, aSourceMapURL) {
        // If the constructor was called by super(), just return Promise<this>.
        // Yes, this is a hack to retain the pre-existing API of the base-class
        // constructor also being an async factory function.
        if (aSourceMap == INTERNAL) {
            return Promise.resolve(this);
        }
        return _factory(aSourceMap, aSourceMapURL);
    }
    var _proto = SourceMapConsumer.prototype;
    /**
   * Parse the mappings in a string in to a data structure which we can easily
   * query (the ordered arrays in the `this.__generatedMappings` and
   * `this.__originalMappings` properties).
   */ _proto._parseMappings = function _parseMappings(aStr, aSourceRoot) {
        throw new Error("Subclasses must implement _parseMappings");
    };
    /**
   * Iterate over each mapping between an original source/line/column and a
   * generated line/column in this source map.
   *
   * @param Function aCallback
   *        The function that is called with each mapping.
   * @param Object aContext
   *        Optional. If specified, this object will be the value of `this` every
   *        time that `aCallback` is called.
   * @param aOrder
   *        Either `SourceMapConsumer.GENERATED_ORDER` or
   *        `SourceMapConsumer.ORIGINAL_ORDER`. Specifies whether you want to
   *        iterate over the mappings sorted by the generated file's line/column
   *        order or the original's source/line/column order, respectively. Defaults to
   *        `SourceMapConsumer.GENERATED_ORDER`.
   */ _proto.eachMapping = function eachMapping(aCallback, aContext, aOrder) {
        throw new Error("Subclasses must implement eachMapping");
    };
    /**
   * Returns all generated line and column information for the original source,
   * line, and column provided. If no column is provided, returns all mappings
   * corresponding to a either the line we are searching for or the next
   * closest line that has any mappings. Otherwise, returns all mappings
   * corresponding to the given line and either the column we are searching for
   * or the next closest column that has any offsets.
   *
   * The only argument is an object with the following properties:
   *
   *   - source: The filename of the original source.
   *   - line: The line number in the original source.  The line number is 1-based.
   *   - column: Optional. the column number in the original source.
   *    The column number is 0-based.
   *
   * and an array of objects is returned, each with the following properties:
   *
   *   - line: The line number in the generated source, or null.  The
   *    line number is 1-based.
   *   - column: The column number in the generated source, or null.
   *    The column number is 0-based.
   */ _proto.allGeneratedPositionsFor = function allGeneratedPositionsFor(aArgs) {
        throw new Error("Subclasses must implement allGeneratedPositionsFor");
    };
    _proto.destroy = function destroy() {
        throw new Error("Subclasses must implement destroy");
    };
    SourceMapConsumer.initialize = function initialize(opts) {
        readWasm.initialize(opts["lib/mappings.wasm"]);
    };
    SourceMapConsumer.fromSourceMap = function fromSourceMap(aSourceMap, aSourceMapURL) {
        return _factoryBSM(aSourceMap, aSourceMapURL);
    };
    /**
   * Construct a new `SourceMapConsumer` from `rawSourceMap` and `sourceMapUrl`
   * (see the `SourceMapConsumer` constructor for details. Then, invoke the `async
   * function f(SourceMapConsumer) -> T` with the newly constructed consumer, wait
   * for `f` to complete, call `destroy` on the consumer, and return `f`'s return
   * value.
   *
   * You must not use the consumer after `f` completes!
   *
   * By using `with`, you do not have to remember to manually call `destroy` on
   * the consumer, since it will be called automatically once `f` completes.
   *
   * ```js
   * const xSquared = await SourceMapConsumer.with(
   *   myRawSourceMap,
   *   null,
   *   async function (consumer) {
   *     // Use `consumer` inside here and don't worry about remembering
   *     // to call `destroy`.
   *
   *     const x = await whatever(consumer);
   *     return x * x;
   *   }
   * );
   *
   * // You may not use that `consumer` anymore out here; it has
   * // been destroyed. But you can use `xSquared`.
   * console.log(xSquared);
   * ```
   */ SourceMapConsumer.with = function _with(rawSourceMap, sourceMapUrl, f) {
        // Note: The `acorn` version that `webpack` currently depends on doesn't
        // support `async` functions, and the nodes that we support don't all have
        // `.finally`. Therefore, this is written a bit more convolutedly than it
        // should really be.
        var consumer = null;
        var promise = new SourceMapConsumer(rawSourceMap, sourceMapUrl);
        return promise.then(function(c) {
            consumer = c;
            return f(c);
        }).then(function(x) {
            if (consumer) {
                consumer.destroy();
            }
            return x;
        }, function(e) {
            if (consumer) {
                consumer.destroy();
            }
            throw e;
        });
    };
    return SourceMapConsumer;
}();
/**
 * The version of the source mapping spec that we are consuming.
 */ SourceMapConsumer.prototype._version = 3;
SourceMapConsumer.GENERATED_ORDER = 1;
SourceMapConsumer.ORIGINAL_ORDER = 2;
SourceMapConsumer.GREATEST_LOWER_BOUND = 1;
SourceMapConsumer.LEAST_UPPER_BOUND = 2;
/**
 * A BasicSourceMapConsumer instance represents a parsed source map which we can
 * query for information about the original file positions by giving it a file
 * position in the generated source.
 *
 * The first parameter is the raw source map (either as a JSON string, or
 * already parsed to an object). According to the spec, source maps have the
 * following attributes:
 *
 *   - version: Which version of the source map spec this map is following.
 *   - sources: An array of URLs to the original source files.
 *   - names: An array of identifiers which can be referenced by individual mappings.
 *   - sourceRoot: Optional. The URL root from which all sources are relative.
 *   - sourcesContent: Optional. An array of contents of the original source files.
 *   - mappings: A string of base64 VLQs which contain the actual mappings.
 *   - file: Optional. The generated file this source map is associated with.
 *
 * Here is an example source map, taken from the source map spec[0]:
 *
 *     {
 *       version : 3,
 *       file: "out.js",
 *       sourceRoot : "",
 *       sources: ["foo.js", "bar.js"],
 *       names: ["src", "maps", "are", "fun"],
 *       mappings: "AA,AB;;ABCDE;"
 *     }
 *
 * The second parameter, if given, is a string whose value is the URL
 * at which the source map was found.  This URL is used to compute the
 * sources array.
 *
 * [0]: https://docs.google.com/document/d/1U1RGAehQwRypUTovF1KRlpiOFze0b-_2gc6fAH0KY0k/edit?pli=1#
 */ var BasicSourceMapConsumer = /*#__PURE__*/ function(SourceMapConsumer1) {
    _inherits(BasicSourceMapConsumer, SourceMapConsumer1);
    function BasicSourceMapConsumer(aSourceMap, aSourceMapURL) {
        var _this;
        return _possibleConstructorReturn(_this, (_this = SourceMapConsumer1.call(this, INTERNAL) || this).then(function(that) {
            var sourceMap = aSourceMap;
            if (typeof aSourceMap === "string") {
                sourceMap = util.parseSourceMapInput(aSourceMap);
            }
            var version = util.getArg(sourceMap, "version");
            var sources = util.getArg(sourceMap, "sources");
            // Sass 3.3 leaves out the 'names' array, so we deviate from the spec (which
            // requires the array) to play nice here.
            var names = util.getArg(sourceMap, "names", []);
            var sourceRoot = util.getArg(sourceMap, "sourceRoot", null);
            var sourcesContent = util.getArg(sourceMap, "sourcesContent", null);
            var mappings = util.getArg(sourceMap, "mappings");
            var file = util.getArg(sourceMap, "file", null);
            // Once again, Sass deviates from the spec and supplies the version as a
            // string rather than a number, so we use loose equality checking here.
            if (version != that._version) {
                throw new Error("Unsupported version: " + version);
            }
            if (sourceRoot) {
                sourceRoot = util.normalize(sourceRoot);
            }
            sources = sources.map(String)// Some source maps produce relative source paths like "./foo.js" instead of
            // "foo.js".  Normalize these first so that future comparisons will succeed.
            // See bugzil.la/1090768.
            .map(util.normalize)// Always ensure that absolute sources are internally stored relative to
            // the source root, if the source root is absolute. Not doing this would
            // be particularly problematic when the source root is a prefix of the
            // source (valid, but why??). See github issue #199 and bugzil.la/1188982.
            .map(function(source) {
                return sourceRoot && util.isAbsolute(sourceRoot) && util.isAbsolute(source) ? util.relative(sourceRoot, source) : source;
            });
            // Pass `true` below to allow duplicate names and sources. While source maps
            // are intended to be compressed and deduplicated, the TypeScript compiler
            // sometimes generates source maps with duplicates in them. See Github issue
            // #72 and bugzil.la/889492.
            that._names = ArraySet.fromArray(names.map(String), true);
            that._sources = ArraySet.fromArray(sources, true);
            that._absoluteSources = that._sources.toArray().map(function(s) {
                return util.computeSourceURL(sourceRoot, s, aSourceMapURL);
            });
            that.sourceRoot = sourceRoot;
            that.sourcesContent = sourcesContent;
            that._mappings = mappings;
            that._sourceMapURL = aSourceMapURL;
            that.file = file;
            that._computedColumnSpans = false;
            that._mappingsPtr = 0;
            that._wasm = null;
            return wasm().then(function(w) {
                that._wasm = w;
                return that;
            });
        }));
    }
    var _proto = BasicSourceMapConsumer.prototype;
    /**
   * Utility function to find the index of a source.  Returns -1 if not
   * found.
   */ _proto._findSourceIndex = function _findSourceIndex(aSource) {
        var relativeSource = aSource;
        if (this.sourceRoot != null) {
            relativeSource = util.relative(this.sourceRoot, relativeSource);
        }
        if (this._sources.has(relativeSource)) {
            return this._sources.indexOf(relativeSource);
        }
        // Maybe aSource is an absolute URL as returned by |sources|.  In
        // this case we can't simply undo the transform.
        for(var i = 0; i < this._absoluteSources.length; ++i){
            if (this._absoluteSources[i] == aSource) {
                return i;
            }
        }
        return -1;
    };
    _proto._getMappingsPtr = function _getMappingsPtr() {
        if (this._mappingsPtr === 0) {
            this._parseMappings(this._mappings, this.sourceRoot);
        }
        return this._mappingsPtr;
    };
    /**
   * Parse the mappings in a string in to a data structure which we can easily
   * query (the ordered arrays in the `this.__generatedMappings` and
   * `this.__originalMappings` properties).
   */ _proto._parseMappings = function _parseMappings(aStr, aSourceRoot) {
        var size = aStr.length;
        var mappingsBufPtr = this._wasm.exports.allocate_mappings(size);
        var mappingsBuf = new Uint8Array(this._wasm.exports.memory.buffer, mappingsBufPtr, size);
        for(var i = 0; i < size; i++){
            mappingsBuf[i] = aStr.charCodeAt(i);
        }
        var mappingsPtr = this._wasm.exports.parse_mappings(mappingsBufPtr);
        if (!mappingsPtr) {
            var error = this._wasm.exports.get_last_error();
            var msg = "Error parsing mappings (code " + error + "): ";
            // XXX: keep these error codes in sync with `fitzgen/source-map-mappings`.
            switch(error){
                case 1:
                    msg += "the mappings contained a negative line, column, source index, or name index";
                    break;
                case 2:
                    msg += "the mappings contained a number larger than 2**32";
                    break;
                case 3:
                    msg += "reached EOF while in the middle of parsing a VLQ";
                    break;
                case 4:
                    msg += "invalid base 64 character while parsing a VLQ";
                    break;
                default:
                    msg += "unknown error code";
                    break;
            }
            throw new Error(msg);
        }
        this._mappingsPtr = mappingsPtr;
    };
    _proto.eachMapping = function eachMapping(aCallback, aContext, aOrder) {
        var _this = this;
        var context = aContext || null;
        var order = aOrder || SourceMapConsumer.GENERATED_ORDER;
        var sourceRoot = this.sourceRoot;
        this._wasm.withMappingCallback(function(mapping) {
            if (mapping.source !== null) {
                mapping.source = _this._sources.at(mapping.source);
                mapping.source = util.computeSourceURL(sourceRoot, mapping.source, _this._sourceMapURL);
                if (mapping.name !== null) {
                    mapping.name = _this._names.at(mapping.name);
                }
            }
            aCallback.call(context, mapping);
        }, function() {
            switch(order){
                case SourceMapConsumer.GENERATED_ORDER:
                    _this._wasm.exports.by_generated_location(_this._getMappingsPtr());
                    break;
                case SourceMapConsumer.ORIGINAL_ORDER:
                    _this._wasm.exports.by_original_location(_this._getMappingsPtr());
                    break;
                default:
                    throw new Error("Unknown order of iteration.");
            }
        });
    };
    _proto.allGeneratedPositionsFor = function allGeneratedPositionsFor(aArgs) {
        var _this = this;
        var source = util.getArg(aArgs, "source");
        var originalLine = util.getArg(aArgs, "line");
        var originalColumn = aArgs.column || 0;
        source = this._findSourceIndex(source);
        if (source < 0) {
            return [];
        }
        if (originalLine < 1) {
            throw new Error("Line numbers must be >= 1");
        }
        if (originalColumn < 0) {
            throw new Error("Column numbers must be >= 0");
        }
        var mappings = [];
        this._wasm.withMappingCallback(function(m) {
            var lastColumn = m.lastGeneratedColumn;
            if (_this._computedColumnSpans && lastColumn === null) {
                lastColumn = Infinity;
            }
            mappings.push({
                line: m.generatedLine,
                column: m.generatedColumn,
                lastColumn: lastColumn
            });
        }, function() {
            _this._wasm.exports.all_generated_locations_for(_this._getMappingsPtr(), source, originalLine - 1, "column" in aArgs, originalColumn);
        });
        return mappings;
    };
    _proto.destroy = function destroy() {
        if (this._mappingsPtr !== 0) {
            this._wasm.exports.free_mappings(this._mappingsPtr);
            this._mappingsPtr = 0;
        }
    };
    /**
   * Compute the last column for each generated mapping. The last column is
   * inclusive.
   */ _proto.computeColumnSpans = function computeColumnSpans() {
        if (this._computedColumnSpans) {
            return;
        }
        this._wasm.exports.compute_column_spans(this._getMappingsPtr());
        this._computedColumnSpans = true;
    };
    /**
   * Returns the original source, line, and column information for the generated
   * source's line and column positions provided. The only argument is an object
   * with the following properties:
   *
   *   - line: The line number in the generated source.  The line number
   *     is 1-based.
   *   - column: The column number in the generated source.  The column
   *     number is 0-based.
   *   - bias: Either 'SourceMapConsumer.GREATEST_LOWER_BOUND' or
   *     'SourceMapConsumer.LEAST_UPPER_BOUND'. Specifies whether to return the
   *     closest element that is smaller than or greater than the one we are
   *     searching for, respectively, if the exact element cannot be found.
   *     Defaults to 'SourceMapConsumer.GREATEST_LOWER_BOUND'.
   *
   * and an object is returned with the following properties:
   *
   *   - source: The original source file, or null.
   *   - line: The line number in the original source, or null.  The
   *     line number is 1-based.
   *   - column: The column number in the original source, or null.  The
   *     column number is 0-based.
   *   - name: The original identifier, or null.
   */ _proto.originalPositionFor = function originalPositionFor(aArgs) {
        var _this = this;
        var needle = {
            generatedLine: util.getArg(aArgs, "line"),
            generatedColumn: util.getArg(aArgs, "column")
        };
        if (needle.generatedLine < 1) {
            throw new Error("Line numbers must be >= 1");
        }
        if (needle.generatedColumn < 0) {
            throw new Error("Column numbers must be >= 0");
        }
        var bias = util.getArg(aArgs, "bias", SourceMapConsumer.GREATEST_LOWER_BOUND);
        if (bias == null) {
            bias = SourceMapConsumer.GREATEST_LOWER_BOUND;
        }
        var mapping;
        this._wasm.withMappingCallback(function(m) {
            return mapping = m;
        }, function() {
            _this._wasm.exports.original_location_for(_this._getMappingsPtr(), needle.generatedLine - 1, needle.generatedColumn, bias);
        });
        if (mapping) {
            if (mapping.generatedLine === needle.generatedLine) {
                var source = util.getArg(mapping, "source", null);
                if (source !== null) {
                    source = this._sources.at(source);
                    source = util.computeSourceURL(this.sourceRoot, source, this._sourceMapURL);
                }
                var name = util.getArg(mapping, "name", null);
                if (name !== null) {
                    name = this._names.at(name);
                }
                return {
                    source: source,
                    line: util.getArg(mapping, "originalLine", null),
                    column: util.getArg(mapping, "originalColumn", null),
                    name: name
                };
            }
        }
        return {
            source: null,
            line: null,
            column: null,
            name: null
        };
    };
    /**
   * Return true if we have the source content for every source in the source
   * map, false otherwise.
   */ _proto.hasContentsOfAllSources = function hasContentsOfAllSources() {
        if (!this.sourcesContent) {
            return false;
        }
        return this.sourcesContent.length >= this._sources.size() && !this.sourcesContent.some(function(sc) {
            return sc == null;
        });
    };
    /**
   * Returns the original source content. The only argument is the url of the
   * original source file. Returns null if no original source content is
   * available.
   */ _proto.sourceContentFor = function sourceContentFor(aSource, nullOnMissing) {
        if (!this.sourcesContent) {
            return null;
        }
        var index = this._findSourceIndex(aSource);
        if (index >= 0) {
            return this.sourcesContent[index];
        }
        var relativeSource = aSource;
        if (this.sourceRoot != null) {
            relativeSource = util.relative(this.sourceRoot, relativeSource);
        }
        var url;
        if (this.sourceRoot != null && (url = util.urlParse(this.sourceRoot))) {
            // XXX: file:// URIs and absolute paths lead to unexpected behavior for
            // many users. We can help them out when they expect file:// URIs to
            // behave like it would if they were running a local HTTP server. See
            // https://bugzilla.mozilla.org/show_bug.cgi?id=885597.
            var fileUriAbsPath = relativeSource.replace(/^file:\/\//, "");
            if (url.scheme == "file" && this._sources.has(fileUriAbsPath)) {
                return this.sourcesContent[this._sources.indexOf(fileUriAbsPath)];
            }
            if ((!url.path || url.path == "/") && this._sources.has("/" + relativeSource)) {
                return this.sourcesContent[this._sources.indexOf("/" + relativeSource)];
            }
        }
        // This function is used recursively from
        // IndexedSourceMapConsumer.prototype.sourceContentFor. In that case, we
        // don't want to throw if we can't find the source - we just want to
        // return null, so we provide a flag to exit gracefully.
        if (nullOnMissing) {
            return null;
        }
        throw new Error('"' + relativeSource + '" is not in the SourceMap.');
    };
    /**
   * Returns the generated line and column information for the original source,
   * line, and column positions provided. The only argument is an object with
   * the following properties:
   *
   *   - source: The filename of the original source.
   *   - line: The line number in the original source.  The line number
   *     is 1-based.
   *   - column: The column number in the original source.  The column
   *     number is 0-based.
   *   - bias: Either 'SourceMapConsumer.GREATEST_LOWER_BOUND' or
   *     'SourceMapConsumer.LEAST_UPPER_BOUND'. Specifies whether to return the
   *     closest element that is smaller than or greater than the one we are
   *     searching for, respectively, if the exact element cannot be found.
   *     Defaults to 'SourceMapConsumer.GREATEST_LOWER_BOUND'.
   *
   * and an object is returned with the following properties:
   *
   *   - line: The line number in the generated source, or null.  The
   *     line number is 1-based.
   *   - column: The column number in the generated source, or null.
   *     The column number is 0-based.
   */ _proto.generatedPositionFor = function generatedPositionFor(aArgs) {
        var _this = this;
        var source = util.getArg(aArgs, "source");
        source = this._findSourceIndex(source);
        if (source < 0) {
            return {
                line: null,
                column: null,
                lastColumn: null
            };
        }
        var needle = {
            source: source,
            originalLine: util.getArg(aArgs, "line"),
            originalColumn: util.getArg(aArgs, "column")
        };
        if (needle.originalLine < 1) {
            throw new Error("Line numbers must be >= 1");
        }
        if (needle.originalColumn < 0) {
            throw new Error("Column numbers must be >= 0");
        }
        var bias = util.getArg(aArgs, "bias", SourceMapConsumer.GREATEST_LOWER_BOUND);
        if (bias == null) {
            bias = SourceMapConsumer.GREATEST_LOWER_BOUND;
        }
        var mapping;
        this._wasm.withMappingCallback(function(m) {
            return mapping = m;
        }, function() {
            _this._wasm.exports.generated_location_for(_this._getMappingsPtr(), needle.source, needle.originalLine - 1, needle.originalColumn, bias);
        });
        if (mapping) {
            if (mapping.source === needle.source) {
                var lastColumn = mapping.lastGeneratedColumn;
                if (this._computedColumnSpans && lastColumn === null) {
                    lastColumn = Infinity;
                }
                return {
                    line: util.getArg(mapping, "generatedLine", null),
                    column: util.getArg(mapping, "generatedColumn", null),
                    lastColumn: lastColumn
                };
            }
        }
        return {
            line: null,
            column: null,
            lastColumn: null
        };
    };
    /**
   * Create a BasicSourceMapConsumer from a SourceMapGenerator.
   *
   * @param SourceMapGenerator aSourceMap
   *        The source map that will be consumed.
   * @param String aSourceMapURL
   *        The URL at which the source map can be found (optional)
   * @returns BasicSourceMapConsumer
   */ BasicSourceMapConsumer.fromSourceMap = function fromSourceMap(aSourceMap, aSourceMapURL) {
        return new BasicSourceMapConsumer(aSourceMap.toString());
    };
    _createClass(BasicSourceMapConsumer, [
        {
            key: "sources",
            get: function get() {
                return this._absoluteSources.slice();
            }
        }
    ]);
    return BasicSourceMapConsumer;
}(SourceMapConsumer);
BasicSourceMapConsumer.prototype.consumer = SourceMapConsumer;
/**
 * An IndexedSourceMapConsumer instance represents a parsed source map which
 * we can query for information. It differs from BasicSourceMapConsumer in
 * that it takes "indexed" source maps (i.e. ones with a "sections" field) as
 * input.
 *
 * The first parameter is a raw source map (either as a JSON string, or already
 * parsed to an object). According to the spec for indexed source maps, they
 * have the following attributes:
 *
 *   - version: Which version of the source map spec this map is following.
 *   - file: Optional. The generated file this source map is associated with.
 *   - sections: A list of section definitions.
 *
 * Each value under the "sections" field has two fields:
 *   - offset: The offset into the original specified at which this section
 *       begins to apply, defined as an object with a "line" and "column"
 *       field.
 *   - map: A source map definition. This source map could also be indexed,
 *       but doesn't have to be.
 *
 * Instead of the "map" field, it's also possible to have a "url" field
 * specifying a URL to retrieve a source map from, but that's currently
 * unsupported.
 *
 * Here's an example source map, taken from the source map spec[0], but
 * modified to omit a section which uses the "url" field.
 *
 *  {
 *    version : 3,
 *    file: "app.js",
 *    sections: [{
 *      offset: {line:100, column:10},
 *      map: {
 *        version : 3,
 *        file: "section.js",
 *        sources: ["foo.js", "bar.js"],
 *        names: ["src", "maps", "are", "fun"],
 *        mappings: "AAAA,E;;ABCDE;"
 *      }
 *    }],
 *  }
 *
 * The second parameter, if given, is a string whose value is the URL
 * at which the source map was found.  This URL is used to compute the
 * sources array.
 *
 * [0]: https://docs.google.com/document/d/1U1RGAehQwRypUTovF1KRlpiOFze0b-_2gc6fAH0KY0k/edit#heading=h.535es3xeprgt
 */ var IndexedSourceMapConsumer = /*#__PURE__*/ function(SourceMapConsumer1) {
    _inherits(IndexedSourceMapConsumer, SourceMapConsumer1);
    function IndexedSourceMapConsumer(aSourceMap, aSourceMapURL) {
        var _this;
        return _possibleConstructorReturn(_this, (_this = SourceMapConsumer1.call(this, INTERNAL) || this).then(function(that) {
            var sourceMap = aSourceMap;
            if (typeof aSourceMap === "string") {
                sourceMap = util.parseSourceMapInput(aSourceMap);
            }
            var version = util.getArg(sourceMap, "version");
            var sections = util.getArg(sourceMap, "sections");
            if (version != that._version) {
                throw new Error("Unsupported version: " + version);
            }
            that._sources = new ArraySet();
            that._names = new ArraySet();
            that.__generatedMappings = null;
            that.__originalMappings = null;
            that.__generatedMappingsUnsorted = null;
            that.__originalMappingsUnsorted = null;
            var lastOffset = {
                line: -1,
                column: 0
            };
            return Promise.all(sections.map(function(s) {
                if (s.url) {
                    // The url field will require support for asynchronicity.
                    // See https://github.com/mozilla/source-map/issues/16
                    throw new Error("Support for url field in sections not implemented.");
                }
                var offset = util.getArg(s, "offset");
                var offsetLine = util.getArg(offset, "line");
                var offsetColumn = util.getArg(offset, "column");
                if (offsetLine < lastOffset.line || offsetLine === lastOffset.line && offsetColumn < lastOffset.column) {
                    throw new Error("Section offsets must be ordered and non-overlapping.");
                }
                lastOffset = offset;
                var cons = new SourceMapConsumer(util.getArg(s, "map"), aSourceMapURL);
                return cons.then(function(consumer) {
                    return {
                        generatedOffset: {
                            // The offset fields are 0-based, but we use 1-based indices when
                            // encoding/decoding from VLQ.
                            generatedLine: offsetLine + 1,
                            generatedColumn: offsetColumn + 1
                        },
                        consumer: consumer
                    };
                });
            })).then(function(s) {
                that._sections = s;
                return that;
            });
        }));
    }
    var _proto = IndexedSourceMapConsumer.prototype;
    _proto._sortGeneratedMappings = function _sortGeneratedMappings() {
        var mappings = this._generatedMappingsUnsorted;
        mappings.sort(util.compareByGeneratedPositionsDeflated);
        this.__generatedMappings = mappings;
    };
    _proto._sortOriginalMappings = function _sortOriginalMappings() {
        var mappings = this._originalMappingsUnsorted;
        mappings.sort(util.compareByOriginalPositions);
        this.__originalMappings = mappings;
    };
    /**
   * Returns the original source, line, and column information for the generated
   * source's line and column positions provided. The only argument is an object
   * with the following properties:
   *
   *   - line: The line number in the generated source.  The line number
   *     is 1-based.
   *   - column: The column number in the generated source.  The column
   *     number is 0-based.
   *
   * and an object is returned with the following properties:
   *
   *   - source: The original source file, or null.
   *   - line: The line number in the original source, or null.  The
   *     line number is 1-based.
   *   - column: The column number in the original source, or null.  The
   *     column number is 0-based.
   *   - name: The original identifier, or null.
   */ _proto.originalPositionFor = function originalPositionFor(aArgs) {
        var needle = {
            generatedLine: util.getArg(aArgs, "line"),
            generatedColumn: util.getArg(aArgs, "column")
        };
        // Find the section containing the generated position we're trying to map
        // to an original position.
        var sectionIndex = binarySearch.search(needle, this._sections, function(aNeedle, section) {
            var cmp = aNeedle.generatedLine - section.generatedOffset.generatedLine;
            if (cmp) {
                return cmp;
            }
            return aNeedle.generatedColumn - section.generatedOffset.generatedColumn;
        });
        var section = this._sections[sectionIndex];
        if (!section) {
            return {
                source: null,
                line: null,
                column: null,
                name: null
            };
        }
        return section.consumer.originalPositionFor({
            line: needle.generatedLine - (section.generatedOffset.generatedLine - 1),
            column: needle.generatedColumn - (section.generatedOffset.generatedLine === needle.generatedLine ? section.generatedOffset.generatedColumn - 1 : 0),
            bias: aArgs.bias
        });
    };
    /**
   * Return true if we have the source content for every source in the source
   * map, false otherwise.
   */ _proto.hasContentsOfAllSources = function hasContentsOfAllSources() {
        return this._sections.every(function(s) {
            return s.consumer.hasContentsOfAllSources();
        });
    };
    /**
   * Returns the original source content. The only argument is the url of the
   * original source file. Returns null if no original source content is
   * available.
   */ _proto.sourceContentFor = function sourceContentFor(aSource, nullOnMissing) {
        for(var i = 0; i < this._sections.length; i++){
            var section = this._sections[i];
            var content = section.consumer.sourceContentFor(aSource, true);
            if (content) {
                return content;
            }
        }
        if (nullOnMissing) {
            return null;
        }
        throw new Error('"' + aSource + '" is not in the SourceMap.');
    };
    /**
   * Returns the generated line and column information for the original source,
   * line, and column positions provided. The only argument is an object with
   * the following properties:
   *
   *   - source: The filename of the original source.
   *   - line: The line number in the original source.  The line number
   *     is 1-based.
   *   - column: The column number in the original source.  The column
   *     number is 0-based.
   *
   * and an object is returned with the following properties:
   *
   *   - line: The line number in the generated source, or null.  The
   *     line number is 1-based.
   *   - column: The column number in the generated source, or null.
   *     The column number is 0-based.
   */ _proto.generatedPositionFor = function generatedPositionFor(aArgs) {
        for(var i = 0; i < this._sections.length; i++){
            var section = this._sections[i];
            // Only consider this section if the requested source is in the list of
            // sources of the consumer.
            if (section.consumer._findSourceIndex(util.getArg(aArgs, "source")) === -1) {
                continue;
            }
            var generatedPosition = section.consumer.generatedPositionFor(aArgs);
            if (generatedPosition) {
                var ret = {
                    line: generatedPosition.line + (section.generatedOffset.generatedLine - 1),
                    column: generatedPosition.column + (section.generatedOffset.generatedLine === generatedPosition.line ? section.generatedOffset.generatedColumn - 1 : 0)
                };
                return ret;
            }
        }
        return {
            line: null,
            column: null
        };
    };
    /**
   * Parse the mappings in a string in to a data structure which we can easily
   * query (the ordered arrays in the `this.__generatedMappings` and
   * `this.__originalMappings` properties).
   */ _proto._parseMappings = function _parseMappings(aStr, aSourceRoot) {
        var _this = this, _loop = function(i) {
            var section = _this._sections[i];
            var sectionMappings = [];
            section.consumer.eachMapping(function(m) {
                return sectionMappings.push(m);
            });
            for(var j = 0; j < sectionMappings.length; j++){
                var mapping = sectionMappings[j];
                // TODO: test if null is correct here.  The original code used
                // `source`, which would actually have gotten used as null because
                // var's get hoisted.
                // See: https://github.com/mozilla/source-map/issues/333
                var source = util.computeSourceURL(section.consumer.sourceRoot, null, _this._sourceMapURL);
                _this._sources.add(source);
                source = _this._sources.indexOf(source);
                var name = null;
                if (mapping.name) {
                    _this._names.add(mapping.name);
                    name = _this._names.indexOf(mapping.name);
                }
                // The mappings coming from the consumer for the section have
                // generated positions relative to the start of the section, so we
                // need to offset them to be relative to the start of the concatenated
                // generated file.
                var adjustedMapping = {
                    source: source,
                    generatedLine: mapping.generatedLine + (section.generatedOffset.generatedLine - 1),
                    generatedColumn: mapping.generatedColumn + (section.generatedOffset.generatedLine === mapping.generatedLine ? section.generatedOffset.generatedColumn - 1 : 0),
                    originalLine: mapping.originalLine,
                    originalColumn: mapping.originalColumn,
                    name: name
                };
                generatedMappings.push(adjustedMapping);
                if (typeof adjustedMapping.originalLine === "number") {
                    originalMappings.push(adjustedMapping);
                }
            }
        };
        var generatedMappings = this.__generatedMappingsUnsorted = [];
        var originalMappings = this.__originalMappingsUnsorted = [];
        for(var i = 0; i < this._sections.length; i++)_loop(i);
    };
    _proto.eachMapping = function eachMapping(aCallback, aContext, aOrder) {
        var context = aContext || null;
        var order = aOrder || SourceMapConsumer.GENERATED_ORDER;
        var mappings;
        switch(order){
            case SourceMapConsumer.GENERATED_ORDER:
                mappings = this._generatedMappings;
                break;
            case SourceMapConsumer.ORIGINAL_ORDER:
                mappings = this._originalMappings;
                break;
            default:
                throw new Error("Unknown order of iteration.");
        }
        var sourceRoot = this.sourceRoot;
        mappings.map(function(mapping) {
            var source = null;
            if (mapping.source !== null) {
                source = this._sources.at(mapping.source);
                source = util.computeSourceURL(sourceRoot, source, this._sourceMapURL);
            }
            return {
                source: source,
                generatedLine: mapping.generatedLine,
                generatedColumn: mapping.generatedColumn,
                originalLine: mapping.originalLine,
                originalColumn: mapping.originalColumn,
                name: mapping.name === null ? null : this._names.at(mapping.name)
            };
        }, this).forEach(aCallback, context);
    };
    /**
   * Find the mapping that best matches the hypothetical "needle" mapping that
   * we are searching for in the given "haystack" of mappings.
   */ _proto._findMapping = function _findMapping(aNeedle, aMappings, aLineName, aColumnName, aComparator, aBias) {
        // To return the position we are searching for, we must first find the
        // mapping for the given position and then return the opposite position it
        // points to. Because the mappings are sorted, we can use binary search to
        // find the best mapping.
        if (aNeedle[aLineName] <= 0) {
            throw new TypeError("Line must be greater than or equal to 1, got " + aNeedle[aLineName]);
        }
        if (aNeedle[aColumnName] < 0) {
            throw new TypeError("Column must be greater than or equal to 0, got " + aNeedle[aColumnName]);
        }
        return binarySearch.search(aNeedle, aMappings, aComparator, aBias);
    };
    _proto.allGeneratedPositionsFor = function allGeneratedPositionsFor(aArgs) {
        var line = util.getArg(aArgs, "line");
        // When there is no exact match, BasicSourceMapConsumer.prototype._findMapping
        // returns the index of the closest mapping less than the needle. By
        // setting needle.originalColumn to 0, we thus find the last mapping for
        // the given line, provided such a mapping exists.
        var needle = {
            source: util.getArg(aArgs, "source"),
            originalLine: line,
            originalColumn: util.getArg(aArgs, "column", 0)
        };
        needle.source = this._findSourceIndex(needle.source);
        if (needle.source < 0) {
            return [];
        }
        if (needle.originalLine < 1) {
            throw new Error("Line numbers must be >= 1");
        }
        if (needle.originalColumn < 0) {
            throw new Error("Column numbers must be >= 0");
        }
        var mappings = [];
        var index = this._findMapping(needle, this._originalMappings, "originalLine", "originalColumn", util.compareByOriginalPositions, binarySearch.LEAST_UPPER_BOUND);
        if (index >= 0) {
            var mapping = this._originalMappings[index];
            if (aArgs.column === undefined) {
                var originalLine = mapping.originalLine;
                // Iterate until either we run out of mappings, or we run into
                // a mapping for a different line than the one we found. Since
                // mappings are sorted, this is guaranteed to find all mappings for
                // the line we found.
                while(mapping && mapping.originalLine === originalLine){
                    var lastColumn = mapping.lastGeneratedColumn;
                    if (this._computedColumnSpans && lastColumn === null) {
                        lastColumn = Infinity;
                    }
                    mappings.push({
                        line: util.getArg(mapping, "generatedLine", null),
                        column: util.getArg(mapping, "generatedColumn", null),
                        lastColumn: lastColumn
                    });
                    mapping = this._originalMappings[++index];
                }
            } else {
                var originalColumn = mapping.originalColumn;
                // Iterate until either we run out of mappings, or we run into
                // a mapping for a different line than the one we were searching for.
                // Since mappings are sorted, this is guaranteed to find all mappings for
                // the line we are searching for.
                while(mapping && mapping.originalLine === line && mapping.originalColumn == originalColumn){
                    var lastColumn1 = mapping.lastGeneratedColumn;
                    if (this._computedColumnSpans && lastColumn1 === null) {
                        lastColumn1 = Infinity;
                    }
                    mappings.push({
                        line: util.getArg(mapping, "generatedLine", null),
                        column: util.getArg(mapping, "generatedColumn", null),
                        lastColumn: lastColumn1
                    });
                    mapping = this._originalMappings[++index];
                }
            }
        }
        return mappings;
    };
    _proto.destroy = function destroy() {
        for(var i = 0; i < this._sections.length; i++){
            this._sections[i].consumer.destroy();
        }
    };
    _createClass(IndexedSourceMapConsumer, [
        {
            key: "_generatedMappings",
            get: // `__generatedMappings` and `__originalMappings` are arrays that hold the
            // parsed mapping coordinates from the source map's "mappings" attribute. They
            // are lazily instantiated, accessed via the `_generatedMappings` and
            // `_originalMappings` getters respectively, and we only parse the mappings
            // and create these arrays once queried for a source location. We jump through
            // these hoops because there can be many thousands of mappings, and parsing
            // them is expensive, so we only want to do it if we must.
            //
            // Each object in the arrays is of the form:
            //
            //     {
            //       generatedLine: The line number in the generated code,
            //       generatedColumn: The column number in the generated code,
            //       source: The path to the original source file that generated this
            //               chunk of code,
            //       originalLine: The line number in the original source that
            //                     corresponds to this chunk of generated code,
            //       originalColumn: The column number in the original source that
            //                       corresponds to this chunk of generated code,
            //       name: The name of the original symbol which generated this chunk of
            //             code.
            //     }
            //
            // All properties except for `generatedLine` and `generatedColumn` can be
            // `null`.
            //
            // `_generatedMappings` is ordered by the generated positions.
            //
            // `_originalMappings` is ordered by the original positions.
            function get() {
                if (!this.__generatedMappings) {
                    this._sortGeneratedMappings();
                }
                return this.__generatedMappings;
            }
        },
        {
            key: "_originalMappings",
            get: function get() {
                if (!this.__originalMappings) {
                    this._sortOriginalMappings();
                }
                return this.__originalMappings;
            }
        },
        {
            key: "_generatedMappingsUnsorted",
            get: function get() {
                if (!this.__generatedMappingsUnsorted) {
                    this._parseMappings(this._mappings, this.sourceRoot);
                }
                return this.__generatedMappingsUnsorted;
            }
        },
        {
            key: "_originalMappingsUnsorted",
            get: function get() {
                if (!this.__originalMappingsUnsorted) {
                    this._parseMappings(this._mappings, this.sourceRoot);
                }
                return this.__originalMappingsUnsorted;
            }
        },
        {
            key: "sources",
            get: /**
   * The list of original sources.
   */ function get() {
                var sources = [];
                for(var i = 0; i < this._sections.length; i++){
                    for(var j = 0; j < this._sections[i].consumer.sources.length; j++){
                        sources.push(this._sections[i].consumer.sources[j]);
                    }
                }
                return sources;
            }
        }
    ]);
    return IndexedSourceMapConsumer;
}(SourceMapConsumer);
/*
 * Cheat to get around inter-twingled classes.  `factory()` can be at the end
 * where it has access to non-hoisted classes, but it gets hoisted itself.
 */ function _factory(aSourceMap, aSourceMapURL) {
    var sourceMap = aSourceMap;
    if (typeof aSourceMap === "string") {
        sourceMap = util.parseSourceMapInput(aSourceMap);
    }
    var consumer = sourceMap.sections != null ? new IndexedSourceMapConsumer(sourceMap, aSourceMapURL) : new BasicSourceMapConsumer(sourceMap, aSourceMapURL);
    return Promise.resolve(consumer);
}
function _factoryBSM(aSourceMap, aSourceMapURL) {
    return BasicSourceMapConsumer.fromSourceMap(aSourceMap, aSourceMapURL);
}

var SourceMapGenerator = sourceMapGenerator.SourceMapGenerator;

var convertSourceMap = {};

var safeBuffer = {exports: {}};

(function(module, exports) {
    var copyProps = // alternative to using Object.keys for old browsers
    function copyProps(src, dst) {
        for(var key in src){
            dst[key] = src[key];
        }
    };
    var SafeBuffer = function SafeBuffer(arg, encodingOrOffset, length) {
        return Buffer(arg, encodingOrOffset, length);
    };
    var buffer = require$$0__default$1["default"];
    var Buffer = buffer.Buffer;
    if (Buffer.from && Buffer.alloc && Buffer.allocUnsafe && Buffer.allocUnsafeSlow) {
        module.exports = buffer;
    } else {
        // Copy properties from require('buffer')
        copyProps(buffer, exports);
        exports.Buffer = SafeBuffer;
    }
    // Copy static methods from Buffer
    copyProps(Buffer, SafeBuffer);
    SafeBuffer.from = function(arg, encodingOrOffset, length) {
        if (typeof arg === "number") {
            throw new TypeError("Argument must not be a number");
        }
        return Buffer(arg, encodingOrOffset, length);
    };
    SafeBuffer.alloc = function(size, fill, encoding) {
        if (typeof size !== "number") {
            throw new TypeError("Argument must be a number");
        }
        var buf = Buffer(size);
        if (fill !== undefined) {
            if (typeof encoding === "string") {
                buf.fill(fill, encoding);
            } else {
                buf.fill(fill);
            }
        } else {
            buf.fill(0);
        }
        return buf;
    };
    SafeBuffer.allocUnsafe = function(size) {
        if (typeof size !== "number") {
            throw new TypeError("Argument must be a number");
        }
        return Buffer(size);
    };
    SafeBuffer.allocUnsafeSlow = function(size) {
        if (typeof size !== "number") {
            throw new TypeError("Argument must be a number");
        }
        return buffer.SlowBuffer(size);
    };
})(safeBuffer, safeBuffer.exports);

(function(exports) {
    var decodeBase64 = function decodeBase64(base64) {
        return SafeBuffer.Buffer.from(base64, "base64").toString();
    };
    var stripComment = function stripComment(sm) {
        return sm.split(",").pop();
    };
    var readFromFileMap = function readFromFileMap(sm, dir) {
        // NOTE: this will only work on the server since it attempts to read the map file
        var r = exports.mapFileCommentRegex.exec(sm);
        // for some odd reason //# .. captures in 1 and /* .. */ in 2
        var filename = r[1] || r[2];
        var filepath = path.resolve(dir, filename);
        try {
            return fs.readFileSync(filepath, "utf8");
        } catch (e) {
            throw new Error("An error occurred while trying to read the map file at " + filepath + "\n" + e);
        }
    };
    var Converter = function Converter(sm, opts) {
        opts = opts || {};
        if (opts.isFileComment) sm = readFromFileMap(sm, opts.commentFileDir);
        if (opts.hasComment) sm = stripComment(sm);
        if (opts.isEncoded) sm = decodeBase64(sm);
        if (opts.isJSON || opts.isEncoded) sm = JSON.parse(sm);
        this.sourcemap = sm;
    };
    var fs = require$$0__default["default"];
    var path = require$$1__default["default"];
    var SafeBuffer = safeBuffer.exports;
    Object.defineProperty(exports, "commentRegex", {
        get: function getCommentRegex() {
            return /^\s*\/(?:\/|\*)[@#]\s+sourceMappingURL=data:(?:application|text)\/json;(?:charset[:=]\S+?;)?base64,(?:.*)$/mg;
        }
    });
    Object.defineProperty(exports, "mapFileCommentRegex", {
        get: function getMapFileCommentRegex() {
            // Matches sourceMappingURL in either // or /* comment styles.
            return /(?:\/\/[@#][ \t]+sourceMappingURL=([^\s'"`]+?)[ \t]*$)|(?:\/\*[@#][ \t]+sourceMappingURL=([^\*]+?)[ \t]*(?:\*\/){1}[ \t]*$)/mg;
        }
    });
    Converter.prototype.toJSON = function(space) {
        return JSON.stringify(this.sourcemap, null, space);
    };
    Converter.prototype.toBase64 = function() {
        var json = this.toJSON();
        return SafeBuffer.Buffer.from(json, "utf8").toString("base64");
    };
    Converter.prototype.toComment = function(options) {
        var base64 = this.toBase64();
        var data = "sourceMappingURL=data:application/json;charset=utf-8;base64," + base64;
        return options && options.multiline ? "/*# " + data + " */" : "//# " + data;
    };
    // returns copy instead of original
    Converter.prototype.toObject = function() {
        return JSON.parse(this.toJSON());
    };
    Converter.prototype.addProperty = function(key, value) {
        if (this.sourcemap.hasOwnProperty(key)) throw new Error('property "' + key + '" already exists on the sourcemap, use set property instead');
        return this.setProperty(key, value);
    };
    Converter.prototype.setProperty = function(key, value) {
        this.sourcemap[key] = value;
        return this;
    };
    Converter.prototype.getProperty = function(key) {
        return this.sourcemap[key];
    };
    exports.fromObject = function(obj) {
        return new Converter(obj);
    };
    exports.fromJSON = function(json) {
        return new Converter(json, {
            isJSON: true
        });
    };
    exports.fromBase64 = function(base64) {
        return new Converter(base64, {
            isEncoded: true
        });
    };
    exports.fromComment = function(comment) {
        comment = comment.replace(/^\/\*/g, "//").replace(/\*\/$/g, "");
        return new Converter(comment, {
            isEncoded: true,
            hasComment: true
        });
    };
    exports.fromMapFileComment = function(comment, dir) {
        return new Converter(comment, {
            commentFileDir: dir,
            isFileComment: true,
            isJSON: true
        });
    };
    // Finds last sourcemap comment in file or returns null if none was found
    exports.fromSource = function(content) {
        var m = content.match(exports.commentRegex);
        return m ? exports.fromComment(m.pop()) : null;
    };
    // Finds last sourcemap comment in file or returns null if none was found
    exports.fromMapFileSource = function(content, dir) {
        var m = content.match(exports.mapFileCommentRegex);
        return m ? exports.fromMapFileComment(m.pop(), dir) : null;
    };
    exports.removeComments = function(src) {
        return src.replace(exports.commentRegex, "");
    };
    exports.removeMapFileComments = function(src) {
        return src.replace(exports.mapFileCommentRegex, "");
    };
    exports.generateMapFileComment = function(file, options) {
        var data = "sourceMappingURL=" + file;
        return options && options.multiline ? "/*# " + data + " */" : "//# " + data;
    };
})(convertSourceMap);

var stylis$1 = {exports: {}};

(function(module, exports) {
    (function(factory) {
        module["exports"] = factory(null) ;
    })(/** @param {*=} options */ function factory(options) {
        var select = /**
		 * Select
		 *
		 * @param {Array<string>} parent
		 * @param {string} current
		 * @param {number} invert
		 * @return {Array<string>}
		 */ function select(parent, current, invert) {
            var selectors = current.trim().split(selectorptn);
            var out = selectors;
            var length = selectors.length;
            var l = parent.length;
            switch(l){
                // 0-1 parent selectors
                case 0:
                case 1:
                    {
                        for(var i = 0, selector = l === 0 ? "" : parent[0] + " "; i < length; ++i){
                            out[i] = scope(selector, out[i], invert, l).trim();
                        }
                        break;
                    }
                // >2 parent selectors, nested
                default:
                    {
                        for(var i = 0, j = 0, out = []; i < length; ++i){
                            for(var k = 0; k < l; ++k){
                                out[j++] = scope(parent[k] + " ", selectors[i], invert, l).trim();
                            }
                        }
                    }
            }
            return out;
        };
        var scope = /**
		 * Scope
		 *
		 * @param {string} parent
		 * @param {string} current
		 * @param {number} invert
		 * @param {number} level
		 * @return {string}
		 */ function scope(parent, current, invert, level) {
            var selector = current;
            var code = selector.charCodeAt(0);
            // trim leading whitespace
            if (code < 33) {
                code = (selector = selector.trim()).charCodeAt(0);
            }
            switch(code){
                // &
                case AND:
                    {
                        switch(cascade + level){
                            case 0:
                            case 1:
                                {
                                    if (parent.trim().length === 0) {
                                        break;
                                    }
                                }
                            default:
                                {
                                    return selector.replace(andptn, "$1" + parent.trim());
                                }
                        }
                        break;
                    }
                // :
                case COLON:
                    {
                        switch(selector.charCodeAt(1)){
                            // g in :global
                            case 103:
                                {
                                    if (escape > 0 && cascade > 0) {
                                        return selector.replace(escapeptn, "$1").replace(andptn, "$1" + nscope);
                                    }
                                    break;
                                }
                            default:
                                {
                                    // :hover
                                    return parent.trim() + selector.replace(andptn, "$1" + parent.trim());
                                }
                        }
                    }
                default:
                    {
                        // html &
                        if (invert * cascade > 0 && selector.indexOf("\f") > 0) {
                            return selector.replace(andptn, (parent.charCodeAt(0) === COLON ? "" : "$1") + parent.trim());
                        }
                    }
            }
            return parent + selector;
        };
        var vendor = /**
		 * Vendor
		 *
		 * @param {string} content
		 * @param {number} context
		 * @return {boolean}
		 */ function vendor(content, context) {
            var index = content.indexOf(context === 1 ? ":" : "{");
            var key = content.substring(0, context !== 3 ? index : 10);
            var value = content.substring(index + 1, content.length - 1);
            return should(context !== 2 ? key : key.replace(pseudofmt, "$1"), value, context);
        };
        var supports = /**
		 * Supports
		 *
		 * @param {string} match
		 * @param {string} group
		 * @return {string}
		 */ function supports(match, group) {
            var out = property(group, group.charCodeAt(0), group.charCodeAt(1), group.charCodeAt(2));
            return out !== group + ";" ? out.replace(propertyptn, " or ($1)").substring(4) : "(" + group + ")";
        };
        var animation = /**
		 * Animation
		 *
		 * @param {string} input
		 * @return {string}
		 */ function animation(input) {
            var length = input.length;
            var index = input.indexOf(":", 9) + 1;
            var declare = input.substring(0, index).trim();
            var out = input.substring(index, length - 1).trim();
            switch(input.charCodeAt(9) * keyed){
                case 0:
                    {
                        break;
                    }
                // animation-*, -
                case DASH:
                    {
                        // animation-name, n
                        if (input.charCodeAt(10) !== 110) {
                            break;
                        }
                    }
                // animation/animation-name
                default:
                    {
                        // split in case of multiple animations
                        var list = out.split((out = "", animationptn));
                        for(var i = 0, index = 0, length = list.length; i < length; index = 0, ++i){
                            var value = list[i];
                            var items = value.split(propertiesptn);
                            while(value = items[index]){
                                var peak = value.charCodeAt(0);
                                if (keyed === 1 && (// letters
                                peak > AT && peak < 90 || peak > 96 && peak < 123 || peak === UNDERSCORE || // dash but not in sequence i.e --
                                peak === DASH && value.charCodeAt(1) !== DASH)) {
                                    // not a number/function
                                    switch(isNaN(parseFloat(value)) + (value.indexOf("(") !== -1)){
                                        case 1:
                                            {
                                                switch(value){
                                                    // not a valid reserved keyword
                                                    case "infinite":
                                                    case "alternate":
                                                    case "backwards":
                                                    case "running":
                                                    case "normal":
                                                    case "forwards":
                                                    case "both":
                                                    case "none":
                                                    case "linear":
                                                    case "ease":
                                                    case "ease-in":
                                                    case "ease-out":
                                                    case "ease-in-out":
                                                    case "paused":
                                                    case "reverse":
                                                    case "alternate-reverse":
                                                    case "inherit":
                                                    case "initial":
                                                    case "unset":
                                                    case "step-start":
                                                    case "step-end":
                                                        {
                                                            break;
                                                        }
                                                    default:
                                                        {
                                                            value += key;
                                                        }
                                                }
                                            }
                                    }
                                }
                                items[index++] = value;
                            }
                            out += (i === 0 ? "" : ",") + items.join(" ");
                        }
                    }
            }
            out = declare + out + ";";
            if (prefix === 1 || prefix === 2 && vendor(out, 1)) return webkit + out + out;
            return out;
        };
        var isolate = /**
		 * Isolate
		 *
		 * @param {Array<string>} current
		 */ function isolate(current) {
            for(var i = 0, length = current.length, selector = Array(length), padding, element; i < length; ++i){
                // split individual elements in a selector i.e h1 h2 === [h1, h2]
                var elements = current[i].split(elementptn);
                var out = "";
                for(var j = 0, size = 0, tail = 0, code = 0, l = elements.length; j < l; ++j){
                    // empty element
                    if ((size = (element = elements[j]).length) === 0 && l > 1) {
                        continue;
                    }
                    tail = out.charCodeAt(out.length - 1);
                    code = element.charCodeAt(0);
                    padding = "";
                    if (j !== 0) {
                        // determine if we need padding
                        switch(tail){
                            case STAR:
                            case TILDE:
                            case GREATERTHAN:
                            case PLUS:
                            case SPACE:
                            case OPENPARENTHESES:
                                {
                                    break;
                                }
                            default:
                                {
                                    padding = " ";
                                }
                        }
                    }
                    switch(code){
                        case AND:
                            {
                                element = padding + nscopealt;
                            }
                        case TILDE:
                        case GREATERTHAN:
                        case PLUS:
                        case SPACE:
                        case CLOSEPARENTHESES:
                        case OPENPARENTHESES:
                            {
                                break;
                            }
                        case OPENBRACKET:
                            {
                                element = padding + element + nscopealt;
                                break;
                            }
                        case COLON:
                            {
                                switch(element.charCodeAt(1) * 2 + element.charCodeAt(2) * 3){
                                    // :global
                                    case 530:
                                        {
                                            if (escape > 0) {
                                                element = padding + element.substring(8, size - 1);
                                                break;
                                            }
                                        }
                                    // :hover, :nth-child(), ...
                                    default:
                                        {
                                            if (j < 1 || elements[j - 1].length < 1) {
                                                element = padding + nscopealt + element;
                                            }
                                        }
                                }
                                break;
                            }
                        case COMMA:
                            {
                                padding = "";
                            }
                        default:
                            {
                                if (size > 1 && element.indexOf(":") > 0) {
                                    element = padding + element.replace(pseudoptn, "$1" + nscopealt + "$2");
                                } else {
                                    element = padding + element + nscopealt;
                                }
                            }
                    }
                    out += element;
                }
                selector[i] = out.replace(formatptn, "").trim();
            }
            return selector;
        };
        var proxy = /**
		 * Proxy
		 *
		 * @param {number} context
		 * @param {string} content
		 * @param {Array<string>} selectors
		 * @param {Array<string>} parents
		 * @param {number} line
		 * @param {number} column
		 * @param {number} length
		 * @param {number} id
		 * @param {number} depth
		 * @param {number} at
		 * @return {(string|void|*)}
		 */ function proxy(context, content, selectors, parents, line, column, length, id, depth, at) {
            for(var i = 0, out = content, next; i < plugged; ++i){
                switch(next = plugins[i].call(stylis, context, out, selectors, parents, line, column, length, id, depth, at)){
                    case void 0:
                    case false:
                    case true:
                    case null:
                        {
                            break;
                        }
                    default:
                        {
                            out = next;
                        }
                }
            }
            if (out !== content) {
                return out;
            }
        };
        var delimited = /**
		 * @param {number} code
		 * @param {number} index
		 * @param {number} length
		 * @param {string} body
		 * @return {number}
		 */ function delimited(code, index, length, body) {
            for(var i = index + 1; i < length; ++i){
                switch(body.charCodeAt(i)){
                    // /*
                    case FOWARDSLASH:
                        {
                            if (code === STAR) {
                                if (body.charCodeAt(i - 1) === STAR && index + 2 !== i) {
                                    return i + 1;
                                }
                            }
                            break;
                        }
                    // //
                    case NEWLINE:
                        {
                            if (code === FOWARDSLASH) {
                                return i + 1;
                            }
                        }
                }
            }
            return i;
        };
        var minify = /**
		 * Minify
		 *
		 * @param {(string|*)} output
		 * @return {string}
		 */ function minify(output) {
            return output.replace(formatptn, "").replace(beforeptn, "").replace(afterptn, "$1").replace(tailptn, "$1").replace(whiteptn, " ");
        };
        /**
		 * Notes
		 *
		 * The ['<method name>'] pattern is used to support closure compiler
		 * the jsdoc signatures are also used to the same effect
		 *
		 * ----
		 *
		 * int + int + int === n4 [faster]
		 *
		 * vs
		 *
		 * int === n1 && int === n2 && int === n3
		 *
		 * ----
		 *
		 * switch (int) { case ints...} [faster]
		 *
		 * vs
		 *
		 * if (int == 1 && int === 2 ...)
		 *
		 * ----
		 *
		 * The (first*n1 + second*n2 + third*n3) format used in the property parser
		 * is a simple way to hash the sequence of characters
		 * taking into account the index they occur in
		 * since any number of 3 character sequences could produce duplicates.
		 *
		 * On the other hand sequences that are directly tied to the index of the character
		 * resolve a far more accurate measure, it's also faster
		 * to evaluate one condition in a switch statement
		 * than three in an if statement regardless of the added math.
		 *
		 * This allows the vendor prefixer to be both small and fast.
		 */ var nullptn = /^\0+/g /* matches leading null characters */ ;
        var formatptn = /[\0\r\f]/g /* matches new line, null and formfeed characters */ ;
        var colonptn = /: */g /* splits animation rules */ ;
        var cursorptn = /zoo|gra/ /* assert cursor varient */ ;
        var transformptn = /([,: ])(transform)/g /* vendor prefix transform, older webkit */ ;
        var animationptn = /,+\s*(?![^(]*[)])/g /* splits multiple shorthand notation animations */ ;
        var propertiesptn = / +\s*(?![^(]*[)])/g /* animation properties */ ;
        var elementptn = / *[\0] */g /* selector elements */ ;
        var selectorptn = /,\r+?/g /* splits selectors */ ;
        var andptn = /([\t\r\n ])*\f?&/g /* match & */ ;
        var escapeptn = /:global\(((?:[^\(\)\[\]]*|\[.*\]|\([^\(\)]*\))*)\)/g /* matches :global(.*) */ ;
        var invalidptn = /\W+/g /* removes invalid characters from keyframes */ ;
        var keyframeptn = /@(k\w+)\s*(\S*)\s*/ /* matches @keyframes $1 */ ;
        var plcholdrptn = /::(place)/g /* match ::placeholder varient */ ;
        var readonlyptn = /:(read-only)/g /* match :read-only varient */ ;
        var beforeptn = /\s+(?=[{\];=:>])/g /* matches \s before ] ; = : */ ;
        var afterptn = /([[}=:>])\s+/g /* matches \s after characters [ } = : */ ;
        var tailptn = /(\{[^{]+?);(?=\})/g /* matches tail semi-colons ;} */ ;
        var whiteptn = /\s{2,}/g /* matches repeating whitespace */ ;
        var pseudoptn = /([^\(])(:+) */g /* pseudo element */ ;
        var writingptn = /[svh]\w+-[tblr]{2}/ /* match writing mode property values */ ;
        var supportsptn = /\(\s*(.*)\s*\)/g /* match supports (groups) */ ;
        var propertyptn = /([\s\S]*?);/g /* match properties leading semicolon */ ;
        var selfptn = /-self|flex-/g /* match flex- and -self in align-self: flex-*; */ ;
        var pseudofmt = /[^]*?(:[rp][el]a[\w-]+)[^]*/ /* extrats :readonly or :placholder from selector */ ;
        var dimensionptn = /stretch|:\s*\w+\-(?:conte|avail)/ /* match max/min/fit-content, fill-available */ ;
        var imgsrcptn = /([^-])(image-set\()/;
        /* vendors */ var webkit = "-webkit-";
        var moz = "-moz-";
        var ms = "-ms-";
        /* character codes */ var SEMICOLON = 59 /* ; */ ;
        var CLOSEBRACES = 125 /* } */ ;
        var OPENBRACES = 123 /* { */ ;
        var OPENPARENTHESES = 40 /* ( */ ;
        var CLOSEPARENTHESES = 41 /* ) */ ;
        var OPENBRACKET = 91 /* [ */ ;
        var CLOSEBRACKET = 93 /* ] */ ;
        var NEWLINE = 10 /* \n */ ;
        var CARRIAGE = 13 /* \r */ ;
        var TAB = 9 /* \t */ ;
        var AT = 64 /* @ */ ;
        var SPACE = 32 /*   */ ;
        var AND = 38 /* & */ ;
        var DASH = 45 /* - */ ;
        var UNDERSCORE = 95 /* _ */ ;
        var STAR = 42 /* * */ ;
        var COMMA = 44 /* , */ ;
        var COLON = 58 /* : */ ;
        var SINGLEQUOTE = 39 /* ' */ ;
        var DOUBLEQUOTE = 34 /* " */ ;
        var FOWARDSLASH = 47 /* / */ ;
        var GREATERTHAN = 62 /* > */ ;
        var PLUS = 43 /* + */ ;
        var TILDE = 126 /* ~ */ ;
        var NULL = 0 /* \0 */ ;
        var FORMFEED = 12 /* \f */ ;
        var VERTICALTAB = 11 /* \v */ ;
        /* special identifiers */ var KEYFRAME = 107 /* k */ ;
        var MEDIA = 109 /* m */ ;
        var SUPPORTS = 115 /* s */ ;
        var PLACEHOLDER = 112 /* p */ ;
        var READONLY = 111 /* o */ ;
        var IMPORT = 105 /* <at>i */ ;
        var CHARSET = 99 /* <at>c */ ;
        var DOCUMENT = 100 /* <at>d */ ;
        var PAGE = 112 /* <at>p */ ;
        var column = 1 /* current column */ ;
        var line = 1 /* current line numebr */ ;
        var pattern = 0 /* :pattern */ ;
        var cascade = 1 /* #id h1 h2 vs h1#id h2#id  */ ;
        var prefix = 1 /* vendor prefix */ ;
        var escape = 1 /* escape :global() pattern */ ;
        var compress = 0 /* compress output */ ;
        var semicolon = 0 /* no/semicolon option */ ;
        var preserve = 0 /* preserve empty selectors */ ;
        /* empty reference */ var array = [];
        /* plugins */ var plugins = [];
        var plugged = 0;
        var should = null;
        /* plugin context */ var POSTS = -2;
        var PREPS = -1;
        var UNKWN = 0;
        var PROPS = 1;
        var BLCKS = 2;
        var ATRUL = 3;
        /* plugin newline context */ var unkwn = 0;
        /* keyframe animation */ var keyed = 1;
        var key = "";
        /* selector namespace */ var nscopealt = "";
        var nscope = "";
        /**
		 * Compile
		 *
		 * @param {Array<string>} parent
		 * @param {Array<string>} current
		 * @param {string} body
		 * @param {number} id
		 * @param {number} depth
		 * @return {string}
		 */ function compile(parent, current, body, id, depth) {
            var bracket = 0 /* brackets [] */ ;
            var comment = 0 /* comments /* // or /* */ ;
            var parentheses = 0 /* functions () */ ;
            var quote = 0 /* quotes '', "" */ ;
            var first = 0 /* first character code */ ;
            var second = 0 /* second character code */ ;
            var code = 0 /* current character code */ ;
            var tail = 0 /* previous character code */ ;
            var trail = 0 /* character before previous code */ ;
            var peak = 0 /* previous non-whitespace code */ ;
            var counter = 0 /* count sequence termination */ ;
            var context = 0 /* track current context */ ;
            var atrule = 0 /* track @at-rule context */ ;
            var pseudo = 0 /* track pseudo token index */ ;
            var caret = 0 /* current character index */ ;
            var format = 0 /* control character formating context */ ;
            var insert = 0 /* auto semicolon insertion */ ;
            var invert = 0 /* inverted selector pattern */ ;
            var length = 0 /* generic length address */ ;
            var eof = body.length /* end of file(length) */ ;
            var eol = eof - 1 /* end of file(characters) */ ;
            var char = "" /* current character */ ;
            var chars = "" /* current buffer of characters */ ;
            var child = "" /* next buffer of characters */ ;
            var out = "" /* compiled body */ ;
            var children = "" /* compiled children */ ;
            var flat = "" /* compiled leafs */ ;
            var selector /* generic selector address */ ;
            var result /* generic address */ ;
            // ...build body
            while(caret < eof){
                code = body.charCodeAt(caret);
                // eof varient
                if (caret === eol) {
                    // last character + noop context, add synthetic padding for noop context to terminate
                    if (comment + quote + parentheses + bracket !== 0) {
                        if (comment !== 0) {
                            code = comment === FOWARDSLASH ? NEWLINE : FOWARDSLASH;
                        }
                        quote = parentheses = bracket = 0;
                        eof++;
                        eol++;
                    }
                }
                if (comment + quote + parentheses + bracket === 0) {
                    // eof varient
                    if (caret === eol) {
                        if (format > 0) {
                            chars = chars.replace(formatptn, "");
                        }
                        if (chars.trim().length > 0) {
                            switch(code){
                                case SPACE:
                                case TAB:
                                case SEMICOLON:
                                case CARRIAGE:
                                case NEWLINE:
                                    {
                                        break;
                                    }
                                default:
                                    {
                                        chars += body.charAt(caret);
                                    }
                            }
                            code = SEMICOLON;
                        }
                    }
                    // auto semicolon insertion
                    if (insert === 1) {
                        switch(code){
                            // false flags
                            case OPENBRACES:
                            case CLOSEBRACES:
                            case SEMICOLON:
                            case DOUBLEQUOTE:
                            case SINGLEQUOTE:
                            case OPENPARENTHESES:
                            case CLOSEPARENTHESES:
                            case COMMA:
                                {
                                    insert = 0;
                                }
                            // ignore
                            case TAB:
                            case CARRIAGE:
                            case NEWLINE:
                            case SPACE:
                                {
                                    break;
                                }
                            // valid
                            default:
                                {
                                    insert = 0;
                                    length = caret;
                                    first = code;
                                    caret--;
                                    code = SEMICOLON;
                                    while(length < eof){
                                        switch(body.charCodeAt(length++)){
                                            case NEWLINE:
                                            case CARRIAGE:
                                            case SEMICOLON:
                                                {
                                                    ++caret;
                                                    code = first;
                                                    length = eof;
                                                    break;
                                                }
                                            case COLON:
                                                {
                                                    if (format > 0) {
                                                        ++caret;
                                                        code = first;
                                                    }
                                                }
                                            case OPENBRACES:
                                                {
                                                    length = eof;
                                                }
                                        }
                                    }
                                }
                        }
                    }
                    // token varient
                    switch(code){
                        case OPENBRACES:
                            {
                                chars = chars.trim();
                                first = chars.charCodeAt(0);
                                counter = 1;
                                length = ++caret;
                                while(caret < eof){
                                    switch(code = body.charCodeAt(caret)){
                                        case OPENBRACES:
                                            {
                                                counter++;
                                                break;
                                            }
                                        case CLOSEBRACES:
                                            {
                                                counter--;
                                                break;
                                            }
                                        case FOWARDSLASH:
                                            {
                                                switch(second = body.charCodeAt(caret + 1)){
                                                    // /*, //
                                                    case STAR:
                                                    case FOWARDSLASH:
                                                        {
                                                            caret = delimited(second, caret, eol, body);
                                                        }
                                                }
                                                break;
                                            }
                                        // given "[" === 91 & "]" === 93 hence forth 91 + 1 + 1 === 93
                                        case OPENBRACKET:
                                            {
                                                code++;
                                            }
                                        // given "(" === 40 & ")" === 41 hence forth 40 + 1 === 41
                                        case OPENPARENTHESES:
                                            {
                                                code++;
                                            }
                                        // quote tail delimiter is identical to the head delimiter hence noop,
                                        // fallthrough clauses have been shifted to the correct tail delimiter
                                        case DOUBLEQUOTE:
                                        case SINGLEQUOTE:
                                            {
                                                while(caret++ < eol){
                                                    if (body.charCodeAt(caret) === code) {
                                                        break;
                                                    }
                                                }
                                            }
                                    }
                                    if (counter === 0) {
                                        break;
                                    }
                                    caret++;
                                }
                                child = body.substring(length, caret);
                                if (first === NULL) {
                                    first = (chars = chars.replace(nullptn, "").trim()).charCodeAt(0);
                                }
                                switch(first){
                                    // @at-rule
                                    case AT:
                                        {
                                            if (format > 0) {
                                                chars = chars.replace(formatptn, "");
                                            }
                                            second = chars.charCodeAt(1);
                                            switch(second){
                                                case DOCUMENT:
                                                case MEDIA:
                                                case SUPPORTS:
                                                case DASH:
                                                    {
                                                        selector = current;
                                                        break;
                                                    }
                                                default:
                                                    {
                                                        selector = array;
                                                    }
                                            }
                                            child = compile(current, selector, child, second, depth + 1);
                                            length = child.length;
                                            // preserve empty @at-rule
                                            if (preserve > 0 && length === 0) {
                                                length = chars.length;
                                            }
                                            // execute plugins, @at-rule context
                                            if (plugged > 0) {
                                                selector = select(array, chars, invert);
                                                result = proxy(ATRUL, child, selector, current, line, column, length, second, depth, id);
                                                chars = selector.join("");
                                                if (result !== void 0) {
                                                    if ((length = (child = result.trim()).length) === 0) {
                                                        second = 0;
                                                        child = "";
                                                    }
                                                }
                                            }
                                            if (length > 0) {
                                                switch(second){
                                                    case SUPPORTS:
                                                        {
                                                            chars = chars.replace(supportsptn, supports);
                                                        }
                                                    case DOCUMENT:
                                                    case MEDIA:
                                                    case DASH:
                                                        {
                                                            child = chars + "{" + child + "}";
                                                            break;
                                                        }
                                                    case KEYFRAME:
                                                        {
                                                            chars = chars.replace(keyframeptn, "$1 $2" + (keyed > 0 ? key : ""));
                                                            child = chars + "{" + child + "}";
                                                            if (prefix === 1 || prefix === 2 && vendor("@" + child, 3)) {
                                                                child = "@" + webkit + child + "@" + child;
                                                            } else {
                                                                child = "@" + child;
                                                            }
                                                            break;
                                                        }
                                                    default:
                                                        {
                                                            child = chars + child;
                                                            if (id === PAGE) {
                                                                child = (out += child, "");
                                                            }
                                                        }
                                                }
                                            } else {
                                                child = "";
                                            }
                                            break;
                                        }
                                    // selector
                                    default:
                                        {
                                            child = compile(current, select(current, chars, invert), child, id, depth + 1);
                                        }
                                }
                                children += child;
                                // reset
                                context = 0;
                                insert = 0;
                                pseudo = 0;
                                format = 0;
                                invert = 0;
                                atrule = 0;
                                chars = "";
                                child = "";
                                code = body.charCodeAt(++caret);
                                break;
                            }
                        case CLOSEBRACES:
                        case SEMICOLON:
                            {
                                chars = (format > 0 ? chars.replace(formatptn, "") : chars).trim();
                                if ((length = chars.length) > 1) {
                                    // monkey-patch missing colon
                                    if (pseudo === 0) {
                                        first = chars.charCodeAt(0);
                                        // first character is a letter or dash, buffer has a space character
                                        if (first === DASH || first > 96 && first < 123) {
                                            length = (chars = chars.replace(" ", ":")).length;
                                        }
                                    }
                                    // execute plugins, property context
                                    if (plugged > 0) {
                                        if ((result = proxy(PROPS, chars, current, parent, line, column, out.length, id, depth, id)) !== void 0) {
                                            if ((length = (chars = result.trim()).length) === 0) {
                                                chars = "\0\0";
                                            }
                                        }
                                    }
                                    first = chars.charCodeAt(0);
                                    second = chars.charCodeAt(1);
                                    switch(first){
                                        case NULL:
                                            {
                                                break;
                                            }
                                        case AT:
                                            {
                                                if (second === IMPORT || second === CHARSET) {
                                                    flat += chars + body.charAt(caret);
                                                    break;
                                                }
                                            }
                                        default:
                                            {
                                                if (chars.charCodeAt(length - 1) === COLON) {
                                                    break;
                                                }
                                                out += property(chars, first, second, chars.charCodeAt(2));
                                            }
                                    }
                                }
                                // reset
                                context = 0;
                                insert = 0;
                                pseudo = 0;
                                format = 0;
                                invert = 0;
                                chars = "";
                                code = body.charCodeAt(++caret);
                                break;
                            }
                    }
                }
                // parse characters
                switch(code){
                    case CARRIAGE:
                    case NEWLINE:
                        {
                            // auto insert semicolon
                            if (comment + quote + parentheses + bracket + semicolon === 0) {
                                // valid non-whitespace characters that
                                // may precede a newline
                                switch(peak){
                                    case CLOSEPARENTHESES:
                                    case SINGLEQUOTE:
                                    case DOUBLEQUOTE:
                                    case AT:
                                    case TILDE:
                                    case GREATERTHAN:
                                    case STAR:
                                    case PLUS:
                                    case FOWARDSLASH:
                                    case DASH:
                                    case COLON:
                                    case COMMA:
                                    case SEMICOLON:
                                    case OPENBRACES:
                                    case CLOSEBRACES:
                                        {
                                            break;
                                        }
                                    default:
                                        {
                                            // current buffer has a colon
                                            if (pseudo > 0) {
                                                insert = 1;
                                            }
                                        }
                                }
                            }
                            // terminate line comment
                            if (comment === FOWARDSLASH) {
                                comment = 0;
                            } else if (cascade + context === 0 && id !== KEYFRAME && chars.length > 0) {
                                format = 1;
                                chars += "\0";
                            }
                            // execute plugins, newline context
                            if (plugged * unkwn > 0) {
                                proxy(UNKWN, chars, current, parent, line, column, out.length, id, depth, id);
                            }
                            // next line, reset column position
                            column = 1;
                            line++;
                            break;
                        }
                    case SEMICOLON:
                    case CLOSEBRACES:
                        {
                            if (comment + quote + parentheses + bracket === 0) {
                                column++;
                                break;
                            }
                        }
                    default:
                        {
                            // increment column position
                            column++;
                            // current character
                            char = body.charAt(caret);
                            // remove comments, escape functions, strings, attributes and prepare selectors
                            switch(code){
                                case TAB:
                                case SPACE:
                                    {
                                        if (quote + bracket + comment === 0) {
                                            switch(tail){
                                                case COMMA:
                                                case COLON:
                                                case TAB:
                                                case SPACE:
                                                    {
                                                        char = "";
                                                        break;
                                                    }
                                                default:
                                                    {
                                                        if (code !== SPACE) {
                                                            char = " ";
                                                        }
                                                    }
                                            }
                                        }
                                        break;
                                    }
                                // escape breaking control characters
                                case NULL:
                                    {
                                        char = "\\0";
                                        break;
                                    }
                                case FORMFEED:
                                    {
                                        char = "\\f";
                                        break;
                                    }
                                case VERTICALTAB:
                                    {
                                        char = "\\v";
                                        break;
                                    }
                                // &
                                case AND:
                                    {
                                        // inverted selector pattern i.e html &
                                        if (quote + comment + bracket === 0 && cascade > 0) {
                                            invert = 1;
                                            format = 1;
                                            char = "\f" + char;
                                        }
                                        break;
                                    }
                                // ::p<l>aceholder, l
                                // :read-on<l>y, l
                                case 108:
                                    {
                                        if (quote + comment + bracket + pattern === 0 && pseudo > 0) {
                                            switch(caret - pseudo){
                                                // ::placeholder
                                                case 2:
                                                    {
                                                        if (tail === PLACEHOLDER && body.charCodeAt(caret - 3) === COLON) {
                                                            pattern = tail;
                                                        }
                                                    }
                                                // :read-only
                                                case 8:
                                                    {
                                                        if (trail === READONLY) {
                                                            pattern = trail;
                                                        }
                                                    }
                                            }
                                        }
                                        break;
                                    }
                                // :<pattern>
                                case COLON:
                                    {
                                        if (quote + comment + bracket === 0) {
                                            pseudo = caret;
                                        }
                                        break;
                                    }
                                // selectors
                                case COMMA:
                                    {
                                        if (comment + parentheses + quote + bracket === 0) {
                                            format = 1;
                                            char += "\r";
                                        }
                                        break;
                                    }
                                // quotes
                                case DOUBLEQUOTE:
                                case SINGLEQUOTE:
                                    {
                                        if (comment === 0) {
                                            quote = quote === code ? 0 : quote === 0 ? code : quote;
                                        }
                                        break;
                                    }
                                // attributes
                                case OPENBRACKET:
                                    {
                                        if (quote + comment + parentheses === 0) {
                                            bracket++;
                                        }
                                        break;
                                    }
                                case CLOSEBRACKET:
                                    {
                                        if (quote + comment + parentheses === 0) {
                                            bracket--;
                                        }
                                        break;
                                    }
                                // functions
                                case CLOSEPARENTHESES:
                                    {
                                        if (quote + comment + bracket === 0) {
                                            parentheses--;
                                        }
                                        break;
                                    }
                                case OPENPARENTHESES:
                                    {
                                        if (quote + comment + bracket === 0) {
                                            if (context === 0) {
                                                switch(tail * 2 + trail * 3){
                                                    // :matches
                                                    case 533:
                                                        {
                                                            break;
                                                        }
                                                    // :global, :not, :nth-child etc...
                                                    default:
                                                        {
                                                            counter = 0;
                                                            context = 1;
                                                        }
                                                }
                                            }
                                            parentheses++;
                                        }
                                        break;
                                    }
                                case AT:
                                    {
                                        if (comment + parentheses + quote + bracket + pseudo + atrule === 0) {
                                            atrule = 1;
                                        }
                                        break;
                                    }
                                // block/line comments
                                case STAR:
                                case FOWARDSLASH:
                                    {
                                        if (quote + bracket + parentheses > 0) {
                                            break;
                                        }
                                        switch(comment){
                                            // initialize line/block comment context
                                            case 0:
                                                {
                                                    switch(code * 2 + body.charCodeAt(caret + 1) * 3){
                                                        // //
                                                        case 235:
                                                            {
                                                                comment = FOWARDSLASH;
                                                                break;
                                                            }
                                                        // /*
                                                        case 220:
                                                            {
                                                                length = caret;
                                                                comment = STAR;
                                                                break;
                                                            }
                                                    }
                                                    break;
                                                }
                                            // end block comment context
                                            case STAR:
                                                {
                                                    if (code === FOWARDSLASH && tail === STAR && length + 2 !== caret) {
                                                        // /*<!> ... */, !
                                                        if (body.charCodeAt(length + 2) === 33) {
                                                            out += body.substring(length, caret + 1);
                                                        }
                                                        char = "";
                                                        comment = 0;
                                                    }
                                                }
                                        }
                                    }
                            }
                            // ignore comment blocks
                            if (comment === 0) {
                                // aggressive isolation mode, divide each individual selector
                                // including selectors in :not function but excluding selectors in :global function
                                if (cascade + quote + bracket + atrule === 0 && id !== KEYFRAME && code !== SEMICOLON) {
                                    switch(code){
                                        case COMMA:
                                        case TILDE:
                                        case GREATERTHAN:
                                        case PLUS:
                                        case CLOSEPARENTHESES:
                                        case OPENPARENTHESES:
                                            {
                                                if (context === 0) {
                                                    // outside of an isolated context i.e nth-child(<...>)
                                                    switch(tail){
                                                        case TAB:
                                                        case SPACE:
                                                        case NEWLINE:
                                                        case CARRIAGE:
                                                            {
                                                                char = char + "\0";
                                                                break;
                                                            }
                                                        default:
                                                            {
                                                                char = "\0" + char + (code === COMMA ? "" : "\0");
                                                            }
                                                    }
                                                    format = 1;
                                                } else {
                                                    // within an isolated context, sleep untill it's terminated
                                                    switch(code){
                                                        case OPENPARENTHESES:
                                                            {
                                                                // :globa<l>(
                                                                if (pseudo + 7 === caret && tail === 108) {
                                                                    pseudo = 0;
                                                                }
                                                                context = ++counter;
                                                                break;
                                                            }
                                                        case CLOSEPARENTHESES:
                                                            {
                                                                if ((context = --counter) === 0) {
                                                                    format = 1;
                                                                    char += "\0";
                                                                }
                                                                break;
                                                            }
                                                    }
                                                }
                                                break;
                                            }
                                        case TAB:
                                        case SPACE:
                                            {
                                                switch(tail){
                                                    case NULL:
                                                    case OPENBRACES:
                                                    case CLOSEBRACES:
                                                    case SEMICOLON:
                                                    case COMMA:
                                                    case FORMFEED:
                                                    case TAB:
                                                    case SPACE:
                                                    case NEWLINE:
                                                    case CARRIAGE:
                                                        {
                                                            break;
                                                        }
                                                    default:
                                                        {
                                                            // ignore in isolated contexts
                                                            if (context === 0) {
                                                                format = 1;
                                                                char += "\0";
                                                            }
                                                        }
                                                }
                                            }
                                    }
                                }
                                // concat buffer of characters
                                chars += char;
                                // previous non-whitespace character code
                                if (code !== SPACE && code !== TAB) {
                                    peak = code;
                                }
                            }
                        }
                }
                // tail character codes
                trail = tail;
                tail = code;
                // visit every character
                caret++;
            }
            length = out.length;
            // preserve empty selector
            if (preserve > 0) {
                if (length === 0 && children.length === 0 && current[0].length === 0 === false) {
                    if (id !== MEDIA || current.length === 1 && (cascade > 0 ? nscopealt : nscope) === current[0]) {
                        length = current.join(",").length + 2;
                    }
                }
            }
            if (length > 0) {
                // cascade isolation mode?
                selector = cascade === 0 && id !== KEYFRAME ? isolate(current) : current;
                // execute plugins, block context
                if (plugged > 0) {
                    result = proxy(BLCKS, out, selector, parent, line, column, length, id, depth, id);
                    if (result !== void 0 && (out = result).length === 0) {
                        return flat + out + children;
                    }
                }
                out = selector.join(",") + "{" + out + "}";
                if (prefix * pattern !== 0) {
                    if (prefix === 2 && !vendor(out, 2)) pattern = 0;
                    switch(pattern){
                        // ::read-only
                        case READONLY:
                            {
                                out = out.replace(readonlyptn, ":" + moz + "$1") + out;
                                break;
                            }
                        // ::placeholder
                        case PLACEHOLDER:
                            {
                                out = out.replace(plcholdrptn, "::" + webkit + "input-$1") + out.replace(plcholdrptn, "::" + moz + "$1") + out.replace(plcholdrptn, ":" + ms + "input-$1") + out;
                                break;
                            }
                    }
                    pattern = 0;
                }
            }
            return flat + out + children;
        }
        /**
		 * Property
		 *
		 * @param {string} input
		 * @param {number} first
		 * @param {number} second
		 * @param {number} third
		 * @return {string}
		 */ function property(input, first, second, third) {
            var index = 0;
            var out = input + ";";
            var hash = first * 2 + second * 3 + third * 4;
            var cache;
            // animation: a, n, i characters
            if (hash === 944) {
                return animation(out);
            } else if (prefix === 0 || prefix === 2 && !vendor(out, 1)) {
                return out;
            }
            // vendor prefix
            switch(hash){
                // text-decoration/text-size-adjust/text-shadow/text-align/text-transform: t, e, x
                case 1015:
                    {
                        // text-shadow/text-align/text-transform, a
                        return out.charCodeAt(10) === 97 ? webkit + out + out : out;
                    }
                // filter/fill f, i, l
                case 951:
                    {
                        // filter, t
                        return out.charCodeAt(3) === 116 ? webkit + out + out : out;
                    }
                // color/column, c, o, l
                case 963:
                    {
                        // column, n
                        return out.charCodeAt(5) === 110 ? webkit + out + out : out;
                    }
                // box-decoration-break, b, o, x
                case 1009:
                    {
                        if (out.charCodeAt(4) !== 100) {
                            break;
                        }
                    }
                // mask, m, a, s
                // clip-path, c, l, i
                case 969:
                case 942:
                    {
                        return webkit + out + out;
                    }
                // appearance: a, p, p
                case 978:
                    {
                        return webkit + out + moz + out + out;
                    }
                // hyphens: h, y, p
                // user-select: u, s, e
                case 1019:
                case 983:
                    {
                        return webkit + out + moz + out + ms + out + out;
                    }
                // background/backface-visibility, b, a, c
                case 883:
                    {
                        // backface-visibility, -
                        if (out.charCodeAt(8) === DASH) {
                            return webkit + out + out;
                        }
                        // image-set(...)
                        if (out.indexOf("image-set(", 11) > 0) {
                            return out.replace(imgsrcptn, "$1" + webkit + "$2") + out;
                        }
                        return out;
                    }
                // flex: f, l, e
                case 932:
                    {
                        if (out.charCodeAt(4) === DASH) {
                            switch(out.charCodeAt(5)){
                                // flex-grow, g
                                case 103:
                                    {
                                        return webkit + "box-" + out.replace("-grow", "") + webkit + out + ms + out.replace("grow", "positive") + out;
                                    }
                                // flex-shrink, s
                                case 115:
                                    {
                                        return webkit + out + ms + out.replace("shrink", "negative") + out;
                                    }
                                // flex-basis, b
                                case 98:
                                    {
                                        return webkit + out + ms + out.replace("basis", "preferred-size") + out;
                                    }
                            }
                        }
                        return webkit + out + ms + out + out;
                    }
                // order: o, r, d
                case 964:
                    {
                        return webkit + out + ms + "flex" + "-" + out + out;
                    }
                // justify-items/justify-content, j, u, s
                case 1023:
                    {
                        // justify-content, c
                        if (out.charCodeAt(8) !== 99) {
                            break;
                        }
                        cache = out.substring(out.indexOf(":", 15)).replace("flex-", "").replace("space-between", "justify");
                        return webkit + "box-pack" + cache + webkit + out + ms + "flex-pack" + cache + out;
                    }
                // cursor, c, u, r
                case 1005:
                    {
                        return cursorptn.test(out) ? out.replace(colonptn, ":" + webkit) + out.replace(colonptn, ":" + moz) + out : out;
                    }
                // writing-mode, w, r, i
                case 1000:
                    {
                        cache = out.substring(13).trim();
                        index = cache.indexOf("-") + 1;
                        switch(cache.charCodeAt(0) + cache.charCodeAt(index)){
                            // vertical-lr
                            case 226:
                                {
                                    cache = out.replace(writingptn, "tb");
                                    break;
                                }
                            // vertical-rl
                            case 232:
                                {
                                    cache = out.replace(writingptn, "tb-rl");
                                    break;
                                }
                            // horizontal-tb
                            case 220:
                                {
                                    cache = out.replace(writingptn, "lr");
                                    break;
                                }
                            default:
                                {
                                    return out;
                                }
                        }
                        return webkit + out + ms + cache + out;
                    }
                // position: sticky
                case 1017:
                    {
                        if (out.indexOf("sticky", 9) === -1) {
                            return out;
                        }
                    }
                // display(flex/inline-flex/inline-box): d, i, s
                case 975:
                    {
                        index = (out = input).length - 10;
                        cache = (out.charCodeAt(index) === 33 ? out.substring(0, index) : out).substring(input.indexOf(":", 7) + 1).trim();
                        switch(hash = cache.charCodeAt(0) + (cache.charCodeAt(7) | 0)){
                            // inline-
                            case 203:
                                {
                                    // inline-box
                                    if (cache.charCodeAt(8) < 111) {
                                        break;
                                    }
                                }
                            // inline-box/sticky
                            case 115:
                                {
                                    out = out.replace(cache, webkit + cache) + ";" + out;
                                    break;
                                }
                            // inline-flex
                            // flex
                            case 207:
                            case 102:
                                {
                                    out = out.replace(cache, webkit + (hash > 102 ? "inline-" : "") + "box") + ";" + out.replace(cache, webkit + cache) + ";" + out.replace(cache, ms + cache + "box") + ";" + out;
                                }
                        }
                        return out + ";";
                    }
                // align-items, align-center, align-self: a, l, i, -
                case 938:
                    {
                        if (out.charCodeAt(5) === DASH) {
                            switch(out.charCodeAt(6)){
                                // align-items, i
                                case 105:
                                    {
                                        cache = out.replace("-items", "");
                                        return webkit + out + webkit + "box-" + cache + ms + "flex-" + cache + out;
                                    }
                                // align-self, s
                                case 115:
                                    {
                                        return webkit + out + ms + "flex-item-" + out.replace(selfptn, "") + out;
                                    }
                                // align-content
                                default:
                                    {
                                        return webkit + out + ms + "flex-line-pack" + out.replace("align-content", "").replace(selfptn, "") + out;
                                    }
                            }
                        }
                        break;
                    }
                // min/max
                case 973:
                case 989:
                    {
                        // min-/max- height/width/block-size/inline-size
                        if (out.charCodeAt(3) !== DASH || out.charCodeAt(4) === 122) {
                            break;
                        }
                    }
                // height/width: min-content / width: max-content
                case 931:
                case 953:
                    {
                        if (dimensionptn.test(input) === true) {
                            // stretch
                            if ((cache = input.substring(input.indexOf(":") + 1)).charCodeAt(0) === 115) return property(input.replace("stretch", "fill-available"), first, second, third).replace(":fill-available", ":stretch");
                            else return out.replace(cache, webkit + cache) + out.replace(cache, moz + cache.replace("fill-", "")) + out;
                        }
                        break;
                    }
                // transform, transition: t, r, a
                case 962:
                    {
                        out = webkit + out + (out.charCodeAt(5) === 102 ? ms + out : "") + out;
                        // transitions
                        if (second + third === 211 && out.charCodeAt(13) === 105 && out.indexOf("transform", 10) > 0) {
                            return out.substring(0, out.indexOf(";", 27) + 1).replace(transformptn, "$1" + webkit + "$2") + out;
                        }
                        break;
                    }
            }
            return out;
        }
        /**
		 * Use
		 *
		 * @param {(Array<function(...?)>|function(...?)|number|void)?} plugin
		 */ function use(plugin) {
            switch(plugin){
                case void 0:
                case null:
                    {
                        plugged = plugins.length = 0;
                        break;
                    }
                default:
                    {
                        if (typeof plugin === "function") {
                            plugins[plugged++] = plugin;
                        } else if (typeof plugin === "object") {
                            for(var i = 0, length = plugin.length; i < length; ++i){
                                use(plugin[i]);
                            }
                        } else {
                            unkwn = !!plugin | 0;
                        }
                    }
            }
            return use;
        }
        /**
		 * Set
		 *
		 * @param {*} options
		 */ function set(options) {
            for(var name in options){
                var value = options[name];
                switch(name){
                    case "keyframe":
                        keyed = value | 0;
                        break;
                    case "global":
                        escape = value | 0;
                        break;
                    case "cascade":
                        cascade = value | 0;
                        break;
                    case "compress":
                        compress = value | 0;
                        break;
                    case "semicolon":
                        semicolon = value | 0;
                        break;
                    case "preserve":
                        preserve = value | 0;
                        break;
                    case "prefix":
                        should = null;
                        if (!value) {
                            prefix = 0;
                        } else if (typeof value !== "function") {
                            prefix = 1;
                        } else {
                            prefix = 2;
                            should = value;
                        }
                }
            }
            return set;
        }
        /**
		 * Stylis
		 *
		 * @param {string} selector
		 * @param {string} input
		 * @return {*}
		 */ function stylis(selector, input) {
            if (this !== void 0 && this.constructor === stylis) {
                return factory(selector);
            }
            // setup
            var ns = selector;
            var code = ns.charCodeAt(0);
            // trim leading whitespace
            if (code < 33) {
                code = (ns = ns.trim()).charCodeAt(0);
            }
            // keyframe/animation namespace
            if (keyed > 0) {
                key = ns.replace(invalidptn, code === OPENBRACKET ? "" : "-");
            }
            // reset, used to assert if a plugin is moneky-patching the return value
            code = 1;
            // cascade/isolate
            if (cascade === 1) {
                nscope = ns;
            } else {
                nscopealt = ns;
            }
            var selectors = [
                nscope
            ];
            var result;
            // execute plugins, pre-process context
            if (plugged > 0) {
                result = proxy(PREPS, input, selectors, selectors, line, column, 0, 0, 0, 0);
                if (result !== void 0 && typeof result === "string") {
                    input = result;
                }
            }
            // build
            var output = compile(array, selectors, input, 0, 0);
            // execute plugins, post-process context
            if (plugged > 0) {
                result = proxy(POSTS, output, selectors, selectors, line, column, output.length, 0, 0, 0);
                // bypass minification
                if (result !== void 0 && typeof (output = result) !== "string") {
                    code = 0;
                }
            }
            // reset
            key = "";
            nscope = "";
            nscopealt = "";
            pattern = 0;
            line = 1;
            column = 1;
            return compress * code === 0 ? output : minify(output);
        }
        stylis["use"] = use;
        stylis["set"] = set;
        if (options !== void 0) {
            set(options);
        }
        return stylis;
    });
})(stylis$1);
var Stylis = stylis$1.exports;

var stylisRuleSheet$1 = {exports: {}};

(function(module, exports) {
    (function(factory) {
        module["exports"] = factory() ;
    })(function() {
        return function(insertRule) {
            var toSheet = function toSheet(block) {
                if (block) try {
                    insertRule(block + "}");
                } catch (e) {}
            };
            var delimiter = "/*|*/";
            var needle = delimiter + "}";
            return function ruleSheet(context, content, selectors, parents, line, column, length, ns, depth, at) {
                switch(context){
                    // property
                    case 1:
                        // @import
                        if (depth === 0 && content.charCodeAt(0) === 64) return insertRule(content + ";"), "";
                        break;
                    // selector
                    case 2:
                        if (ns === 0) return content + delimiter;
                        break;
                    // at-rule
                    case 3:
                        switch(ns){
                            // @font-face, @page
                            case 102:
                            case 112:
                                return insertRule(selectors[0] + content), "";
                            default:
                                return content + (at === 0 ? delimiter : "");
                        }
                    case -2:
                        content.split(needle).forEach(toSheet);
                }
            };
        };
    });
})(stylisRuleSheet$1);
var stylisRuleSheet = stylisRuleSheet$1.exports;

var stylis = new Stylis();
function disableNestingPlugin() {
    for(var _len = arguments.length, args = new Array(_len), _key = 0; _key < _len; _key++){
        args[_key] = arguments[_key];
    }
    var context = args[0], tmp = args[3], parent = tmp === void 0 ? [] : tmp, line = args[4], column = args[5];
    if (context === 2) {
        // replace null characters and trim
        // eslint-disable-next-line no-control-regex
        parent = (parent[0] || "").replace(/\u0000/g, "").trim();
        if (parent.length > 0 && parent.charAt(0) !== "@") {
            throw new Error("Nesting detected at " + line + ":" + column + ". " + "Unfortunately nesting is not supported by styled-jsx.");
        }
    }
}
var generator;
var filename;
var offset;
function sourceMapsPlugin() {
    for(var _len = arguments.length, args = new Array(_len), _key = 0; _key < _len; _key++){
        args[_key] = arguments[_key];
    }
    var context = args[0], line = args[4], column = args[5], length = args[6];
    // Pre-processed, init source map
    if (context === -1 && generator !== undefined) {
        generator.addMapping({
            generated: {
                line: 1,
                column: 0
            },
            source: filename,
            original: offset
        });
        return;
    }
    // Post-processed
    if (context === -2 && generator !== undefined) {
        generator = undefined;
        offset = undefined;
        filename = undefined;
        return;
    }
    // Selector/property, update source map
    if ((context === 1 || context === 2) && generator !== undefined) {
        generator.addMapping({
            generated: {
                line: 1,
                column: length
            },
            source: filename,
            original: {
                line: line + offset.line,
                column: column + offset.column
            }
        });
    }
}
/**
 * splitRulesPlugin
 * Used to split a blob of css into an array of rules
 * that can inserted via sheet.insertRule
 */ var splitRules = [];
var splitRulesPlugin = stylisRuleSheet(function(rule) {
    splitRules.push(rule);
});
stylis.use(disableNestingPlugin);
stylis.use(sourceMapsPlugin);
stylis.use(splitRulesPlugin);
stylis.set({
    cascade: false,
    compress: true
});
/**
 * Public transform function
 *
 * @param {String} hash
 * @param {String} styles
 * @param {Object} settings
 * @return {string}
 */ function transform(hash, styles, settings) {
    if (settings === void 0) settings = {};
    generator = settings.generator;
    offset = settings.offset;
    filename = settings.filename;
    splitRules = [];
    stylis.set({
        prefix: typeof settings.vendorPrefixes === "boolean" ? settings.vendorPrefixes : true
    });
    stylis(hash, styles);
    if (settings.splitRules) {
        return splitRules;
    }
    return splitRules.join("");
}

var GLOBAL_ATTRIBUTE = "global";
var STYLE_ATTRIBUTE = "jsx";
var STYLE_COMPONENT = "_JSXStyle";
var STYLE_COMPONENT_DYNAMIC = "dynamic";
var STYLE_COMPONENT_ID = "id";

function _extends$2() {
    _extends$2 = Object.assign || function(target) {
        for(var i = 1; i < arguments.length; i++){
            var source = arguments[i];
            for(var key in source){
                if (Object.prototype.hasOwnProperty.call(source, key)) {
                    target[key] = source[key];
                }
            }
        }
        return target;
    };
    return _extends$2.apply(this, arguments);
}
var _typeof = function(obj) {
    "@swc/helpers - typeof";
    return obj && typeof Symbol !== "undefined" && obj.constructor === Symbol ? "symbol" : typeof obj;
};
var concat = function(a, b) {
    return lib$1.binaryExpression("+", a, b);
};
var and = function(a, b) {
    return lib$1.logicalExpression("&&", a, b);
};
var or = function(a, b) {
    return lib$1.logicalExpression("||", a, b);
};
var joinSpreads = function(spreads) {
    return spreads.reduce(function(acc, curr) {
        return or(acc, curr);
    });
};
var hashString = function(str) {
    return String(stringHash(str));
};
var addClassName = function(path, jsxId) {
    var jsxIdWithSpace = concat(jsxId, lib$1.stringLiteral(" "));
    var attributes = path.get("attributes");
    var spreads = [];
    var className = null;
    // Find className and collect spreads
    for(var i = attributes.length - 1, attr; attr = attributes[i]; i--){
        var node = attr.node;
        if (lib$1.isJSXSpreadAttribute(attr)) {
            if (lib$1.isObjectExpression(node.argument)) {
                var properties = node.argument.properties;
                var index = properties.findIndex(function(property) {
                    return property.key.name === "className";
                });
                if (~index) {
                    className = attr.get("argument").get("properties." + index);
                    // Remove jsx spread attribute if there is only className property
                    if (properties.length === 1) {
                        attr.remove();
                    }
                    break;
                }
            }
            if (lib$1.isMemberExpression(node.argument) || lib$1.isIdentifier(node.argument)) {
                var name = node.argument.name;
                var spreadObj = lib$1.isMemberExpression(node.argument) ? node.argument : lib$1.identifier(name);
                var attrNameDotClassName = lib$1.memberExpression(spreadObj, lib$1.identifier("className"));
                spreads.push(// `${name} && ${name}.className != null && ${name}.className`
                and(spreadObj, and(lib$1.binaryExpression("!=", attrNameDotClassName, lib$1.nullLiteral()), attrNameDotClassName)));
            }
            continue;
        }
        if (lib$1.isJSXAttribute(attr) && node.name.name === "className") {
            className = attributes[i];
            break;
        }
    }
    if (className) {
        var newClassName = className.node.value.expression || className.node.value;
        newClassName = lib$1.isStringLiteral(newClassName) || lib$1.isTemplateLiteral(newClassName) ? newClassName : or(newClassName, lib$1.stringLiteral(""));
        className.remove();
        className = lib$1.jSXExpressionContainer(spreads.length === 0 ? concat(jsxIdWithSpace, newClassName) : concat(jsxIdWithSpace, or(joinSpreads(spreads), newClassName)));
    } else {
        className = lib$1.jSXExpressionContainer(spreads.length === 0 ? jsxId : concat(jsxIdWithSpace, or(joinSpreads(spreads), lib$1.stringLiteral(""))));
    }
    path.node.attributes.push(lib$1.jSXAttribute(lib$1.jSXIdentifier("className"), className));
};
var getScope = function(path) {
    return (path.findParent(function(path) {
        return path.isFunctionDeclaration() || path.isArrowFunctionExpression() || path.isClassMethod();
    }) || path).scope;
};
var isGlobalEl = function(el) {
    return el && el.attributes.some(function(param) {
        var name = param.name;
        return name && name.name === GLOBAL_ATTRIBUTE;
    });
};
var isStyledJsx = function(param) {
    var el = param.node;
    return lib$1.isJSXElement(el) && el.openingElement.name.name === "style" && el.openingElement.attributes.some(function(attr) {
        return attr.name.name === STYLE_ATTRIBUTE;
    });
};
var findStyles = function(path) {
    if (isStyledJsx(path)) {
        var node = path.node;
        return isGlobalEl(node.openingElement) ? [
            path
        ] : [];
    }
    return path.get("children").filter(isStyledJsx);
};
var validateExternalExpressionsVisitor = {
    Identifier: function Identifier(path) {
        if (lib$1.isMemberExpression(path.parentPath)) {
            return;
        }
        var name = path.node.name;
        if (!path.scope.hasBinding(name)) {
            throw path.buildCodeFrameError(path.getSource());
        }
    },
    MemberExpression: function MemberExpression(path) {
        var node = path.node;
        if (!lib$1.isIdentifier(node.object)) {
            return;
        }
        if (!path.scope.hasBinding(node.object.name)) {
            throw path.buildCodeFrameError(path.getSource());
        }
    },
    ThisExpression: function ThisExpression(path) {
        throw new Error(path.parentPath.getSource());
    }
};
var validateExternalExpressions = function(path) {
    try {
        path.traverse(validateExternalExpressionsVisitor);
    } catch (error) {
        throw path.buildCodeFrameError("\n      Found an `undefined` or invalid value in your styles: `" + error.message + "`.\n\n      If you are trying to use dynamic styles in external files this is unfortunately not possible yet.\n      Please put the dynamic parts alongside the component. E.g.\n\n      <button>\n        <style jsx>{externalStylesReference}</style>\n        <style jsx>{`\n          button { background-color: ${" + error.message + "} }\n        `}</style>\n      </button>\n    ");
    }
};
var getJSXStyleInfo = function(expr, scope) {
    var node = expr.node;
    var location = node.loc;
    // Assume string literal
    if (lib$1.isStringLiteral(node)) {
        return {
            hash: hashString(node.value),
            css: node.value,
            expressions: [],
            dynamic: false,
            location: location
        };
    }
    // Simple template literal without expressions
    if (node.expressions.length === 0) {
        return {
            hash: hashString(node.quasis[0].value.raw),
            css: node.quasis[0].value.raw,
            expressions: [],
            dynamic: false,
            location: location
        };
    }
    // Special treatment for template literals that contain expressions:
    //
    // Expressions are replaced with a placeholder
    // so that the CSS compiler can parse and
    // transform the css source string
    // without having to know about js literal expressions.
    // Later expressions are restored.
    //
    // e.g.
    // p { color: ${myConstant}; }
    // becomes
    // p { color: %%styled-jsx-placeholder-${id}%%; }
    var quasis = node.quasis, expressions = node.expressions;
    var hash = hashString(expr.getSource().slice(1, -1));
    var dynamic = Boolean(scope);
    if (dynamic) {
        try {
            var val = expr.evaluate();
            if (val.confident) {
                dynamic = false;
            } else if (val.deopt) {
                var computedObject = val.deopt.get("object").resolve().evaluate();
                dynamic = !computedObject.confident;
            }
        } catch (_) {}
    }
    var css = quasis.reduce(function(css, quasi, index) {
        return "" + css + quasi.value.raw + (quasis.length === index + 1 ? "" : "%%styled-jsx-placeholder-" + index + "%%");
    }, "");
    return {
        hash: hash,
        css: css,
        expressions: expressions,
        dynamic: dynamic,
        location: location
    };
};
var computeClassNames = function(styles, externalJsxId, styleComponentImportName) {
    if (styles.length === 0) {
        return {
            className: externalJsxId
        };
    }
    var hashes = styles.reduce(function(acc, styles) {
        if (styles.dynamic === false) {
            acc.static.push(styles.hash);
        } else {
            acc.dynamic.push(styles);
        }
        return acc;
    }, {
        static: [],
        dynamic: []
    });
    var staticClassName = "jsx-" + hashString(hashes.static.join(","));
    // Static and optionally external classes. E.g.
    // '[jsx-externalClasses] jsx-staticClasses'
    if (hashes.dynamic.length === 0) {
        return {
            staticClassName: staticClassName,
            className: externalJsxId ? concat(lib$1.stringLiteral(staticClassName + " "), externalJsxId) : lib$1.stringLiteral(staticClassName)
        };
    }
    // _JSXStyle.dynamic([ ['1234', [props.foo, bar, fn(props)]], ... ])
    var dynamic = lib$1.callExpression(// Callee: _JSXStyle.dynamic
    lib$1.memberExpression(lib$1.identifier(styleComponentImportName), lib$1.identifier(STYLE_COMPONENT_DYNAMIC)), // Arguments
    [
        lib$1.arrayExpression(hashes.dynamic.map(function(styles) {
            return lib$1.arrayExpression([
                lib$1.stringLiteral(hashString(styles.hash + staticClassName)),
                lib$1.arrayExpression(styles.expressions)
            ]);
        }))
    ]);
    // Dynamic and optionally external classes. E.g.
    // '[jsx-externalClasses] ' + _JSXStyle.dynamic([ ['1234', [props.foo, bar, fn(props)]], ... ])
    if (hashes.static.length === 0) {
        return {
            staticClassName: staticClassName,
            className: externalJsxId ? concat(concat(externalJsxId, lib$1.stringLiteral(" ")), dynamic) : dynamic
        };
    }
    // Static, dynamic and optionally external classes. E.g.
    // '[jsx-externalClasses] jsx-staticClasses ' + _JSXStyle.dynamic([ ['5678', [props.foo, bar, fn(props)]], ... ])
    return {
        staticClassName: staticClassName,
        className: externalJsxId ? concat(concat(externalJsxId, lib$1.stringLiteral(" " + staticClassName + " ")), dynamic) : concat(lib$1.stringLiteral("" + staticClassName + " "), dynamic)
    };
};
var templateLiteralFromPreprocessedCss = function(css, expressions) {
    var quasis = [];
    var finalExpressions = [];
    var parts = css.split(/(?:%%styled-jsx-placeholder-(\d+)%%)/g);
    if (parts.length === 1) {
        return lib$1.stringLiteral(css);
    }
    parts.forEach(function(part, index) {
        if (index % 2 > 0) {
            // This is necessary because, after preprocessing, declarations might have been alterate.
            // eg. properties are auto prefixed and therefore expressions need to match.
            finalExpressions.push(expressions[part]);
        } else {
            quasis.push(part);
        }
    });
    return lib$1.templateLiteral(quasis.map(function(quasi, index) {
        return lib$1.templateElement({
            raw: quasi,
            cooked: quasi
        }, quasis.length === index + 1);
    }), finalExpressions);
};
var cssToBabelType = function(css) {
    if (typeof css === "string") {
        return lib$1.stringLiteral(css);
    }
    if (Array.isArray(css)) {
        return lib$1.arrayExpression(css);
    }
    return lib$1.cloneDeep(css);
};
var makeStyledJsxTag = function(id, transformedCss, expressions, styleComponentImportName) {
    if (expressions === void 0) expressions = [];
    var css = cssToBabelType(transformedCss);
    var attributes = [
        lib$1.jSXAttribute(lib$1.jSXIdentifier(STYLE_COMPONENT_ID), lib$1.jSXExpressionContainer(typeof id === "string" ? lib$1.stringLiteral(id) : id))
    ];
    if (expressions.length > 0) {
        attributes.push(lib$1.jSXAttribute(lib$1.jSXIdentifier(STYLE_COMPONENT_DYNAMIC), lib$1.jSXExpressionContainer(lib$1.arrayExpression(expressions))));
    }
    return lib$1.jSXElement(lib$1.jSXOpeningElement(lib$1.jSXIdentifier(styleComponentImportName), attributes), lib$1.jSXClosingElement(lib$1.jSXIdentifier(styleComponentImportName)), [
        lib$1.jSXExpressionContainer(css)
    ]);
};
var makeSourceMapGenerator = function(file) {
    var filename = file.sourceFileName;
    var generator = new SourceMapGenerator({
        file: filename,
        sourceRoot: file.sourceRoot
    });
    generator.setSourceContent(filename, file.code);
    return generator;
};
var addSourceMaps = function(code, generator, filename) {
    var sourceMaps = [
        convertSourceMap.fromObject(generator).toComment({
            multiline: true
        }),
        "/*@ sourceURL=" + filename.replace(/\\/g, "\\\\") + " */"
    ];
    if (Array.isArray(code)) {
        return code.concat(sourceMaps);
    }
    return [
        code
    ].concat(sourceMaps).join("\n");
};
var combinedPluginsCache = {
    plugins: null,
    combined: null
};
var combinePlugins = function(plugins) {
    if (!plugins) {
        return function(css) {
            return css;
        };
    }
    var pluginsToString = JSON.stringify(plugins);
    if (combinedPluginsCache.plugins === pluginsToString) {
        return combinedPluginsCache.combined;
    }
    if (!Array.isArray(plugins) || plugins.some(function(p) {
        return !Array.isArray(p) && typeof p !== "string";
    })) {
        throw new Error("`plugins` must be an array of plugins names (string) or an array `[plugin-name, {options}]`");
    }
    combinedPluginsCache.plugins = pluginsToString;
    combinedPluginsCache.combined = plugins.map(function(plugin, i) {
        var options = {};
        if (Array.isArray(plugin)) {
            options = plugin[1] || {};
            plugin = plugin[0];
            if (Object.prototype.hasOwnProperty.call(options, "babel")) {
                throw new Error("\n            Error while trying to register the styled-jsx plugin: " + plugin + "\n            The option name `babel` is reserved.\n          ");
            }
        }
        log("Loading plugin from path: " + plugin);
        var p = require(plugin);
        if (p.default) {
            p = p.default;
        }
        var type = typeof p === "undefined" ? "undefined" : _typeof(p);
        if (type !== "function") {
            throw new Error("Expected plugin " + plugins[i] + " to be a function but instead got " + type);
        }
        return {
            plugin: p,
            options: options
        };
    }).reduce(function(previous, param) {
        var plugin = param.plugin, options = param.options;
        return function(css, babelOptions) {
            return plugin(previous ? previous(css, babelOptions) : css, _extends$2({}, options, {
                babel: babelOptions
            }));
        };
    }, null);
    return combinedPluginsCache.combined;
};
var getPrefix = function(isDynamic, id) {
    return isDynamic ? ".__jsx-style-dynamic-selector" : "." + id;
};
var processCss = function(stylesInfo, options) {
    var hash = stylesInfo.hash, css = stylesInfo.css, expressions = stylesInfo.expressions, dynamic = stylesInfo.dynamic, location = stylesInfo.location, file = stylesInfo.file, isGlobal = stylesInfo.isGlobal, plugins = stylesInfo.plugins, vendorPrefixes = stylesInfo.vendorPrefixes, sourceMaps = stylesInfo.sourceMaps;
    var fileInfo = {
        code: file.code,
        sourceRoot: file.opts.sourceRoot,
        filename: file.opts.filename || file.filename
    };
    fileInfo.sourceFileName = file.opts.sourceFileName || file.sourceFileName || // According to https://babeljs.io/docs/en/options#source-map-options
    // filenameRelative = path.relative(file.opts.cwd, file.opts.filename)
    // sourceFileName = path.basename(filenameRelative)
    // or simply
    // sourceFileName = path.basename(file.opts.filename)
    fileInfo.filename && require$$1__default["default"].basename(fileInfo.filename);
    var staticClassName = stylesInfo.staticClassName || "jsx-" + hashString(hash);
    var splitRules = options.splitRules;
    var useSourceMaps = Boolean(sourceMaps) && !splitRules;
    var pluginsOptions = {
        location: {
            start: _extends$2({}, location.start),
            end: _extends$2({}, location.end)
        },
        vendorPrefixes: vendorPrefixes,
        sourceMaps: useSourceMaps,
        isGlobal: isGlobal,
        filename: fileInfo.filename
    };
    var transformedCss;
    if (useSourceMaps) {
        var generator = makeSourceMapGenerator(fileInfo);
        var filename = fileInfo.sourceFileName;
        transformedCss = addSourceMaps(transform(isGlobal ? "" : getPrefix(dynamic, staticClassName), plugins(css, pluginsOptions), {
            generator: generator,
            offset: location.start,
            filename: filename,
            splitRules: splitRules,
            vendorPrefixes: vendorPrefixes
        }), generator, filename);
    } else {
        transformedCss = transform(isGlobal ? "" : getPrefix(dynamic, staticClassName), plugins(css, pluginsOptions), {
            splitRules: splitRules,
            vendorPrefixes: vendorPrefixes
        });
    }
    if (expressions.length > 0) {
        if (typeof transformedCss === "string") {
            transformedCss = templateLiteralFromPreprocessedCss(transformedCss, expressions);
        } else {
            transformedCss = transformedCss.map(function(transformedCss) {
                return templateLiteralFromPreprocessedCss(transformedCss, expressions);
            });
        }
    } else if (Array.isArray(transformedCss)) {
        transformedCss = transformedCss.map(function(transformedCss) {
            return lib$1.stringLiteral(transformedCss);
        });
    }
    return {
        hash: dynamic ? hashString(hash + staticClassName) : hashString(hash),
        css: transformedCss,
        expressions: dynamic && expressions
    };
};
var booleanOption = function(opts) {
    var ret;
    opts.some(function(opt) {
        if (typeof opt === "boolean") {
            ret = opt;
            return true;
        }
        return false;
    });
    return ret;
};
var createReactComponentImportDeclaration = function(state) {
    return lib$1.importDeclaration([
        lib$1.importDefaultSpecifier(lib$1.identifier(state.styleComponentImportName))
    ], lib$1.stringLiteral(state.styleModule));
};
var setStateOptions = function(state) {
    var vendorPrefixes = booleanOption([
        state.opts.vendorPrefixes,
        state.file.opts.vendorPrefixes
    ]);
    state.opts.vendorPrefixes = typeof vendorPrefixes === "boolean" ? vendorPrefixes : true;
    var sourceMaps = booleanOption([
        state.opts.sourceMaps,
        state.file.opts.sourceMaps
    ]);
    state.opts.sourceMaps = Boolean(sourceMaps);
    if (!state.plugins) {
        state.plugins = combinePlugins(state.opts.plugins);
    }
    state.styleModule = typeof state.opts.styleModule === "string" ? state.opts.styleModule : "styled-jsx/style";
};
function log(message) {
    console.log("[styled-jsx] " + message);
}

function _extends$1() {
    _extends$1 = Object.assign || function(target) {
        for(var i = 1; i < arguments.length; i++){
            var source = arguments[i];
            for(var key in source){
                if (Object.prototype.hasOwnProperty.call(source, key)) {
                    target[key] = source[key];
                }
            }
        }
        return target;
    };
    return _extends$1.apply(this, arguments);
}
var isModuleExports = lib$1.buildMatchMemberExpression("module.exports");
function processTaggedTemplateExpression(param) {
    var type = param.type, path = param.path, file = param.file, splitRules = param.splitRules, plugins = param.plugins, vendorPrefixes = param.vendorPrefixes, sourceMaps = param.sourceMaps, styleComponentImportName = param.styleComponentImportName;
    var templateLiteral = path.get("quasi");
    var scope;
    // Check whether there are undefined references or
    // references to this.something (e.g. props or state).
    // We allow dynamic styles only when resolving styles.
    if (type !== "resolve") {
        validateExternalExpressions(templateLiteral);
    } else if (!path.scope.path.isProgram()) {
        scope = getScope(path);
    }
    var stylesInfo = getJSXStyleInfo(templateLiteral, scope);
    var ref = computeClassNames([
        stylesInfo
    ], undefined, styleComponentImportName), staticClassName = ref.staticClassName, className = ref.className;
    var styles = processCss(_extends$1({}, stylesInfo, {
        staticClassName: staticClassName,
        file: file,
        isGlobal: type === "global",
        plugins: plugins,
        vendorPrefixes: vendorPrefixes,
        sourceMaps: sourceMaps
    }), {
        splitRules: splitRules
    });
    if (type === "resolve") {
        var hash = styles.hash, css = styles.css, expressions = styles.expressions;
        path.replaceWith(// {
        //   styles: <_JSXStyle ... />,
        //   className: 'jsx-123'
        // }
        lib$1.objectExpression([
            lib$1.objectProperty(lib$1.identifier("styles"), makeStyledJsxTag(hash, css, expressions, styleComponentImportName)),
            lib$1.objectProperty(lib$1.identifier("className"), className)
        ]));
        return;
    }
    var id = path.parentPath.node.id;
    var baseExportName = id ? id.name : "default";
    var parentPath = baseExportName === "default" ? path.parentPath : path.findParent(function(path) {
        return path.isVariableDeclaration() || path.isAssignmentExpression() && isModuleExports(path.get("left").node);
    });
    if (baseExportName !== "default" && !parentPath.parentPath.isProgram()) {
        parentPath = parentPath.parentPath;
    }
    var css1 = cssToBabelType(styles.css);
    var newPath = lib$1.isArrayExpression(css1) ? css1 : lib$1.newExpression(lib$1.identifier("String"), [
        css1
    ]);
    // default exports
    if (baseExportName === "default") {
        var defaultExportIdentifier = path.scope.generateUidIdentifier("defaultExport");
        parentPath.insertBefore(lib$1.variableDeclaration("const", [
            lib$1.variableDeclarator(defaultExportIdentifier, newPath)
        ]));
        parentPath.insertBefore(addHash(defaultExportIdentifier, styles.hash));
        path.replaceWith(defaultExportIdentifier);
        return;
    }
    // local and named exports
    parentPath.insertAfter(addHash(lib$1.identifier(baseExportName), styles.hash));
    path.replaceWith(newPath);
}
function addHash(exportIdentifier, hash) {
    var value = typeof hash === "string" ? lib$1.stringLiteral(hash) : hash;
    return lib$1.expressionStatement(lib$1.assignmentExpression("=", lib$1.memberExpression(exportIdentifier, lib$1.identifier("__hash")), value));
}
var visitor = {
    ImportDeclaration: function ImportDeclaration(path, state) {
        // import css from 'styled-jsx/css'
        if (path.node.source.value !== "styled-jsx/css") {
            return;
        }
        // Find all the imported specifiers.
        // e.g import css, { global, resolve } from 'styled-jsx/css'
        // -> ['css', 'global', 'resolve']
        var specifiersNames = path.node.specifiers.map(function(specifier) {
            return specifier.local.name;
        });
        specifiersNames.forEach(function(tagName) {
            // Get all the reference paths i.e. the places that use the tagName above
            // eg.
            // css`div { color: red }`
            // css.global`div { color: red }`
            // global`div { color: red `
            var binding = path.scope.getBinding(tagName);
            if (!binding || !Array.isArray(binding.referencePaths)) {
                return;
            }
            // Produces an object containing all the TaggedTemplateExpression paths detected.
            // The object contains { scoped, global, resolve }
            var taggedTemplateExpressions = binding.referencePaths.map(function(ref) {
                return ref.parentPath;
            }).reduce(function(result, path) {
                var taggedTemplateExpression;
                if (path.isTaggedTemplateExpression()) {
                    // css`` global`` resolve``
                    taggedTemplateExpression = path;
                } else if (path.parentPath && path.isMemberExpression() && path.parentPath.isTaggedTemplateExpression()) {
                    // This part is for css.global`` or css.resolve``
                    // using the default import css
                    taggedTemplateExpression = path.parentPath;
                } else {
                    return result;
                }
                var tag = taggedTemplateExpression.get("tag");
                var id = tag.isIdentifier() ? tag.node.name : tag.get("property").node.name;
                if (result[id]) {
                    result[id].push(taggedTemplateExpression);
                } else {
                    result.scoped.push(taggedTemplateExpression);
                }
                return result;
            }, {
                scoped: [],
                global: [],
                resolve: []
            });
            var hasJSXStyle = false;
            var _opts = state.opts, vendorPrefixes = _opts.vendorPrefixes, sourceMaps = _opts.sourceMaps;
            Object.keys(taggedTemplateExpressions).forEach(function(type) {
                return taggedTemplateExpressions[type].forEach(function(path) {
                    hasJSXStyle = true;
                    // Process each css block
                    processTaggedTemplateExpression({
                        type: type,
                        path: path,
                        file: state.file,
                        splitRules: typeof state.opts.optimizeForSpeed === "boolean" ? state.opts.optimizeForSpeed : process.env.NODE_ENV === "production",
                        plugins: state.plugins,
                        vendorPrefixes: vendorPrefixes,
                        sourceMaps: sourceMaps,
                        styleComponentImportName: state.styleComponentImportName
                    });
                });
            });
            var hasCssResolve = hasJSXStyle && taggedTemplateExpressions.resolve.length > 0;
            // When using the `resolve` helper we need to add an import
            // for the _JSXStyle component `styled-jsx/style`
            if (hasCssResolve) {
                state.file.hasCssResolve = true;
            }
        });
        // Finally remove the import
        path.remove();
    }
};

function babelMacro(param) {
    var createMacro = param.createMacro, MacroError = param.MacroError;
    var styledJsxMacro = function styledJsxMacro(param) {
        var references = param.references, state = param.state;
        setStateOptions(state);
        // Holds a reference to all the lines where strings are tagged using the `css` tag name.
        // We print a warning at the end of the macro in case there is any reference to css,
        // because `css` is generally used as default import name for 'styled-jsx/css'.
        // People who want to migrate from this macro to pure styled-jsx might have name conflicts issues.
        var cssReferences = [];
        // references looks like this
        // {
        //    default: [path, path],
        //    resolve: [path],
        // }
        Object.keys(references).forEach(function(refName) {
            // Enforce `resolve` as named import so people
            // can only import { resolve } from 'styled-jsx/macro'
            // or an alias of it eg. { resolve as foo }
            if (refName !== "default" && refName !== "resolve") {
                throw new MacroError("Imported an invalid named import: " + refName + ". Please import: resolve");
            }
            // Start processing the references for refName
            references[refName].forEach(function(path) {
                // We grab the parent path. Eg.
                // path -> css
                // path.parenPath -> css`div { color: red }`
                var templateExpression = path.parentPath;
                // templateExpression member expression?
                // path -> css
                // path.parentPath -> css.resolve
                if (templateExpression.isMemberExpression()) {
                    // grab .resolve
                    var tagPropertyName = templateExpression.get("property").node.name;
                    // Member expressions are only valid on default imports
                    // eg. import css from 'styled-jsx/macro'
                    if (refName !== "default") {
                        throw new MacroError("Can't use named import " + path.node.name + " as a member expression: " + path.node.name + "." + tagPropertyName + "`div { color: red }` Please use it directly: " + path.node.name + "`div { color: red }`");
                    }
                    // Otherwise enforce `css.resolve`
                    if (tagPropertyName !== "resolve") {
                        throw new MacroError("Using an invalid tag: " + tagPropertyName + ". Please use " + templateExpression.get("object").node.name + ".resolve");
                    }
                    // Grab the TaggedTemplateExpression
                    // i.e. css.resolve`div { color: red }`
                    templateExpression = templateExpression.parentPath;
                } else {
                    if (refName === "default") {
                        var name = path.node.name;
                        throw new MacroError("Can't use default import directly eg. " + name + "`div { color: red }`. Please use " + name + ".resolve`div { color: red }` instead.");
                    }
                    if (path.node.name === "css") {
                        // If the path node name is `css` we push it to the references above to emit a warning later.
                        cssReferences.push(path.node.loc.start.line);
                    }
                }
                if (!state.styleComponentImportName) {
                    var programPath = path.findParent(function(p) {
                        return p.isProgram();
                    });
                    state.styleComponentImportName = programPath.scope.generateUidIdentifier(STYLE_COMPONENT).name;
                    var importDeclaration = createReactComponentImportDeclaration(state);
                    programPath.unshiftContainer("body", importDeclaration);
                }
                // Finally transform the path :)
                processTaggedTemplateExpression({
                    type: "resolve",
                    path: templateExpression,
                    file: state.file,
                    splitRules: typeof state.opts.optimizeForSpeed === "boolean" ? state.opts.optimizeForSpeed : process.env.NODE_ENV === "production",
                    plugins: state.plugins,
                    vendorPrefixes: state.opts.vendorPrefixes,
                    sourceMaps: state.opts.sourceMaps,
                    styleComponentImportName: state.styleComponentImportName
                });
            });
        });
        if (cssReferences.length > 0) {
            console.warn("styled-jsx - Warning - We detected that you named your tag as `css` at lines: " + cssReferences.join(", ") + ".\n" + "This tag name is usually used as default import name for `styled-jsx/css`.\n" + "Porting macro code to pure styled-jsx in the future might be a bit problematic.");
        }
    };
    return createMacro(styledJsxMacro);
}

function babelTest() {
    return {
        inherits: default_1,
        visitor: {
            JSXOpeningElement: function JSXOpeningElement(path) {
                var el = path.node;
                var name = (el.name || {}).name;
                if (name !== "style") {
                    return;
                }
                el.attributes = el.attributes.filter(function(a) {
                    var name = a.name.name;
                    return name !== "jsx" && name !== "global";
                });
            }
        }
    };
}

function _arrayLikeToArray(arr, len) {
    if (len == null || len > arr.length) len = arr.length;
    for(var i = 0, arr2 = new Array(len); i < len; i++)arr2[i] = arr[i];
    return arr2;
}
function _arrayWithoutHoles(arr) {
    if (Array.isArray(arr)) return _arrayLikeToArray(arr);
}
function _extends() {
    _extends = Object.assign || function(target) {
        for(var i = 1; i < arguments.length; i++){
            var source = arguments[i];
            for(var key in source){
                if (Object.prototype.hasOwnProperty.call(source, key)) {
                    target[key] = source[key];
                }
            }
        }
        return target;
    };
    return _extends.apply(this, arguments);
}
function _iterableToArray(iter) {
    if (typeof Symbol !== "undefined" && iter[Symbol.iterator] != null || iter["@@iterator"] != null) return Array.from(iter);
}
function _nonIterableSpread() {
    throw new TypeError("Invalid attempt to spread non-iterable instance.\\nIn order to be iterable, non-array objects must have a [Symbol.iterator]() method.");
}
function _toConsumableArray(arr) {
    return _arrayWithoutHoles(arr) || _iterableToArray(arr) || _unsupportedIterableToArray(arr) || _nonIterableSpread();
}
function _unsupportedIterableToArray(o, minLen) {
    if (!o) return;
    if (typeof o === "string") return _arrayLikeToArray(o, minLen);
    var n = Object.prototype.toString.call(o).slice(8, -1);
    if (n === "Object" && o.constructor) n = o.constructor.name;
    if (n === "Map" || n === "Set") return Array.from(n);
    if (n === "Arguments" || /^(?:Ui|I)nt(?:8|16|32)(?:Clamped)?Array$/.test(n)) return _arrayLikeToArray(o, minLen);
}
function _createForOfIteratorHelperLoose(o, allowArrayLike) {
    var it = typeof Symbol !== "undefined" && o[Symbol.iterator] || o["@@iterator"];
    if (it) return (it = it.call(o)).next.bind(it);
    if (Array.isArray(o) || (it = unsupportedIterableToArray(o)) || allowArrayLike && o && typeof o.length === "number") {
        if (it) o = it;
        var i = 0;
        return function() {
            if (i >= o.length) return {
                done: true
            };
            return {
                done: false,
                value: o[i++]
            };
        };
    }
    throw new TypeError("Invalid attempt to iterate non-iterable instance.\\nIn order to be iterable, non-array objects must have a [Symbol.iterator]() method.");
}
function macro() {
    return babelMacro(require("babel-plugin-macros"));
}
function test() {
    return babelTest;
}
function babel(param) {
    var t = param.types;
    var jsxVisitors = {
        JSXOpeningElement: function JSXOpeningElement(path, state) {
            var el = path.node;
            var name = (el.name || {}).name;
            if (!state.hasJSXStyle) {
                return;
            }
            if (state.ignoreClosing === null) {
                // We keep a counter of elements inside so that we
                // can keep track of when we exit the parent to reset state
                // note: if we wished to add an option to turn off
                // selectors to reach parent elements, it would suffice to
                // set this to `1` and do an early return instead
                state.ignoreClosing = 0;
            }
            var tag = path.get("name");
            if (name && name !== "style" && name !== state.styleComponentImportName && (name.charAt(0) !== name.charAt(0).toUpperCase() || Object.values(path.scope.bindings).some(function(binding) {
                return binding.referencePaths.some(function(r) {
                    return r === tag;
                });
            }))) {
                if (state.className) {
                    addClassName(path, state.className);
                }
            }
            state.ignoreClosing++;
        // Next visit will be: JSXElement exit()
        },
        JSXElement: {
            enter: function enter(path, state) {
                if (state.hasJSXStyle !== null) {
                    return;
                }
                var styles = findStyles(path);
                if (styles.length === 0) {
                    return;
                }
                state.styles = [];
                state.externalStyles = [];
                var scope = getScope(path);
                for(var _iterator = _createForOfIteratorHelperLoose(styles), _step; !(_step = _iterator()).done;){
                    var style = _step.value;
                    // Compute children excluding whitespace
                    var children = style.get("children").filter(function(c) {
                        return t.isJSXExpressionContainer(c.node) || // Ignore whitespace around the expression container
                        t.isJSXText(c.node) && c.node.value.trim() !== "";
                    });
                    if (children.length !== 1) {
                        throw path.buildCodeFrameError("Expected one child under " + ("JSX Style tag, but got " + children.length + " ") + "(eg: <style jsx>{`hi`}</style>)");
                    }
                    var child = children[0];
                    if (!t.isJSXExpressionContainer(child)) {
                        throw path.buildCodeFrameError("Expected a child of " + "type JSXExpressionContainer under JSX Style tag " + ("(eg: <style jsx>{`hi`}</style>), got " + child.type));
                    }
                    var expression = child.get("expression");
                    if (t.isIdentifier(expression)) {
                        var idName = expression.node.name;
                        if (expression.scope.hasBinding(idName)) {
                            var externalStylesIdentifier = t.identifier(idName);
                            var isGlobal = isGlobalEl(style.get("openingElement").node);
                            state.externalStyles.push([
                                t.memberExpression(externalStylesIdentifier, t.identifier("__hash")),
                                externalStylesIdentifier,
                                isGlobal
                            ]);
                            continue;
                        }
                        throw path.buildCodeFrameError("The Identifier " + ("`" + expression.getSource() + "` is either `undefined` or ") + "it is not an external StyleSheet reference i.e. " + "it doesn't come from an `import` or `require` statement");
                    }
                    if (!t.isTemplateLiteral(expression) && !t.isStringLiteral(expression)) {
                        throw path.buildCodeFrameError("Expected a template " + "literal or String literal as the child of the " + "JSX Style tag (eg: <style jsx>{`some css`}</style>)," + (" but got " + expression.type));
                    }
                    state.styles.push(getJSXStyleInfo(expression, scope));
                }
                var externalJsxId;
                if (state.externalStyles.length > 0) {
                    var expressions = state.externalStyles// Remove globals
                    .filter(function(s) {
                        return !s[2];
                    }).map(function(s) {
                        return s[0];
                    });
                    var expressionsLength = expressions.length;
                    if (expressionsLength === 0) {
                        externalJsxId = null;
                    } else {
                        // Construct a template literal of this form:
                        // `jsx-${styles.__scopedHash} jsx-${otherStyles.__scopedHash}`
                        externalJsxId = t.templateLiteral([
                            t.templateElement({
                                raw: "jsx-",
                                cooked: "jsx-"
                            })
                        ].concat(_toConsumableArray([].concat(new Array(expressionsLength - 1).fill(null)).map(function() {
                            return t.templateElement({
                                raw: " jsx-",
                                cooked: " jsx-"
                            });
                        })), [
                            t.templateElement({
                                raw: "",
                                cooked: ""
                            }, true)
                        ]), expressions);
                    }
                }
                if (state.styles.length > 0 || externalJsxId) {
                    var ref = computeClassNames(state.styles, externalJsxId, state.styleComponentImportName), staticClassName = ref.staticClassName, className = ref.className;
                    state.className = className;
                    state.staticClassName = staticClassName;
                }
                state.hasJSXStyle = true;
                state.file.hasJSXStyle = true;
            // Next visit will be: JSXOpeningElement
            },
            exit: function exit(path, state) {
                var isGlobal = isGlobalEl(path.node.openingElement);
                if (state.hasJSXStyle && !--state.ignoreClosing && !isGlobal) {
                    state.hasJSXStyle = null;
                    state.className = null;
                    state.externalJsxId = null;
                }
                if (!state.hasJSXStyle || !isStyledJsx(path)) {
                    return;
                }
                if (state.ignoreClosing > 1) {
                    var styleTagSrc;
                    try {
                        styleTagSrc = path.getSource();
                    } catch (error) {}
                    throw path.buildCodeFrameError("Detected nested style tag" + (styleTagSrc ? ": \n\n" + styleTagSrc + "\n\n" : " ") + "styled-jsx only allows style tags " + "to be direct descendants (children) of the outermost " + "JSX element i.e. the subtree root.");
                }
                if (state.externalStyles.length > 0 && path.get("children").filter(function(child) {
                    if (!t.isJSXExpressionContainer(child)) {
                        return false;
                    }
                    var expression = child.get("expression");
                    return expression && expression.isIdentifier();
                }).length === 1) {
                    var ref = state.externalStyles.shift(), id = ref[0], css = ref[1];
                    path.replaceWith(makeStyledJsxTag(id, css, [], state.styleComponentImportName));
                    return;
                }
                var _opts = state.opts, vendorPrefixes = _opts.vendorPrefixes, sourceMaps = _opts.sourceMaps;
                var stylesInfo = _extends({}, state.styles.shift(), {
                    file: state.file,
                    staticClassName: state.staticClassName,
                    isGlobal: isGlobal,
                    plugins: state.plugins,
                    vendorPrefixes: vendorPrefixes,
                    sourceMaps: sourceMaps
                });
                var splitRules = typeof state.opts.optimizeForSpeed === "boolean" ? state.opts.optimizeForSpeed : process.env.NODE_ENV === "production";
                var ref1 = processCss(stylesInfo, {
                    splitRules: splitRules
                }), hash = ref1.hash, css1 = ref1.css, expressions = ref1.expressions;
                path.replaceWith(makeStyledJsxTag(hash, css1, expressions, state.styleComponentImportName));
            }
        }
    };
    // only apply JSXFragment visitor if supported
    if (t.isJSXFragment) {
        jsxVisitors.JSXFragment = jsxVisitors.JSXElement;
        jsxVisitors.JSXOpeningFragment = {
            enter: function enter(path, state) {
                if (!state.hasJSXStyle) {
                    return;
                }
                if (state.ignoreClosing === null) {
                    // We keep a counter of elements inside so that we
                    // can keep track of when we exit the parent to reset state
                    // note: if we wished to add an option to turn off
                    // selectors to reach parent elements, it would suffice to
                    // set this to `1` and do an early return instead
                    state.ignoreClosing = 0;
                }
                state.ignoreClosing++;
            }
        };
    }
    var visitors = {
        inherits: default_1,
        visitor: {
            Program: {
                enter: function enter(path, state) {
                    setStateOptions(state);
                    state.hasJSXStyle = null;
                    state.ignoreClosing = null;
                    state.file.hasJSXStyle = false;
                    state.file.hasCssResolve = false;
                    // create unique identifier for _JSXStyle component
                    state.styleComponentImportName = path.scope.generateUidIdentifier(STYLE_COMPONENT).name;
                    // we need to beat the arrow function transform and
                    // possibly others so we traverse from here or else
                    // dynamic values in classNames could be incorrect
                    path.traverse(jsxVisitors, state);
                    // Transpile external styles
                    path.traverse(visitor, state);
                },
                exit: function exit(path, state) {
                    if (!state.file.hasJSXStyle && !state.file.hasCssResolve) {
                        return;
                    }
                    state.file.hasJSXStyle = true;
                    var importDeclaration = createReactComponentImportDeclaration(state);
                    path.unshiftContainer("body", importDeclaration);
                }
            }
        }
    };
    return visitors;
}

exports["default"] = babel;
exports.macro = macro;
exports.test = test;
