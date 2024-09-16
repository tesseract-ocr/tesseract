var require$$0$1 = require('path');
var require$$1 = require('crypto');

function _interopDefaultLegacy (e) { return e && typeof e === 'object' && 'default' in e ? e : { 'default': e }; }

var require$$0__default = /*#__PURE__*/_interopDefaultLegacy(require$$0$1);
var require$$1__default = /*#__PURE__*/_interopDefaultLegacy(require$$1);

function getAugmentedNamespace(n) {
  var f = n.default;
	if (typeof f == "function") {
		var a = function () {
			return f.apply(this, arguments);
		};
		a.prototype = f.prototype;
  } else a = {};
  Object.defineProperty(a, '__esModule', {value: true});
	Object.keys(n).forEach(function (k) {
		var d = Object.getOwnPropertyDescriptor(n, k);
		Object.defineProperty(a, k, d.get ? d : {
			enumerable: true,
			get: function () {
				return n[k];
			}
		});
	});
	return a;
}

var lib$1 = {};

var lib = {exports: {}};

var parse$1 = {exports: {}};

var util = {};

var unicode$1 = {};

Object.defineProperty(unicode$1, "__esModule", {
    value: true
});
unicode$1.Space_Separator = /[\u1680\u2000-\u200A\u202F\u205F\u3000]/;
unicode$1.ID_Start = /[\xAA\xB5\xBA\xC0-\xD6\xD8-\xF6\xF8-\u02C1\u02C6-\u02D1\u02E0-\u02E4\u02EC\u02EE\u0370-\u0374\u0376\u0377\u037A-\u037D\u037F\u0386\u0388-\u038A\u038C\u038E-\u03A1\u03A3-\u03F5\u03F7-\u0481\u048A-\u052F\u0531-\u0556\u0559\u0561-\u0587\u05D0-\u05EA\u05F0-\u05F2\u0620-\u064A\u066E\u066F\u0671-\u06D3\u06D5\u06E5\u06E6\u06EE\u06EF\u06FA-\u06FC\u06FF\u0710\u0712-\u072F\u074D-\u07A5\u07B1\u07CA-\u07EA\u07F4\u07F5\u07FA\u0800-\u0815\u081A\u0824\u0828\u0840-\u0858\u08A0-\u08B4\u08B6-\u08BD\u0904-\u0939\u093D\u0950\u0958-\u0961\u0971-\u0980\u0985-\u098C\u098F\u0990\u0993-\u09A8\u09AA-\u09B0\u09B2\u09B6-\u09B9\u09BD\u09CE\u09DC\u09DD\u09DF-\u09E1\u09F0\u09F1\u0A05-\u0A0A\u0A0F\u0A10\u0A13-\u0A28\u0A2A-\u0A30\u0A32\u0A33\u0A35\u0A36\u0A38\u0A39\u0A59-\u0A5C\u0A5E\u0A72-\u0A74\u0A85-\u0A8D\u0A8F-\u0A91\u0A93-\u0AA8\u0AAA-\u0AB0\u0AB2\u0AB3\u0AB5-\u0AB9\u0ABD\u0AD0\u0AE0\u0AE1\u0AF9\u0B05-\u0B0C\u0B0F\u0B10\u0B13-\u0B28\u0B2A-\u0B30\u0B32\u0B33\u0B35-\u0B39\u0B3D\u0B5C\u0B5D\u0B5F-\u0B61\u0B71\u0B83\u0B85-\u0B8A\u0B8E-\u0B90\u0B92-\u0B95\u0B99\u0B9A\u0B9C\u0B9E\u0B9F\u0BA3\u0BA4\u0BA8-\u0BAA\u0BAE-\u0BB9\u0BD0\u0C05-\u0C0C\u0C0E-\u0C10\u0C12-\u0C28\u0C2A-\u0C39\u0C3D\u0C58-\u0C5A\u0C60\u0C61\u0C80\u0C85-\u0C8C\u0C8E-\u0C90\u0C92-\u0CA8\u0CAA-\u0CB3\u0CB5-\u0CB9\u0CBD\u0CDE\u0CE0\u0CE1\u0CF1\u0CF2\u0D05-\u0D0C\u0D0E-\u0D10\u0D12-\u0D3A\u0D3D\u0D4E\u0D54-\u0D56\u0D5F-\u0D61\u0D7A-\u0D7F\u0D85-\u0D96\u0D9A-\u0DB1\u0DB3-\u0DBB\u0DBD\u0DC0-\u0DC6\u0E01-\u0E30\u0E32\u0E33\u0E40-\u0E46\u0E81\u0E82\u0E84\u0E87\u0E88\u0E8A\u0E8D\u0E94-\u0E97\u0E99-\u0E9F\u0EA1-\u0EA3\u0EA5\u0EA7\u0EAA\u0EAB\u0EAD-\u0EB0\u0EB2\u0EB3\u0EBD\u0EC0-\u0EC4\u0EC6\u0EDC-\u0EDF\u0F00\u0F40-\u0F47\u0F49-\u0F6C\u0F88-\u0F8C\u1000-\u102A\u103F\u1050-\u1055\u105A-\u105D\u1061\u1065\u1066\u106E-\u1070\u1075-\u1081\u108E\u10A0-\u10C5\u10C7\u10CD\u10D0-\u10FA\u10FC-\u1248\u124A-\u124D\u1250-\u1256\u1258\u125A-\u125D\u1260-\u1288\u128A-\u128D\u1290-\u12B0\u12B2-\u12B5\u12B8-\u12BE\u12C0\u12C2-\u12C5\u12C8-\u12D6\u12D8-\u1310\u1312-\u1315\u1318-\u135A\u1380-\u138F\u13A0-\u13F5\u13F8-\u13FD\u1401-\u166C\u166F-\u167F\u1681-\u169A\u16A0-\u16EA\u16EE-\u16F8\u1700-\u170C\u170E-\u1711\u1720-\u1731\u1740-\u1751\u1760-\u176C\u176E-\u1770\u1780-\u17B3\u17D7\u17DC\u1820-\u1877\u1880-\u1884\u1887-\u18A8\u18AA\u18B0-\u18F5\u1900-\u191E\u1950-\u196D\u1970-\u1974\u1980-\u19AB\u19B0-\u19C9\u1A00-\u1A16\u1A20-\u1A54\u1AA7\u1B05-\u1B33\u1B45-\u1B4B\u1B83-\u1BA0\u1BAE\u1BAF\u1BBA-\u1BE5\u1C00-\u1C23\u1C4D-\u1C4F\u1C5A-\u1C7D\u1C80-\u1C88\u1CE9-\u1CEC\u1CEE-\u1CF1\u1CF5\u1CF6\u1D00-\u1DBF\u1E00-\u1F15\u1F18-\u1F1D\u1F20-\u1F45\u1F48-\u1F4D\u1F50-\u1F57\u1F59\u1F5B\u1F5D\u1F5F-\u1F7D\u1F80-\u1FB4\u1FB6-\u1FBC\u1FBE\u1FC2-\u1FC4\u1FC6-\u1FCC\u1FD0-\u1FD3\u1FD6-\u1FDB\u1FE0-\u1FEC\u1FF2-\u1FF4\u1FF6-\u1FFC\u2071\u207F\u2090-\u209C\u2102\u2107\u210A-\u2113\u2115\u2119-\u211D\u2124\u2126\u2128\u212A-\u212D\u212F-\u2139\u213C-\u213F\u2145-\u2149\u214E\u2160-\u2188\u2C00-\u2C2E\u2C30-\u2C5E\u2C60-\u2CE4\u2CEB-\u2CEE\u2CF2\u2CF3\u2D00-\u2D25\u2D27\u2D2D\u2D30-\u2D67\u2D6F\u2D80-\u2D96\u2DA0-\u2DA6\u2DA8-\u2DAE\u2DB0-\u2DB6\u2DB8-\u2DBE\u2DC0-\u2DC6\u2DC8-\u2DCE\u2DD0-\u2DD6\u2DD8-\u2DDE\u2E2F\u3005-\u3007\u3021-\u3029\u3031-\u3035\u3038-\u303C\u3041-\u3096\u309D-\u309F\u30A1-\u30FA\u30FC-\u30FF\u3105-\u312D\u3131-\u318E\u31A0-\u31BA\u31F0-\u31FF\u3400-\u4DB5\u4E00-\u9FD5\uA000-\uA48C\uA4D0-\uA4FD\uA500-\uA60C\uA610-\uA61F\uA62A\uA62B\uA640-\uA66E\uA67F-\uA69D\uA6A0-\uA6EF\uA717-\uA71F\uA722-\uA788\uA78B-\uA7AE\uA7B0-\uA7B7\uA7F7-\uA801\uA803-\uA805\uA807-\uA80A\uA80C-\uA822\uA840-\uA873\uA882-\uA8B3\uA8F2-\uA8F7\uA8FB\uA8FD\uA90A-\uA925\uA930-\uA946\uA960-\uA97C\uA984-\uA9B2\uA9CF\uA9E0-\uA9E4\uA9E6-\uA9EF\uA9FA-\uA9FE\uAA00-\uAA28\uAA40-\uAA42\uAA44-\uAA4B\uAA60-\uAA76\uAA7A\uAA7E-\uAAAF\uAAB1\uAAB5\uAAB6\uAAB9-\uAABD\uAAC0\uAAC2\uAADB-\uAADD\uAAE0-\uAAEA\uAAF2-\uAAF4\uAB01-\uAB06\uAB09-\uAB0E\uAB11-\uAB16\uAB20-\uAB26\uAB28-\uAB2E\uAB30-\uAB5A\uAB5C-\uAB65\uAB70-\uABE2\uAC00-\uD7A3\uD7B0-\uD7C6\uD7CB-\uD7FB\uF900-\uFA6D\uFA70-\uFAD9\uFB00-\uFB06\uFB13-\uFB17\uFB1D\uFB1F-\uFB28\uFB2A-\uFB36\uFB38-\uFB3C\uFB3E\uFB40\uFB41\uFB43\uFB44\uFB46-\uFBB1\uFBD3-\uFD3D\uFD50-\uFD8F\uFD92-\uFDC7\uFDF0-\uFDFB\uFE70-\uFE74\uFE76-\uFEFC\uFF21-\uFF3A\uFF41-\uFF5A\uFF66-\uFFBE\uFFC2-\uFFC7\uFFCA-\uFFCF\uFFD2-\uFFD7\uFFDA-\uFFDC]|\uD800[\uDC00-\uDC0B\uDC0D-\uDC26\uDC28-\uDC3A\uDC3C\uDC3D\uDC3F-\uDC4D\uDC50-\uDC5D\uDC80-\uDCFA\uDD40-\uDD74\uDE80-\uDE9C\uDEA0-\uDED0\uDF00-\uDF1F\uDF30-\uDF4A\uDF50-\uDF75\uDF80-\uDF9D\uDFA0-\uDFC3\uDFC8-\uDFCF\uDFD1-\uDFD5]|\uD801[\uDC00-\uDC9D\uDCB0-\uDCD3\uDCD8-\uDCFB\uDD00-\uDD27\uDD30-\uDD63\uDE00-\uDF36\uDF40-\uDF55\uDF60-\uDF67]|\uD802[\uDC00-\uDC05\uDC08\uDC0A-\uDC35\uDC37\uDC38\uDC3C\uDC3F-\uDC55\uDC60-\uDC76\uDC80-\uDC9E\uDCE0-\uDCF2\uDCF4\uDCF5\uDD00-\uDD15\uDD20-\uDD39\uDD80-\uDDB7\uDDBE\uDDBF\uDE00\uDE10-\uDE13\uDE15-\uDE17\uDE19-\uDE33\uDE60-\uDE7C\uDE80-\uDE9C\uDEC0-\uDEC7\uDEC9-\uDEE4\uDF00-\uDF35\uDF40-\uDF55\uDF60-\uDF72\uDF80-\uDF91]|\uD803[\uDC00-\uDC48\uDC80-\uDCB2\uDCC0-\uDCF2]|\uD804[\uDC03-\uDC37\uDC83-\uDCAF\uDCD0-\uDCE8\uDD03-\uDD26\uDD50-\uDD72\uDD76\uDD83-\uDDB2\uDDC1-\uDDC4\uDDDA\uDDDC\uDE00-\uDE11\uDE13-\uDE2B\uDE80-\uDE86\uDE88\uDE8A-\uDE8D\uDE8F-\uDE9D\uDE9F-\uDEA8\uDEB0-\uDEDE\uDF05-\uDF0C\uDF0F\uDF10\uDF13-\uDF28\uDF2A-\uDF30\uDF32\uDF33\uDF35-\uDF39\uDF3D\uDF50\uDF5D-\uDF61]|\uD805[\uDC00-\uDC34\uDC47-\uDC4A\uDC80-\uDCAF\uDCC4\uDCC5\uDCC7\uDD80-\uDDAE\uDDD8-\uDDDB\uDE00-\uDE2F\uDE44\uDE80-\uDEAA\uDF00-\uDF19]|\uD806[\uDCA0-\uDCDF\uDCFF\uDEC0-\uDEF8]|\uD807[\uDC00-\uDC08\uDC0A-\uDC2E\uDC40\uDC72-\uDC8F]|\uD808[\uDC00-\uDF99]|\uD809[\uDC00-\uDC6E\uDC80-\uDD43]|[\uD80C\uD81C-\uD820\uD840-\uD868\uD86A-\uD86C\uD86F-\uD872][\uDC00-\uDFFF]|\uD80D[\uDC00-\uDC2E]|\uD811[\uDC00-\uDE46]|\uD81A[\uDC00-\uDE38\uDE40-\uDE5E\uDED0-\uDEED\uDF00-\uDF2F\uDF40-\uDF43\uDF63-\uDF77\uDF7D-\uDF8F]|\uD81B[\uDF00-\uDF44\uDF50\uDF93-\uDF9F\uDFE0]|\uD821[\uDC00-\uDFEC]|\uD822[\uDC00-\uDEF2]|\uD82C[\uDC00\uDC01]|\uD82F[\uDC00-\uDC6A\uDC70-\uDC7C\uDC80-\uDC88\uDC90-\uDC99]|\uD835[\uDC00-\uDC54\uDC56-\uDC9C\uDC9E\uDC9F\uDCA2\uDCA5\uDCA6\uDCA9-\uDCAC\uDCAE-\uDCB9\uDCBB\uDCBD-\uDCC3\uDCC5-\uDD05\uDD07-\uDD0A\uDD0D-\uDD14\uDD16-\uDD1C\uDD1E-\uDD39\uDD3B-\uDD3E\uDD40-\uDD44\uDD46\uDD4A-\uDD50\uDD52-\uDEA5\uDEA8-\uDEC0\uDEC2-\uDEDA\uDEDC-\uDEFA\uDEFC-\uDF14\uDF16-\uDF34\uDF36-\uDF4E\uDF50-\uDF6E\uDF70-\uDF88\uDF8A-\uDFA8\uDFAA-\uDFC2\uDFC4-\uDFCB]|\uD83A[\uDC00-\uDCC4\uDD00-\uDD43]|\uD83B[\uDE00-\uDE03\uDE05-\uDE1F\uDE21\uDE22\uDE24\uDE27\uDE29-\uDE32\uDE34-\uDE37\uDE39\uDE3B\uDE42\uDE47\uDE49\uDE4B\uDE4D-\uDE4F\uDE51\uDE52\uDE54\uDE57\uDE59\uDE5B\uDE5D\uDE5F\uDE61\uDE62\uDE64\uDE67-\uDE6A\uDE6C-\uDE72\uDE74-\uDE77\uDE79-\uDE7C\uDE7E\uDE80-\uDE89\uDE8B-\uDE9B\uDEA1-\uDEA3\uDEA5-\uDEA9\uDEAB-\uDEBB]|\uD869[\uDC00-\uDED6\uDF00-\uDFFF]|\uD86D[\uDC00-\uDF34\uDF40-\uDFFF]|\uD86E[\uDC00-\uDC1D\uDC20-\uDFFF]|\uD873[\uDC00-\uDEA1]|\uD87E[\uDC00-\uDE1D]/;
unicode$1.ID_Continue = /[\xAA\xB5\xBA\xC0-\xD6\xD8-\xF6\xF8-\u02C1\u02C6-\u02D1\u02E0-\u02E4\u02EC\u02EE\u0300-\u0374\u0376\u0377\u037A-\u037D\u037F\u0386\u0388-\u038A\u038C\u038E-\u03A1\u03A3-\u03F5\u03F7-\u0481\u0483-\u0487\u048A-\u052F\u0531-\u0556\u0559\u0561-\u0587\u0591-\u05BD\u05BF\u05C1\u05C2\u05C4\u05C5\u05C7\u05D0-\u05EA\u05F0-\u05F2\u0610-\u061A\u0620-\u0669\u066E-\u06D3\u06D5-\u06DC\u06DF-\u06E8\u06EA-\u06FC\u06FF\u0710-\u074A\u074D-\u07B1\u07C0-\u07F5\u07FA\u0800-\u082D\u0840-\u085B\u08A0-\u08B4\u08B6-\u08BD\u08D4-\u08E1\u08E3-\u0963\u0966-\u096F\u0971-\u0983\u0985-\u098C\u098F\u0990\u0993-\u09A8\u09AA-\u09B0\u09B2\u09B6-\u09B9\u09BC-\u09C4\u09C7\u09C8\u09CB-\u09CE\u09D7\u09DC\u09DD\u09DF-\u09E3\u09E6-\u09F1\u0A01-\u0A03\u0A05-\u0A0A\u0A0F\u0A10\u0A13-\u0A28\u0A2A-\u0A30\u0A32\u0A33\u0A35\u0A36\u0A38\u0A39\u0A3C\u0A3E-\u0A42\u0A47\u0A48\u0A4B-\u0A4D\u0A51\u0A59-\u0A5C\u0A5E\u0A66-\u0A75\u0A81-\u0A83\u0A85-\u0A8D\u0A8F-\u0A91\u0A93-\u0AA8\u0AAA-\u0AB0\u0AB2\u0AB3\u0AB5-\u0AB9\u0ABC-\u0AC5\u0AC7-\u0AC9\u0ACB-\u0ACD\u0AD0\u0AE0-\u0AE3\u0AE6-\u0AEF\u0AF9\u0B01-\u0B03\u0B05-\u0B0C\u0B0F\u0B10\u0B13-\u0B28\u0B2A-\u0B30\u0B32\u0B33\u0B35-\u0B39\u0B3C-\u0B44\u0B47\u0B48\u0B4B-\u0B4D\u0B56\u0B57\u0B5C\u0B5D\u0B5F-\u0B63\u0B66-\u0B6F\u0B71\u0B82\u0B83\u0B85-\u0B8A\u0B8E-\u0B90\u0B92-\u0B95\u0B99\u0B9A\u0B9C\u0B9E\u0B9F\u0BA3\u0BA4\u0BA8-\u0BAA\u0BAE-\u0BB9\u0BBE-\u0BC2\u0BC6-\u0BC8\u0BCA-\u0BCD\u0BD0\u0BD7\u0BE6-\u0BEF\u0C00-\u0C03\u0C05-\u0C0C\u0C0E-\u0C10\u0C12-\u0C28\u0C2A-\u0C39\u0C3D-\u0C44\u0C46-\u0C48\u0C4A-\u0C4D\u0C55\u0C56\u0C58-\u0C5A\u0C60-\u0C63\u0C66-\u0C6F\u0C80-\u0C83\u0C85-\u0C8C\u0C8E-\u0C90\u0C92-\u0CA8\u0CAA-\u0CB3\u0CB5-\u0CB9\u0CBC-\u0CC4\u0CC6-\u0CC8\u0CCA-\u0CCD\u0CD5\u0CD6\u0CDE\u0CE0-\u0CE3\u0CE6-\u0CEF\u0CF1\u0CF2\u0D01-\u0D03\u0D05-\u0D0C\u0D0E-\u0D10\u0D12-\u0D3A\u0D3D-\u0D44\u0D46-\u0D48\u0D4A-\u0D4E\u0D54-\u0D57\u0D5F-\u0D63\u0D66-\u0D6F\u0D7A-\u0D7F\u0D82\u0D83\u0D85-\u0D96\u0D9A-\u0DB1\u0DB3-\u0DBB\u0DBD\u0DC0-\u0DC6\u0DCA\u0DCF-\u0DD4\u0DD6\u0DD8-\u0DDF\u0DE6-\u0DEF\u0DF2\u0DF3\u0E01-\u0E3A\u0E40-\u0E4E\u0E50-\u0E59\u0E81\u0E82\u0E84\u0E87\u0E88\u0E8A\u0E8D\u0E94-\u0E97\u0E99-\u0E9F\u0EA1-\u0EA3\u0EA5\u0EA7\u0EAA\u0EAB\u0EAD-\u0EB9\u0EBB-\u0EBD\u0EC0-\u0EC4\u0EC6\u0EC8-\u0ECD\u0ED0-\u0ED9\u0EDC-\u0EDF\u0F00\u0F18\u0F19\u0F20-\u0F29\u0F35\u0F37\u0F39\u0F3E-\u0F47\u0F49-\u0F6C\u0F71-\u0F84\u0F86-\u0F97\u0F99-\u0FBC\u0FC6\u1000-\u1049\u1050-\u109D\u10A0-\u10C5\u10C7\u10CD\u10D0-\u10FA\u10FC-\u1248\u124A-\u124D\u1250-\u1256\u1258\u125A-\u125D\u1260-\u1288\u128A-\u128D\u1290-\u12B0\u12B2-\u12B5\u12B8-\u12BE\u12C0\u12C2-\u12C5\u12C8-\u12D6\u12D8-\u1310\u1312-\u1315\u1318-\u135A\u135D-\u135F\u1380-\u138F\u13A0-\u13F5\u13F8-\u13FD\u1401-\u166C\u166F-\u167F\u1681-\u169A\u16A0-\u16EA\u16EE-\u16F8\u1700-\u170C\u170E-\u1714\u1720-\u1734\u1740-\u1753\u1760-\u176C\u176E-\u1770\u1772\u1773\u1780-\u17D3\u17D7\u17DC\u17DD\u17E0-\u17E9\u180B-\u180D\u1810-\u1819\u1820-\u1877\u1880-\u18AA\u18B0-\u18F5\u1900-\u191E\u1920-\u192B\u1930-\u193B\u1946-\u196D\u1970-\u1974\u1980-\u19AB\u19B0-\u19C9\u19D0-\u19D9\u1A00-\u1A1B\u1A20-\u1A5E\u1A60-\u1A7C\u1A7F-\u1A89\u1A90-\u1A99\u1AA7\u1AB0-\u1ABD\u1B00-\u1B4B\u1B50-\u1B59\u1B6B-\u1B73\u1B80-\u1BF3\u1C00-\u1C37\u1C40-\u1C49\u1C4D-\u1C7D\u1C80-\u1C88\u1CD0-\u1CD2\u1CD4-\u1CF6\u1CF8\u1CF9\u1D00-\u1DF5\u1DFB-\u1F15\u1F18-\u1F1D\u1F20-\u1F45\u1F48-\u1F4D\u1F50-\u1F57\u1F59\u1F5B\u1F5D\u1F5F-\u1F7D\u1F80-\u1FB4\u1FB6-\u1FBC\u1FBE\u1FC2-\u1FC4\u1FC6-\u1FCC\u1FD0-\u1FD3\u1FD6-\u1FDB\u1FE0-\u1FEC\u1FF2-\u1FF4\u1FF6-\u1FFC\u203F\u2040\u2054\u2071\u207F\u2090-\u209C\u20D0-\u20DC\u20E1\u20E5-\u20F0\u2102\u2107\u210A-\u2113\u2115\u2119-\u211D\u2124\u2126\u2128\u212A-\u212D\u212F-\u2139\u213C-\u213F\u2145-\u2149\u214E\u2160-\u2188\u2C00-\u2C2E\u2C30-\u2C5E\u2C60-\u2CE4\u2CEB-\u2CF3\u2D00-\u2D25\u2D27\u2D2D\u2D30-\u2D67\u2D6F\u2D7F-\u2D96\u2DA0-\u2DA6\u2DA8-\u2DAE\u2DB0-\u2DB6\u2DB8-\u2DBE\u2DC0-\u2DC6\u2DC8-\u2DCE\u2DD0-\u2DD6\u2DD8-\u2DDE\u2DE0-\u2DFF\u2E2F\u3005-\u3007\u3021-\u302F\u3031-\u3035\u3038-\u303C\u3041-\u3096\u3099\u309A\u309D-\u309F\u30A1-\u30FA\u30FC-\u30FF\u3105-\u312D\u3131-\u318E\u31A0-\u31BA\u31F0-\u31FF\u3400-\u4DB5\u4E00-\u9FD5\uA000-\uA48C\uA4D0-\uA4FD\uA500-\uA60C\uA610-\uA62B\uA640-\uA66F\uA674-\uA67D\uA67F-\uA6F1\uA717-\uA71F\uA722-\uA788\uA78B-\uA7AE\uA7B0-\uA7B7\uA7F7-\uA827\uA840-\uA873\uA880-\uA8C5\uA8D0-\uA8D9\uA8E0-\uA8F7\uA8FB\uA8FD\uA900-\uA92D\uA930-\uA953\uA960-\uA97C\uA980-\uA9C0\uA9CF-\uA9D9\uA9E0-\uA9FE\uAA00-\uAA36\uAA40-\uAA4D\uAA50-\uAA59\uAA60-\uAA76\uAA7A-\uAAC2\uAADB-\uAADD\uAAE0-\uAAEF\uAAF2-\uAAF6\uAB01-\uAB06\uAB09-\uAB0E\uAB11-\uAB16\uAB20-\uAB26\uAB28-\uAB2E\uAB30-\uAB5A\uAB5C-\uAB65\uAB70-\uABEA\uABEC\uABED\uABF0-\uABF9\uAC00-\uD7A3\uD7B0-\uD7C6\uD7CB-\uD7FB\uF900-\uFA6D\uFA70-\uFAD9\uFB00-\uFB06\uFB13-\uFB17\uFB1D-\uFB28\uFB2A-\uFB36\uFB38-\uFB3C\uFB3E\uFB40\uFB41\uFB43\uFB44\uFB46-\uFBB1\uFBD3-\uFD3D\uFD50-\uFD8F\uFD92-\uFDC7\uFDF0-\uFDFB\uFE00-\uFE0F\uFE20-\uFE2F\uFE33\uFE34\uFE4D-\uFE4F\uFE70-\uFE74\uFE76-\uFEFC\uFF10-\uFF19\uFF21-\uFF3A\uFF3F\uFF41-\uFF5A\uFF66-\uFFBE\uFFC2-\uFFC7\uFFCA-\uFFCF\uFFD2-\uFFD7\uFFDA-\uFFDC]|\uD800[\uDC00-\uDC0B\uDC0D-\uDC26\uDC28-\uDC3A\uDC3C\uDC3D\uDC3F-\uDC4D\uDC50-\uDC5D\uDC80-\uDCFA\uDD40-\uDD74\uDDFD\uDE80-\uDE9C\uDEA0-\uDED0\uDEE0\uDF00-\uDF1F\uDF30-\uDF4A\uDF50-\uDF7A\uDF80-\uDF9D\uDFA0-\uDFC3\uDFC8-\uDFCF\uDFD1-\uDFD5]|\uD801[\uDC00-\uDC9D\uDCA0-\uDCA9\uDCB0-\uDCD3\uDCD8-\uDCFB\uDD00-\uDD27\uDD30-\uDD63\uDE00-\uDF36\uDF40-\uDF55\uDF60-\uDF67]|\uD802[\uDC00-\uDC05\uDC08\uDC0A-\uDC35\uDC37\uDC38\uDC3C\uDC3F-\uDC55\uDC60-\uDC76\uDC80-\uDC9E\uDCE0-\uDCF2\uDCF4\uDCF5\uDD00-\uDD15\uDD20-\uDD39\uDD80-\uDDB7\uDDBE\uDDBF\uDE00-\uDE03\uDE05\uDE06\uDE0C-\uDE13\uDE15-\uDE17\uDE19-\uDE33\uDE38-\uDE3A\uDE3F\uDE60-\uDE7C\uDE80-\uDE9C\uDEC0-\uDEC7\uDEC9-\uDEE6\uDF00-\uDF35\uDF40-\uDF55\uDF60-\uDF72\uDF80-\uDF91]|\uD803[\uDC00-\uDC48\uDC80-\uDCB2\uDCC0-\uDCF2]|\uD804[\uDC00-\uDC46\uDC66-\uDC6F\uDC7F-\uDCBA\uDCD0-\uDCE8\uDCF0-\uDCF9\uDD00-\uDD34\uDD36-\uDD3F\uDD50-\uDD73\uDD76\uDD80-\uDDC4\uDDCA-\uDDCC\uDDD0-\uDDDA\uDDDC\uDE00-\uDE11\uDE13-\uDE37\uDE3E\uDE80-\uDE86\uDE88\uDE8A-\uDE8D\uDE8F-\uDE9D\uDE9F-\uDEA8\uDEB0-\uDEEA\uDEF0-\uDEF9\uDF00-\uDF03\uDF05-\uDF0C\uDF0F\uDF10\uDF13-\uDF28\uDF2A-\uDF30\uDF32\uDF33\uDF35-\uDF39\uDF3C-\uDF44\uDF47\uDF48\uDF4B-\uDF4D\uDF50\uDF57\uDF5D-\uDF63\uDF66-\uDF6C\uDF70-\uDF74]|\uD805[\uDC00-\uDC4A\uDC50-\uDC59\uDC80-\uDCC5\uDCC7\uDCD0-\uDCD9\uDD80-\uDDB5\uDDB8-\uDDC0\uDDD8-\uDDDD\uDE00-\uDE40\uDE44\uDE50-\uDE59\uDE80-\uDEB7\uDEC0-\uDEC9\uDF00-\uDF19\uDF1D-\uDF2B\uDF30-\uDF39]|\uD806[\uDCA0-\uDCE9\uDCFF\uDEC0-\uDEF8]|\uD807[\uDC00-\uDC08\uDC0A-\uDC36\uDC38-\uDC40\uDC50-\uDC59\uDC72-\uDC8F\uDC92-\uDCA7\uDCA9-\uDCB6]|\uD808[\uDC00-\uDF99]|\uD809[\uDC00-\uDC6E\uDC80-\uDD43]|[\uD80C\uD81C-\uD820\uD840-\uD868\uD86A-\uD86C\uD86F-\uD872][\uDC00-\uDFFF]|\uD80D[\uDC00-\uDC2E]|\uD811[\uDC00-\uDE46]|\uD81A[\uDC00-\uDE38\uDE40-\uDE5E\uDE60-\uDE69\uDED0-\uDEED\uDEF0-\uDEF4\uDF00-\uDF36\uDF40-\uDF43\uDF50-\uDF59\uDF63-\uDF77\uDF7D-\uDF8F]|\uD81B[\uDF00-\uDF44\uDF50-\uDF7E\uDF8F-\uDF9F\uDFE0]|\uD821[\uDC00-\uDFEC]|\uD822[\uDC00-\uDEF2]|\uD82C[\uDC00\uDC01]|\uD82F[\uDC00-\uDC6A\uDC70-\uDC7C\uDC80-\uDC88\uDC90-\uDC99\uDC9D\uDC9E]|\uD834[\uDD65-\uDD69\uDD6D-\uDD72\uDD7B-\uDD82\uDD85-\uDD8B\uDDAA-\uDDAD\uDE42-\uDE44]|\uD835[\uDC00-\uDC54\uDC56-\uDC9C\uDC9E\uDC9F\uDCA2\uDCA5\uDCA6\uDCA9-\uDCAC\uDCAE-\uDCB9\uDCBB\uDCBD-\uDCC3\uDCC5-\uDD05\uDD07-\uDD0A\uDD0D-\uDD14\uDD16-\uDD1C\uDD1E-\uDD39\uDD3B-\uDD3E\uDD40-\uDD44\uDD46\uDD4A-\uDD50\uDD52-\uDEA5\uDEA8-\uDEC0\uDEC2-\uDEDA\uDEDC-\uDEFA\uDEFC-\uDF14\uDF16-\uDF34\uDF36-\uDF4E\uDF50-\uDF6E\uDF70-\uDF88\uDF8A-\uDFA8\uDFAA-\uDFC2\uDFC4-\uDFCB\uDFCE-\uDFFF]|\uD836[\uDE00-\uDE36\uDE3B-\uDE6C\uDE75\uDE84\uDE9B-\uDE9F\uDEA1-\uDEAF]|\uD838[\uDC00-\uDC06\uDC08-\uDC18\uDC1B-\uDC21\uDC23\uDC24\uDC26-\uDC2A]|\uD83A[\uDC00-\uDCC4\uDCD0-\uDCD6\uDD00-\uDD4A\uDD50-\uDD59]|\uD83B[\uDE00-\uDE03\uDE05-\uDE1F\uDE21\uDE22\uDE24\uDE27\uDE29-\uDE32\uDE34-\uDE37\uDE39\uDE3B\uDE42\uDE47\uDE49\uDE4B\uDE4D-\uDE4F\uDE51\uDE52\uDE54\uDE57\uDE59\uDE5B\uDE5D\uDE5F\uDE61\uDE62\uDE64\uDE67-\uDE6A\uDE6C-\uDE72\uDE74-\uDE77\uDE79-\uDE7C\uDE7E\uDE80-\uDE89\uDE8B-\uDE9B\uDEA1-\uDEA3\uDEA5-\uDEA9\uDEAB-\uDEBB]|\uD869[\uDC00-\uDED6\uDF00-\uDFFF]|\uD86D[\uDC00-\uDF34\uDF40-\uDFFF]|\uD86E[\uDC00-\uDC1D\uDC20-\uDFFF]|\uD873[\uDC00-\uDEA1]|\uD87E[\uDC00-\uDE1D]|\uDB40[\uDD00-\uDDEF]/;

Object.defineProperty(util, "__esModule", {
    value: true
});
util.isSpaceSeparator = isSpaceSeparator;
util.isIdStartChar = isIdStartChar;
util.isIdContinueChar = isIdContinueChar;
util.isDigit = isDigit;
util.isHexDigit = isHexDigit;
var _unicode = unicode$1;
var unicode = _interopRequireWildcard(_unicode);
function _interopRequireWildcard(obj) {
    if (obj && obj.__esModule) {
        return obj;
    } else {
        var newObj = {};
        if (obj != null) {
            for(var key in obj){
                if (Object.prototype.hasOwnProperty.call(obj, key)) newObj[key] = obj[key];
            }
        }
        newObj.default = obj;
        return newObj;
    }
}
function isSpaceSeparator(c) {
    return unicode.Space_Separator.test(c);
}
function isIdStartChar(c) {
    return c >= "a" && c <= "z" || c >= "A" && c <= "Z" || c === "$" || c === "_" || unicode.ID_Start.test(c);
}
function isIdContinueChar(c) {
    return c >= "a" && c <= "z" || c >= "A" && c <= "Z" || c >= "0" && c <= "9" || c === "$" || c === "_" || c === "‌" || c === "‍" || unicode.ID_Continue.test(c);
}
function isDigit(c) {
    return /[0-9]/.test(c);
}
function isHexDigit(c) {
    return /[0-9A-Fa-f]/.test(c);
}

var _typeof$2 = function(obj) {
    "@swc/helpers - typeof";
    return obj && typeof Symbol !== "undefined" && obj.constructor === Symbol ? "symbol" : typeof obj;
};
(function(module, exports) {
    var _interopRequireWildcard = function _interopRequireWildcard(obj) {
        if (obj && obj.__esModule) {
            return obj;
        } else {
            var newObj = {};
            if (obj != null) {
                for(var key in obj){
                    if (Object.prototype.hasOwnProperty.call(obj, key)) newObj[key] = obj[key];
                }
            }
            newObj.default = obj;
            return newObj;
        }
    };
    var parse = function parse(text, reviver) {
        source = String(text);
        parseState = "start";
        stack = [];
        pos = 0;
        line = 1;
        column = 0;
        token = undefined;
        key = undefined;
        root = undefined;
        do {
            token = lex();
            parseStates[parseState]();
        }while (token.type !== "eof");
        if (typeof reviver === "function") {
            return internalize({
                "": root
            }, "", reviver);
        }
        return root;
    };
    var lex = function lex() {
        lexState = "default";
        buffer = "";
        doubleQuote = false;
        _sign = 1;
        for(;;){
            c = peek();
            var _token = lexStates[lexState]();
            if (_token) {
                return _token;
            }
        }
    };
    var peek = function peek() {
        if (source[pos]) {
            return String.fromCodePoint(source.codePointAt(pos));
        }
    };
    var read = function read() {
        var c = peek();
        if (c === "\n") {
            line++;
            column = 0;
        } else if (c) {
            column += c.length;
        } else {
            column++;
        }
        if (c) {
            pos += c.length;
        }
        return c;
    };
    var newToken = function newToken(type, value) {
        return {
            type: type,
            value: value,
            line: line,
            column: column
        };
    };
    var literal = function literal(s) {
        var _iteratorNormalCompletion = true;
        var _didIteratorError = false;
        var _iteratorError = undefined;
        try {
            for(var _iterator = s[Symbol.iterator](), _step; !(_iteratorNormalCompletion = (_step = _iterator.next()).done); _iteratorNormalCompletion = true){
                var _c = _step.value;
                var p = peek();
                if (p !== _c) {
                    throw invalidChar(read());
                }
                read();
            }
        } catch (err) {
            _didIteratorError = true;
            _iteratorError = err;
        } finally{
            try {
                if (!_iteratorNormalCompletion && _iterator.return) {
                    _iterator.return();
                }
            } finally{
                if (_didIteratorError) {
                    throw _iteratorError;
                }
            }
        }
    };
    var escape = function escape() {
        var c = peek();
        switch(c){
            case "b":
                read();
                return "\b";
            case "f":
                read();
                return "\f";
            case "n":
                read();
                return "\n";
            case "r":
                read();
                return "\r";
            case "t":
                read();
                return "	";
            case "v":
                read();
                return "\v";
            case "0":
                read();
                if (util$1.isDigit(peek())) {
                    throw invalidChar(read());
                }
                return "\0";
            case "x":
                read();
                return hexEscape();
            case "u":
                read();
                return unicodeEscape();
            case "\n":
            case "\u2028":
            case "\u2029":
                read();
                return "";
            case "\r":
                read();
                if (peek() === "\n") {
                    read();
                }
                return "";
            case "1":
            case "2":
            case "3":
            case "4":
            case "5":
            case "6":
            case "7":
            case "8":
            case "9":
                throw invalidChar(read());
            case undefined:
                throw invalidChar(read());
        }
        return read();
    };
    var hexEscape = function hexEscape() {
        var buffer = "";
        var c = peek();
        if (!util$1.isHexDigit(c)) {
            throw invalidChar(read());
        }
        buffer += read();
        c = peek();
        if (!util$1.isHexDigit(c)) {
            throw invalidChar(read());
        }
        buffer += read();
        return String.fromCodePoint(parseInt(buffer, 16));
    };
    var unicodeEscape = function unicodeEscape() {
        var buffer = "";
        var count = 4;
        while(count-- > 0){
            var _c2 = peek();
            if (!util$1.isHexDigit(_c2)) {
                throw invalidChar(read());
            }
            buffer += read();
        }
        return String.fromCodePoint(parseInt(buffer, 16));
    };
    var push = function push() {
        var value = void 0;
        switch(token.type){
            case "punctuator":
                switch(token.value){
                    case "{":
                        value = {};
                        break;
                    case "[":
                        value = [];
                        break;
                }
                break;
            case "null":
            case "boolean":
            case "numeric":
            case "string":
                value = token.value;
                break;
        }
        if (root === undefined) {
            root = value;
        } else {
            var parent = stack[stack.length - 1];
            if (Array.isArray(parent)) {
                parent.push(value);
            } else {
                parent[key] = value;
            }
        }
        if (value !== null && (typeof value === "undefined" ? "undefined" : _$_typeof(value)) === "object") {
            stack.push(value);
            if (Array.isArray(value)) {
                parseState = "beforeArrayValue";
            } else {
                parseState = "beforePropertyName";
            }
        } else {
            var current = stack[stack.length - 1];
            if (current == null) {
                parseState = "end";
            } else if (Array.isArray(current)) {
                parseState = "afterArrayValue";
            } else {
                parseState = "afterPropertyValue";
            }
        }
    };
    var pop = function pop() {
        stack.pop();
        var current = stack[stack.length - 1];
        if (current == null) {
            parseState = "end";
        } else if (Array.isArray(current)) {
            parseState = "afterArrayValue";
        } else {
            parseState = "afterPropertyValue";
        }
    };
    var invalidChar = function invalidChar(c) {
        if (c === undefined) {
            return syntaxError("JSON5: invalid end of input at " + line + ":" + column);
        }
        return syntaxError("JSON5: invalid character '" + formatChar(c) + "' at " + line + ":" + column);
    };
    var invalidEOF = function invalidEOF() {
        return syntaxError("JSON5: invalid end of input at " + line + ":" + column);
    };
    var invalidIdentifier = function invalidIdentifier() {
        column -= 5;
        return syntaxError("JSON5: invalid identifier character at " + line + ":" + column);
    };
    var separatorChar = function separatorChar(c) {
        console.warn("JSON5: '" + c + "' is not valid ECMAScript; consider escaping");
    };
    var formatChar = function formatChar(c) {
        var replacements = {
            "'": "\\'",
            '"': '\\"',
            "\\": "\\\\",
            "\b": "\\b",
            "\f": "\\f",
            "\n": "\\n",
            "\r": "\\r",
            "	": "\\t",
            "\v": "\\v",
            "\0": "\\0",
            "\u2028": "\\u2028",
            "\u2029": "\\u2029"
        };
        if (replacements[c]) {
            return replacements[c];
        }
        if (c < " ") {
            var hexString = c.charCodeAt(0).toString(16);
            return "\\x" + ("00" + hexString).substring(hexString.length);
        }
        return c;
    };
    var syntaxError = function syntaxError(message) {
        var err = new SyntaxError(message);
        err.lineNumber = line;
        err.columnNumber = column;
        return err;
    };
    Object.defineProperty(exports, "__esModule", {
        value: true
    });
    var _$_typeof = typeof Symbol === "function" && _typeof$2(Symbol.iterator) === "symbol" ? function _$_typeof(obj) {
        return typeof obj === "undefined" ? "undefined" : _typeof$2(obj);
    } : function(obj) {
        return obj && typeof Symbol === "function" && obj.constructor === Symbol && obj !== Symbol.prototype ? "symbol" : typeof obj === "undefined" ? "undefined" : _typeof$2(obj);
    };
    exports.default = parse;
    var _util = util;
    var util$1 = _interopRequireWildcard(_util);
    var source = void 0;
    var parseState = void 0;
    var stack = void 0;
    var pos = void 0;
    var line = void 0;
    var column = void 0;
    var token = void 0;
    var key = void 0;
    var root = void 0;
    function internalize(holder, name, reviver) {
        var value = holder[name];
        if (value != null && (typeof value === "undefined" ? "undefined" : _$_typeof(value)) === "object") {
            for(var _key in value){
                var replacement = internalize(value, _key, reviver);
                if (replacement === undefined) {
                    delete value[_key];
                } else {
                    value[_key] = replacement;
                }
            }
        }
        return reviver.call(holder, name, value);
    }
    var lexState = void 0;
    var buffer = void 0;
    var doubleQuote = void 0;
    var _sign = void 0;
    var c = void 0;
    var lexStates = {
        default: function _default() {
            switch(c){
                case "	":
                case "\v":
                case "\f":
                case " ":
                case "\xa0":
                case "\uFEFF":
                case "\n":
                case "\r":
                case "\u2028":
                case "\u2029":
                    read();
                    return;
                case "/":
                    read();
                    lexState = "comment";
                    return;
                case undefined:
                    read();
                    return newToken("eof");
            }
            if (util$1.isSpaceSeparator(c)) {
                read();
                return;
            }
            return lexStates[parseState]();
        },
        comment: function comment() {
            switch(c){
                case "*":
                    read();
                    lexState = "multiLineComment";
                    return;
                case "/":
                    read();
                    lexState = "singleLineComment";
                    return;
            }
            throw invalidChar(read());
        },
        multiLineComment: function multiLineComment() {
            switch(c){
                case "*":
                    read();
                    lexState = "multiLineCommentAsterisk";
                    return;
                case undefined:
                    throw invalidChar(read());
            }
            read();
        },
        multiLineCommentAsterisk: function multiLineCommentAsterisk() {
            switch(c){
                case "*":
                    read();
                    return;
                case "/":
                    read();
                    lexState = "default";
                    return;
                case undefined:
                    throw invalidChar(read());
            }
            read();
            lexState = "multiLineComment";
        },
        singleLineComment: function singleLineComment() {
            switch(c){
                case "\n":
                case "\r":
                case "\u2028":
                case "\u2029":
                    read();
                    lexState = "default";
                    return;
                case undefined:
                    read();
                    return newToken("eof");
            }
            read();
        },
        value: function value() {
            switch(c){
                case "{":
                case "[":
                    return newToken("punctuator", read());
                case "n":
                    read();
                    literal("ull");
                    return newToken("null", null);
                case "t":
                    read();
                    literal("rue");
                    return newToken("boolean", true);
                case "f":
                    read();
                    literal("alse");
                    return newToken("boolean", false);
                case "-":
                case "+":
                    if (read() === "-") {
                        _sign = -1;
                    }
                    lexState = "sign";
                    return;
                case ".":
                    buffer = read();
                    lexState = "decimalPointLeading";
                    return;
                case "0":
                    buffer = read();
                    lexState = "zero";
                    return;
                case "1":
                case "2":
                case "3":
                case "4":
                case "5":
                case "6":
                case "7":
                case "8":
                case "9":
                    buffer = read();
                    lexState = "decimalInteger";
                    return;
                case "I":
                    read();
                    literal("nfinity");
                    return newToken("numeric", Infinity);
                case "N":
                    read();
                    literal("aN");
                    return newToken("numeric", NaN);
                case '"':
                case "'":
                    doubleQuote = read() === '"';
                    buffer = "";
                    lexState = "string";
                    return;
            }
            throw invalidChar(read());
        },
        identifierNameStartEscape: function identifierNameStartEscape() {
            if (c !== "u") {
                throw invalidChar(read());
            }
            read();
            var u = unicodeEscape();
            switch(u){
                case "$":
                case "_":
                    break;
                default:
                    if (!util$1.isIdStartChar(u)) {
                        throw invalidIdentifier();
                    }
                    break;
            }
            buffer += u;
            lexState = "identifierName";
        },
        identifierName: function identifierName() {
            switch(c){
                case "$":
                case "_":
                case "‌":
                case "‍":
                    buffer += read();
                    return;
                case "\\":
                    read();
                    lexState = "identifierNameEscape";
                    return;
            }
            if (util$1.isIdContinueChar(c)) {
                buffer += read();
                return;
            }
            return newToken("identifier", buffer);
        },
        identifierNameEscape: function identifierNameEscape() {
            if (c !== "u") {
                throw invalidChar(read());
            }
            read();
            var u = unicodeEscape();
            switch(u){
                case "$":
                case "_":
                case "‌":
                case "‍":
                    break;
                default:
                    if (!util$1.isIdContinueChar(u)) {
                        throw invalidIdentifier();
                    }
                    break;
            }
            buffer += u;
            lexState = "identifierName";
        },
        sign: function sign() {
            switch(c){
                case ".":
                    buffer = read();
                    lexState = "decimalPointLeading";
                    return;
                case "0":
                    buffer = read();
                    lexState = "zero";
                    return;
                case "1":
                case "2":
                case "3":
                case "4":
                case "5":
                case "6":
                case "7":
                case "8":
                case "9":
                    buffer = read();
                    lexState = "decimalInteger";
                    return;
                case "I":
                    read();
                    literal("nfinity");
                    return newToken("numeric", _sign * Infinity);
                case "N":
                    read();
                    literal("aN");
                    return newToken("numeric", NaN);
            }
            throw invalidChar(read());
        },
        zero: function zero() {
            switch(c){
                case ".":
                    buffer += read();
                    lexState = "decimalPoint";
                    return;
                case "e":
                case "E":
                    buffer += read();
                    lexState = "decimalExponent";
                    return;
                case "x":
                case "X":
                    buffer += read();
                    lexState = "hexadecimal";
                    return;
            }
            return newToken("numeric", _sign * 0);
        },
        decimalInteger: function decimalInteger() {
            switch(c){
                case ".":
                    buffer += read();
                    lexState = "decimalPoint";
                    return;
                case "e":
                case "E":
                    buffer += read();
                    lexState = "decimalExponent";
                    return;
            }
            if (util$1.isDigit(c)) {
                buffer += read();
                return;
            }
            return newToken("numeric", _sign * Number(buffer));
        },
        decimalPointLeading: function decimalPointLeading() {
            if (util$1.isDigit(c)) {
                buffer += read();
                lexState = "decimalFraction";
                return;
            }
            throw invalidChar(read());
        },
        decimalPoint: function decimalPoint() {
            switch(c){
                case "e":
                case "E":
                    buffer += read();
                    lexState = "decimalExponent";
                    return;
            }
            if (util$1.isDigit(c)) {
                buffer += read();
                lexState = "decimalFraction";
                return;
            }
            return newToken("numeric", _sign * Number(buffer));
        },
        decimalFraction: function decimalFraction() {
            switch(c){
                case "e":
                case "E":
                    buffer += read();
                    lexState = "decimalExponent";
                    return;
            }
            if (util$1.isDigit(c)) {
                buffer += read();
                return;
            }
            return newToken("numeric", _sign * Number(buffer));
        },
        decimalExponent: function decimalExponent() {
            switch(c){
                case "+":
                case "-":
                    buffer += read();
                    lexState = "decimalExponentSign";
                    return;
            }
            if (util$1.isDigit(c)) {
                buffer += read();
                lexState = "decimalExponentInteger";
                return;
            }
            throw invalidChar(read());
        },
        decimalExponentSign: function decimalExponentSign() {
            if (util$1.isDigit(c)) {
                buffer += read();
                lexState = "decimalExponentInteger";
                return;
            }
            throw invalidChar(read());
        },
        decimalExponentInteger: function decimalExponentInteger() {
            if (util$1.isDigit(c)) {
                buffer += read();
                return;
            }
            return newToken("numeric", _sign * Number(buffer));
        },
        hexadecimal: function hexadecimal() {
            if (util$1.isHexDigit(c)) {
                buffer += read();
                lexState = "hexadecimalInteger";
                return;
            }
            throw invalidChar(read());
        },
        hexadecimalInteger: function hexadecimalInteger() {
            if (util$1.isHexDigit(c)) {
                buffer += read();
                return;
            }
            return newToken("numeric", _sign * Number(buffer));
        },
        string: function string() {
            switch(c){
                case "\\":
                    read();
                    buffer += escape();
                    return;
                case '"':
                    if (doubleQuote) {
                        read();
                        return newToken("string", buffer);
                    }
                    buffer += read();
                    return;
                case "'":
                    if (!doubleQuote) {
                        read();
                        return newToken("string", buffer);
                    }
                    buffer += read();
                    return;
                case "\n":
                case "\r":
                    throw invalidChar(read());
                case "\u2028":
                case "\u2029":
                    separatorChar(c);
                    break;
                case undefined:
                    throw invalidChar(read());
            }
            buffer += read();
        },
        start: function start() {
            switch(c){
                case "{":
                case "[":
                    return newToken("punctuator", read());
            }
            lexState = "value";
        },
        beforePropertyName: function beforePropertyName() {
            switch(c){
                case "$":
                case "_":
                    buffer = read();
                    lexState = "identifierName";
                    return;
                case "\\":
                    read();
                    lexState = "identifierNameStartEscape";
                    return;
                case "}":
                    return newToken("punctuator", read());
                case '"':
                case "'":
                    doubleQuote = read() === '"';
                    lexState = "string";
                    return;
            }
            if (util$1.isIdStartChar(c)) {
                buffer += read();
                lexState = "identifierName";
                return;
            }
            throw invalidChar(read());
        },
        afterPropertyName: function afterPropertyName() {
            if (c === ":") {
                return newToken("punctuator", read());
            }
            throw invalidChar(read());
        },
        beforePropertyValue: function beforePropertyValue() {
            lexState = "value";
        },
        afterPropertyValue: function afterPropertyValue() {
            switch(c){
                case ",":
                case "}":
                    return newToken("punctuator", read());
            }
            throw invalidChar(read());
        },
        beforeArrayValue: function beforeArrayValue() {
            if (c === "]") {
                return newToken("punctuator", read());
            }
            lexState = "value";
        },
        afterArrayValue: function afterArrayValue() {
            switch(c){
                case ",":
                case "]":
                    return newToken("punctuator", read());
            }
            throw invalidChar(read());
        },
        end: function end() {
            throw invalidChar(read());
        }
    };
    var parseStates = {
        start: function start() {
            if (token.type === "eof") {
                throw invalidEOF();
            }
            push();
        },
        beforePropertyName: function beforePropertyName() {
            switch(token.type){
                case "identifier":
                case "string":
                    key = token.value;
                    parseState = "afterPropertyName";
                    return;
                case "punctuator":
                    pop();
                    return;
                case "eof":
                    throw invalidEOF();
            }
        },
        afterPropertyName: function afterPropertyName() {
            if (token.type === "eof") {
                throw invalidEOF();
            }
            parseState = "beforePropertyValue";
        },
        beforePropertyValue: function beforePropertyValue() {
            if (token.type === "eof") {
                throw invalidEOF();
            }
            push();
        },
        beforeArrayValue: function beforeArrayValue() {
            if (token.type === "eof") {
                throw invalidEOF();
            }
            if (token.type === "punctuator" && token.value === "]") {
                pop();
                return;
            }
            push();
        },
        afterPropertyValue: function afterPropertyValue() {
            if (token.type === "eof") {
                throw invalidEOF();
            }
            switch(token.value){
                case ",":
                    parseState = "beforePropertyName";
                    return;
                case "}":
                    pop();
            }
        },
        afterArrayValue: function afterArrayValue() {
            if (token.type === "eof") {
                throw invalidEOF();
            }
            switch(token.value){
                case ",":
                    parseState = "beforeArrayValue";
                    return;
                case "]":
                    pop();
            }
        },
        end: function end() {}
    };
    module.exports = exports["default"];
})(parse$1, parse$1.exports);

var stringify$1 = {exports: {}};

function _instanceof$1(left, right) {
    if (right != null && typeof Symbol !== "undefined" && right[Symbol.hasInstance]) {
        return !!right[Symbol.hasInstance](left);
    } else {
        return left instanceof right;
    }
}
var _typeof$1 = function(obj) {
    "@swc/helpers - typeof";
    return obj && typeof Symbol !== "undefined" && obj.constructor === Symbol ? "symbol" : typeof obj;
};
(function(module, exports) {
    var _interopRequireWildcard = function _interopRequireWildcard(obj) {
        if (obj && obj.__esModule) {
            return obj;
        } else {
            var newObj = {};
            if (obj != null) {
                for(var key in obj){
                    if (Object.prototype.hasOwnProperty.call(obj, key)) newObj[key] = obj[key];
                }
            }
            newObj.default = obj;
            return newObj;
        }
    };
    var stringify = function stringify(value, replacer, space) {
        var stack = [];
        var indent = "";
        var propertyList = void 0;
        var replacerFunc = void 0;
        var gap = "";
        var quote = void 0;
        if (replacer != null && (typeof replacer === "undefined" ? "undefined" : _$_typeof(replacer)) === "object" && !Array.isArray(replacer)) {
            space = replacer.space;
            quote = replacer.quote;
            replacer = replacer.replacer;
        }
        if (typeof replacer === "function") {
            replacerFunc = replacer;
        } else if (Array.isArray(replacer)) {
            propertyList = [];
            var _iteratorNormalCompletion = true;
            var _didIteratorError = false;
            var _iteratorError = undefined;
            try {
                for(var _iterator = replacer[Symbol.iterator](), _step; !(_iteratorNormalCompletion = (_step = _iterator.next()).done); _iteratorNormalCompletion = true){
                    var v = _step.value;
                    var item = void 0;
                    if (typeof v === "string") {
                        item = v;
                    } else if (typeof v === "number" || _instanceof$1(v, String) || _instanceof$1(v, Number)) {
                        item = String(v);
                    }
                    if (item !== undefined && propertyList.indexOf(item) < 0) {
                        propertyList.push(item);
                    }
                }
            } catch (err) {
                _didIteratorError = true;
                _iteratorError = err;
            } finally{
                try {
                    if (!_iteratorNormalCompletion && _iterator.return) {
                        _iterator.return();
                    }
                } finally{
                    if (_didIteratorError) {
                        throw _iteratorError;
                    }
                }
            }
        }
        if (_instanceof$1(space, Number)) {
            space = Number(space);
        } else if (_instanceof$1(space, String)) {
            space = String(space);
        }
        if (typeof space === "number") {
            if (space > 0) {
                space = Math.min(10, Math.floor(space));
                gap = "          ".substr(0, space);
            }
        } else if (typeof space === "string") {
            gap = space.substr(0, 10);
        }
        return serializeProperty("", {
            "": value
        });
        function serializeProperty(key, holder) {
            var _$value = holder[key];
            if (_$value != null) {
                if (typeof _$value.toJSON5 === "function") {
                    _$value = _$value.toJSON5(key);
                } else if (typeof _$value.toJSON === "function") {
                    _$value = _$value.toJSON(key);
                }
            }
            if (replacerFunc) {
                _$value = replacerFunc.call(holder, key, _$value);
            }
            if (_instanceof$1(_$value, Number)) {
                _$value = Number(_$value);
            } else if (_instanceof$1(_$value, String)) {
                _$value = String(_$value);
            } else if (_instanceof$1(_$value, Boolean)) {
                _$value = _$value.valueOf();
            }
            switch(_$value){
                case null:
                    return "null";
                case true:
                    return "true";
                case false:
                    return "false";
            }
            if (typeof _$value === "string") {
                return quoteString(_$value);
            }
            if (typeof _$value === "number") {
                return String(_$value);
            }
            if ((typeof _$value === "undefined" ? "undefined" : _$_typeof(_$value)) === "object") {
                return Array.isArray(_$value) ? serializeArray(_$value) : serializeObject(_$value);
            }
            return undefined;
        }
        function quoteString(value) {
            var quotes = {
                "'": 0.1,
                '"': 0.2
            };
            var replacements = {
                "'": "\\'",
                '"': '\\"',
                "\\": "\\\\",
                "\b": "\\b",
                "\f": "\\f",
                "\n": "\\n",
                "\r": "\\r",
                "	": "\\t",
                "\v": "\\v",
                "\0": "\\0",
                "\u2028": "\\u2028",
                "\u2029": "\\u2029"
            };
            var product = "";
            var _iteratorNormalCompletion2 = true;
            var _didIteratorError2 = false;
            var _iteratorError2 = undefined;
            try {
                for(var _iterator2 = value[Symbol.iterator](), _step2; !(_iteratorNormalCompletion2 = (_step2 = _iterator2.next()).done); _iteratorNormalCompletion2 = true){
                    var c = _step2.value;
                    switch(c){
                        case "'":
                        case '"':
                            quotes[c]++;
                            product += c;
                            continue;
                    }
                    if (replacements[c]) {
                        product += replacements[c];
                        continue;
                    }
                    if (c < " ") {
                        var hexString = c.charCodeAt(0).toString(16);
                        product += "\\x" + ("00" + hexString).substring(hexString.length);
                        continue;
                    }
                    product += c;
                }
            } catch (err) {
                _didIteratorError2 = true;
                _iteratorError2 = err;
            } finally{
                try {
                    if (!_iteratorNormalCompletion2 && _iterator2.return) {
                        _iterator2.return();
                    }
                } finally{
                    if (_didIteratorError2) {
                        throw _iteratorError2;
                    }
                }
            }
            var quoteChar = quote || Object.keys(quotes).reduce(function(a, b) {
                return quotes[a] < quotes[b] ? a : b;
            });
            product = product.replace(new RegExp(quoteChar, "g"), replacements[quoteChar]);
            return quoteChar + product + quoteChar;
        }
        function serializeObject(value) {
            if (stack.indexOf(value) >= 0) {
                throw TypeError("Converting circular structure to JSON5");
            }
            stack.push(value);
            var stepback = indent;
            indent = indent + gap;
            var keys = propertyList || Object.keys(value);
            var partial = [];
            var _iteratorNormalCompletion3 = true;
            var _didIteratorError3 = false;
            var _iteratorError3 = undefined;
            try {
                for(var _iterator3 = keys[Symbol.iterator](), _step3; !(_iteratorNormalCompletion3 = (_step3 = _iterator3.next()).done); _iteratorNormalCompletion3 = true){
                    var key = _step3.value;
                    var propertyString = serializeProperty(key, value);
                    if (propertyString !== undefined) {
                        var member = serializeKey(key) + ":";
                        if (gap !== "") {
                            member += " ";
                        }
                        member += propertyString;
                        partial.push(member);
                    }
                }
            } catch (err) {
                _didIteratorError3 = true;
                _iteratorError3 = err;
            } finally{
                try {
                    if (!_iteratorNormalCompletion3 && _iterator3.return) {
                        _iterator3.return();
                    }
                } finally{
                    if (_didIteratorError3) {
                        throw _iteratorError3;
                    }
                }
            }
            var final = void 0;
            if (partial.length === 0) {
                final = "{}";
            } else {
                var properties = void 0;
                if (gap === "") {
                    properties = partial.join(",");
                    final = "{" + properties + "}";
                } else {
                    var separator = ",\n" + indent;
                    properties = partial.join(separator);
                    final = "{\n" + indent + properties + ",\n" + stepback + "}";
                }
            }
            stack.pop();
            indent = stepback;
            return final;
        }
        function serializeKey(key) {
            if (key.length === 0) {
                return quoteString(key);
            }
            var firstChar = String.fromCodePoint(key.codePointAt(0));
            if (!util$1.isIdStartChar(firstChar)) {
                return quoteString(key);
            }
            for(var i = firstChar.length; i < key.length; i++){
                if (!util$1.isIdContinueChar(String.fromCodePoint(key.codePointAt(i)))) {
                    return quoteString(key);
                }
            }
            return key;
        }
        function serializeArray(value) {
            if (stack.indexOf(value) >= 0) {
                throw TypeError("Converting circular structure to JSON5");
            }
            stack.push(value);
            var stepback = indent;
            indent = indent + gap;
            var partial = [];
            for(var i = 0; i < value.length; i++){
                var propertyString = serializeProperty(String(i), value);
                partial.push(propertyString !== undefined ? propertyString : "null");
            }
            var final = void 0;
            if (partial.length === 0) {
                final = "[]";
            } else {
                if (gap === "") {
                    var properties = partial.join(",");
                    final = "[" + properties + "]";
                } else {
                    var separator = ",\n" + indent;
                    var _properties = partial.join(separator);
                    final = "[\n" + indent + _properties + ",\n" + stepback + "]";
                }
            }
            stack.pop();
            indent = stepback;
            return final;
        }
    };
    Object.defineProperty(exports, "__esModule", {
        value: true
    });
    var _$_typeof = typeof Symbol === "function" && _typeof$1(Symbol.iterator) === "symbol" ? function _$_typeof(obj) {
        return typeof obj === "undefined" ? "undefined" : _typeof$1(obj);
    } : function(obj) {
        return obj && typeof Symbol === "function" && obj.constructor === Symbol && obj !== Symbol.prototype ? "symbol" : typeof obj === "undefined" ? "undefined" : _typeof$1(obj);
    };
    exports.default = stringify;
    var _util = util;
    var util$1 = _interopRequireWildcard(_util);
    module.exports = exports["default"];
})(stringify$1, stringify$1.exports);

(function(module, exports) {
    var _interopRequireDefault = function _interopRequireDefault(obj) {
        return obj && obj.__esModule ? obj : {
            default: obj
        };
    };
    Object.defineProperty(exports, "__esModule", {
        value: true
    });
    var _parse = parse$1.exports;
    var _parse2 = _interopRequireDefault(_parse);
    var _stringify = stringify$1.exports;
    var _stringify2 = _interopRequireDefault(_stringify);
    exports.default = {
        parse: _parse2.default,
        stringify: _stringify2.default
    };
    module.exports = exports["default"];
})(lib, lib.exports);

var JSON5 = lib.exports;
var specialValues = {
    null: null,
    true: true,
    false: false
};
function parseQuery$2(query) {
    if (query.substr(0, 1) !== "?") {
        throw new Error("A valid query string passed to parseQuery should begin with '?'");
    }
    query = query.substr(1);
    if (!query) {
        return {};
    }
    if (query.substr(0, 1) === "{" && query.substr(-1) === "}") {
        return JSON5.parse(query);
    }
    var queryArgs = query.split(/[,&]/g);
    var result = Object.create(null);
    queryArgs.forEach(function(arg) {
        var idx = arg.indexOf("=");
        if (idx >= 0) {
            var name = arg.substr(0, idx);
            var value = decodeURIComponent(arg.substr(idx + 1));
            if (specialValues.hasOwnProperty(value)) {
                value = specialValues[value];
            }
            if (name.substr(-2) === "[]") {
                name = decodeURIComponent(name.substr(0, name.length - 2));
                if (!Array.isArray(result[name])) {
                    result[name] = [];
                }
                result[name].push(value);
            } else {
                name = decodeURIComponent(name);
                result[name] = value;
            }
        } else {
            if (arg.substr(0, 1) === "-") {
                result[decodeURIComponent(arg.substr(1))] = false;
            } else if (arg.substr(0, 1) === "+") {
                result[decodeURIComponent(arg.substr(1))] = true;
            } else {
                result[decodeURIComponent(arg)] = true;
            }
        }
    });
    return result;
}
var parseQuery_1 = parseQuery$2;

var parseQuery$1 = parseQuery_1;
function getOptions$1(loaderContext) {
    var query = loaderContext.query;
    if (typeof query === "string" && query !== "") {
        return parseQuery$1(loaderContext.query);
    }
    if (!query || typeof query !== "object") {
        // Not object-like queries are not supported.
        return null;
    }
    return query;
}
var getOptions_1 = getOptions$1;

var path$2 = require$$0__default["default"];
var matchRelativePath = /^\.\.?[/\\]/;
function isAbsolutePath(str) {
    return path$2.posix.isAbsolute(str) || path$2.win32.isAbsolute(str);
}
function isRelativePath(str) {
    return matchRelativePath.test(str);
}
function stringifyRequest$1(loaderContext, request) {
    var splitted = request.split("!");
    var context = loaderContext.context || loaderContext.options && loaderContext.options.context;
    return JSON.stringify(splitted.map(function(part) {
        // First, separate singlePath from query, because the query might contain paths again
        var splittedPart = part.match(/^(.*?)(\?.*)/);
        var query = splittedPart ? splittedPart[2] : "";
        var singlePath = splittedPart ? splittedPart[1] : part;
        if (isAbsolutePath(singlePath) && context) {
            singlePath = path$2.relative(context, singlePath);
            if (isAbsolutePath(singlePath)) {
                // If singlePath still matches an absolute path, singlePath was on a different drive than context.
                // In this case, we leave the path platform-specific without replacing any separators.
                // @see https://github.com/webpack/loader-utils/pull/14
                return singlePath + query;
            }
            if (isRelativePath(singlePath) === false) {
                // Ensure that the relative path starts at least with ./ otherwise it would be a request into the modules directory (like node_modules).
                singlePath = "./" + singlePath;
            }
        }
        return singlePath.replace(/\\/g, "/") + query;
    }).join("!"));
}
var stringifyRequest_1 = stringifyRequest$1;

function getRemainingRequest$1(loaderContext) {
    if (loaderContext.remainingRequest) {
        return loaderContext.remainingRequest;
    }
    var request = loaderContext.loaders.slice(loaderContext.loaderIndex + 1).map(function(obj) {
        return obj.request;
    }).concat([
        loaderContext.resource
    ]);
    return request.join("!");
}
var getRemainingRequest_1 = getRemainingRequest$1;

function getCurrentRequest$1(loaderContext) {
    if (loaderContext.currentRequest) {
        return loaderContext.currentRequest;
    }
    var request = loaderContext.loaders.slice(loaderContext.loaderIndex).map(function(obj) {
        return obj.request;
    }).concat([
        loaderContext.resource
    ]);
    return request.join("!");
}
var getCurrentRequest_1 = getCurrentRequest$1;

var path$1 = require$$0__default["default"];
function isUrlRequest$1(url, root) {
    // An URL is not an request if
    // 1. It's an absolute url and it is not `windows` path like `C:\dir\file`
    if (/^[a-z][a-z0-9+.-]*:/i.test(url) && !path$1.win32.isAbsolute(url)) {
        return false;
    }
    // 2. It's a protocol-relative
    if (/^\/\//.test(url)) {
        return false;
    }
    // 3. It's some kind of url for a template
    if (/^[{}[\]#*;,'§$%&(=?`´^°<>]/.test(url)) {
        return false;
    }
    // 4. It's also not an request if root isn't set and it's a root-relative url
    if ((root === undefined || root === false) && /^\//.test(url)) {
        return false;
    }
    return true;
}
var isUrlRequest_1 = isUrlRequest$1;

var _typeof = function(obj) {
    "@swc/helpers - typeof";
    return obj && typeof Symbol !== "undefined" && obj.constructor === Symbol ? "symbol" : typeof obj;
};
// we can't use path.win32.isAbsolute because it also matches paths starting with a forward slash
var matchNativeWin32Path = /^[A-Z]:[/\\]|^\\\\/i;
function urlToRequest$1(url, root) {
    // Do not rewrite an empty url
    if (url === "") {
        return "";
    }
    var moduleRequestRegex = /^[^?]*~/;
    var request;
    if (matchNativeWin32Path.test(url)) {
        // absolute windows path, keep it
        request = url;
    } else if (root !== undefined && root !== false && /^\//.test(url)) {
        // if root is set and the url is root-relative
        switch(typeof root === "undefined" ? "undefined" : _typeof(root)){
            // 1. root is a string: root is prefixed to the url
            case "string":
                // special case: `~` roots convert to module request
                if (moduleRequestRegex.test(root)) {
                    request = root.replace(/([^~/])$/, "$1/") + url.slice(1);
                } else {
                    request = root + url;
                }
                break;
            // 2. root is `true`: absolute paths are allowed
            //    *nix only, windows-style absolute paths are always allowed as they doesn't start with a `/`
            case "boolean":
                request = url;
                break;
            default:
                throw new Error("Unexpected parameters to loader-utils 'urlToRequest': url = " + url + ", root = " + root + ".");
        }
    } else if (/^\.\.?\//.test(url)) {
        // A relative url stays
        request = url;
    } else {
        // every other url is threaded like a relative url
        request = "./" + url;
    }
    // A `~` makes the url an module
    if (moduleRequestRegex.test(request)) {
        request = request.replace(moduleRequestRegex, "");
    }
    return request;
}
var urlToRequest_1 = urlToRequest$1;

function parseString$1(str) {
    try {
        if (str[0] === '"') {
            return JSON.parse(str);
        }
        if (str[0] === "'" && str.substr(str.length - 1) === "'") {
            return parseString$1(str.replace(/\\.|"/g, function(x) {
                return x === '"' ? '\\"' : x;
            }).replace(/^'|'$/g, '"'));
        }
        return JSON.parse('"' + str + '"');
    } catch (e) {
        return str;
    }
}
var parseString_1 = parseString$1;

/*
 *  big.js v5.2.2
 *  A small, fast, easy-to-use library for arbitrary-precision decimal arithmetic.
 *  Copyright (c) 2018 Michael Mclaughlin <M8ch88l@gmail.com>
 *  https://github.com/MikeMcl/big.js/LICENCE
 */ /************************************** EDITABLE DEFAULTS *****************************************/ // The default values below must be integers within the stated ranges.
/*
   * The maximum number of decimal places (DP) of the results of operations involving division:
   * div and sqrt, and pow with negative exponents.
   */ function _instanceof(left, right) {
    if (right != null && typeof Symbol !== "undefined" && right[Symbol.hasInstance]) {
        return !!right[Symbol.hasInstance](left);
    } else {
        return left instanceof right;
    }
}
var DP = 20, /*
   * The rounding mode (RM) used when rounding to the above decimal places.
   *
   *  0  Towards zero (i.e. truncate, no rounding).       (ROUND_DOWN)
   *  1  To nearest neighbour. If equidistant, round up.  (ROUND_HALF_UP)
   *  2  To nearest neighbour. If equidistant, to even.   (ROUND_HALF_EVEN)
   *  3  Away from zero.                                  (ROUND_UP)
   */ RM = 1, // The maximum value of DP and Big.DP.
MAX_DP = 1E6, // The maximum magnitude of the exponent argument to the pow method.
MAX_POWER = 1E6, /*
   * The negative exponent (NE) at and beneath which toString returns exponential notation.
   * (JavaScript numbers: -7)
   * -1000000 is the minimum recommended exponent value of a Big.
   */ NE = -7, /*
   * The positive exponent (PE) at and above which toString returns exponential notation.
   * (JavaScript numbers: 21)
   * 1000000 is the maximum recommended exponent value of a Big.
   * (This limit is not enforced or checked.)
   */ PE = 21, /**************************************************************************************************/ // Error messages.
NAME = "[big.js] ", INVALID = NAME + "Invalid ", INVALID_DP = INVALID + "decimal places", INVALID_RM = INVALID + "rounding mode", DIV_BY_ZERO = NAME + "Division by zero", // The shared prototype object.
P = {}, UNDEFINED = void 0, NUMERIC = /^-?(\d+(\.\d*)?|\.\d+)(e[+-]?\d+)?$/i;
/*
 * Create and return a Big constructor.
 *
 */ function _Big_() {
    /*
   * The Big constructor and exported function.
   * Create and return a new instance of a Big number object.
   *
   * n {number|string|Big} A numeric value.
   */ function Big(n) {
        var x = this;
        // Enable constructor usage without new.
        if (!_instanceof(x, Big)) return n === UNDEFINED ? _Big_() : new Big(n);
        // Duplicate.
        if (_instanceof(n, Big)) {
            x.s = n.s;
            x.e = n.e;
            x.c = n.c.slice();
        } else {
            parse(x, n);
        }
        /*
     * Retain a reference to this Big constructor, and shadow Big.prototype.constructor which
     * points to Object.
     */ x.constructor = Big;
    }
    Big.prototype = P;
    Big.DP = DP;
    Big.RM = RM;
    Big.NE = NE;
    Big.PE = PE;
    Big.version = "5.2.2";
    return Big;
}
/*
 * Parse the number or string value passed to a Big constructor.
 *
 * x {Big} A Big number instance.
 * n {number|string} A numeric value.
 */ function parse(x, n) {
    var e, i, nl;
    // Minus zero?
    if (n === 0 && 1 / n < 0) n = "-0";
    else if (!NUMERIC.test(n += "")) throw Error(INVALID + "number");
    // Determine sign.
    x.s = n.charAt(0) == "-" ? (n = n.slice(1), -1) : 1;
    // Decimal point?
    if ((e = n.indexOf(".")) > -1) n = n.replace(".", "");
    // Exponential form?
    if ((i = n.search(/e/i)) > 0) {
        // Determine exponent.
        if (e < 0) e = i;
        e += +n.slice(i + 1);
        n = n.substring(0, i);
    } else if (e < 0) {
        // Integer.
        e = n.length;
    }
    nl = n.length;
    // Determine leading zeros.
    for(i = 0; i < nl && n.charAt(i) == "0";)++i;
    if (i == nl) {
        // Zero.
        x.c = [
            x.e = 0
        ];
    } else {
        // Determine trailing zeros.
        for(; nl > 0 && n.charAt(--nl) == "0";);
        x.e = e - i - 1;
        x.c = [];
        // Convert string to array of digits without leading/trailing zeros.
        for(e = 0; i <= nl;)x.c[e++] = +n.charAt(i++);
    }
    return x;
}
/*
 * Round Big x to a maximum of dp decimal places using rounding mode rm.
 * Called by stringify, P.div, P.round and P.sqrt.
 *
 * x {Big} The Big to round.
 * dp {number} Integer, 0 to MAX_DP inclusive.
 * rm {number} 0, 1, 2 or 3 (DOWN, HALF_UP, HALF_EVEN, UP)
 * [more] {boolean} Whether the result of division was truncated.
 */ function round(x, dp, rm, more) {
    var xc = x.c, i = x.e + dp + 1;
    if (i < xc.length) {
        if (rm === 1) {
            // xc[i] is the digit after the digit that may be rounded up.
            more = xc[i] >= 5;
        } else if (rm === 2) {
            more = xc[i] > 5 || xc[i] == 5 && (more || i < 0 || xc[i + 1] !== UNDEFINED || xc[i - 1] & 1);
        } else if (rm === 3) {
            more = more || !!xc[0];
        } else {
            more = false;
            if (rm !== 0) throw Error(INVALID_RM);
        }
        if (i < 1) {
            xc.length = 1;
            if (more) {
                // 1, 0.1, 0.01, 0.001, 0.0001 etc.
                x.e = -dp;
                xc[0] = 1;
            } else {
                // Zero.
                xc[0] = x.e = 0;
            }
        } else {
            // Remove any digits after the required decimal places.
            xc.length = i--;
            // Round up?
            if (more) {
                // Rounding up may mean the previous digit has to be rounded up.
                for(; ++xc[i] > 9;){
                    xc[i] = 0;
                    if (!i--) {
                        ++x.e;
                        xc.unshift(1);
                    }
                }
            }
            // Remove trailing zeros.
            for(i = xc.length; !xc[--i];)xc.pop();
        }
    } else if (rm < 0 || rm > 3 || rm !== ~~rm) {
        throw Error(INVALID_RM);
    }
    return x;
}
/*
 * Return a string representing the value of Big x in normal or exponential notation.
 * Handles P.toExponential, P.toFixed, P.toJSON, P.toPrecision, P.toString and P.valueOf.
 *
 * x {Big}
 * id? {number} Caller id.
 *         1 toExponential
 *         2 toFixed
 *         3 toPrecision
 *         4 valueOf
 * n? {number|undefined} Caller's argument.
 * k? {number|undefined}
 */ function stringify(x, id, n, k) {
    var e, s, Big = x.constructor, z = !x.c[0];
    if (n !== UNDEFINED) {
        if (n !== ~~n || n < (id == 3) || n > MAX_DP) {
            throw Error(id == 3 ? INVALID + "precision" : INVALID_DP);
        }
        x = new Big(x);
        // The index of the digit that may be rounded up.
        n = k - x.e;
        // Round?
        if (x.c.length > ++k) round(x, n, Big.RM);
        // toFixed: recalculate k as x.e may have changed if value rounded up.
        if (id == 2) k = x.e + n + 1;
        // Append zeros?
        for(; x.c.length < k;)x.c.push(0);
    }
    e = x.e;
    s = x.c.join("");
    n = s.length;
    // Exponential notation?
    if (id != 2 && (id == 1 || id == 3 && k <= e || e <= Big.NE || e >= Big.PE)) {
        s = s.charAt(0) + (n > 1 ? "." + s.slice(1) : "") + (e < 0 ? "e" : "e+") + e;
    // Normal notation.
    } else if (e < 0) {
        for(; ++e;)s = "0" + s;
        s = "0." + s;
    } else if (e > 0) {
        if (++e > n) for(e -= n; e--;)s += "0";
        else if (e < n) s = s.slice(0, e) + "." + s.slice(e);
    } else if (n > 1) {
        s = s.charAt(0) + "." + s.slice(1);
    }
    return x.s < 0 && (!z || id == 4) ? "-" + s : s;
}
// Prototype/instance methods
/*
 * Return a new Big whose value is the absolute value of this Big.
 */ P.abs = function() {
    var x = new this.constructor(this);
    x.s = 1;
    return x;
};
/*
 * Return 1 if the value of this Big is greater than the value of Big y,
 *       -1 if the value of this Big is less than the value of Big y, or
 *        0 if they have the same value.
*/ P.cmp = function(y) {
    var isneg, x = this, xc = x.c, yc = (y = new x.constructor(y)).c, i = x.s, j = y.s, k = x.e, l = y.e;
    // Either zero?
    if (!xc[0] || !yc[0]) return !xc[0] ? !yc[0] ? 0 : -j : i;
    // Signs differ?
    if (i != j) return i;
    isneg = i < 0;
    // Compare exponents.
    if (k != l) return k > l ^ isneg ? 1 : -1;
    j = (k = xc.length) < (l = yc.length) ? k : l;
    // Compare digit by digit.
    for(i = -1; ++i < j;){
        if (xc[i] != yc[i]) return xc[i] > yc[i] ^ isneg ? 1 : -1;
    }
    // Compare lengths.
    return k == l ? 0 : k > l ^ isneg ? 1 : -1;
};
/*
 * Return a new Big whose value is the value of this Big divided by the value of Big y, rounded,
 * if necessary, to a maximum of Big.DP decimal places using rounding mode Big.RM.
 */ P.div = function(y) {
    var x = this, Big = x.constructor, a = x.c, b = (y = new Big(y)).c, k = x.s == y.s ? 1 : -1, dp = Big.DP;
    if (dp !== ~~dp || dp < 0 || dp > MAX_DP) throw Error(INVALID_DP);
    // Divisor is zero?
    if (!b[0]) throw Error(DIV_BY_ZERO);
    // Dividend is 0? Return +-0.
    if (!a[0]) return new Big(k * 0);
    var bl, bt, n, cmp, ri, bz = b.slice(), ai = bl = b.length, al = a.length, r = a.slice(0, bl), rl = r.length, q = y, qc = q.c = [], qi = 0, d = dp + (q.e = x.e - y.e) + 1; // number of digits of the result
    q.s = k;
    k = d < 0 ? 0 : d;
    // Create version of divisor with leading zero.
    bz.unshift(0);
    // Add zeros to make remainder as long as divisor.
    for(; rl++ < bl;)r.push(0);
    do {
        // n is how many times the divisor goes into current remainder.
        for(n = 0; n < 10; n++){
            // Compare divisor and remainder.
            if (bl != (rl = r.length)) {
                cmp = bl > rl ? 1 : -1;
            } else {
                for(ri = -1, cmp = 0; ++ri < bl;){
                    if (b[ri] != r[ri]) {
                        cmp = b[ri] > r[ri] ? 1 : -1;
                        break;
                    }
                }
            }
            // If divisor < remainder, subtract divisor from remainder.
            if (cmp < 0) {
                // Remainder can't be more than 1 digit longer than divisor.
                // Equalise lengths using divisor with extra leading zero?
                for(bt = rl == bl ? b : bz; rl;){
                    if (r[--rl] < bt[rl]) {
                        ri = rl;
                        for(; ri && !r[--ri];)r[ri] = 9;
                        --r[ri];
                        r[rl] += 10;
                    }
                    r[rl] -= bt[rl];
                }
                for(; !r[0];)r.shift();
            } else {
                break;
            }
        }
        // Add the digit n to the result array.
        qc[qi++] = cmp ? n : ++n;
        // Update the remainder.
        if (r[0] && cmp) r[rl] = a[ai] || 0;
        else r = [
            a[ai]
        ];
    }while ((ai++ < al || r[0] !== UNDEFINED) && k--);
    // Leading zero? Do not remove if result is simply zero (qi == 1).
    if (!qc[0] && qi != 1) {
        // There can't be more than one zero.
        qc.shift();
        q.e--;
    }
    // Round?
    if (qi > d) round(q, dp, Big.RM, r[0] !== UNDEFINED);
    return q;
};
/*
 * Return true if the value of this Big is equal to the value of Big y, otherwise return false.
 */ P.eq = function(y) {
    return !this.cmp(y);
};
/*
 * Return true if the value of this Big is greater than the value of Big y, otherwise return
 * false.
 */ P.gt = function(y) {
    return this.cmp(y) > 0;
};
/*
 * Return true if the value of this Big is greater than or equal to the value of Big y, otherwise
 * return false.
 */ P.gte = function(y) {
    return this.cmp(y) > -1;
};
/*
 * Return true if the value of this Big is less than the value of Big y, otherwise return false.
 */ P.lt = function(y) {
    return this.cmp(y) < 0;
};
/*
 * Return true if the value of this Big is less than or equal to the value of Big y, otherwise
 * return false.
 */ P.lte = function(y) {
    return this.cmp(y) < 1;
};
/*
 * Return a new Big whose value is the value of this Big minus the value of Big y.
 */ P.minus = P.sub = function(y) {
    var i, j, t, xlty, x = this, Big = x.constructor, a = x.s, b = (y = new Big(y)).s;
    // Signs differ?
    if (a != b) {
        y.s = -b;
        return x.plus(y);
    }
    var xc = x.c.slice(), xe = x.e, yc = y.c, ye = y.e;
    // Either zero?
    if (!xc[0] || !yc[0]) {
        // y is non-zero? x is non-zero? Or both are zero.
        return yc[0] ? (y.s = -b, y) : new Big(xc[0] ? x : 0);
    }
    // Determine which is the bigger number. Prepend zeros to equalise exponents.
    if (a = xe - ye) {
        if (xlty = a < 0) {
            a = -a;
            t = xc;
        } else {
            ye = xe;
            t = yc;
        }
        t.reverse();
        for(b = a; b--;)t.push(0);
        t.reverse();
    } else {
        // Exponents equal. Check digit by digit.
        j = ((xlty = xc.length < yc.length) ? xc : yc).length;
        for(a = b = 0; b < j; b++){
            if (xc[b] != yc[b]) {
                xlty = xc[b] < yc[b];
                break;
            }
        }
    }
    // x < y? Point xc to the array of the bigger number.
    if (xlty) {
        t = xc;
        xc = yc;
        yc = t;
        y.s = -y.s;
    }
    /*
   * Append zeros to xc if shorter. No need to add zeros to yc if shorter as subtraction only
   * needs to start at yc.length.
   */ if ((b = (j = yc.length) - (i = xc.length)) > 0) for(; b--;)xc[i++] = 0;
    // Subtract yc from xc.
    for(b = i; j > a;){
        if (xc[--j] < yc[j]) {
            for(i = j; i && !xc[--i];)xc[i] = 9;
            --xc[i];
            xc[j] += 10;
        }
        xc[j] -= yc[j];
    }
    // Remove trailing zeros.
    for(; xc[--b] === 0;)xc.pop();
    // Remove leading zeros and adjust exponent accordingly.
    for(; xc[0] === 0;){
        xc.shift();
        --ye;
    }
    if (!xc[0]) {
        // n - n = +0
        y.s = 1;
        // Result must be zero.
        xc = [
            ye = 0
        ];
    }
    y.c = xc;
    y.e = ye;
    return y;
};
/*
 * Return a new Big whose value is the value of this Big modulo the value of Big y.
 */ P.mod = function(y) {
    var ygtx, x = this, Big = x.constructor, a = x.s, b = (y = new Big(y)).s;
    if (!y.c[0]) throw Error(DIV_BY_ZERO);
    x.s = y.s = 1;
    ygtx = y.cmp(x) == 1;
    x.s = a;
    y.s = b;
    if (ygtx) return new Big(x);
    a = Big.DP;
    b = Big.RM;
    Big.DP = Big.RM = 0;
    x = x.div(y);
    Big.DP = a;
    Big.RM = b;
    return this.minus(x.times(y));
};
/*
 * Return a new Big whose value is the value of this Big plus the value of Big y.
 */ P.plus = P.add = function(y) {
    var t, x = this, Big = x.constructor, a = x.s, b = (y = new Big(y)).s;
    // Signs differ?
    if (a != b) {
        y.s = -b;
        return x.minus(y);
    }
    var xe = x.e, xc = x.c, ye = y.e, yc = y.c;
    // Either zero? y is non-zero? x is non-zero? Or both are zero.
    if (!xc[0] || !yc[0]) return yc[0] ? y : new Big(xc[0] ? x : a * 0);
    xc = xc.slice();
    // Prepend zeros to equalise exponents.
    // Note: reverse faster than unshifts.
    if (a = xe - ye) {
        if (a > 0) {
            ye = xe;
            t = yc;
        } else {
            a = -a;
            t = xc;
        }
        t.reverse();
        for(; a--;)t.push(0);
        t.reverse();
    }
    // Point xc to the longer array.
    if (xc.length - yc.length < 0) {
        t = yc;
        yc = xc;
        xc = t;
    }
    a = yc.length;
    // Only start adding at yc.length - 1 as the further digits of xc can be left as they are.
    for(b = 0; a; xc[a] %= 10)b = (xc[--a] = xc[a] + yc[a] + b) / 10 | 0;
    // No need to check for zero, as +x + +y != 0 && -x + -y != 0
    if (b) {
        xc.unshift(b);
        ++ye;
    }
    // Remove trailing zeros.
    for(a = xc.length; xc[--a] === 0;)xc.pop();
    y.c = xc;
    y.e = ye;
    return y;
};
/*
 * Return a Big whose value is the value of this Big raised to the power n.
 * If n is negative, round to a maximum of Big.DP decimal places using rounding
 * mode Big.RM.
 *
 * n {number} Integer, -MAX_POWER to MAX_POWER inclusive.
 */ P.pow = function(n) {
    var x = this, one = new x.constructor(1), y = one, isneg = n < 0;
    if (n !== ~~n || n < -MAX_POWER || n > MAX_POWER) throw Error(INVALID + "exponent");
    if (isneg) n = -n;
    for(;;){
        if (n & 1) y = y.times(x);
        n >>= 1;
        if (!n) break;
        x = x.times(x);
    }
    return isneg ? one.div(y) : y;
};
/*
 * Return a new Big whose value is the value of this Big rounded using rounding mode rm
 * to a maximum of dp decimal places, or, if dp is negative, to an integer which is a
 * multiple of 10**-dp.
 * If dp is not specified, round to 0 decimal places.
 * If rm is not specified, use Big.RM.
 *
 * dp? {number} Integer, -MAX_DP to MAX_DP inclusive.
 * rm? 0, 1, 2 or 3 (ROUND_DOWN, ROUND_HALF_UP, ROUND_HALF_EVEN, ROUND_UP)
 */ P.round = function(dp, rm) {
    var Big = this.constructor;
    if (dp === UNDEFINED) dp = 0;
    else if (dp !== ~~dp || dp < -MAX_DP || dp > MAX_DP) throw Error(INVALID_DP);
    return round(new Big(this), dp, rm === UNDEFINED ? Big.RM : rm);
};
/*
 * Return a new Big whose value is the square root of the value of this Big, rounded, if
 * necessary, to a maximum of Big.DP decimal places using rounding mode Big.RM.
 */ P.sqrt = function() {
    var r, c, t, x = this, Big = x.constructor, s = x.s, e = x.e, half = new Big(0.5);
    // Zero?
    if (!x.c[0]) return new Big(x);
    // Negative?
    if (s < 0) throw Error(NAME + "No square root");
    // Estimate.
    s = Math.sqrt(x + "");
    // Math.sqrt underflow/overflow?
    // Re-estimate: pass x coefficient to Math.sqrt as integer, then adjust the result exponent.
    if (s === 0 || s === 1 / 0) {
        c = x.c.join("");
        if (!(c.length + e & 1)) c += "0";
        s = Math.sqrt(c);
        e = ((e + 1) / 2 | 0) - (e < 0 || e & 1);
        r = new Big((s == 1 / 0 ? "1e" : (s = s.toExponential()).slice(0, s.indexOf("e") + 1)) + e);
    } else {
        r = new Big(s);
    }
    e = r.e + (Big.DP += 4);
    // Newton-Raphson iteration.
    do {
        t = r;
        r = half.times(t.plus(x.div(t)));
    }while (t.c.slice(0, e).join("") !== r.c.slice(0, e).join(""));
    return round(r, Big.DP -= 4, Big.RM);
};
/*
 * Return a new Big whose value is the value of this Big times the value of Big y.
 */ P.times = P.mul = function(y) {
    var c, x = this, Big = x.constructor, xc = x.c, yc = (y = new Big(y)).c, a = xc.length, b = yc.length, i = x.e, j = y.e;
    // Determine sign of result.
    y.s = x.s == y.s ? 1 : -1;
    // Return signed 0 if either 0.
    if (!xc[0] || !yc[0]) return new Big(y.s * 0);
    // Initialise exponent of result as x.e + y.e.
    y.e = i + j;
    // If array xc has fewer digits than yc, swap xc and yc, and lengths.
    if (a < b) {
        c = xc;
        xc = yc;
        yc = c;
        j = a;
        a = b;
        b = j;
    }
    // Initialise coefficient array of result with zeros.
    for(c = new Array(j = a + b); j--;)c[j] = 0;
    // Multiply.
    // i is initially xc.length.
    for(i = b; i--;){
        b = 0;
        // a is yc.length.
        for(j = a + i; j > i;){
            // Current sum of products at this digit position, plus carry.
            b = c[j] + yc[i] * xc[j - i - 1] + b;
            c[j--] = b % 10;
            // carry
            b = b / 10 | 0;
        }
        c[j] = (c[j] + b) % 10;
    }
    // Increment result exponent if there is a final carry, otherwise remove leading zero.
    if (b) ++y.e;
    else c.shift();
    // Remove trailing zeros.
    for(i = c.length; !c[--i];)c.pop();
    y.c = c;
    return y;
};
/*
 * Return a string representing the value of this Big in exponential notation to dp fixed decimal
 * places and rounded using Big.RM.
 *
 * dp? {number} Integer, 0 to MAX_DP inclusive.
 */ P.toExponential = function(dp) {
    return stringify(this, 1, dp, dp);
};
/*
 * Return a string representing the value of this Big in normal notation to dp fixed decimal
 * places and rounded using Big.RM.
 *
 * dp? {number} Integer, 0 to MAX_DP inclusive.
 *
 * (-0).toFixed(0) is '0', but (-0.1).toFixed(0) is '-0'.
 * (-0).toFixed(1) is '0.0', but (-0.01).toFixed(1) is '-0.0'.
 */ P.toFixed = function(dp) {
    return stringify(this, 2, dp, this.e + dp);
};
/*
 * Return a string representing the value of this Big rounded to sd significant digits using
 * Big.RM. Use exponential notation if sd is less than the number of digits necessary to represent
 * the integer part of the value in normal notation.
 *
 * sd {number} Integer, 1 to MAX_DP inclusive.
 */ P.toPrecision = function(sd) {
    return stringify(this, 3, sd, sd - 1);
};
/*
 * Return a string representing the value of this Big.
 * Return exponential notation if this Big has a positive exponent equal to or greater than
 * Big.PE, or a negative exponent equal to or less than Big.NE.
 * Omit the sign for negative zero.
 */ P.toString = function() {
    return stringify(this);
};
/*
 * Return a string representing the value of this Big.
 * Return exponential notation if this Big has a positive exponent equal to or greater than
 * Big.PE, or a negative exponent equal to or less than Big.NE.
 * Include the sign for negative zero.
 */ P.valueOf = P.toJSON = function() {
    return stringify(this, 4);
};
// Export
var Big = _Big_();

var big = {
	__proto__: null,
	Big: Big,
	'default': Big
};

var require$$0 = /*@__PURE__*/getAugmentedNamespace(big);

var baseEncodeTables = {
    26: "abcdefghijklmnopqrstuvwxyz",
    32: "123456789abcdefghjkmnpqrstuvwxyz",
    36: "0123456789abcdefghijklmnopqrstuvwxyz",
    49: "abcdefghijkmnopqrstuvwxyzABCDEFGHJKLMNPQRSTUVWXYZ",
    52: "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ",
    58: "123456789abcdefghijkmnopqrstuvwxyzABCDEFGHJKLMNPQRSTUVWXYZ",
    62: "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ",
    64: "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ-_"
};
function encodeBufferToBase(buffer, base) {
    var encodeTable = baseEncodeTables[base];
    if (!encodeTable) {
        throw new Error("Unknown encoding base" + base);
    }
    var readLength = buffer.length;
    var Big = require$$0;
    Big.RM = Big.DP = 0;
    var b = new Big(0);
    for(var i = readLength - 1; i >= 0; i--){
        b = b.times(256).plus(buffer[i]);
    }
    var output = "";
    while(b.gt(0)){
        output = encodeTable[b.mod(base)] + output;
        b = b.div(base);
    }
    Big.DP = 20;
    Big.RM = 1;
    return output;
}
function getHashDigest$2(buffer, hashType, digestType, maxLength) {
    hashType = hashType || "md5";
    maxLength = maxLength || 9999;
    var hash = require$$1__default["default"].createHash(hashType);
    hash.update(buffer);
    if (digestType === "base26" || digestType === "base32" || digestType === "base36" || digestType === "base49" || digestType === "base52" || digestType === "base58" || digestType === "base62" || digestType === "base64") {
        return encodeBufferToBase(hash.digest(), digestType.substr(4)).substr(0, maxLength);
    } else {
        return hash.digest(digestType || "hex").substr(0, maxLength);
    }
}
var getHashDigest_1 = getHashDigest$2;

var emojisList$1 = [
    "\uD83C\uDC04️",
    "\uD83C\uDCCF",
    "\uD83C\uDD70️",
    "\uD83C\uDD71️",
    "\uD83C\uDD7E️",
    "\uD83C\uDD7F️",
    "\uD83C\uDD8E",
    "\uD83C\uDD91",
    "\uD83C\uDD92",
    "\uD83C\uDD93",
    "\uD83C\uDD94",
    "\uD83C\uDD95",
    "\uD83C\uDD96",
    "\uD83C\uDD97",
    "\uD83C\uDD98",
    "\uD83C\uDD99",
    "\uD83C\uDD9A",
    "\uD83C\uDDE6\uD83C\uDDE8",
    "\uD83C\uDDE6\uD83C\uDDE9",
    "\uD83C\uDDE6\uD83C\uDDEA",
    "\uD83C\uDDE6\uD83C\uDDEB",
    "\uD83C\uDDE6\uD83C\uDDEC",
    "\uD83C\uDDE6\uD83C\uDDEE",
    "\uD83C\uDDE6\uD83C\uDDF1",
    "\uD83C\uDDE6\uD83C\uDDF2",
    "\uD83C\uDDE6\uD83C\uDDF4",
    "\uD83C\uDDE6\uD83C\uDDF6",
    "\uD83C\uDDE6\uD83C\uDDF7",
    "\uD83C\uDDE6\uD83C\uDDF8",
    "\uD83C\uDDE6\uD83C\uDDF9",
    "\uD83C\uDDE6\uD83C\uDDFA",
    "\uD83C\uDDE6\uD83C\uDDFC",
    "\uD83C\uDDE6\uD83C\uDDFD",
    "\uD83C\uDDE6\uD83C\uDDFF",
    "\uD83C\uDDE6",
    "\uD83C\uDDE7\uD83C\uDDE6",
    "\uD83C\uDDE7\uD83C\uDDE7",
    "\uD83C\uDDE7\uD83C\uDDE9",
    "\uD83C\uDDE7\uD83C\uDDEA",
    "\uD83C\uDDE7\uD83C\uDDEB",
    "\uD83C\uDDE7\uD83C\uDDEC",
    "\uD83C\uDDE7\uD83C\uDDED",
    "\uD83C\uDDE7\uD83C\uDDEE",
    "\uD83C\uDDE7\uD83C\uDDEF",
    "\uD83C\uDDE7\uD83C\uDDF1",
    "\uD83C\uDDE7\uD83C\uDDF2",
    "\uD83C\uDDE7\uD83C\uDDF3",
    "\uD83C\uDDE7\uD83C\uDDF4",
    "\uD83C\uDDE7\uD83C\uDDF6",
    "\uD83C\uDDE7\uD83C\uDDF7",
    "\uD83C\uDDE7\uD83C\uDDF8",
    "\uD83C\uDDE7\uD83C\uDDF9",
    "\uD83C\uDDE7\uD83C\uDDFB",
    "\uD83C\uDDE7\uD83C\uDDFC",
    "\uD83C\uDDE7\uD83C\uDDFE",
    "\uD83C\uDDE7\uD83C\uDDFF",
    "\uD83C\uDDE7",
    "\uD83C\uDDE8\uD83C\uDDE6",
    "\uD83C\uDDE8\uD83C\uDDE8",
    "\uD83C\uDDE8\uD83C\uDDE9",
    "\uD83C\uDDE8\uD83C\uDDEB",
    "\uD83C\uDDE8\uD83C\uDDEC",
    "\uD83C\uDDE8\uD83C\uDDED",
    "\uD83C\uDDE8\uD83C\uDDEE",
    "\uD83C\uDDE8\uD83C\uDDF0",
    "\uD83C\uDDE8\uD83C\uDDF1",
    "\uD83C\uDDE8\uD83C\uDDF2",
    "\uD83C\uDDE8\uD83C\uDDF3",
    "\uD83C\uDDE8\uD83C\uDDF4",
    "\uD83C\uDDE8\uD83C\uDDF5",
    "\uD83C\uDDE8\uD83C\uDDF7",
    "\uD83C\uDDE8\uD83C\uDDFA",
    "\uD83C\uDDE8\uD83C\uDDFB",
    "\uD83C\uDDE8\uD83C\uDDFC",
    "\uD83C\uDDE8\uD83C\uDDFD",
    "\uD83C\uDDE8\uD83C\uDDFE",
    "\uD83C\uDDE8\uD83C\uDDFF",
    "\uD83C\uDDE8",
    "\uD83C\uDDE9\uD83C\uDDEA",
    "\uD83C\uDDE9\uD83C\uDDEC",
    "\uD83C\uDDE9\uD83C\uDDEF",
    "\uD83C\uDDE9\uD83C\uDDF0",
    "\uD83C\uDDE9\uD83C\uDDF2",
    "\uD83C\uDDE9\uD83C\uDDF4",
    "\uD83C\uDDE9\uD83C\uDDFF",
    "\uD83C\uDDE9",
    "\uD83C\uDDEA\uD83C\uDDE6",
    "\uD83C\uDDEA\uD83C\uDDE8",
    "\uD83C\uDDEA\uD83C\uDDEA",
    "\uD83C\uDDEA\uD83C\uDDEC",
    "\uD83C\uDDEA\uD83C\uDDED",
    "\uD83C\uDDEA\uD83C\uDDF7",
    "\uD83C\uDDEA\uD83C\uDDF8",
    "\uD83C\uDDEA\uD83C\uDDF9",
    "\uD83C\uDDEA\uD83C\uDDFA",
    "\uD83C\uDDEA",
    "\uD83C\uDDEB\uD83C\uDDEE",
    "\uD83C\uDDEB\uD83C\uDDEF",
    "\uD83C\uDDEB\uD83C\uDDF0",
    "\uD83C\uDDEB\uD83C\uDDF2",
    "\uD83C\uDDEB\uD83C\uDDF4",
    "\uD83C\uDDEB\uD83C\uDDF7",
    "\uD83C\uDDEB",
    "\uD83C\uDDEC\uD83C\uDDE6",
    "\uD83C\uDDEC\uD83C\uDDE7",
    "\uD83C\uDDEC\uD83C\uDDE9",
    "\uD83C\uDDEC\uD83C\uDDEA",
    "\uD83C\uDDEC\uD83C\uDDEB",
    "\uD83C\uDDEC\uD83C\uDDEC",
    "\uD83C\uDDEC\uD83C\uDDED",
    "\uD83C\uDDEC\uD83C\uDDEE",
    "\uD83C\uDDEC\uD83C\uDDF1",
    "\uD83C\uDDEC\uD83C\uDDF2",
    "\uD83C\uDDEC\uD83C\uDDF3",
    "\uD83C\uDDEC\uD83C\uDDF5",
    "\uD83C\uDDEC\uD83C\uDDF6",
    "\uD83C\uDDEC\uD83C\uDDF7",
    "\uD83C\uDDEC\uD83C\uDDF8",
    "\uD83C\uDDEC\uD83C\uDDF9",
    "\uD83C\uDDEC\uD83C\uDDFA",
    "\uD83C\uDDEC\uD83C\uDDFC",
    "\uD83C\uDDEC\uD83C\uDDFE",
    "\uD83C\uDDEC",
    "\uD83C\uDDED\uD83C\uDDF0",
    "\uD83C\uDDED\uD83C\uDDF2",
    "\uD83C\uDDED\uD83C\uDDF3",
    "\uD83C\uDDED\uD83C\uDDF7",
    "\uD83C\uDDED\uD83C\uDDF9",
    "\uD83C\uDDED\uD83C\uDDFA",
    "\uD83C\uDDED",
    "\uD83C\uDDEE\uD83C\uDDE8",
    "\uD83C\uDDEE\uD83C\uDDE9",
    "\uD83C\uDDEE\uD83C\uDDEA",
    "\uD83C\uDDEE\uD83C\uDDF1",
    "\uD83C\uDDEE\uD83C\uDDF2",
    "\uD83C\uDDEE\uD83C\uDDF3",
    "\uD83C\uDDEE\uD83C\uDDF4",
    "\uD83C\uDDEE\uD83C\uDDF6",
    "\uD83C\uDDEE\uD83C\uDDF7",
    "\uD83C\uDDEE\uD83C\uDDF8",
    "\uD83C\uDDEE\uD83C\uDDF9",
    "\uD83C\uDDEE",
    "\uD83C\uDDEF\uD83C\uDDEA",
    "\uD83C\uDDEF\uD83C\uDDF2",
    "\uD83C\uDDEF\uD83C\uDDF4",
    "\uD83C\uDDEF\uD83C\uDDF5",
    "\uD83C\uDDEF",
    "\uD83C\uDDF0\uD83C\uDDEA",
    "\uD83C\uDDF0\uD83C\uDDEC",
    "\uD83C\uDDF0\uD83C\uDDED",
    "\uD83C\uDDF0\uD83C\uDDEE",
    "\uD83C\uDDF0\uD83C\uDDF2",
    "\uD83C\uDDF0\uD83C\uDDF3",
    "\uD83C\uDDF0\uD83C\uDDF5",
    "\uD83C\uDDF0\uD83C\uDDF7",
    "\uD83C\uDDF0\uD83C\uDDFC",
    "\uD83C\uDDF0\uD83C\uDDFE",
    "\uD83C\uDDF0\uD83C\uDDFF",
    "\uD83C\uDDF0",
    "\uD83C\uDDF1\uD83C\uDDE6",
    "\uD83C\uDDF1\uD83C\uDDE7",
    "\uD83C\uDDF1\uD83C\uDDE8",
    "\uD83C\uDDF1\uD83C\uDDEE",
    "\uD83C\uDDF1\uD83C\uDDF0",
    "\uD83C\uDDF1\uD83C\uDDF7",
    "\uD83C\uDDF1\uD83C\uDDF8",
    "\uD83C\uDDF1\uD83C\uDDF9",
    "\uD83C\uDDF1\uD83C\uDDFA",
    "\uD83C\uDDF1\uD83C\uDDFB",
    "\uD83C\uDDF1\uD83C\uDDFE",
    "\uD83C\uDDF1",
    "\uD83C\uDDF2\uD83C\uDDE6",
    "\uD83C\uDDF2\uD83C\uDDE8",
    "\uD83C\uDDF2\uD83C\uDDE9",
    "\uD83C\uDDF2\uD83C\uDDEA",
    "\uD83C\uDDF2\uD83C\uDDEB",
    "\uD83C\uDDF2\uD83C\uDDEC",
    "\uD83C\uDDF2\uD83C\uDDED",
    "\uD83C\uDDF2\uD83C\uDDF0",
    "\uD83C\uDDF2\uD83C\uDDF1",
    "\uD83C\uDDF2\uD83C\uDDF2",
    "\uD83C\uDDF2\uD83C\uDDF3",
    "\uD83C\uDDF2\uD83C\uDDF4",
    "\uD83C\uDDF2\uD83C\uDDF5",
    "\uD83C\uDDF2\uD83C\uDDF6",
    "\uD83C\uDDF2\uD83C\uDDF7",
    "\uD83C\uDDF2\uD83C\uDDF8",
    "\uD83C\uDDF2\uD83C\uDDF9",
    "\uD83C\uDDF2\uD83C\uDDFA",
    "\uD83C\uDDF2\uD83C\uDDFB",
    "\uD83C\uDDF2\uD83C\uDDFC",
    "\uD83C\uDDF2\uD83C\uDDFD",
    "\uD83C\uDDF2\uD83C\uDDFE",
    "\uD83C\uDDF2\uD83C\uDDFF",
    "\uD83C\uDDF2",
    "\uD83C\uDDF3\uD83C\uDDE6",
    "\uD83C\uDDF3\uD83C\uDDE8",
    "\uD83C\uDDF3\uD83C\uDDEA",
    "\uD83C\uDDF3\uD83C\uDDEB",
    "\uD83C\uDDF3\uD83C\uDDEC",
    "\uD83C\uDDF3\uD83C\uDDEE",
    "\uD83C\uDDF3\uD83C\uDDF1",
    "\uD83C\uDDF3\uD83C\uDDF4",
    "\uD83C\uDDF3\uD83C\uDDF5",
    "\uD83C\uDDF3\uD83C\uDDF7",
    "\uD83C\uDDF3\uD83C\uDDFA",
    "\uD83C\uDDF3\uD83C\uDDFF",
    "\uD83C\uDDF3",
    "\uD83C\uDDF4\uD83C\uDDF2",
    "\uD83C\uDDF4",
    "\uD83C\uDDF5\uD83C\uDDE6",
    "\uD83C\uDDF5\uD83C\uDDEA",
    "\uD83C\uDDF5\uD83C\uDDEB",
    "\uD83C\uDDF5\uD83C\uDDEC",
    "\uD83C\uDDF5\uD83C\uDDED",
    "\uD83C\uDDF5\uD83C\uDDF0",
    "\uD83C\uDDF5\uD83C\uDDF1",
    "\uD83C\uDDF5\uD83C\uDDF2",
    "\uD83C\uDDF5\uD83C\uDDF3",
    "\uD83C\uDDF5\uD83C\uDDF7",
    "\uD83C\uDDF5\uD83C\uDDF8",
    "\uD83C\uDDF5\uD83C\uDDF9",
    "\uD83C\uDDF5\uD83C\uDDFC",
    "\uD83C\uDDF5\uD83C\uDDFE",
    "\uD83C\uDDF5",
    "\uD83C\uDDF6\uD83C\uDDE6",
    "\uD83C\uDDF6",
    "\uD83C\uDDF7\uD83C\uDDEA",
    "\uD83C\uDDF7\uD83C\uDDF4",
    "\uD83C\uDDF7\uD83C\uDDF8",
    "\uD83C\uDDF7\uD83C\uDDFA",
    "\uD83C\uDDF7\uD83C\uDDFC",
    "\uD83C\uDDF7",
    "\uD83C\uDDF8\uD83C\uDDE6",
    "\uD83C\uDDF8\uD83C\uDDE7",
    "\uD83C\uDDF8\uD83C\uDDE8",
    "\uD83C\uDDF8\uD83C\uDDE9",
    "\uD83C\uDDF8\uD83C\uDDEA",
    "\uD83C\uDDF8\uD83C\uDDEC",
    "\uD83C\uDDF8\uD83C\uDDED",
    "\uD83C\uDDF8\uD83C\uDDEE",
    "\uD83C\uDDF8\uD83C\uDDEF",
    "\uD83C\uDDF8\uD83C\uDDF0",
    "\uD83C\uDDF8\uD83C\uDDF1",
    "\uD83C\uDDF8\uD83C\uDDF2",
    "\uD83C\uDDF8\uD83C\uDDF3",
    "\uD83C\uDDF8\uD83C\uDDF4",
    "\uD83C\uDDF8\uD83C\uDDF7",
    "\uD83C\uDDF8\uD83C\uDDF8",
    "\uD83C\uDDF8\uD83C\uDDF9",
    "\uD83C\uDDF8\uD83C\uDDFB",
    "\uD83C\uDDF8\uD83C\uDDFD",
    "\uD83C\uDDF8\uD83C\uDDFE",
    "\uD83C\uDDF8\uD83C\uDDFF",
    "\uD83C\uDDF8",
    "\uD83C\uDDF9\uD83C\uDDE6",
    "\uD83C\uDDF9\uD83C\uDDE8",
    "\uD83C\uDDF9\uD83C\uDDE9",
    "\uD83C\uDDF9\uD83C\uDDEB",
    "\uD83C\uDDF9\uD83C\uDDEC",
    "\uD83C\uDDF9\uD83C\uDDED",
    "\uD83C\uDDF9\uD83C\uDDEF",
    "\uD83C\uDDF9\uD83C\uDDF0",
    "\uD83C\uDDF9\uD83C\uDDF1",
    "\uD83C\uDDF9\uD83C\uDDF2",
    "\uD83C\uDDF9\uD83C\uDDF3",
    "\uD83C\uDDF9\uD83C\uDDF4",
    "\uD83C\uDDF9\uD83C\uDDF7",
    "\uD83C\uDDF9\uD83C\uDDF9",
    "\uD83C\uDDF9\uD83C\uDDFB",
    "\uD83C\uDDF9\uD83C\uDDFC",
    "\uD83C\uDDF9\uD83C\uDDFF",
    "\uD83C\uDDF9",
    "\uD83C\uDDFA\uD83C\uDDE6",
    "\uD83C\uDDFA\uD83C\uDDEC",
    "\uD83C\uDDFA\uD83C\uDDF2",
    "\uD83C\uDDFA\uD83C\uDDF3",
    "\uD83C\uDDFA\uD83C\uDDF8",
    "\uD83C\uDDFA\uD83C\uDDFE",
    "\uD83C\uDDFA\uD83C\uDDFF",
    "\uD83C\uDDFA",
    "\uD83C\uDDFB\uD83C\uDDE6",
    "\uD83C\uDDFB\uD83C\uDDE8",
    "\uD83C\uDDFB\uD83C\uDDEA",
    "\uD83C\uDDFB\uD83C\uDDEC",
    "\uD83C\uDDFB\uD83C\uDDEE",
    "\uD83C\uDDFB\uD83C\uDDF3",
    "\uD83C\uDDFB\uD83C\uDDFA",
    "\uD83C\uDDFB",
    "\uD83C\uDDFC\uD83C\uDDEB",
    "\uD83C\uDDFC\uD83C\uDDF8",
    "\uD83C\uDDFC",
    "\uD83C\uDDFD\uD83C\uDDF0",
    "\uD83C\uDDFD",
    "\uD83C\uDDFE\uD83C\uDDEA",
    "\uD83C\uDDFE\uD83C\uDDF9",
    "\uD83C\uDDFE",
    "\uD83C\uDDFF\uD83C\uDDE6",
    "\uD83C\uDDFF\uD83C\uDDF2",
    "\uD83C\uDDFF\uD83C\uDDFC",
    "\uD83C\uDDFF",
    "\uD83C\uDE01",
    "\uD83C\uDE02️",
    "\uD83C\uDE1A️",
    "\uD83C\uDE2F️",
    "\uD83C\uDE32",
    "\uD83C\uDE33",
    "\uD83C\uDE34",
    "\uD83C\uDE35",
    "\uD83C\uDE36",
    "\uD83C\uDE37️",
    "\uD83C\uDE38",
    "\uD83C\uDE39",
    "\uD83C\uDE3A",
    "\uD83C\uDE50",
    "\uD83C\uDE51",
    "\uD83C\uDF00",
    "\uD83C\uDF01",
    "\uD83C\uDF02",
    "\uD83C\uDF03",
    "\uD83C\uDF04",
    "\uD83C\uDF05",
    "\uD83C\uDF06",
    "\uD83C\uDF07",
    "\uD83C\uDF08",
    "\uD83C\uDF09",
    "\uD83C\uDF0A",
    "\uD83C\uDF0B",
    "\uD83C\uDF0C",
    "\uD83C\uDF0D",
    "\uD83C\uDF0E",
    "\uD83C\uDF0F",
    "\uD83C\uDF10",
    "\uD83C\uDF11",
    "\uD83C\uDF12",
    "\uD83C\uDF13",
    "\uD83C\uDF14",
    "\uD83C\uDF15",
    "\uD83C\uDF16",
    "\uD83C\uDF17",
    "\uD83C\uDF18",
    "\uD83C\uDF19",
    "\uD83C\uDF1A",
    "\uD83C\uDF1B",
    "\uD83C\uDF1C",
    "\uD83C\uDF1D",
    "\uD83C\uDF1E",
    "\uD83C\uDF1F",
    "\uD83C\uDF20",
    "\uD83C\uDF21️",
    "\uD83C\uDF24️",
    "\uD83C\uDF25️",
    "\uD83C\uDF26️",
    "\uD83C\uDF27️",
    "\uD83C\uDF28️",
    "\uD83C\uDF29️",
    "\uD83C\uDF2A️",
    "\uD83C\uDF2B️",
    "\uD83C\uDF2C️",
    "\uD83C\uDF2D",
    "\uD83C\uDF2E",
    "\uD83C\uDF2F",
    "\uD83C\uDF30",
    "\uD83C\uDF31",
    "\uD83C\uDF32",
    "\uD83C\uDF33",
    "\uD83C\uDF34",
    "\uD83C\uDF35",
    "\uD83C\uDF36️",
    "\uD83C\uDF37",
    "\uD83C\uDF38",
    "\uD83C\uDF39",
    "\uD83C\uDF3A",
    "\uD83C\uDF3B",
    "\uD83C\uDF3C",
    "\uD83C\uDF3D",
    "\uD83C\uDF3E",
    "\uD83C\uDF3F",
    "\uD83C\uDF40",
    "\uD83C\uDF41",
    "\uD83C\uDF42",
    "\uD83C\uDF43",
    "\uD83C\uDF44",
    "\uD83C\uDF45",
    "\uD83C\uDF46",
    "\uD83C\uDF47",
    "\uD83C\uDF48",
    "\uD83C\uDF49",
    "\uD83C\uDF4A",
    "\uD83C\uDF4B",
    "\uD83C\uDF4C",
    "\uD83C\uDF4D",
    "\uD83C\uDF4E",
    "\uD83C\uDF4F",
    "\uD83C\uDF50",
    "\uD83C\uDF51",
    "\uD83C\uDF52",
    "\uD83C\uDF53",
    "\uD83C\uDF54",
    "\uD83C\uDF55",
    "\uD83C\uDF56",
    "\uD83C\uDF57",
    "\uD83C\uDF58",
    "\uD83C\uDF59",
    "\uD83C\uDF5A",
    "\uD83C\uDF5B",
    "\uD83C\uDF5C",
    "\uD83C\uDF5D",
    "\uD83C\uDF5E",
    "\uD83C\uDF5F",
    "\uD83C\uDF60",
    "\uD83C\uDF61",
    "\uD83C\uDF62",
    "\uD83C\uDF63",
    "\uD83C\uDF64",
    "\uD83C\uDF65",
    "\uD83C\uDF66",
    "\uD83C\uDF67",
    "\uD83C\uDF68",
    "\uD83C\uDF69",
    "\uD83C\uDF6A",
    "\uD83C\uDF6B",
    "\uD83C\uDF6C",
    "\uD83C\uDF6D",
    "\uD83C\uDF6E",
    "\uD83C\uDF6F",
    "\uD83C\uDF70",
    "\uD83C\uDF71",
    "\uD83C\uDF72",
    "\uD83C\uDF73",
    "\uD83C\uDF74",
    "\uD83C\uDF75",
    "\uD83C\uDF76",
    "\uD83C\uDF77",
    "\uD83C\uDF78",
    "\uD83C\uDF79",
    "\uD83C\uDF7A",
    "\uD83C\uDF7B",
    "\uD83C\uDF7C",
    "\uD83C\uDF7D️",
    "\uD83C\uDF7E",
    "\uD83C\uDF7F",
    "\uD83C\uDF80",
    "\uD83C\uDF81",
    "\uD83C\uDF82",
    "\uD83C\uDF83",
    "\uD83C\uDF84",
    "\uD83C\uDF85\uD83C\uDFFB",
    "\uD83C\uDF85\uD83C\uDFFC",
    "\uD83C\uDF85\uD83C\uDFFD",
    "\uD83C\uDF85\uD83C\uDFFE",
    "\uD83C\uDF85\uD83C\uDFFF",
    "\uD83C\uDF85",
    "\uD83C\uDF86",
    "\uD83C\uDF87",
    "\uD83C\uDF88",
    "\uD83C\uDF89",
    "\uD83C\uDF8A",
    "\uD83C\uDF8B",
    "\uD83C\uDF8C",
    "\uD83C\uDF8D",
    "\uD83C\uDF8E",
    "\uD83C\uDF8F",
    "\uD83C\uDF90",
    "\uD83C\uDF91",
    "\uD83C\uDF92",
    "\uD83C\uDF93",
    "\uD83C\uDF96️",
    "\uD83C\uDF97️",
    "\uD83C\uDF99️",
    "\uD83C\uDF9A️",
    "\uD83C\uDF9B️",
    "\uD83C\uDF9E️",
    "\uD83C\uDF9F️",
    "\uD83C\uDFA0",
    "\uD83C\uDFA1",
    "\uD83C\uDFA2",
    "\uD83C\uDFA3",
    "\uD83C\uDFA4",
    "\uD83C\uDFA5",
    "\uD83C\uDFA6",
    "\uD83C\uDFA7",
    "\uD83C\uDFA8",
    "\uD83C\uDFA9",
    "\uD83C\uDFAA",
    "\uD83C\uDFAB",
    "\uD83C\uDFAC",
    "\uD83C\uDFAD",
    "\uD83C\uDFAE",
    "\uD83C\uDFAF",
    "\uD83C\uDFB0",
    "\uD83C\uDFB1",
    "\uD83C\uDFB2",
    "\uD83C\uDFB3",
    "\uD83C\uDFB4",
    "\uD83C\uDFB5",
    "\uD83C\uDFB6",
    "\uD83C\uDFB7",
    "\uD83C\uDFB8",
    "\uD83C\uDFB9",
    "\uD83C\uDFBA",
    "\uD83C\uDFBB",
    "\uD83C\uDFBC",
    "\uD83C\uDFBD",
    "\uD83C\uDFBE",
    "\uD83C\uDFBF",
    "\uD83C\uDFC0",
    "\uD83C\uDFC1",
    "\uD83C\uDFC2\uD83C\uDFFB",
    "\uD83C\uDFC2\uD83C\uDFFC",
    "\uD83C\uDFC2\uD83C\uDFFD",
    "\uD83C\uDFC2\uD83C\uDFFE",
    "\uD83C\uDFC2\uD83C\uDFFF",
    "\uD83C\uDFC2",
    "\uD83C\uDFC3\uD83C\uDFFB‍♀️",
    "\uD83C\uDFC3\uD83C\uDFFB‍♂️",
    "\uD83C\uDFC3\uD83C\uDFFB",
    "\uD83C\uDFC3\uD83C\uDFFC‍♀️",
    "\uD83C\uDFC3\uD83C\uDFFC‍♂️",
    "\uD83C\uDFC3\uD83C\uDFFC",
    "\uD83C\uDFC3\uD83C\uDFFD‍♀️",
    "\uD83C\uDFC3\uD83C\uDFFD‍♂️",
    "\uD83C\uDFC3\uD83C\uDFFD",
    "\uD83C\uDFC3\uD83C\uDFFE‍♀️",
    "\uD83C\uDFC3\uD83C\uDFFE‍♂️",
    "\uD83C\uDFC3\uD83C\uDFFE",
    "\uD83C\uDFC3\uD83C\uDFFF‍♀️",
    "\uD83C\uDFC3\uD83C\uDFFF‍♂️",
    "\uD83C\uDFC3\uD83C\uDFFF",
    "\uD83C\uDFC3‍♀️",
    "\uD83C\uDFC3‍♂️",
    "\uD83C\uDFC3",
    "\uD83C\uDFC4\uD83C\uDFFB‍♀️",
    "\uD83C\uDFC4\uD83C\uDFFB‍♂️",
    "\uD83C\uDFC4\uD83C\uDFFB",
    "\uD83C\uDFC4\uD83C\uDFFC‍♀️",
    "\uD83C\uDFC4\uD83C\uDFFC‍♂️",
    "\uD83C\uDFC4\uD83C\uDFFC",
    "\uD83C\uDFC4\uD83C\uDFFD‍♀️",
    "\uD83C\uDFC4\uD83C\uDFFD‍♂️",
    "\uD83C\uDFC4\uD83C\uDFFD",
    "\uD83C\uDFC4\uD83C\uDFFE‍♀️",
    "\uD83C\uDFC4\uD83C\uDFFE‍♂️",
    "\uD83C\uDFC4\uD83C\uDFFE",
    "\uD83C\uDFC4\uD83C\uDFFF‍♀️",
    "\uD83C\uDFC4\uD83C\uDFFF‍♂️",
    "\uD83C\uDFC4\uD83C\uDFFF",
    "\uD83C\uDFC4‍♀️",
    "\uD83C\uDFC4‍♂️",
    "\uD83C\uDFC4",
    "\uD83C\uDFC5",
    "\uD83C\uDFC6",
    "\uD83C\uDFC7\uD83C\uDFFB",
    "\uD83C\uDFC7\uD83C\uDFFC",
    "\uD83C\uDFC7\uD83C\uDFFD",
    "\uD83C\uDFC7\uD83C\uDFFE",
    "\uD83C\uDFC7\uD83C\uDFFF",
    "\uD83C\uDFC7",
    "\uD83C\uDFC8",
    "\uD83C\uDFC9",
    "\uD83C\uDFCA\uD83C\uDFFB‍♀️",
    "\uD83C\uDFCA\uD83C\uDFFB‍♂️",
    "\uD83C\uDFCA\uD83C\uDFFB",
    "\uD83C\uDFCA\uD83C\uDFFC‍♀️",
    "\uD83C\uDFCA\uD83C\uDFFC‍♂️",
    "\uD83C\uDFCA\uD83C\uDFFC",
    "\uD83C\uDFCA\uD83C\uDFFD‍♀️",
    "\uD83C\uDFCA\uD83C\uDFFD‍♂️",
    "\uD83C\uDFCA\uD83C\uDFFD",
    "\uD83C\uDFCA\uD83C\uDFFE‍♀️",
    "\uD83C\uDFCA\uD83C\uDFFE‍♂️",
    "\uD83C\uDFCA\uD83C\uDFFE",
    "\uD83C\uDFCA\uD83C\uDFFF‍♀️",
    "\uD83C\uDFCA\uD83C\uDFFF‍♂️",
    "\uD83C\uDFCA\uD83C\uDFFF",
    "\uD83C\uDFCA‍♀️",
    "\uD83C\uDFCA‍♂️",
    "\uD83C\uDFCA",
    "\uD83C\uDFCB\uD83C\uDFFB‍♀️",
    "\uD83C\uDFCB\uD83C\uDFFB‍♂️",
    "\uD83C\uDFCB\uD83C\uDFFB",
    "\uD83C\uDFCB\uD83C\uDFFC‍♀️",
    "\uD83C\uDFCB\uD83C\uDFFC‍♂️",
    "\uD83C\uDFCB\uD83C\uDFFC",
    "\uD83C\uDFCB\uD83C\uDFFD‍♀️",
    "\uD83C\uDFCB\uD83C\uDFFD‍♂️",
    "\uD83C\uDFCB\uD83C\uDFFD",
    "\uD83C\uDFCB\uD83C\uDFFE‍♀️",
    "\uD83C\uDFCB\uD83C\uDFFE‍♂️",
    "\uD83C\uDFCB\uD83C\uDFFE",
    "\uD83C\uDFCB\uD83C\uDFFF‍♀️",
    "\uD83C\uDFCB\uD83C\uDFFF‍♂️",
    "\uD83C\uDFCB\uD83C\uDFFF",
    "\uD83C\uDFCB️‍♀️",
    "\uD83C\uDFCB️‍♂️",
    "\uD83C\uDFCB️",
    "\uD83C\uDFCC\uD83C\uDFFB‍♀️",
    "\uD83C\uDFCC\uD83C\uDFFB‍♂️",
    "\uD83C\uDFCC\uD83C\uDFFB",
    "\uD83C\uDFCC\uD83C\uDFFC‍♀️",
    "\uD83C\uDFCC\uD83C\uDFFC‍♂️",
    "\uD83C\uDFCC\uD83C\uDFFC",
    "\uD83C\uDFCC\uD83C\uDFFD‍♀️",
    "\uD83C\uDFCC\uD83C\uDFFD‍♂️",
    "\uD83C\uDFCC\uD83C\uDFFD",
    "\uD83C\uDFCC\uD83C\uDFFE‍♀️",
    "\uD83C\uDFCC\uD83C\uDFFE‍♂️",
    "\uD83C\uDFCC\uD83C\uDFFE",
    "\uD83C\uDFCC\uD83C\uDFFF‍♀️",
    "\uD83C\uDFCC\uD83C\uDFFF‍♂️",
    "\uD83C\uDFCC\uD83C\uDFFF",
    "\uD83C\uDFCC️‍♀️",
    "\uD83C\uDFCC️‍♂️",
    "\uD83C\uDFCC️",
    "\uD83C\uDFCD️",
    "\uD83C\uDFCE️",
    "\uD83C\uDFCF",
    "\uD83C\uDFD0",
    "\uD83C\uDFD1",
    "\uD83C\uDFD2",
    "\uD83C\uDFD3",
    "\uD83C\uDFD4️",
    "\uD83C\uDFD5️",
    "\uD83C\uDFD6️",
    "\uD83C\uDFD7️",
    "\uD83C\uDFD8️",
    "\uD83C\uDFD9️",
    "\uD83C\uDFDA️",
    "\uD83C\uDFDB️",
    "\uD83C\uDFDC️",
    "\uD83C\uDFDD️",
    "\uD83C\uDFDE️",
    "\uD83C\uDFDF️",
    "\uD83C\uDFE0",
    "\uD83C\uDFE1",
    "\uD83C\uDFE2",
    "\uD83C\uDFE3",
    "\uD83C\uDFE4",
    "\uD83C\uDFE5",
    "\uD83C\uDFE6",
    "\uD83C\uDFE7",
    "\uD83C\uDFE8",
    "\uD83C\uDFE9",
    "\uD83C\uDFEA",
    "\uD83C\uDFEB",
    "\uD83C\uDFEC",
    "\uD83C\uDFED",
    "\uD83C\uDFEE",
    "\uD83C\uDFEF",
    "\uD83C\uDFF0",
    "\uD83C\uDFF3️‍\uD83C\uDF08",
    "\uD83C\uDFF3️",
    "\uD83C\uDFF4‍☠️",
    "\uD83C\uDFF4\uDB40\uDC67\uDB40\uDC62\uDB40\uDC65\uDB40\uDC6E\uDB40\uDC67\uDB40\uDC7F",
    "\uD83C\uDFF4\uDB40\uDC67\uDB40\uDC62\uDB40\uDC73\uDB40\uDC63\uDB40\uDC74\uDB40\uDC7F",
    "\uD83C\uDFF4\uDB40\uDC67\uDB40\uDC62\uDB40\uDC77\uDB40\uDC6C\uDB40\uDC73\uDB40\uDC7F",
    "\uD83C\uDFF4",
    "\uD83C\uDFF5️",
    "\uD83C\uDFF7️",
    "\uD83C\uDFF8",
    "\uD83C\uDFF9",
    "\uD83C\uDFFA",
    "\uD83C\uDFFB",
    "\uD83C\uDFFC",
    "\uD83C\uDFFD",
    "\uD83C\uDFFE",
    "\uD83C\uDFFF",
    "\uD83D\uDC00",
    "\uD83D\uDC01",
    "\uD83D\uDC02",
    "\uD83D\uDC03",
    "\uD83D\uDC04",
    "\uD83D\uDC05",
    "\uD83D\uDC06",
    "\uD83D\uDC07",
    "\uD83D\uDC08",
    "\uD83D\uDC09",
    "\uD83D\uDC0A",
    "\uD83D\uDC0B",
    "\uD83D\uDC0C",
    "\uD83D\uDC0D",
    "\uD83D\uDC0E",
    "\uD83D\uDC0F",
    "\uD83D\uDC10",
    "\uD83D\uDC11",
    "\uD83D\uDC12",
    "\uD83D\uDC13",
    "\uD83D\uDC14",
    "\uD83D\uDC15‍\uD83E\uDDBA",
    "\uD83D\uDC15",
    "\uD83D\uDC16",
    "\uD83D\uDC17",
    "\uD83D\uDC18",
    "\uD83D\uDC19",
    "\uD83D\uDC1A",
    "\uD83D\uDC1B",
    "\uD83D\uDC1C",
    "\uD83D\uDC1D",
    "\uD83D\uDC1E",
    "\uD83D\uDC1F",
    "\uD83D\uDC20",
    "\uD83D\uDC21",
    "\uD83D\uDC22",
    "\uD83D\uDC23",
    "\uD83D\uDC24",
    "\uD83D\uDC25",
    "\uD83D\uDC26",
    "\uD83D\uDC27",
    "\uD83D\uDC28",
    "\uD83D\uDC29",
    "\uD83D\uDC2A",
    "\uD83D\uDC2B",
    "\uD83D\uDC2C",
    "\uD83D\uDC2D",
    "\uD83D\uDC2E",
    "\uD83D\uDC2F",
    "\uD83D\uDC30",
    "\uD83D\uDC31",
    "\uD83D\uDC32",
    "\uD83D\uDC33",
    "\uD83D\uDC34",
    "\uD83D\uDC35",
    "\uD83D\uDC36",
    "\uD83D\uDC37",
    "\uD83D\uDC38",
    "\uD83D\uDC39",
    "\uD83D\uDC3A",
    "\uD83D\uDC3B",
    "\uD83D\uDC3C",
    "\uD83D\uDC3D",
    "\uD83D\uDC3E",
    "\uD83D\uDC3F️",
    "\uD83D\uDC40",
    "\uD83D\uDC41‍\uD83D\uDDE8",
    "\uD83D\uDC41️",
    "\uD83D\uDC42\uD83C\uDFFB",
    "\uD83D\uDC42\uD83C\uDFFC",
    "\uD83D\uDC42\uD83C\uDFFD",
    "\uD83D\uDC42\uD83C\uDFFE",
    "\uD83D\uDC42\uD83C\uDFFF",
    "\uD83D\uDC42",
    "\uD83D\uDC43\uD83C\uDFFB",
    "\uD83D\uDC43\uD83C\uDFFC",
    "\uD83D\uDC43\uD83C\uDFFD",
    "\uD83D\uDC43\uD83C\uDFFE",
    "\uD83D\uDC43\uD83C\uDFFF",
    "\uD83D\uDC43",
    "\uD83D\uDC44",
    "\uD83D\uDC45",
    "\uD83D\uDC46\uD83C\uDFFB",
    "\uD83D\uDC46\uD83C\uDFFC",
    "\uD83D\uDC46\uD83C\uDFFD",
    "\uD83D\uDC46\uD83C\uDFFE",
    "\uD83D\uDC46\uD83C\uDFFF",
    "\uD83D\uDC46",
    "\uD83D\uDC47\uD83C\uDFFB",
    "\uD83D\uDC47\uD83C\uDFFC",
    "\uD83D\uDC47\uD83C\uDFFD",
    "\uD83D\uDC47\uD83C\uDFFE",
    "\uD83D\uDC47\uD83C\uDFFF",
    "\uD83D\uDC47",
    "\uD83D\uDC48\uD83C\uDFFB",
    "\uD83D\uDC48\uD83C\uDFFC",
    "\uD83D\uDC48\uD83C\uDFFD",
    "\uD83D\uDC48\uD83C\uDFFE",
    "\uD83D\uDC48\uD83C\uDFFF",
    "\uD83D\uDC48",
    "\uD83D\uDC49\uD83C\uDFFB",
    "\uD83D\uDC49\uD83C\uDFFC",
    "\uD83D\uDC49\uD83C\uDFFD",
    "\uD83D\uDC49\uD83C\uDFFE",
    "\uD83D\uDC49\uD83C\uDFFF",
    "\uD83D\uDC49",
    "\uD83D\uDC4A\uD83C\uDFFB",
    "\uD83D\uDC4A\uD83C\uDFFC",
    "\uD83D\uDC4A\uD83C\uDFFD",
    "\uD83D\uDC4A\uD83C\uDFFE",
    "\uD83D\uDC4A\uD83C\uDFFF",
    "\uD83D\uDC4A",
    "\uD83D\uDC4B\uD83C\uDFFB",
    "\uD83D\uDC4B\uD83C\uDFFC",
    "\uD83D\uDC4B\uD83C\uDFFD",
    "\uD83D\uDC4B\uD83C\uDFFE",
    "\uD83D\uDC4B\uD83C\uDFFF",
    "\uD83D\uDC4B",
    "\uD83D\uDC4C\uD83C\uDFFB",
    "\uD83D\uDC4C\uD83C\uDFFC",
    "\uD83D\uDC4C\uD83C\uDFFD",
    "\uD83D\uDC4C\uD83C\uDFFE",
    "\uD83D\uDC4C\uD83C\uDFFF",
    "\uD83D\uDC4C",
    "\uD83D\uDC4D\uD83C\uDFFB",
    "\uD83D\uDC4D\uD83C\uDFFC",
    "\uD83D\uDC4D\uD83C\uDFFD",
    "\uD83D\uDC4D\uD83C\uDFFE",
    "\uD83D\uDC4D\uD83C\uDFFF",
    "\uD83D\uDC4D",
    "\uD83D\uDC4E\uD83C\uDFFB",
    "\uD83D\uDC4E\uD83C\uDFFC",
    "\uD83D\uDC4E\uD83C\uDFFD",
    "\uD83D\uDC4E\uD83C\uDFFE",
    "\uD83D\uDC4E\uD83C\uDFFF",
    "\uD83D\uDC4E",
    "\uD83D\uDC4F\uD83C\uDFFB",
    "\uD83D\uDC4F\uD83C\uDFFC",
    "\uD83D\uDC4F\uD83C\uDFFD",
    "\uD83D\uDC4F\uD83C\uDFFE",
    "\uD83D\uDC4F\uD83C\uDFFF",
    "\uD83D\uDC4F",
    "\uD83D\uDC50\uD83C\uDFFB",
    "\uD83D\uDC50\uD83C\uDFFC",
    "\uD83D\uDC50\uD83C\uDFFD",
    "\uD83D\uDC50\uD83C\uDFFE",
    "\uD83D\uDC50\uD83C\uDFFF",
    "\uD83D\uDC50",
    "\uD83D\uDC51",
    "\uD83D\uDC52",
    "\uD83D\uDC53",
    "\uD83D\uDC54",
    "\uD83D\uDC55",
    "\uD83D\uDC56",
    "\uD83D\uDC57",
    "\uD83D\uDC58",
    "\uD83D\uDC59",
    "\uD83D\uDC5A",
    "\uD83D\uDC5B",
    "\uD83D\uDC5C",
    "\uD83D\uDC5D",
    "\uD83D\uDC5E",
    "\uD83D\uDC5F",
    "\uD83D\uDC60",
    "\uD83D\uDC61",
    "\uD83D\uDC62",
    "\uD83D\uDC63",
    "\uD83D\uDC64",
    "\uD83D\uDC65",
    "\uD83D\uDC66\uD83C\uDFFB",
    "\uD83D\uDC66\uD83C\uDFFC",
    "\uD83D\uDC66\uD83C\uDFFD",
    "\uD83D\uDC66\uD83C\uDFFE",
    "\uD83D\uDC66\uD83C\uDFFF",
    "\uD83D\uDC66",
    "\uD83D\uDC67\uD83C\uDFFB",
    "\uD83D\uDC67\uD83C\uDFFC",
    "\uD83D\uDC67\uD83C\uDFFD",
    "\uD83D\uDC67\uD83C\uDFFE",
    "\uD83D\uDC67\uD83C\uDFFF",
    "\uD83D\uDC67",
    "\uD83D\uDC68\uD83C\uDFFB‍\uD83C\uDF3E",
    "\uD83D\uDC68\uD83C\uDFFB‍\uD83C\uDF73",
    "\uD83D\uDC68\uD83C\uDFFB‍\uD83C\uDF93",
    "\uD83D\uDC68\uD83C\uDFFB‍\uD83C\uDFA4",
    "\uD83D\uDC68\uD83C\uDFFB‍\uD83C\uDFA8",
    "\uD83D\uDC68\uD83C\uDFFB‍\uD83C\uDFEB",
    "\uD83D\uDC68\uD83C\uDFFB‍\uD83C\uDFED",
    "\uD83D\uDC68\uD83C\uDFFB‍\uD83D\uDCBB",
    "\uD83D\uDC68\uD83C\uDFFB‍\uD83D\uDCBC",
    "\uD83D\uDC68\uD83C\uDFFB‍\uD83D\uDD27",
    "\uD83D\uDC68\uD83C\uDFFB‍\uD83D\uDD2C",
    "\uD83D\uDC68\uD83C\uDFFB‍\uD83D\uDE80",
    "\uD83D\uDC68\uD83C\uDFFB‍\uD83D\uDE92",
    "\uD83D\uDC68\uD83C\uDFFB‍\uD83E\uDDAF",
    "\uD83D\uDC68\uD83C\uDFFB‍\uD83E\uDDB0",
    "\uD83D\uDC68\uD83C\uDFFB‍\uD83E\uDDB1",
    "\uD83D\uDC68\uD83C\uDFFB‍\uD83E\uDDB2",
    "\uD83D\uDC68\uD83C\uDFFB‍\uD83E\uDDB3",
    "\uD83D\uDC68\uD83C\uDFFB‍\uD83E\uDDBC",
    "\uD83D\uDC68\uD83C\uDFFB‍\uD83E\uDDBD",
    "\uD83D\uDC68\uD83C\uDFFB‍⚕️",
    "\uD83D\uDC68\uD83C\uDFFB‍⚖️",
    "\uD83D\uDC68\uD83C\uDFFB‍✈️",
    "\uD83D\uDC68\uD83C\uDFFB",
    "\uD83D\uDC68\uD83C\uDFFC‍\uD83C\uDF3E",
    "\uD83D\uDC68\uD83C\uDFFC‍\uD83C\uDF73",
    "\uD83D\uDC68\uD83C\uDFFC‍\uD83C\uDF93",
    "\uD83D\uDC68\uD83C\uDFFC‍\uD83C\uDFA4",
    "\uD83D\uDC68\uD83C\uDFFC‍\uD83C\uDFA8",
    "\uD83D\uDC68\uD83C\uDFFC‍\uD83C\uDFEB",
    "\uD83D\uDC68\uD83C\uDFFC‍\uD83C\uDFED",
    "\uD83D\uDC68\uD83C\uDFFC‍\uD83D\uDCBB",
    "\uD83D\uDC68\uD83C\uDFFC‍\uD83D\uDCBC",
    "\uD83D\uDC68\uD83C\uDFFC‍\uD83D\uDD27",
    "\uD83D\uDC68\uD83C\uDFFC‍\uD83D\uDD2C",
    "\uD83D\uDC68\uD83C\uDFFC‍\uD83D\uDE80",
    "\uD83D\uDC68\uD83C\uDFFC‍\uD83D\uDE92",
    "\uD83D\uDC68\uD83C\uDFFC‍\uD83E\uDD1D‍\uD83D\uDC68\uD83C\uDFFB",
    "\uD83D\uDC68\uD83C\uDFFC‍\uD83E\uDDAF",
    "\uD83D\uDC68\uD83C\uDFFC‍\uD83E\uDDB0",
    "\uD83D\uDC68\uD83C\uDFFC‍\uD83E\uDDB1",
    "\uD83D\uDC68\uD83C\uDFFC‍\uD83E\uDDB2",
    "\uD83D\uDC68\uD83C\uDFFC‍\uD83E\uDDB3",
    "\uD83D\uDC68\uD83C\uDFFC‍\uD83E\uDDBC",
    "\uD83D\uDC68\uD83C\uDFFC‍\uD83E\uDDBD",
    "\uD83D\uDC68\uD83C\uDFFC‍⚕️",
    "\uD83D\uDC68\uD83C\uDFFC‍⚖️",
    "\uD83D\uDC68\uD83C\uDFFC‍✈️",
    "\uD83D\uDC68\uD83C\uDFFC",
    "\uD83D\uDC68\uD83C\uDFFD‍\uD83C\uDF3E",
    "\uD83D\uDC68\uD83C\uDFFD‍\uD83C\uDF73",
    "\uD83D\uDC68\uD83C\uDFFD‍\uD83C\uDF93",
    "\uD83D\uDC68\uD83C\uDFFD‍\uD83C\uDFA4",
    "\uD83D\uDC68\uD83C\uDFFD‍\uD83C\uDFA8",
    "\uD83D\uDC68\uD83C\uDFFD‍\uD83C\uDFEB",
    "\uD83D\uDC68\uD83C\uDFFD‍\uD83C\uDFED",
    "\uD83D\uDC68\uD83C\uDFFD‍\uD83D\uDCBB",
    "\uD83D\uDC68\uD83C\uDFFD‍\uD83D\uDCBC",
    "\uD83D\uDC68\uD83C\uDFFD‍\uD83D\uDD27",
    "\uD83D\uDC68\uD83C\uDFFD‍\uD83D\uDD2C",
    "\uD83D\uDC68\uD83C\uDFFD‍\uD83D\uDE80",
    "\uD83D\uDC68\uD83C\uDFFD‍\uD83D\uDE92",
    "\uD83D\uDC68\uD83C\uDFFD‍\uD83E\uDD1D‍\uD83D\uDC68\uD83C\uDFFB",
    "\uD83D\uDC68\uD83C\uDFFD‍\uD83E\uDD1D‍\uD83D\uDC68\uD83C\uDFFC",
    "\uD83D\uDC68\uD83C\uDFFD‍\uD83E\uDDAF",
    "\uD83D\uDC68\uD83C\uDFFD‍\uD83E\uDDB0",
    "\uD83D\uDC68\uD83C\uDFFD‍\uD83E\uDDB1",
    "\uD83D\uDC68\uD83C\uDFFD‍\uD83E\uDDB2",
    "\uD83D\uDC68\uD83C\uDFFD‍\uD83E\uDDB3",
    "\uD83D\uDC68\uD83C\uDFFD‍\uD83E\uDDBC",
    "\uD83D\uDC68\uD83C\uDFFD‍\uD83E\uDDBD",
    "\uD83D\uDC68\uD83C\uDFFD‍⚕️",
    "\uD83D\uDC68\uD83C\uDFFD‍⚖️",
    "\uD83D\uDC68\uD83C\uDFFD‍✈️",
    "\uD83D\uDC68\uD83C\uDFFD",
    "\uD83D\uDC68\uD83C\uDFFE‍\uD83C\uDF3E",
    "\uD83D\uDC68\uD83C\uDFFE‍\uD83C\uDF73",
    "\uD83D\uDC68\uD83C\uDFFE‍\uD83C\uDF93",
    "\uD83D\uDC68\uD83C\uDFFE‍\uD83C\uDFA4",
    "\uD83D\uDC68\uD83C\uDFFE‍\uD83C\uDFA8",
    "\uD83D\uDC68\uD83C\uDFFE‍\uD83C\uDFEB",
    "\uD83D\uDC68\uD83C\uDFFE‍\uD83C\uDFED",
    "\uD83D\uDC68\uD83C\uDFFE‍\uD83D\uDCBB",
    "\uD83D\uDC68\uD83C\uDFFE‍\uD83D\uDCBC",
    "\uD83D\uDC68\uD83C\uDFFE‍\uD83D\uDD27",
    "\uD83D\uDC68\uD83C\uDFFE‍\uD83D\uDD2C",
    "\uD83D\uDC68\uD83C\uDFFE‍\uD83D\uDE80",
    "\uD83D\uDC68\uD83C\uDFFE‍\uD83D\uDE92",
    "\uD83D\uDC68\uD83C\uDFFE‍\uD83E\uDD1D‍\uD83D\uDC68\uD83C\uDFFB",
    "\uD83D\uDC68\uD83C\uDFFE‍\uD83E\uDD1D‍\uD83D\uDC68\uD83C\uDFFC",
    "\uD83D\uDC68\uD83C\uDFFE‍\uD83E\uDD1D‍\uD83D\uDC68\uD83C\uDFFD",
    "\uD83D\uDC68\uD83C\uDFFE‍\uD83E\uDDAF",
    "\uD83D\uDC68\uD83C\uDFFE‍\uD83E\uDDB0",
    "\uD83D\uDC68\uD83C\uDFFE‍\uD83E\uDDB1",
    "\uD83D\uDC68\uD83C\uDFFE‍\uD83E\uDDB2",
    "\uD83D\uDC68\uD83C\uDFFE‍\uD83E\uDDB3",
    "\uD83D\uDC68\uD83C\uDFFE‍\uD83E\uDDBC",
    "\uD83D\uDC68\uD83C\uDFFE‍\uD83E\uDDBD",
    "\uD83D\uDC68\uD83C\uDFFE‍⚕️",
    "\uD83D\uDC68\uD83C\uDFFE‍⚖️",
    "\uD83D\uDC68\uD83C\uDFFE‍✈️",
    "\uD83D\uDC68\uD83C\uDFFE",
    "\uD83D\uDC68\uD83C\uDFFF‍\uD83C\uDF3E",
    "\uD83D\uDC68\uD83C\uDFFF‍\uD83C\uDF73",
    "\uD83D\uDC68\uD83C\uDFFF‍\uD83C\uDF93",
    "\uD83D\uDC68\uD83C\uDFFF‍\uD83C\uDFA4",
    "\uD83D\uDC68\uD83C\uDFFF‍\uD83C\uDFA8",
    "\uD83D\uDC68\uD83C\uDFFF‍\uD83C\uDFEB",
    "\uD83D\uDC68\uD83C\uDFFF‍\uD83C\uDFED",
    "\uD83D\uDC68\uD83C\uDFFF‍\uD83D\uDCBB",
    "\uD83D\uDC68\uD83C\uDFFF‍\uD83D\uDCBC",
    "\uD83D\uDC68\uD83C\uDFFF‍\uD83D\uDD27",
    "\uD83D\uDC68\uD83C\uDFFF‍\uD83D\uDD2C",
    "\uD83D\uDC68\uD83C\uDFFF‍\uD83D\uDE80",
    "\uD83D\uDC68\uD83C\uDFFF‍\uD83D\uDE92",
    "\uD83D\uDC68\uD83C\uDFFF‍\uD83E\uDD1D‍\uD83D\uDC68\uD83C\uDFFB",
    "\uD83D\uDC68\uD83C\uDFFF‍\uD83E\uDD1D‍\uD83D\uDC68\uD83C\uDFFC",
    "\uD83D\uDC68\uD83C\uDFFF‍\uD83E\uDD1D‍\uD83D\uDC68\uD83C\uDFFD",
    "\uD83D\uDC68\uD83C\uDFFF‍\uD83E\uDD1D‍\uD83D\uDC68\uD83C\uDFFE",
    "\uD83D\uDC68\uD83C\uDFFF‍\uD83E\uDDAF",
    "\uD83D\uDC68\uD83C\uDFFF‍\uD83E\uDDB0",
    "\uD83D\uDC68\uD83C\uDFFF‍\uD83E\uDDB1",
    "\uD83D\uDC68\uD83C\uDFFF‍\uD83E\uDDB2",
    "\uD83D\uDC68\uD83C\uDFFF‍\uD83E\uDDB3",
    "\uD83D\uDC68\uD83C\uDFFF‍\uD83E\uDDBC",
    "\uD83D\uDC68\uD83C\uDFFF‍\uD83E\uDDBD",
    "\uD83D\uDC68\uD83C\uDFFF‍⚕️",
    "\uD83D\uDC68\uD83C\uDFFF‍⚖️",
    "\uD83D\uDC68\uD83C\uDFFF‍✈️",
    "\uD83D\uDC68\uD83C\uDFFF",
    "\uD83D\uDC68‍\uD83C\uDF3E",
    "\uD83D\uDC68‍\uD83C\uDF73",
    "\uD83D\uDC68‍\uD83C\uDF93",
    "\uD83D\uDC68‍\uD83C\uDFA4",
    "\uD83D\uDC68‍\uD83C\uDFA8",
    "\uD83D\uDC68‍\uD83C\uDFEB",
    "\uD83D\uDC68‍\uD83C\uDFED",
    "\uD83D\uDC68‍\uD83D\uDC66‍\uD83D\uDC66",
    "\uD83D\uDC68‍\uD83D\uDC66",
    "\uD83D\uDC68‍\uD83D\uDC67‍\uD83D\uDC66",
    "\uD83D\uDC68‍\uD83D\uDC67‍\uD83D\uDC67",
    "\uD83D\uDC68‍\uD83D\uDC67",
    "\uD83D\uDC68‍\uD83D\uDC68‍\uD83D\uDC66‍\uD83D\uDC66",
    "\uD83D\uDC68‍\uD83D\uDC68‍\uD83D\uDC66",
    "\uD83D\uDC68‍\uD83D\uDC68‍\uD83D\uDC67‍\uD83D\uDC66",
    "\uD83D\uDC68‍\uD83D\uDC68‍\uD83D\uDC67‍\uD83D\uDC67",
    "\uD83D\uDC68‍\uD83D\uDC68‍\uD83D\uDC67",
    "\uD83D\uDC68‍\uD83D\uDC69‍\uD83D\uDC66‍\uD83D\uDC66",
    "\uD83D\uDC68‍\uD83D\uDC69‍\uD83D\uDC66",
    "\uD83D\uDC68‍\uD83D\uDC69‍\uD83D\uDC67‍\uD83D\uDC66",
    "\uD83D\uDC68‍\uD83D\uDC69‍\uD83D\uDC67‍\uD83D\uDC67",
    "\uD83D\uDC68‍\uD83D\uDC69‍\uD83D\uDC67",
    "\uD83D\uDC68‍\uD83D\uDCBB",
    "\uD83D\uDC68‍\uD83D\uDCBC",
    "\uD83D\uDC68‍\uD83D\uDD27",
    "\uD83D\uDC68‍\uD83D\uDD2C",
    "\uD83D\uDC68‍\uD83D\uDE80",
    "\uD83D\uDC68‍\uD83D\uDE92",
    "\uD83D\uDC68‍\uD83E\uDDAF",
    "\uD83D\uDC68‍\uD83E\uDDB0",
    "\uD83D\uDC68‍\uD83E\uDDB1",
    "\uD83D\uDC68‍\uD83E\uDDB2",
    "\uD83D\uDC68‍\uD83E\uDDB3",
    "\uD83D\uDC68‍\uD83E\uDDBC",
    "\uD83D\uDC68‍\uD83E\uDDBD",
    "\uD83D\uDC68‍⚕️",
    "\uD83D\uDC68‍⚖️",
    "\uD83D\uDC68‍✈️",
    "\uD83D\uDC68‍❤️‍\uD83D\uDC68",
    "\uD83D\uDC68‍❤️‍\uD83D\uDC8B‍\uD83D\uDC68",
    "\uD83D\uDC68",
    "\uD83D\uDC69\uD83C\uDFFB‍\uD83C\uDF3E",
    "\uD83D\uDC69\uD83C\uDFFB‍\uD83C\uDF73",
    "\uD83D\uDC69\uD83C\uDFFB‍\uD83C\uDF93",
    "\uD83D\uDC69\uD83C\uDFFB‍\uD83C\uDFA4",
    "\uD83D\uDC69\uD83C\uDFFB‍\uD83C\uDFA8",
    "\uD83D\uDC69\uD83C\uDFFB‍\uD83C\uDFEB",
    "\uD83D\uDC69\uD83C\uDFFB‍\uD83C\uDFED",
    "\uD83D\uDC69\uD83C\uDFFB‍\uD83D\uDCBB",
    "\uD83D\uDC69\uD83C\uDFFB‍\uD83D\uDCBC",
    "\uD83D\uDC69\uD83C\uDFFB‍\uD83D\uDD27",
    "\uD83D\uDC69\uD83C\uDFFB‍\uD83D\uDD2C",
    "\uD83D\uDC69\uD83C\uDFFB‍\uD83D\uDE80",
    "\uD83D\uDC69\uD83C\uDFFB‍\uD83D\uDE92",
    "\uD83D\uDC69\uD83C\uDFFB‍\uD83E\uDD1D‍\uD83D\uDC68\uD83C\uDFFC",
    "\uD83D\uDC69\uD83C\uDFFB‍\uD83E\uDD1D‍\uD83D\uDC68\uD83C\uDFFD",
    "\uD83D\uDC69\uD83C\uDFFB‍\uD83E\uDD1D‍\uD83D\uDC68\uD83C\uDFFE",
    "\uD83D\uDC69\uD83C\uDFFB‍\uD83E\uDD1D‍\uD83D\uDC68\uD83C\uDFFF",
    "\uD83D\uDC69\uD83C\uDFFB‍\uD83E\uDDAF",
    "\uD83D\uDC69\uD83C\uDFFB‍\uD83E\uDDB0",
    "\uD83D\uDC69\uD83C\uDFFB‍\uD83E\uDDB1",
    "\uD83D\uDC69\uD83C\uDFFB‍\uD83E\uDDB2",
    "\uD83D\uDC69\uD83C\uDFFB‍\uD83E\uDDB3",
    "\uD83D\uDC69\uD83C\uDFFB‍\uD83E\uDDBC",
    "\uD83D\uDC69\uD83C\uDFFB‍\uD83E\uDDBD",
    "\uD83D\uDC69\uD83C\uDFFB‍⚕️",
    "\uD83D\uDC69\uD83C\uDFFB‍⚖️",
    "\uD83D\uDC69\uD83C\uDFFB‍✈️",
    "\uD83D\uDC69\uD83C\uDFFB",
    "\uD83D\uDC69\uD83C\uDFFC‍\uD83C\uDF3E",
    "\uD83D\uDC69\uD83C\uDFFC‍\uD83C\uDF73",
    "\uD83D\uDC69\uD83C\uDFFC‍\uD83C\uDF93",
    "\uD83D\uDC69\uD83C\uDFFC‍\uD83C\uDFA4",
    "\uD83D\uDC69\uD83C\uDFFC‍\uD83C\uDFA8",
    "\uD83D\uDC69\uD83C\uDFFC‍\uD83C\uDFEB",
    "\uD83D\uDC69\uD83C\uDFFC‍\uD83C\uDFED",
    "\uD83D\uDC69\uD83C\uDFFC‍\uD83D\uDCBB",
    "\uD83D\uDC69\uD83C\uDFFC‍\uD83D\uDCBC",
    "\uD83D\uDC69\uD83C\uDFFC‍\uD83D\uDD27",
    "\uD83D\uDC69\uD83C\uDFFC‍\uD83D\uDD2C",
    "\uD83D\uDC69\uD83C\uDFFC‍\uD83D\uDE80",
    "\uD83D\uDC69\uD83C\uDFFC‍\uD83D\uDE92",
    "\uD83D\uDC69\uD83C\uDFFC‍\uD83E\uDD1D‍\uD83D\uDC68\uD83C\uDFFB",
    "\uD83D\uDC69\uD83C\uDFFC‍\uD83E\uDD1D‍\uD83D\uDC68\uD83C\uDFFD",
    "\uD83D\uDC69\uD83C\uDFFC‍\uD83E\uDD1D‍\uD83D\uDC68\uD83C\uDFFE",
    "\uD83D\uDC69\uD83C\uDFFC‍\uD83E\uDD1D‍\uD83D\uDC68\uD83C\uDFFF",
    "\uD83D\uDC69\uD83C\uDFFC‍\uD83E\uDD1D‍\uD83D\uDC69\uD83C\uDFFB",
    "\uD83D\uDC69\uD83C\uDFFC‍\uD83E\uDDAF",
    "\uD83D\uDC69\uD83C\uDFFC‍\uD83E\uDDB0",
    "\uD83D\uDC69\uD83C\uDFFC‍\uD83E\uDDB1",
    "\uD83D\uDC69\uD83C\uDFFC‍\uD83E\uDDB2",
    "\uD83D\uDC69\uD83C\uDFFC‍\uD83E\uDDB3",
    "\uD83D\uDC69\uD83C\uDFFC‍\uD83E\uDDBC",
    "\uD83D\uDC69\uD83C\uDFFC‍\uD83E\uDDBD",
    "\uD83D\uDC69\uD83C\uDFFC‍⚕️",
    "\uD83D\uDC69\uD83C\uDFFC‍⚖️",
    "\uD83D\uDC69\uD83C\uDFFC‍✈️",
    "\uD83D\uDC69\uD83C\uDFFC",
    "\uD83D\uDC69\uD83C\uDFFD‍\uD83C\uDF3E",
    "\uD83D\uDC69\uD83C\uDFFD‍\uD83C\uDF73",
    "\uD83D\uDC69\uD83C\uDFFD‍\uD83C\uDF93",
    "\uD83D\uDC69\uD83C\uDFFD‍\uD83C\uDFA4",
    "\uD83D\uDC69\uD83C\uDFFD‍\uD83C\uDFA8",
    "\uD83D\uDC69\uD83C\uDFFD‍\uD83C\uDFEB",
    "\uD83D\uDC69\uD83C\uDFFD‍\uD83C\uDFED",
    "\uD83D\uDC69\uD83C\uDFFD‍\uD83D\uDCBB",
    "\uD83D\uDC69\uD83C\uDFFD‍\uD83D\uDCBC",
    "\uD83D\uDC69\uD83C\uDFFD‍\uD83D\uDD27",
    "\uD83D\uDC69\uD83C\uDFFD‍\uD83D\uDD2C",
    "\uD83D\uDC69\uD83C\uDFFD‍\uD83D\uDE80",
    "\uD83D\uDC69\uD83C\uDFFD‍\uD83D\uDE92",
    "\uD83D\uDC69\uD83C\uDFFD‍\uD83E\uDD1D‍\uD83D\uDC68\uD83C\uDFFB",
    "\uD83D\uDC69\uD83C\uDFFD‍\uD83E\uDD1D‍\uD83D\uDC68\uD83C\uDFFC",
    "\uD83D\uDC69\uD83C\uDFFD‍\uD83E\uDD1D‍\uD83D\uDC68\uD83C\uDFFE",
    "\uD83D\uDC69\uD83C\uDFFD‍\uD83E\uDD1D‍\uD83D\uDC68\uD83C\uDFFF",
    "\uD83D\uDC69\uD83C\uDFFD‍\uD83E\uDD1D‍\uD83D\uDC69\uD83C\uDFFB",
    "\uD83D\uDC69\uD83C\uDFFD‍\uD83E\uDD1D‍\uD83D\uDC69\uD83C\uDFFC",
    "\uD83D\uDC69\uD83C\uDFFD‍\uD83E\uDDAF",
    "\uD83D\uDC69\uD83C\uDFFD‍\uD83E\uDDB0",
    "\uD83D\uDC69\uD83C\uDFFD‍\uD83E\uDDB1",
    "\uD83D\uDC69\uD83C\uDFFD‍\uD83E\uDDB2",
    "\uD83D\uDC69\uD83C\uDFFD‍\uD83E\uDDB3",
    "\uD83D\uDC69\uD83C\uDFFD‍\uD83E\uDDBC",
    "\uD83D\uDC69\uD83C\uDFFD‍\uD83E\uDDBD",
    "\uD83D\uDC69\uD83C\uDFFD‍⚕️",
    "\uD83D\uDC69\uD83C\uDFFD‍⚖️",
    "\uD83D\uDC69\uD83C\uDFFD‍✈️",
    "\uD83D\uDC69\uD83C\uDFFD",
    "\uD83D\uDC69\uD83C\uDFFE‍\uD83C\uDF3E",
    "\uD83D\uDC69\uD83C\uDFFE‍\uD83C\uDF73",
    "\uD83D\uDC69\uD83C\uDFFE‍\uD83C\uDF93",
    "\uD83D\uDC69\uD83C\uDFFE‍\uD83C\uDFA4",
    "\uD83D\uDC69\uD83C\uDFFE‍\uD83C\uDFA8",
    "\uD83D\uDC69\uD83C\uDFFE‍\uD83C\uDFEB",
    "\uD83D\uDC69\uD83C\uDFFE‍\uD83C\uDFED",
    "\uD83D\uDC69\uD83C\uDFFE‍\uD83D\uDCBB",
    "\uD83D\uDC69\uD83C\uDFFE‍\uD83D\uDCBC",
    "\uD83D\uDC69\uD83C\uDFFE‍\uD83D\uDD27",
    "\uD83D\uDC69\uD83C\uDFFE‍\uD83D\uDD2C",
    "\uD83D\uDC69\uD83C\uDFFE‍\uD83D\uDE80",
    "\uD83D\uDC69\uD83C\uDFFE‍\uD83D\uDE92",
    "\uD83D\uDC69\uD83C\uDFFE‍\uD83E\uDD1D‍\uD83D\uDC68\uD83C\uDFFB",
    "\uD83D\uDC69\uD83C\uDFFE‍\uD83E\uDD1D‍\uD83D\uDC68\uD83C\uDFFC",
    "\uD83D\uDC69\uD83C\uDFFE‍\uD83E\uDD1D‍\uD83D\uDC68\uD83C\uDFFD",
    "\uD83D\uDC69\uD83C\uDFFE‍\uD83E\uDD1D‍\uD83D\uDC68\uD83C\uDFFF",
    "\uD83D\uDC69\uD83C\uDFFE‍\uD83E\uDD1D‍\uD83D\uDC69\uD83C\uDFFB",
    "\uD83D\uDC69\uD83C\uDFFE‍\uD83E\uDD1D‍\uD83D\uDC69\uD83C\uDFFC",
    "\uD83D\uDC69\uD83C\uDFFE‍\uD83E\uDD1D‍\uD83D\uDC69\uD83C\uDFFD",
    "\uD83D\uDC69\uD83C\uDFFE‍\uD83E\uDDAF",
    "\uD83D\uDC69\uD83C\uDFFE‍\uD83E\uDDB0",
    "\uD83D\uDC69\uD83C\uDFFE‍\uD83E\uDDB1",
    "\uD83D\uDC69\uD83C\uDFFE‍\uD83E\uDDB2",
    "\uD83D\uDC69\uD83C\uDFFE‍\uD83E\uDDB3",
    "\uD83D\uDC69\uD83C\uDFFE‍\uD83E\uDDBC",
    "\uD83D\uDC69\uD83C\uDFFE‍\uD83E\uDDBD",
    "\uD83D\uDC69\uD83C\uDFFE‍⚕️",
    "\uD83D\uDC69\uD83C\uDFFE‍⚖️",
    "\uD83D\uDC69\uD83C\uDFFE‍✈️",
    "\uD83D\uDC69\uD83C\uDFFE",
    "\uD83D\uDC69\uD83C\uDFFF‍\uD83C\uDF3E",
    "\uD83D\uDC69\uD83C\uDFFF‍\uD83C\uDF73",
    "\uD83D\uDC69\uD83C\uDFFF‍\uD83C\uDF93",
    "\uD83D\uDC69\uD83C\uDFFF‍\uD83C\uDFA4",
    "\uD83D\uDC69\uD83C\uDFFF‍\uD83C\uDFA8",
    "\uD83D\uDC69\uD83C\uDFFF‍\uD83C\uDFEB",
    "\uD83D\uDC69\uD83C\uDFFF‍\uD83C\uDFED",
    "\uD83D\uDC69\uD83C\uDFFF‍\uD83D\uDCBB",
    "\uD83D\uDC69\uD83C\uDFFF‍\uD83D\uDCBC",
    "\uD83D\uDC69\uD83C\uDFFF‍\uD83D\uDD27",
    "\uD83D\uDC69\uD83C\uDFFF‍\uD83D\uDD2C",
    "\uD83D\uDC69\uD83C\uDFFF‍\uD83D\uDE80",
    "\uD83D\uDC69\uD83C\uDFFF‍\uD83D\uDE92",
    "\uD83D\uDC69\uD83C\uDFFF‍\uD83E\uDD1D‍\uD83D\uDC68\uD83C\uDFFB",
    "\uD83D\uDC69\uD83C\uDFFF‍\uD83E\uDD1D‍\uD83D\uDC68\uD83C\uDFFC",
    "\uD83D\uDC69\uD83C\uDFFF‍\uD83E\uDD1D‍\uD83D\uDC68\uD83C\uDFFD",
    "\uD83D\uDC69\uD83C\uDFFF‍\uD83E\uDD1D‍\uD83D\uDC68\uD83C\uDFFE",
    "\uD83D\uDC69\uD83C\uDFFF‍\uD83E\uDD1D‍\uD83D\uDC69\uD83C\uDFFB",
    "\uD83D\uDC69\uD83C\uDFFF‍\uD83E\uDD1D‍\uD83D\uDC69\uD83C\uDFFC",
    "\uD83D\uDC69\uD83C\uDFFF‍\uD83E\uDD1D‍\uD83D\uDC69\uD83C\uDFFD",
    "\uD83D\uDC69\uD83C\uDFFF‍\uD83E\uDD1D‍\uD83D\uDC69\uD83C\uDFFE",
    "\uD83D\uDC69\uD83C\uDFFF‍\uD83E\uDDAF",
    "\uD83D\uDC69\uD83C\uDFFF‍\uD83E\uDDB0",
    "\uD83D\uDC69\uD83C\uDFFF‍\uD83E\uDDB1",
    "\uD83D\uDC69\uD83C\uDFFF‍\uD83E\uDDB2",
    "\uD83D\uDC69\uD83C\uDFFF‍\uD83E\uDDB3",
    "\uD83D\uDC69\uD83C\uDFFF‍\uD83E\uDDBC",
    "\uD83D\uDC69\uD83C\uDFFF‍\uD83E\uDDBD",
    "\uD83D\uDC69\uD83C\uDFFF‍⚕️",
    "\uD83D\uDC69\uD83C\uDFFF‍⚖️",
    "\uD83D\uDC69\uD83C\uDFFF‍✈️",
    "\uD83D\uDC69\uD83C\uDFFF",
    "\uD83D\uDC69‍\uD83C\uDF3E",
    "\uD83D\uDC69‍\uD83C\uDF73",
    "\uD83D\uDC69‍\uD83C\uDF93",
    "\uD83D\uDC69‍\uD83C\uDFA4",
    "\uD83D\uDC69‍\uD83C\uDFA8",
    "\uD83D\uDC69‍\uD83C\uDFEB",
    "\uD83D\uDC69‍\uD83C\uDFED",
    "\uD83D\uDC69‍\uD83D\uDC66‍\uD83D\uDC66",
    "\uD83D\uDC69‍\uD83D\uDC66",
    "\uD83D\uDC69‍\uD83D\uDC67‍\uD83D\uDC66",
    "\uD83D\uDC69‍\uD83D\uDC67‍\uD83D\uDC67",
    "\uD83D\uDC69‍\uD83D\uDC67",
    "\uD83D\uDC69‍\uD83D\uDC69‍\uD83D\uDC66‍\uD83D\uDC66",
    "\uD83D\uDC69‍\uD83D\uDC69‍\uD83D\uDC66",
    "\uD83D\uDC69‍\uD83D\uDC69‍\uD83D\uDC67‍\uD83D\uDC66",
    "\uD83D\uDC69‍\uD83D\uDC69‍\uD83D\uDC67‍\uD83D\uDC67",
    "\uD83D\uDC69‍\uD83D\uDC69‍\uD83D\uDC67",
    "\uD83D\uDC69‍\uD83D\uDCBB",
    "\uD83D\uDC69‍\uD83D\uDCBC",
    "\uD83D\uDC69‍\uD83D\uDD27",
    "\uD83D\uDC69‍\uD83D\uDD2C",
    "\uD83D\uDC69‍\uD83D\uDE80",
    "\uD83D\uDC69‍\uD83D\uDE92",
    "\uD83D\uDC69‍\uD83E\uDDAF",
    "\uD83D\uDC69‍\uD83E\uDDB0",
    "\uD83D\uDC69‍\uD83E\uDDB1",
    "\uD83D\uDC69‍\uD83E\uDDB2",
    "\uD83D\uDC69‍\uD83E\uDDB3",
    "\uD83D\uDC69‍\uD83E\uDDBC",
    "\uD83D\uDC69‍\uD83E\uDDBD",
    "\uD83D\uDC69‍⚕️",
    "\uD83D\uDC69‍⚖️",
    "\uD83D\uDC69‍✈️",
    "\uD83D\uDC69‍❤️‍\uD83D\uDC68",
    "\uD83D\uDC69‍❤️‍\uD83D\uDC69",
    "\uD83D\uDC69‍❤️‍\uD83D\uDC8B‍\uD83D\uDC68",
    "\uD83D\uDC69‍❤️‍\uD83D\uDC8B‍\uD83D\uDC69",
    "\uD83D\uDC69",
    "\uD83D\uDC6A",
    "\uD83D\uDC6B\uD83C\uDFFB",
    "\uD83D\uDC6B\uD83C\uDFFC",
    "\uD83D\uDC6B\uD83C\uDFFD",
    "\uD83D\uDC6B\uD83C\uDFFE",
    "\uD83D\uDC6B\uD83C\uDFFF",
    "\uD83D\uDC6B",
    "\uD83D\uDC6C\uD83C\uDFFB",
    "\uD83D\uDC6C\uD83C\uDFFC",
    "\uD83D\uDC6C\uD83C\uDFFD",
    "\uD83D\uDC6C\uD83C\uDFFE",
    "\uD83D\uDC6C\uD83C\uDFFF",
    "\uD83D\uDC6C",
    "\uD83D\uDC6D\uD83C\uDFFB",
    "\uD83D\uDC6D\uD83C\uDFFC",
    "\uD83D\uDC6D\uD83C\uDFFD",
    "\uD83D\uDC6D\uD83C\uDFFE",
    "\uD83D\uDC6D\uD83C\uDFFF",
    "\uD83D\uDC6D",
    "\uD83D\uDC6E\uD83C\uDFFB‍♀️",
    "\uD83D\uDC6E\uD83C\uDFFB‍♂️",
    "\uD83D\uDC6E\uD83C\uDFFB",
    "\uD83D\uDC6E\uD83C\uDFFC‍♀️",
    "\uD83D\uDC6E\uD83C\uDFFC‍♂️",
    "\uD83D\uDC6E\uD83C\uDFFC",
    "\uD83D\uDC6E\uD83C\uDFFD‍♀️",
    "\uD83D\uDC6E\uD83C\uDFFD‍♂️",
    "\uD83D\uDC6E\uD83C\uDFFD",
    "\uD83D\uDC6E\uD83C\uDFFE‍♀️",
    "\uD83D\uDC6E\uD83C\uDFFE‍♂️",
    "\uD83D\uDC6E\uD83C\uDFFE",
    "\uD83D\uDC6E\uD83C\uDFFF‍♀️",
    "\uD83D\uDC6E\uD83C\uDFFF‍♂️",
    "\uD83D\uDC6E\uD83C\uDFFF",
    "\uD83D\uDC6E‍♀️",
    "\uD83D\uDC6E‍♂️",
    "\uD83D\uDC6E",
    "\uD83D\uDC6F‍♀️",
    "\uD83D\uDC6F‍♂️",
    "\uD83D\uDC6F",
    "\uD83D\uDC70\uD83C\uDFFB",
    "\uD83D\uDC70\uD83C\uDFFC",
    "\uD83D\uDC70\uD83C\uDFFD",
    "\uD83D\uDC70\uD83C\uDFFE",
    "\uD83D\uDC70\uD83C\uDFFF",
    "\uD83D\uDC70",
    "\uD83D\uDC71\uD83C\uDFFB‍♀️",
    "\uD83D\uDC71\uD83C\uDFFB‍♂️",
    "\uD83D\uDC71\uD83C\uDFFB",
    "\uD83D\uDC71\uD83C\uDFFC‍♀️",
    "\uD83D\uDC71\uD83C\uDFFC‍♂️",
    "\uD83D\uDC71\uD83C\uDFFC",
    "\uD83D\uDC71\uD83C\uDFFD‍♀️",
    "\uD83D\uDC71\uD83C\uDFFD‍♂️",
    "\uD83D\uDC71\uD83C\uDFFD",
    "\uD83D\uDC71\uD83C\uDFFE‍♀️",
    "\uD83D\uDC71\uD83C\uDFFE‍♂️",
    "\uD83D\uDC71\uD83C\uDFFE",
    "\uD83D\uDC71\uD83C\uDFFF‍♀️",
    "\uD83D\uDC71\uD83C\uDFFF‍♂️",
    "\uD83D\uDC71\uD83C\uDFFF",
    "\uD83D\uDC71‍♀️",
    "\uD83D\uDC71‍♂️",
    "\uD83D\uDC71",
    "\uD83D\uDC72\uD83C\uDFFB",
    "\uD83D\uDC72\uD83C\uDFFC",
    "\uD83D\uDC72\uD83C\uDFFD",
    "\uD83D\uDC72\uD83C\uDFFE",
    "\uD83D\uDC72\uD83C\uDFFF",
    "\uD83D\uDC72",
    "\uD83D\uDC73\uD83C\uDFFB‍♀️",
    "\uD83D\uDC73\uD83C\uDFFB‍♂️",
    "\uD83D\uDC73\uD83C\uDFFB",
    "\uD83D\uDC73\uD83C\uDFFC‍♀️",
    "\uD83D\uDC73\uD83C\uDFFC‍♂️",
    "\uD83D\uDC73\uD83C\uDFFC",
    "\uD83D\uDC73\uD83C\uDFFD‍♀️",
    "\uD83D\uDC73\uD83C\uDFFD‍♂️",
    "\uD83D\uDC73\uD83C\uDFFD",
    "\uD83D\uDC73\uD83C\uDFFE‍♀️",
    "\uD83D\uDC73\uD83C\uDFFE‍♂️",
    "\uD83D\uDC73\uD83C\uDFFE",
    "\uD83D\uDC73\uD83C\uDFFF‍♀️",
    "\uD83D\uDC73\uD83C\uDFFF‍♂️",
    "\uD83D\uDC73\uD83C\uDFFF",
    "\uD83D\uDC73‍♀️",
    "\uD83D\uDC73‍♂️",
    "\uD83D\uDC73",
    "\uD83D\uDC74\uD83C\uDFFB",
    "\uD83D\uDC74\uD83C\uDFFC",
    "\uD83D\uDC74\uD83C\uDFFD",
    "\uD83D\uDC74\uD83C\uDFFE",
    "\uD83D\uDC74\uD83C\uDFFF",
    "\uD83D\uDC74",
    "\uD83D\uDC75\uD83C\uDFFB",
    "\uD83D\uDC75\uD83C\uDFFC",
    "\uD83D\uDC75\uD83C\uDFFD",
    "\uD83D\uDC75\uD83C\uDFFE",
    "\uD83D\uDC75\uD83C\uDFFF",
    "\uD83D\uDC75",
    "\uD83D\uDC76\uD83C\uDFFB",
    "\uD83D\uDC76\uD83C\uDFFC",
    "\uD83D\uDC76\uD83C\uDFFD",
    "\uD83D\uDC76\uD83C\uDFFE",
    "\uD83D\uDC76\uD83C\uDFFF",
    "\uD83D\uDC76",
    "\uD83D\uDC77\uD83C\uDFFB‍♀️",
    "\uD83D\uDC77\uD83C\uDFFB‍♂️",
    "\uD83D\uDC77\uD83C\uDFFB",
    "\uD83D\uDC77\uD83C\uDFFC‍♀️",
    "\uD83D\uDC77\uD83C\uDFFC‍♂️",
    "\uD83D\uDC77\uD83C\uDFFC",
    "\uD83D\uDC77\uD83C\uDFFD‍♀️",
    "\uD83D\uDC77\uD83C\uDFFD‍♂️",
    "\uD83D\uDC77\uD83C\uDFFD",
    "\uD83D\uDC77\uD83C\uDFFE‍♀️",
    "\uD83D\uDC77\uD83C\uDFFE‍♂️",
    "\uD83D\uDC77\uD83C\uDFFE",
    "\uD83D\uDC77\uD83C\uDFFF‍♀️",
    "\uD83D\uDC77\uD83C\uDFFF‍♂️",
    "\uD83D\uDC77\uD83C\uDFFF",
    "\uD83D\uDC77‍♀️",
    "\uD83D\uDC77‍♂️",
    "\uD83D\uDC77",
    "\uD83D\uDC78\uD83C\uDFFB",
    "\uD83D\uDC78\uD83C\uDFFC",
    "\uD83D\uDC78\uD83C\uDFFD",
    "\uD83D\uDC78\uD83C\uDFFE",
    "\uD83D\uDC78\uD83C\uDFFF",
    "\uD83D\uDC78",
    "\uD83D\uDC79",
    "\uD83D\uDC7A",
    "\uD83D\uDC7B",
    "\uD83D\uDC7C\uD83C\uDFFB",
    "\uD83D\uDC7C\uD83C\uDFFC",
    "\uD83D\uDC7C\uD83C\uDFFD",
    "\uD83D\uDC7C\uD83C\uDFFE",
    "\uD83D\uDC7C\uD83C\uDFFF",
    "\uD83D\uDC7C",
    "\uD83D\uDC7D",
    "\uD83D\uDC7E",
    "\uD83D\uDC7F",
    "\uD83D\uDC80",
    "\uD83D\uDC81\uD83C\uDFFB‍♀️",
    "\uD83D\uDC81\uD83C\uDFFB‍♂️",
    "\uD83D\uDC81\uD83C\uDFFB",
    "\uD83D\uDC81\uD83C\uDFFC‍♀️",
    "\uD83D\uDC81\uD83C\uDFFC‍♂️",
    "\uD83D\uDC81\uD83C\uDFFC",
    "\uD83D\uDC81\uD83C\uDFFD‍♀️",
    "\uD83D\uDC81\uD83C\uDFFD‍♂️",
    "\uD83D\uDC81\uD83C\uDFFD",
    "\uD83D\uDC81\uD83C\uDFFE‍♀️",
    "\uD83D\uDC81\uD83C\uDFFE‍♂️",
    "\uD83D\uDC81\uD83C\uDFFE",
    "\uD83D\uDC81\uD83C\uDFFF‍♀️",
    "\uD83D\uDC81\uD83C\uDFFF‍♂️",
    "\uD83D\uDC81\uD83C\uDFFF",
    "\uD83D\uDC81‍♀️",
    "\uD83D\uDC81‍♂️",
    "\uD83D\uDC81",
    "\uD83D\uDC82\uD83C\uDFFB‍♀️",
    "\uD83D\uDC82\uD83C\uDFFB‍♂️",
    "\uD83D\uDC82\uD83C\uDFFB",
    "\uD83D\uDC82\uD83C\uDFFC‍♀️",
    "\uD83D\uDC82\uD83C\uDFFC‍♂️",
    "\uD83D\uDC82\uD83C\uDFFC",
    "\uD83D\uDC82\uD83C\uDFFD‍♀️",
    "\uD83D\uDC82\uD83C\uDFFD‍♂️",
    "\uD83D\uDC82\uD83C\uDFFD",
    "\uD83D\uDC82\uD83C\uDFFE‍♀️",
    "\uD83D\uDC82\uD83C\uDFFE‍♂️",
    "\uD83D\uDC82\uD83C\uDFFE",
    "\uD83D\uDC82\uD83C\uDFFF‍♀️",
    "\uD83D\uDC82\uD83C\uDFFF‍♂️",
    "\uD83D\uDC82\uD83C\uDFFF",
    "\uD83D\uDC82‍♀️",
    "\uD83D\uDC82‍♂️",
    "\uD83D\uDC82",
    "\uD83D\uDC83\uD83C\uDFFB",
    "\uD83D\uDC83\uD83C\uDFFC",
    "\uD83D\uDC83\uD83C\uDFFD",
    "\uD83D\uDC83\uD83C\uDFFE",
    "\uD83D\uDC83\uD83C\uDFFF",
    "\uD83D\uDC83",
    "\uD83D\uDC84",
    "\uD83D\uDC85\uD83C\uDFFB",
    "\uD83D\uDC85\uD83C\uDFFC",
    "\uD83D\uDC85\uD83C\uDFFD",
    "\uD83D\uDC85\uD83C\uDFFE",
    "\uD83D\uDC85\uD83C\uDFFF",
    "\uD83D\uDC85",
    "\uD83D\uDC86\uD83C\uDFFB‍♀️",
    "\uD83D\uDC86\uD83C\uDFFB‍♂️",
    "\uD83D\uDC86\uD83C\uDFFB",
    "\uD83D\uDC86\uD83C\uDFFC‍♀️",
    "\uD83D\uDC86\uD83C\uDFFC‍♂️",
    "\uD83D\uDC86\uD83C\uDFFC",
    "\uD83D\uDC86\uD83C\uDFFD‍♀️",
    "\uD83D\uDC86\uD83C\uDFFD‍♂️",
    "\uD83D\uDC86\uD83C\uDFFD",
    "\uD83D\uDC86\uD83C\uDFFE‍♀️",
    "\uD83D\uDC86\uD83C\uDFFE‍♂️",
    "\uD83D\uDC86\uD83C\uDFFE",
    "\uD83D\uDC86\uD83C\uDFFF‍♀️",
    "\uD83D\uDC86\uD83C\uDFFF‍♂️",
    "\uD83D\uDC86\uD83C\uDFFF",
    "\uD83D\uDC86‍♀️",
    "\uD83D\uDC86‍♂️",
    "\uD83D\uDC86",
    "\uD83D\uDC87\uD83C\uDFFB‍♀️",
    "\uD83D\uDC87\uD83C\uDFFB‍♂️",
    "\uD83D\uDC87\uD83C\uDFFB",
    "\uD83D\uDC87\uD83C\uDFFC‍♀️",
    "\uD83D\uDC87\uD83C\uDFFC‍♂️",
    "\uD83D\uDC87\uD83C\uDFFC",
    "\uD83D\uDC87\uD83C\uDFFD‍♀️",
    "\uD83D\uDC87\uD83C\uDFFD‍♂️",
    "\uD83D\uDC87\uD83C\uDFFD",
    "\uD83D\uDC87\uD83C\uDFFE‍♀️",
    "\uD83D\uDC87\uD83C\uDFFE‍♂️",
    "\uD83D\uDC87\uD83C\uDFFE",
    "\uD83D\uDC87\uD83C\uDFFF‍♀️",
    "\uD83D\uDC87\uD83C\uDFFF‍♂️",
    "\uD83D\uDC87\uD83C\uDFFF",
    "\uD83D\uDC87‍♀️",
    "\uD83D\uDC87‍♂️",
    "\uD83D\uDC87",
    "\uD83D\uDC88",
    "\uD83D\uDC89",
    "\uD83D\uDC8A",
    "\uD83D\uDC8B",
    "\uD83D\uDC8C",
    "\uD83D\uDC8D",
    "\uD83D\uDC8E",
    "\uD83D\uDC8F",
    "\uD83D\uDC90",
    "\uD83D\uDC91",
    "\uD83D\uDC92",
    "\uD83D\uDC93",
    "\uD83D\uDC94",
    "\uD83D\uDC95",
    "\uD83D\uDC96",
    "\uD83D\uDC97",
    "\uD83D\uDC98",
    "\uD83D\uDC99",
    "\uD83D\uDC9A",
    "\uD83D\uDC9B",
    "\uD83D\uDC9C",
    "\uD83D\uDC9D",
    "\uD83D\uDC9E",
    "\uD83D\uDC9F",
    "\uD83D\uDCA0",
    "\uD83D\uDCA1",
    "\uD83D\uDCA2",
    "\uD83D\uDCA3",
    "\uD83D\uDCA4",
    "\uD83D\uDCA5",
    "\uD83D\uDCA6",
    "\uD83D\uDCA7",
    "\uD83D\uDCA8",
    "\uD83D\uDCA9",
    "\uD83D\uDCAA\uD83C\uDFFB",
    "\uD83D\uDCAA\uD83C\uDFFC",
    "\uD83D\uDCAA\uD83C\uDFFD",
    "\uD83D\uDCAA\uD83C\uDFFE",
    "\uD83D\uDCAA\uD83C\uDFFF",
    "\uD83D\uDCAA",
    "\uD83D\uDCAB",
    "\uD83D\uDCAC",
    "\uD83D\uDCAD",
    "\uD83D\uDCAE",
    "\uD83D\uDCAF",
    "\uD83D\uDCB0",
    "\uD83D\uDCB1",
    "\uD83D\uDCB2",
    "\uD83D\uDCB3",
    "\uD83D\uDCB4",
    "\uD83D\uDCB5",
    "\uD83D\uDCB6",
    "\uD83D\uDCB7",
    "\uD83D\uDCB8",
    "\uD83D\uDCB9",
    "\uD83D\uDCBA",
    "\uD83D\uDCBB",
    "\uD83D\uDCBC",
    "\uD83D\uDCBD",
    "\uD83D\uDCBE",
    "\uD83D\uDCBF",
    "\uD83D\uDCC0",
    "\uD83D\uDCC1",
    "\uD83D\uDCC2",
    "\uD83D\uDCC3",
    "\uD83D\uDCC4",
    "\uD83D\uDCC5",
    "\uD83D\uDCC6",
    "\uD83D\uDCC7",
    "\uD83D\uDCC8",
    "\uD83D\uDCC9",
    "\uD83D\uDCCA",
    "\uD83D\uDCCB",
    "\uD83D\uDCCC",
    "\uD83D\uDCCD",
    "\uD83D\uDCCE",
    "\uD83D\uDCCF",
    "\uD83D\uDCD0",
    "\uD83D\uDCD1",
    "\uD83D\uDCD2",
    "\uD83D\uDCD3",
    "\uD83D\uDCD4",
    "\uD83D\uDCD5",
    "\uD83D\uDCD6",
    "\uD83D\uDCD7",
    "\uD83D\uDCD8",
    "\uD83D\uDCD9",
    "\uD83D\uDCDA",
    "\uD83D\uDCDB",
    "\uD83D\uDCDC",
    "\uD83D\uDCDD",
    "\uD83D\uDCDE",
    "\uD83D\uDCDF",
    "\uD83D\uDCE0",
    "\uD83D\uDCE1",
    "\uD83D\uDCE2",
    "\uD83D\uDCE3",
    "\uD83D\uDCE4",
    "\uD83D\uDCE5",
    "\uD83D\uDCE6",
    "\uD83D\uDCE7",
    "\uD83D\uDCE8",
    "\uD83D\uDCE9",
    "\uD83D\uDCEA",
    "\uD83D\uDCEB",
    "\uD83D\uDCEC",
    "\uD83D\uDCED",
    "\uD83D\uDCEE",
    "\uD83D\uDCEF",
    "\uD83D\uDCF0",
    "\uD83D\uDCF1",
    "\uD83D\uDCF2",
    "\uD83D\uDCF3",
    "\uD83D\uDCF4",
    "\uD83D\uDCF5",
    "\uD83D\uDCF6",
    "\uD83D\uDCF7",
    "\uD83D\uDCF8",
    "\uD83D\uDCF9",
    "\uD83D\uDCFA",
    "\uD83D\uDCFB",
    "\uD83D\uDCFC",
    "\uD83D\uDCFD️",
    "\uD83D\uDCFF",
    "\uD83D\uDD00",
    "\uD83D\uDD01",
    "\uD83D\uDD02",
    "\uD83D\uDD03",
    "\uD83D\uDD04",
    "\uD83D\uDD05",
    "\uD83D\uDD06",
    "\uD83D\uDD07",
    "\uD83D\uDD08",
    "\uD83D\uDD09",
    "\uD83D\uDD0A",
    "\uD83D\uDD0B",
    "\uD83D\uDD0C",
    "\uD83D\uDD0D",
    "\uD83D\uDD0E",
    "\uD83D\uDD0F",
    "\uD83D\uDD10",
    "\uD83D\uDD11",
    "\uD83D\uDD12",
    "\uD83D\uDD13",
    "\uD83D\uDD14",
    "\uD83D\uDD15",
    "\uD83D\uDD16",
    "\uD83D\uDD17",
    "\uD83D\uDD18",
    "\uD83D\uDD19",
    "\uD83D\uDD1A",
    "\uD83D\uDD1B",
    "\uD83D\uDD1C",
    "\uD83D\uDD1D",
    "\uD83D\uDD1E",
    "\uD83D\uDD1F",
    "\uD83D\uDD20",
    "\uD83D\uDD21",
    "\uD83D\uDD22",
    "\uD83D\uDD23",
    "\uD83D\uDD24",
    "\uD83D\uDD25",
    "\uD83D\uDD26",
    "\uD83D\uDD27",
    "\uD83D\uDD28",
    "\uD83D\uDD29",
    "\uD83D\uDD2A",
    "\uD83D\uDD2B",
    "\uD83D\uDD2C",
    "\uD83D\uDD2D",
    "\uD83D\uDD2E",
    "\uD83D\uDD2F",
    "\uD83D\uDD30",
    "\uD83D\uDD31",
    "\uD83D\uDD32",
    "\uD83D\uDD33",
    "\uD83D\uDD34",
    "\uD83D\uDD35",
    "\uD83D\uDD36",
    "\uD83D\uDD37",
    "\uD83D\uDD38",
    "\uD83D\uDD39",
    "\uD83D\uDD3A",
    "\uD83D\uDD3B",
    "\uD83D\uDD3C",
    "\uD83D\uDD3D",
    "\uD83D\uDD49️",
    "\uD83D\uDD4A️",
    "\uD83D\uDD4B",
    "\uD83D\uDD4C",
    "\uD83D\uDD4D",
    "\uD83D\uDD4E",
    "\uD83D\uDD50",
    "\uD83D\uDD51",
    "\uD83D\uDD52",
    "\uD83D\uDD53",
    "\uD83D\uDD54",
    "\uD83D\uDD55",
    "\uD83D\uDD56",
    "\uD83D\uDD57",
    "\uD83D\uDD58",
    "\uD83D\uDD59",
    "\uD83D\uDD5A",
    "\uD83D\uDD5B",
    "\uD83D\uDD5C",
    "\uD83D\uDD5D",
    "\uD83D\uDD5E",
    "\uD83D\uDD5F",
    "\uD83D\uDD60",
    "\uD83D\uDD61",
    "\uD83D\uDD62",
    "\uD83D\uDD63",
    "\uD83D\uDD64",
    "\uD83D\uDD65",
    "\uD83D\uDD66",
    "\uD83D\uDD67",
    "\uD83D\uDD6F️",
    "\uD83D\uDD70️",
    "\uD83D\uDD73️",
    "\uD83D\uDD74\uD83C\uDFFB‍♀️",
    "\uD83D\uDD74\uD83C\uDFFB‍♂️",
    "\uD83D\uDD74\uD83C\uDFFB",
    "\uD83D\uDD74\uD83C\uDFFC‍♀️",
    "\uD83D\uDD74\uD83C\uDFFC‍♂️",
    "\uD83D\uDD74\uD83C\uDFFC",
    "\uD83D\uDD74\uD83C\uDFFD‍♀️",
    "\uD83D\uDD74\uD83C\uDFFD‍♂️",
    "\uD83D\uDD74\uD83C\uDFFD",
    "\uD83D\uDD74\uD83C\uDFFE‍♀️",
    "\uD83D\uDD74\uD83C\uDFFE‍♂️",
    "\uD83D\uDD74\uD83C\uDFFE",
    "\uD83D\uDD74\uD83C\uDFFF‍♀️",
    "\uD83D\uDD74\uD83C\uDFFF‍♂️",
    "\uD83D\uDD74\uD83C\uDFFF",
    "\uD83D\uDD74️‍♀️",
    "\uD83D\uDD74️‍♂️",
    "\uD83D\uDD74️",
    "\uD83D\uDD75\uD83C\uDFFB‍♀️",
    "\uD83D\uDD75\uD83C\uDFFB‍♂️",
    "\uD83D\uDD75\uD83C\uDFFB",
    "\uD83D\uDD75\uD83C\uDFFC‍♀️",
    "\uD83D\uDD75\uD83C\uDFFC‍♂️",
    "\uD83D\uDD75\uD83C\uDFFC",
    "\uD83D\uDD75\uD83C\uDFFD‍♀️",
    "\uD83D\uDD75\uD83C\uDFFD‍♂️",
    "\uD83D\uDD75\uD83C\uDFFD",
    "\uD83D\uDD75\uD83C\uDFFE‍♀️",
    "\uD83D\uDD75\uD83C\uDFFE‍♂️",
    "\uD83D\uDD75\uD83C\uDFFE",
    "\uD83D\uDD75\uD83C\uDFFF‍♀️",
    "\uD83D\uDD75\uD83C\uDFFF‍♂️",
    "\uD83D\uDD75\uD83C\uDFFF",
    "\uD83D\uDD75️‍♀️",
    "\uD83D\uDD75️‍♂️",
    "\uD83D\uDD75️",
    "\uD83D\uDD76️",
    "\uD83D\uDD77️",
    "\uD83D\uDD78️",
    "\uD83D\uDD79️",
    "\uD83D\uDD7A\uD83C\uDFFB",
    "\uD83D\uDD7A\uD83C\uDFFC",
    "\uD83D\uDD7A\uD83C\uDFFD",
    "\uD83D\uDD7A\uD83C\uDFFE",
    "\uD83D\uDD7A\uD83C\uDFFF",
    "\uD83D\uDD7A",
    "\uD83D\uDD87️",
    "\uD83D\uDD8A️",
    "\uD83D\uDD8B️",
    "\uD83D\uDD8C️",
    "\uD83D\uDD8D️",
    "\uD83D\uDD90\uD83C\uDFFB",
    "\uD83D\uDD90\uD83C\uDFFC",
    "\uD83D\uDD90\uD83C\uDFFD",
    "\uD83D\uDD90\uD83C\uDFFE",
    "\uD83D\uDD90\uD83C\uDFFF",
    "\uD83D\uDD90️",
    "\uD83D\uDD95\uD83C\uDFFB",
    "\uD83D\uDD95\uD83C\uDFFC",
    "\uD83D\uDD95\uD83C\uDFFD",
    "\uD83D\uDD95\uD83C\uDFFE",
    "\uD83D\uDD95\uD83C\uDFFF",
    "\uD83D\uDD95",
    "\uD83D\uDD96\uD83C\uDFFB",
    "\uD83D\uDD96\uD83C\uDFFC",
    "\uD83D\uDD96\uD83C\uDFFD",
    "\uD83D\uDD96\uD83C\uDFFE",
    "\uD83D\uDD96\uD83C\uDFFF",
    "\uD83D\uDD96",
    "\uD83D\uDDA4",
    "\uD83D\uDDA5️",
    "\uD83D\uDDA8️",
    "\uD83D\uDDB1️",
    "\uD83D\uDDB2️",
    "\uD83D\uDDBC️",
    "\uD83D\uDDC2️",
    "\uD83D\uDDC3️",
    "\uD83D\uDDC4️",
    "\uD83D\uDDD1️",
    "\uD83D\uDDD2️",
    "\uD83D\uDDD3️",
    "\uD83D\uDDDC️",
    "\uD83D\uDDDD️",
    "\uD83D\uDDDE️",
    "\uD83D\uDDE1️",
    "\uD83D\uDDE3️",
    "\uD83D\uDDE8️",
    "\uD83D\uDDEF️",
    "\uD83D\uDDF3️",
    "\uD83D\uDDFA️",
    "\uD83D\uDDFB",
    "\uD83D\uDDFC",
    "\uD83D\uDDFD",
    "\uD83D\uDDFE",
    "\uD83D\uDDFF",
    "\uD83D\uDE00",
    "\uD83D\uDE01",
    "\uD83D\uDE02",
    "\uD83D\uDE03",
    "\uD83D\uDE04",
    "\uD83D\uDE05",
    "\uD83D\uDE06",
    "\uD83D\uDE07",
    "\uD83D\uDE08",
    "\uD83D\uDE09",
    "\uD83D\uDE0A",
    "\uD83D\uDE0B",
    "\uD83D\uDE0C",
    "\uD83D\uDE0D",
    "\uD83D\uDE0E",
    "\uD83D\uDE0F",
    "\uD83D\uDE10",
    "\uD83D\uDE11",
    "\uD83D\uDE12",
    "\uD83D\uDE13",
    "\uD83D\uDE14",
    "\uD83D\uDE15",
    "\uD83D\uDE16",
    "\uD83D\uDE17",
    "\uD83D\uDE18",
    "\uD83D\uDE19",
    "\uD83D\uDE1A",
    "\uD83D\uDE1B",
    "\uD83D\uDE1C",
    "\uD83D\uDE1D",
    "\uD83D\uDE1E",
    "\uD83D\uDE1F",
    "\uD83D\uDE20",
    "\uD83D\uDE21",
    "\uD83D\uDE22",
    "\uD83D\uDE23",
    "\uD83D\uDE24",
    "\uD83D\uDE25",
    "\uD83D\uDE26",
    "\uD83D\uDE27",
    "\uD83D\uDE28",
    "\uD83D\uDE29",
    "\uD83D\uDE2A",
    "\uD83D\uDE2B",
    "\uD83D\uDE2C",
    "\uD83D\uDE2D",
    "\uD83D\uDE2E",
    "\uD83D\uDE2F",
    "\uD83D\uDE30",
    "\uD83D\uDE31",
    "\uD83D\uDE32",
    "\uD83D\uDE33",
    "\uD83D\uDE34",
    "\uD83D\uDE35",
    "\uD83D\uDE36",
    "\uD83D\uDE37",
    "\uD83D\uDE38",
    "\uD83D\uDE39",
    "\uD83D\uDE3A",
    "\uD83D\uDE3B",
    "\uD83D\uDE3C",
    "\uD83D\uDE3D",
    "\uD83D\uDE3E",
    "\uD83D\uDE3F",
    "\uD83D\uDE40",
    "\uD83D\uDE41",
    "\uD83D\uDE42",
    "\uD83D\uDE43",
    "\uD83D\uDE44",
    "\uD83D\uDE45\uD83C\uDFFB‍♀️",
    "\uD83D\uDE45\uD83C\uDFFB‍♂️",
    "\uD83D\uDE45\uD83C\uDFFB",
    "\uD83D\uDE45\uD83C\uDFFC‍♀️",
    "\uD83D\uDE45\uD83C\uDFFC‍♂️",
    "\uD83D\uDE45\uD83C\uDFFC",
    "\uD83D\uDE45\uD83C\uDFFD‍♀️",
    "\uD83D\uDE45\uD83C\uDFFD‍♂️",
    "\uD83D\uDE45\uD83C\uDFFD",
    "\uD83D\uDE45\uD83C\uDFFE‍♀️",
    "\uD83D\uDE45\uD83C\uDFFE‍♂️",
    "\uD83D\uDE45\uD83C\uDFFE",
    "\uD83D\uDE45\uD83C\uDFFF‍♀️",
    "\uD83D\uDE45\uD83C\uDFFF‍♂️",
    "\uD83D\uDE45\uD83C\uDFFF",
    "\uD83D\uDE45‍♀️",
    "\uD83D\uDE45‍♂️",
    "\uD83D\uDE45",
    "\uD83D\uDE46\uD83C\uDFFB‍♀️",
    "\uD83D\uDE46\uD83C\uDFFB‍♂️",
    "\uD83D\uDE46\uD83C\uDFFB",
    "\uD83D\uDE46\uD83C\uDFFC‍♀️",
    "\uD83D\uDE46\uD83C\uDFFC‍♂️",
    "\uD83D\uDE46\uD83C\uDFFC",
    "\uD83D\uDE46\uD83C\uDFFD‍♀️",
    "\uD83D\uDE46\uD83C\uDFFD‍♂️",
    "\uD83D\uDE46\uD83C\uDFFD",
    "\uD83D\uDE46\uD83C\uDFFE‍♀️",
    "\uD83D\uDE46\uD83C\uDFFE‍♂️",
    "\uD83D\uDE46\uD83C\uDFFE",
    "\uD83D\uDE46\uD83C\uDFFF‍♀️",
    "\uD83D\uDE46\uD83C\uDFFF‍♂️",
    "\uD83D\uDE46\uD83C\uDFFF",
    "\uD83D\uDE46‍♀️",
    "\uD83D\uDE46‍♂️",
    "\uD83D\uDE46",
    "\uD83D\uDE47\uD83C\uDFFB‍♀️",
    "\uD83D\uDE47\uD83C\uDFFB‍♂️",
    "\uD83D\uDE47\uD83C\uDFFB",
    "\uD83D\uDE47\uD83C\uDFFC‍♀️",
    "\uD83D\uDE47\uD83C\uDFFC‍♂️",
    "\uD83D\uDE47\uD83C\uDFFC",
    "\uD83D\uDE47\uD83C\uDFFD‍♀️",
    "\uD83D\uDE47\uD83C\uDFFD‍♂️",
    "\uD83D\uDE47\uD83C\uDFFD",
    "\uD83D\uDE47\uD83C\uDFFE‍♀️",
    "\uD83D\uDE47\uD83C\uDFFE‍♂️",
    "\uD83D\uDE47\uD83C\uDFFE",
    "\uD83D\uDE47\uD83C\uDFFF‍♀️",
    "\uD83D\uDE47\uD83C\uDFFF‍♂️",
    "\uD83D\uDE47\uD83C\uDFFF",
    "\uD83D\uDE47‍♀️",
    "\uD83D\uDE47‍♂️",
    "\uD83D\uDE47",
    "\uD83D\uDE48",
    "\uD83D\uDE49",
    "\uD83D\uDE4A",
    "\uD83D\uDE4B\uD83C\uDFFB‍♀️",
    "\uD83D\uDE4B\uD83C\uDFFB‍♂️",
    "\uD83D\uDE4B\uD83C\uDFFB",
    "\uD83D\uDE4B\uD83C\uDFFC‍♀️",
    "\uD83D\uDE4B\uD83C\uDFFC‍♂️",
    "\uD83D\uDE4B\uD83C\uDFFC",
    "\uD83D\uDE4B\uD83C\uDFFD‍♀️",
    "\uD83D\uDE4B\uD83C\uDFFD‍♂️",
    "\uD83D\uDE4B\uD83C\uDFFD",
    "\uD83D\uDE4B\uD83C\uDFFE‍♀️",
    "\uD83D\uDE4B\uD83C\uDFFE‍♂️",
    "\uD83D\uDE4B\uD83C\uDFFE",
    "\uD83D\uDE4B\uD83C\uDFFF‍♀️",
    "\uD83D\uDE4B\uD83C\uDFFF‍♂️",
    "\uD83D\uDE4B\uD83C\uDFFF",
    "\uD83D\uDE4B‍♀️",
    "\uD83D\uDE4B‍♂️",
    "\uD83D\uDE4B",
    "\uD83D\uDE4C\uD83C\uDFFB",
    "\uD83D\uDE4C\uD83C\uDFFC",
    "\uD83D\uDE4C\uD83C\uDFFD",
    "\uD83D\uDE4C\uD83C\uDFFE",
    "\uD83D\uDE4C\uD83C\uDFFF",
    "\uD83D\uDE4C",
    "\uD83D\uDE4D\uD83C\uDFFB‍♀️",
    "\uD83D\uDE4D\uD83C\uDFFB‍♂️",
    "\uD83D\uDE4D\uD83C\uDFFB",
    "\uD83D\uDE4D\uD83C\uDFFC‍♀️",
    "\uD83D\uDE4D\uD83C\uDFFC‍♂️",
    "\uD83D\uDE4D\uD83C\uDFFC",
    "\uD83D\uDE4D\uD83C\uDFFD‍♀️",
    "\uD83D\uDE4D\uD83C\uDFFD‍♂️",
    "\uD83D\uDE4D\uD83C\uDFFD",
    "\uD83D\uDE4D\uD83C\uDFFE‍♀️",
    "\uD83D\uDE4D\uD83C\uDFFE‍♂️",
    "\uD83D\uDE4D\uD83C\uDFFE",
    "\uD83D\uDE4D\uD83C\uDFFF‍♀️",
    "\uD83D\uDE4D\uD83C\uDFFF‍♂️",
    "\uD83D\uDE4D\uD83C\uDFFF",
    "\uD83D\uDE4D‍♀️",
    "\uD83D\uDE4D‍♂️",
    "\uD83D\uDE4D",
    "\uD83D\uDE4E\uD83C\uDFFB‍♀️",
    "\uD83D\uDE4E\uD83C\uDFFB‍♂️",
    "\uD83D\uDE4E\uD83C\uDFFB",
    "\uD83D\uDE4E\uD83C\uDFFC‍♀️",
    "\uD83D\uDE4E\uD83C\uDFFC‍♂️",
    "\uD83D\uDE4E\uD83C\uDFFC",
    "\uD83D\uDE4E\uD83C\uDFFD‍♀️",
    "\uD83D\uDE4E\uD83C\uDFFD‍♂️",
    "\uD83D\uDE4E\uD83C\uDFFD",
    "\uD83D\uDE4E\uD83C\uDFFE‍♀️",
    "\uD83D\uDE4E\uD83C\uDFFE‍♂️",
    "\uD83D\uDE4E\uD83C\uDFFE",
    "\uD83D\uDE4E\uD83C\uDFFF‍♀️",
    "\uD83D\uDE4E\uD83C\uDFFF‍♂️",
    "\uD83D\uDE4E\uD83C\uDFFF",
    "\uD83D\uDE4E‍♀️",
    "\uD83D\uDE4E‍♂️",
    "\uD83D\uDE4E",
    "\uD83D\uDE4F\uD83C\uDFFB",
    "\uD83D\uDE4F\uD83C\uDFFC",
    "\uD83D\uDE4F\uD83C\uDFFD",
    "\uD83D\uDE4F\uD83C\uDFFE",
    "\uD83D\uDE4F\uD83C\uDFFF",
    "\uD83D\uDE4F",
    "\uD83D\uDE80",
    "\uD83D\uDE81",
    "\uD83D\uDE82",
    "\uD83D\uDE83",
    "\uD83D\uDE84",
    "\uD83D\uDE85",
    "\uD83D\uDE86",
    "\uD83D\uDE87",
    "\uD83D\uDE88",
    "\uD83D\uDE89",
    "\uD83D\uDE8A",
    "\uD83D\uDE8B",
    "\uD83D\uDE8C",
    "\uD83D\uDE8D",
    "\uD83D\uDE8E",
    "\uD83D\uDE8F",
    "\uD83D\uDE90",
    "\uD83D\uDE91",
    "\uD83D\uDE92",
    "\uD83D\uDE93",
    "\uD83D\uDE94",
    "\uD83D\uDE95",
    "\uD83D\uDE96",
    "\uD83D\uDE97",
    "\uD83D\uDE98",
    "\uD83D\uDE99",
    "\uD83D\uDE9A",
    "\uD83D\uDE9B",
    "\uD83D\uDE9C",
    "\uD83D\uDE9D",
    "\uD83D\uDE9E",
    "\uD83D\uDE9F",
    "\uD83D\uDEA0",
    "\uD83D\uDEA1",
    "\uD83D\uDEA2",
    "\uD83D\uDEA3\uD83C\uDFFB‍♀️",
    "\uD83D\uDEA3\uD83C\uDFFB‍♂️",
    "\uD83D\uDEA3\uD83C\uDFFB",
    "\uD83D\uDEA3\uD83C\uDFFC‍♀️",
    "\uD83D\uDEA3\uD83C\uDFFC‍♂️",
    "\uD83D\uDEA3\uD83C\uDFFC",
    "\uD83D\uDEA3\uD83C\uDFFD‍♀️",
    "\uD83D\uDEA3\uD83C\uDFFD‍♂️",
    "\uD83D\uDEA3\uD83C\uDFFD",
    "\uD83D\uDEA3\uD83C\uDFFE‍♀️",
    "\uD83D\uDEA3\uD83C\uDFFE‍♂️",
    "\uD83D\uDEA3\uD83C\uDFFE",
    "\uD83D\uDEA3\uD83C\uDFFF‍♀️",
    "\uD83D\uDEA3\uD83C\uDFFF‍♂️",
    "\uD83D\uDEA3\uD83C\uDFFF",
    "\uD83D\uDEA3‍♀️",
    "\uD83D\uDEA3‍♂️",
    "\uD83D\uDEA3",
    "\uD83D\uDEA4",
    "\uD83D\uDEA5",
    "\uD83D\uDEA6",
    "\uD83D\uDEA7",
    "\uD83D\uDEA8",
    "\uD83D\uDEA9",
    "\uD83D\uDEAA",
    "\uD83D\uDEAB",
    "\uD83D\uDEAC",
    "\uD83D\uDEAD",
    "\uD83D\uDEAE",
    "\uD83D\uDEAF",
    "\uD83D\uDEB0",
    "\uD83D\uDEB1",
    "\uD83D\uDEB2",
    "\uD83D\uDEB3",
    "\uD83D\uDEB4\uD83C\uDFFB‍♀️",
    "\uD83D\uDEB4\uD83C\uDFFB‍♂️",
    "\uD83D\uDEB4\uD83C\uDFFB",
    "\uD83D\uDEB4\uD83C\uDFFC‍♀️",
    "\uD83D\uDEB4\uD83C\uDFFC‍♂️",
    "\uD83D\uDEB4\uD83C\uDFFC",
    "\uD83D\uDEB4\uD83C\uDFFD‍♀️",
    "\uD83D\uDEB4\uD83C\uDFFD‍♂️",
    "\uD83D\uDEB4\uD83C\uDFFD",
    "\uD83D\uDEB4\uD83C\uDFFE‍♀️",
    "\uD83D\uDEB4\uD83C\uDFFE‍♂️",
    "\uD83D\uDEB4\uD83C\uDFFE",
    "\uD83D\uDEB4\uD83C\uDFFF‍♀️",
    "\uD83D\uDEB4\uD83C\uDFFF‍♂️",
    "\uD83D\uDEB4\uD83C\uDFFF",
    "\uD83D\uDEB4‍♀️",
    "\uD83D\uDEB4‍♂️",
    "\uD83D\uDEB4",
    "\uD83D\uDEB5\uD83C\uDFFB‍♀️",
    "\uD83D\uDEB5\uD83C\uDFFB‍♂️",
    "\uD83D\uDEB5\uD83C\uDFFB",
    "\uD83D\uDEB5\uD83C\uDFFC‍♀️",
    "\uD83D\uDEB5\uD83C\uDFFC‍♂️",
    "\uD83D\uDEB5\uD83C\uDFFC",
    "\uD83D\uDEB5\uD83C\uDFFD‍♀️",
    "\uD83D\uDEB5\uD83C\uDFFD‍♂️",
    "\uD83D\uDEB5\uD83C\uDFFD",
    "\uD83D\uDEB5\uD83C\uDFFE‍♀️",
    "\uD83D\uDEB5\uD83C\uDFFE‍♂️",
    "\uD83D\uDEB5\uD83C\uDFFE",
    "\uD83D\uDEB5\uD83C\uDFFF‍♀️",
    "\uD83D\uDEB5\uD83C\uDFFF‍♂️",
    "\uD83D\uDEB5\uD83C\uDFFF",
    "\uD83D\uDEB5‍♀️",
    "\uD83D\uDEB5‍♂️",
    "\uD83D\uDEB5",
    "\uD83D\uDEB6\uD83C\uDFFB‍♀️",
    "\uD83D\uDEB6\uD83C\uDFFB‍♂️",
    "\uD83D\uDEB6\uD83C\uDFFB",
    "\uD83D\uDEB6\uD83C\uDFFC‍♀️",
    "\uD83D\uDEB6\uD83C\uDFFC‍♂️",
    "\uD83D\uDEB6\uD83C\uDFFC",
    "\uD83D\uDEB6\uD83C\uDFFD‍♀️",
    "\uD83D\uDEB6\uD83C\uDFFD‍♂️",
    "\uD83D\uDEB6\uD83C\uDFFD",
    "\uD83D\uDEB6\uD83C\uDFFE‍♀️",
    "\uD83D\uDEB6\uD83C\uDFFE‍♂️",
    "\uD83D\uDEB6\uD83C\uDFFE",
    "\uD83D\uDEB6\uD83C\uDFFF‍♀️",
    "\uD83D\uDEB6\uD83C\uDFFF‍♂️",
    "\uD83D\uDEB6\uD83C\uDFFF",
    "\uD83D\uDEB6‍♀️",
    "\uD83D\uDEB6‍♂️",
    "\uD83D\uDEB6",
    "\uD83D\uDEB7",
    "\uD83D\uDEB8",
    "\uD83D\uDEB9",
    "\uD83D\uDEBA",
    "\uD83D\uDEBB",
    "\uD83D\uDEBC",
    "\uD83D\uDEBD",
    "\uD83D\uDEBE",
    "\uD83D\uDEBF",
    "\uD83D\uDEC0\uD83C\uDFFB",
    "\uD83D\uDEC0\uD83C\uDFFC",
    "\uD83D\uDEC0\uD83C\uDFFD",
    "\uD83D\uDEC0\uD83C\uDFFE",
    "\uD83D\uDEC0\uD83C\uDFFF",
    "\uD83D\uDEC0",
    "\uD83D\uDEC1",
    "\uD83D\uDEC2",
    "\uD83D\uDEC3",
    "\uD83D\uDEC4",
    "\uD83D\uDEC5",
    "\uD83D\uDECB️",
    "\uD83D\uDECC\uD83C\uDFFB",
    "\uD83D\uDECC\uD83C\uDFFC",
    "\uD83D\uDECC\uD83C\uDFFD",
    "\uD83D\uDECC\uD83C\uDFFE",
    "\uD83D\uDECC\uD83C\uDFFF",
    "\uD83D\uDECC",
    "\uD83D\uDECD️",
    "\uD83D\uDECE️",
    "\uD83D\uDECF️",
    "\uD83D\uDED0",
    "\uD83D\uDED1",
    "\uD83D\uDED2",
    "\uD83D\uDED5",
    "\uD83D\uDEE0️",
    "\uD83D\uDEE1️",
    "\uD83D\uDEE2️",
    "\uD83D\uDEE3️",
    "\uD83D\uDEE4️",
    "\uD83D\uDEE5️",
    "\uD83D\uDEE9️",
    "\uD83D\uDEEB",
    "\uD83D\uDEEC",
    "\uD83D\uDEF0️",
    "\uD83D\uDEF3️",
    "\uD83D\uDEF4",
    "\uD83D\uDEF5",
    "\uD83D\uDEF6",
    "\uD83D\uDEF7",
    "\uD83D\uDEF8",
    "\uD83D\uDEF9",
    "\uD83D\uDEFA",
    "\uD83D\uDFE0",
    "\uD83D\uDFE1",
    "\uD83D\uDFE2",
    "\uD83D\uDFE3",
    "\uD83D\uDFE4",
    "\uD83D\uDFE5",
    "\uD83D\uDFE6",
    "\uD83D\uDFE7",
    "\uD83D\uDFE8",
    "\uD83D\uDFE9",
    "\uD83D\uDFEA",
    "\uD83D\uDFEB",
    "\uD83E\uDD0D",
    "\uD83E\uDD0E",
    "\uD83E\uDD0F\uD83C\uDFFB",
    "\uD83E\uDD0F\uD83C\uDFFC",
    "\uD83E\uDD0F\uD83C\uDFFD",
    "\uD83E\uDD0F\uD83C\uDFFE",
    "\uD83E\uDD0F\uD83C\uDFFF",
    "\uD83E\uDD0F",
    "\uD83E\uDD10",
    "\uD83E\uDD11",
    "\uD83E\uDD12",
    "\uD83E\uDD13",
    "\uD83E\uDD14",
    "\uD83E\uDD15",
    "\uD83E\uDD16",
    "\uD83E\uDD17",
    "\uD83E\uDD18\uD83C\uDFFB",
    "\uD83E\uDD18\uD83C\uDFFC",
    "\uD83E\uDD18\uD83C\uDFFD",
    "\uD83E\uDD18\uD83C\uDFFE",
    "\uD83E\uDD18\uD83C\uDFFF",
    "\uD83E\uDD18",
    "\uD83E\uDD19\uD83C\uDFFB",
    "\uD83E\uDD19\uD83C\uDFFC",
    "\uD83E\uDD19\uD83C\uDFFD",
    "\uD83E\uDD19\uD83C\uDFFE",
    "\uD83E\uDD19\uD83C\uDFFF",
    "\uD83E\uDD19",
    "\uD83E\uDD1A\uD83C\uDFFB",
    "\uD83E\uDD1A\uD83C\uDFFC",
    "\uD83E\uDD1A\uD83C\uDFFD",
    "\uD83E\uDD1A\uD83C\uDFFE",
    "\uD83E\uDD1A\uD83C\uDFFF",
    "\uD83E\uDD1A",
    "\uD83E\uDD1B\uD83C\uDFFB",
    "\uD83E\uDD1B\uD83C\uDFFC",
    "\uD83E\uDD1B\uD83C\uDFFD",
    "\uD83E\uDD1B\uD83C\uDFFE",
    "\uD83E\uDD1B\uD83C\uDFFF",
    "\uD83E\uDD1B",
    "\uD83E\uDD1C\uD83C\uDFFB",
    "\uD83E\uDD1C\uD83C\uDFFC",
    "\uD83E\uDD1C\uD83C\uDFFD",
    "\uD83E\uDD1C\uD83C\uDFFE",
    "\uD83E\uDD1C\uD83C\uDFFF",
    "\uD83E\uDD1C",
    "\uD83E\uDD1D",
    "\uD83E\uDD1E\uD83C\uDFFB",
    "\uD83E\uDD1E\uD83C\uDFFC",
    "\uD83E\uDD1E\uD83C\uDFFD",
    "\uD83E\uDD1E\uD83C\uDFFE",
    "\uD83E\uDD1E\uD83C\uDFFF",
    "\uD83E\uDD1E",
    "\uD83E\uDD1F\uD83C\uDFFB",
    "\uD83E\uDD1F\uD83C\uDFFC",
    "\uD83E\uDD1F\uD83C\uDFFD",
    "\uD83E\uDD1F\uD83C\uDFFE",
    "\uD83E\uDD1F\uD83C\uDFFF",
    "\uD83E\uDD1F",
    "\uD83E\uDD20",
    "\uD83E\uDD21",
    "\uD83E\uDD22",
    "\uD83E\uDD23",
    "\uD83E\uDD24",
    "\uD83E\uDD25",
    "\uD83E\uDD26\uD83C\uDFFB‍♀️",
    "\uD83E\uDD26\uD83C\uDFFB‍♂️",
    "\uD83E\uDD26\uD83C\uDFFB",
    "\uD83E\uDD26\uD83C\uDFFC‍♀️",
    "\uD83E\uDD26\uD83C\uDFFC‍♂️",
    "\uD83E\uDD26\uD83C\uDFFC",
    "\uD83E\uDD26\uD83C\uDFFD‍♀️",
    "\uD83E\uDD26\uD83C\uDFFD‍♂️",
    "\uD83E\uDD26\uD83C\uDFFD",
    "\uD83E\uDD26\uD83C\uDFFE‍♀️",
    "\uD83E\uDD26\uD83C\uDFFE‍♂️",
    "\uD83E\uDD26\uD83C\uDFFE",
    "\uD83E\uDD26\uD83C\uDFFF‍♀️",
    "\uD83E\uDD26\uD83C\uDFFF‍♂️",
    "\uD83E\uDD26\uD83C\uDFFF",
    "\uD83E\uDD26‍♀️",
    "\uD83E\uDD26‍♂️",
    "\uD83E\uDD26",
    "\uD83E\uDD27",
    "\uD83E\uDD28",
    "\uD83E\uDD29",
    "\uD83E\uDD2A",
    "\uD83E\uDD2B",
    "\uD83E\uDD2C",
    "\uD83E\uDD2D",
    "\uD83E\uDD2E",
    "\uD83E\uDD2F",
    "\uD83E\uDD30\uD83C\uDFFB",
    "\uD83E\uDD30\uD83C\uDFFC",
    "\uD83E\uDD30\uD83C\uDFFD",
    "\uD83E\uDD30\uD83C\uDFFE",
    "\uD83E\uDD30\uD83C\uDFFF",
    "\uD83E\uDD30",
    "\uD83E\uDD31\uD83C\uDFFB",
    "\uD83E\uDD31\uD83C\uDFFC",
    "\uD83E\uDD31\uD83C\uDFFD",
    "\uD83E\uDD31\uD83C\uDFFE",
    "\uD83E\uDD31\uD83C\uDFFF",
    "\uD83E\uDD31",
    "\uD83E\uDD32\uD83C\uDFFB",
    "\uD83E\uDD32\uD83C\uDFFC",
    "\uD83E\uDD32\uD83C\uDFFD",
    "\uD83E\uDD32\uD83C\uDFFE",
    "\uD83E\uDD32\uD83C\uDFFF",
    "\uD83E\uDD32",
    "\uD83E\uDD33\uD83C\uDFFB",
    "\uD83E\uDD33\uD83C\uDFFC",
    "\uD83E\uDD33\uD83C\uDFFD",
    "\uD83E\uDD33\uD83C\uDFFE",
    "\uD83E\uDD33\uD83C\uDFFF",
    "\uD83E\uDD33",
    "\uD83E\uDD34\uD83C\uDFFB",
    "\uD83E\uDD34\uD83C\uDFFC",
    "\uD83E\uDD34\uD83C\uDFFD",
    "\uD83E\uDD34\uD83C\uDFFE",
    "\uD83E\uDD34\uD83C\uDFFF",
    "\uD83E\uDD34",
    "\uD83E\uDD35\uD83C\uDFFB‍♀️",
    "\uD83E\uDD35\uD83C\uDFFB‍♂️",
    "\uD83E\uDD35\uD83C\uDFFB",
    "\uD83E\uDD35\uD83C\uDFFC‍♀️",
    "\uD83E\uDD35\uD83C\uDFFC‍♂️",
    "\uD83E\uDD35\uD83C\uDFFC",
    "\uD83E\uDD35\uD83C\uDFFD‍♀️",
    "\uD83E\uDD35\uD83C\uDFFD‍♂️",
    "\uD83E\uDD35\uD83C\uDFFD",
    "\uD83E\uDD35\uD83C\uDFFE‍♀️",
    "\uD83E\uDD35\uD83C\uDFFE‍♂️",
    "\uD83E\uDD35\uD83C\uDFFE",
    "\uD83E\uDD35\uD83C\uDFFF‍♀️",
    "\uD83E\uDD35\uD83C\uDFFF‍♂️",
    "\uD83E\uDD35\uD83C\uDFFF",
    "\uD83E\uDD35‍♀️",
    "\uD83E\uDD35‍♂️",
    "\uD83E\uDD35",
    "\uD83E\uDD36\uD83C\uDFFB",
    "\uD83E\uDD36\uD83C\uDFFC",
    "\uD83E\uDD36\uD83C\uDFFD",
    "\uD83E\uDD36\uD83C\uDFFE",
    "\uD83E\uDD36\uD83C\uDFFF",
    "\uD83E\uDD36",
    "\uD83E\uDD37\uD83C\uDFFB‍♀️",
    "\uD83E\uDD37\uD83C\uDFFB‍♂️",
    "\uD83E\uDD37\uD83C\uDFFB",
    "\uD83E\uDD37\uD83C\uDFFC‍♀️",
    "\uD83E\uDD37\uD83C\uDFFC‍♂️",
    "\uD83E\uDD37\uD83C\uDFFC",
    "\uD83E\uDD37\uD83C\uDFFD‍♀️",
    "\uD83E\uDD37\uD83C\uDFFD‍♂️",
    "\uD83E\uDD37\uD83C\uDFFD",
    "\uD83E\uDD37\uD83C\uDFFE‍♀️",
    "\uD83E\uDD37\uD83C\uDFFE‍♂️",
    "\uD83E\uDD37\uD83C\uDFFE",
    "\uD83E\uDD37\uD83C\uDFFF‍♀️",
    "\uD83E\uDD37\uD83C\uDFFF‍♂️",
    "\uD83E\uDD37\uD83C\uDFFF",
    "\uD83E\uDD37‍♀️",
    "\uD83E\uDD37‍♂️",
    "\uD83E\uDD37",
    "\uD83E\uDD38\uD83C\uDFFB‍♀️",
    "\uD83E\uDD38\uD83C\uDFFB‍♂️",
    "\uD83E\uDD38\uD83C\uDFFB",
    "\uD83E\uDD38\uD83C\uDFFC‍♀️",
    "\uD83E\uDD38\uD83C\uDFFC‍♂️",
    "\uD83E\uDD38\uD83C\uDFFC",
    "\uD83E\uDD38\uD83C\uDFFD‍♀️",
    "\uD83E\uDD38\uD83C\uDFFD‍♂️",
    "\uD83E\uDD38\uD83C\uDFFD",
    "\uD83E\uDD38\uD83C\uDFFE‍♀️",
    "\uD83E\uDD38\uD83C\uDFFE‍♂️",
    "\uD83E\uDD38\uD83C\uDFFE",
    "\uD83E\uDD38\uD83C\uDFFF‍♀️",
    "\uD83E\uDD38\uD83C\uDFFF‍♂️",
    "\uD83E\uDD38\uD83C\uDFFF",
    "\uD83E\uDD38‍♀️",
    "\uD83E\uDD38‍♂️",
    "\uD83E\uDD38",
    "\uD83E\uDD39\uD83C\uDFFB‍♀️",
    "\uD83E\uDD39\uD83C\uDFFB‍♂️",
    "\uD83E\uDD39\uD83C\uDFFB",
    "\uD83E\uDD39\uD83C\uDFFC‍♀️",
    "\uD83E\uDD39\uD83C\uDFFC‍♂️",
    "\uD83E\uDD39\uD83C\uDFFC",
    "\uD83E\uDD39\uD83C\uDFFD‍♀️",
    "\uD83E\uDD39\uD83C\uDFFD‍♂️",
    "\uD83E\uDD39\uD83C\uDFFD",
    "\uD83E\uDD39\uD83C\uDFFE‍♀️",
    "\uD83E\uDD39\uD83C\uDFFE‍♂️",
    "\uD83E\uDD39\uD83C\uDFFE",
    "\uD83E\uDD39\uD83C\uDFFF‍♀️",
    "\uD83E\uDD39\uD83C\uDFFF‍♂️",
    "\uD83E\uDD39\uD83C\uDFFF",
    "\uD83E\uDD39‍♀️",
    "\uD83E\uDD39‍♂️",
    "\uD83E\uDD39",
    "\uD83E\uDD3A",
    "\uD83E\uDD3C‍♀️",
    "\uD83E\uDD3C‍♂️",
    "\uD83E\uDD3C",
    "\uD83E\uDD3D\uD83C\uDFFB‍♀️",
    "\uD83E\uDD3D\uD83C\uDFFB‍♂️",
    "\uD83E\uDD3D\uD83C\uDFFB",
    "\uD83E\uDD3D\uD83C\uDFFC‍♀️",
    "\uD83E\uDD3D\uD83C\uDFFC‍♂️",
    "\uD83E\uDD3D\uD83C\uDFFC",
    "\uD83E\uDD3D\uD83C\uDFFD‍♀️",
    "\uD83E\uDD3D\uD83C\uDFFD‍♂️",
    "\uD83E\uDD3D\uD83C\uDFFD",
    "\uD83E\uDD3D\uD83C\uDFFE‍♀️",
    "\uD83E\uDD3D\uD83C\uDFFE‍♂️",
    "\uD83E\uDD3D\uD83C\uDFFE",
    "\uD83E\uDD3D\uD83C\uDFFF‍♀️",
    "\uD83E\uDD3D\uD83C\uDFFF‍♂️",
    "\uD83E\uDD3D\uD83C\uDFFF",
    "\uD83E\uDD3D‍♀️",
    "\uD83E\uDD3D‍♂️",
    "\uD83E\uDD3D",
    "\uD83E\uDD3E\uD83C\uDFFB‍♀️",
    "\uD83E\uDD3E\uD83C\uDFFB‍♂️",
    "\uD83E\uDD3E\uD83C\uDFFB",
    "\uD83E\uDD3E\uD83C\uDFFC‍♀️",
    "\uD83E\uDD3E\uD83C\uDFFC‍♂️",
    "\uD83E\uDD3E\uD83C\uDFFC",
    "\uD83E\uDD3E\uD83C\uDFFD‍♀️",
    "\uD83E\uDD3E\uD83C\uDFFD‍♂️",
    "\uD83E\uDD3E\uD83C\uDFFD",
    "\uD83E\uDD3E\uD83C\uDFFE‍♀️",
    "\uD83E\uDD3E\uD83C\uDFFE‍♂️",
    "\uD83E\uDD3E\uD83C\uDFFE",
    "\uD83E\uDD3E\uD83C\uDFFF‍♀️",
    "\uD83E\uDD3E\uD83C\uDFFF‍♂️",
    "\uD83E\uDD3E\uD83C\uDFFF",
    "\uD83E\uDD3E‍♀️",
    "\uD83E\uDD3E‍♂️",
    "\uD83E\uDD3E",
    "\uD83E\uDD3F",
    "\uD83E\uDD40",
    "\uD83E\uDD41",
    "\uD83E\uDD42",
    "\uD83E\uDD43",
    "\uD83E\uDD44",
    "\uD83E\uDD45",
    "\uD83E\uDD47",
    "\uD83E\uDD48",
    "\uD83E\uDD49",
    "\uD83E\uDD4A",
    "\uD83E\uDD4B",
    "\uD83E\uDD4C",
    "\uD83E\uDD4D",
    "\uD83E\uDD4E",
    "\uD83E\uDD4F",
    "\uD83E\uDD50",
    "\uD83E\uDD51",
    "\uD83E\uDD52",
    "\uD83E\uDD53",
    "\uD83E\uDD54",
    "\uD83E\uDD55",
    "\uD83E\uDD56",
    "\uD83E\uDD57",
    "\uD83E\uDD58",
    "\uD83E\uDD59",
    "\uD83E\uDD5A",
    "\uD83E\uDD5B",
    "\uD83E\uDD5C",
    "\uD83E\uDD5D",
    "\uD83E\uDD5E",
    "\uD83E\uDD5F",
    "\uD83E\uDD60",
    "\uD83E\uDD61",
    "\uD83E\uDD62",
    "\uD83E\uDD63",
    "\uD83E\uDD64",
    "\uD83E\uDD65",
    "\uD83E\uDD66",
    "\uD83E\uDD67",
    "\uD83E\uDD68",
    "\uD83E\uDD69",
    "\uD83E\uDD6A",
    "\uD83E\uDD6B",
    "\uD83E\uDD6C",
    "\uD83E\uDD6D",
    "\uD83E\uDD6E",
    "\uD83E\uDD6F",
    "\uD83E\uDD70",
    "\uD83E\uDD71",
    "\uD83E\uDD73",
    "\uD83E\uDD74",
    "\uD83E\uDD75",
    "\uD83E\uDD76",
    "\uD83E\uDD7A",
    "\uD83E\uDD7B",
    "\uD83E\uDD7C",
    "\uD83E\uDD7D",
    "\uD83E\uDD7E",
    "\uD83E\uDD7F",
    "\uD83E\uDD80",
    "\uD83E\uDD81",
    "\uD83E\uDD82",
    "\uD83E\uDD83",
    "\uD83E\uDD84",
    "\uD83E\uDD85",
    "\uD83E\uDD86",
    "\uD83E\uDD87",
    "\uD83E\uDD88",
    "\uD83E\uDD89",
    "\uD83E\uDD8A",
    "\uD83E\uDD8B",
    "\uD83E\uDD8C",
    "\uD83E\uDD8D",
    "\uD83E\uDD8E",
    "\uD83E\uDD8F",
    "\uD83E\uDD90",
    "\uD83E\uDD91",
    "\uD83E\uDD92",
    "\uD83E\uDD93",
    "\uD83E\uDD94",
    "\uD83E\uDD95",
    "\uD83E\uDD96",
    "\uD83E\uDD97",
    "\uD83E\uDD98",
    "\uD83E\uDD99",
    "\uD83E\uDD9A",
    "\uD83E\uDD9B",
    "\uD83E\uDD9C",
    "\uD83E\uDD9D",
    "\uD83E\uDD9E",
    "\uD83E\uDD9F",
    "\uD83E\uDDA0",
    "\uD83E\uDDA1",
    "\uD83E\uDDA2",
    "\uD83E\uDDA5",
    "\uD83E\uDDA6",
    "\uD83E\uDDA7",
    "\uD83E\uDDA8",
    "\uD83E\uDDA9",
    "\uD83E\uDDAA",
    "\uD83E\uDDAE",
    "\uD83E\uDDAF",
    "\uD83E\uDDB0",
    "\uD83E\uDDB1",
    "\uD83E\uDDB2",
    "\uD83E\uDDB3",
    "\uD83E\uDDB4",
    "\uD83E\uDDB5\uD83C\uDFFB",
    "\uD83E\uDDB5\uD83C\uDFFC",
    "\uD83E\uDDB5\uD83C\uDFFD",
    "\uD83E\uDDB5\uD83C\uDFFE",
    "\uD83E\uDDB5\uD83C\uDFFF",
    "\uD83E\uDDB5",
    "\uD83E\uDDB6\uD83C\uDFFB",
    "\uD83E\uDDB6\uD83C\uDFFC",
    "\uD83E\uDDB6\uD83C\uDFFD",
    "\uD83E\uDDB6\uD83C\uDFFE",
    "\uD83E\uDDB6\uD83C\uDFFF",
    "\uD83E\uDDB6",
    "\uD83E\uDDB7",
    "\uD83E\uDDB8\uD83C\uDFFB‍♀️",
    "\uD83E\uDDB8\uD83C\uDFFB‍♂️",
    "\uD83E\uDDB8\uD83C\uDFFB",
    "\uD83E\uDDB8\uD83C\uDFFC‍♀️",
    "\uD83E\uDDB8\uD83C\uDFFC‍♂️",
    "\uD83E\uDDB8\uD83C\uDFFC",
    "\uD83E\uDDB8\uD83C\uDFFD‍♀️",
    "\uD83E\uDDB8\uD83C\uDFFD‍♂️",
    "\uD83E\uDDB8\uD83C\uDFFD",
    "\uD83E\uDDB8\uD83C\uDFFE‍♀️",
    "\uD83E\uDDB8\uD83C\uDFFE‍♂️",
    "\uD83E\uDDB8\uD83C\uDFFE",
    "\uD83E\uDDB8\uD83C\uDFFF‍♀️",
    "\uD83E\uDDB8\uD83C\uDFFF‍♂️",
    "\uD83E\uDDB8\uD83C\uDFFF",
    "\uD83E\uDDB8‍♀️",
    "\uD83E\uDDB8‍♂️",
    "\uD83E\uDDB8",
    "\uD83E\uDDB9\uD83C\uDFFB‍♀️",
    "\uD83E\uDDB9\uD83C\uDFFB‍♂️",
    "\uD83E\uDDB9\uD83C\uDFFB",
    "\uD83E\uDDB9\uD83C\uDFFC‍♀️",
    "\uD83E\uDDB9\uD83C\uDFFC‍♂️",
    "\uD83E\uDDB9\uD83C\uDFFC",
    "\uD83E\uDDB9\uD83C\uDFFD‍♀️",
    "\uD83E\uDDB9\uD83C\uDFFD‍♂️",
    "\uD83E\uDDB9\uD83C\uDFFD",
    "\uD83E\uDDB9\uD83C\uDFFE‍♀️",
    "\uD83E\uDDB9\uD83C\uDFFE‍♂️",
    "\uD83E\uDDB9\uD83C\uDFFE",
    "\uD83E\uDDB9\uD83C\uDFFF‍♀️",
    "\uD83E\uDDB9\uD83C\uDFFF‍♂️",
    "\uD83E\uDDB9\uD83C\uDFFF",
    "\uD83E\uDDB9‍♀️",
    "\uD83E\uDDB9‍♂️",
    "\uD83E\uDDB9",
    "\uD83E\uDDBA",
    "\uD83E\uDDBB\uD83C\uDFFB",
    "\uD83E\uDDBB\uD83C\uDFFC",
    "\uD83E\uDDBB\uD83C\uDFFD",
    "\uD83E\uDDBB\uD83C\uDFFE",
    "\uD83E\uDDBB\uD83C\uDFFF",
    "\uD83E\uDDBB",
    "\uD83E\uDDBC",
    "\uD83E\uDDBD",
    "\uD83E\uDDBE",
    "\uD83E\uDDBF",
    "\uD83E\uDDC0",
    "\uD83E\uDDC1",
    "\uD83E\uDDC2",
    "\uD83E\uDDC3",
    "\uD83E\uDDC4",
    "\uD83E\uDDC5",
    "\uD83E\uDDC6",
    "\uD83E\uDDC7",
    "\uD83E\uDDC8",
    "\uD83E\uDDC9",
    "\uD83E\uDDCA",
    "\uD83E\uDDCD\uD83C\uDFFB‍♀️",
    "\uD83E\uDDCD\uD83C\uDFFB‍♂️",
    "\uD83E\uDDCD\uD83C\uDFFB",
    "\uD83E\uDDCD\uD83C\uDFFC‍♀️",
    "\uD83E\uDDCD\uD83C\uDFFC‍♂️",
    "\uD83E\uDDCD\uD83C\uDFFC",
    "\uD83E\uDDCD\uD83C\uDFFD‍♀️",
    "\uD83E\uDDCD\uD83C\uDFFD‍♂️",
    "\uD83E\uDDCD\uD83C\uDFFD",
    "\uD83E\uDDCD\uD83C\uDFFE‍♀️",
    "\uD83E\uDDCD\uD83C\uDFFE‍♂️",
    "\uD83E\uDDCD\uD83C\uDFFE",
    "\uD83E\uDDCD\uD83C\uDFFF‍♀️",
    "\uD83E\uDDCD\uD83C\uDFFF‍♂️",
    "\uD83E\uDDCD\uD83C\uDFFF",
    "\uD83E\uDDCD‍♀️",
    "\uD83E\uDDCD‍♂️",
    "\uD83E\uDDCD",
    "\uD83E\uDDCE\uD83C\uDFFB‍♀️",
    "\uD83E\uDDCE\uD83C\uDFFB‍♂️",
    "\uD83E\uDDCE\uD83C\uDFFB",
    "\uD83E\uDDCE\uD83C\uDFFC‍♀️",
    "\uD83E\uDDCE\uD83C\uDFFC‍♂️",
    "\uD83E\uDDCE\uD83C\uDFFC",
    "\uD83E\uDDCE\uD83C\uDFFD‍♀️",
    "\uD83E\uDDCE\uD83C\uDFFD‍♂️",
    "\uD83E\uDDCE\uD83C\uDFFD",
    "\uD83E\uDDCE\uD83C\uDFFE‍♀️",
    "\uD83E\uDDCE\uD83C\uDFFE‍♂️",
    "\uD83E\uDDCE\uD83C\uDFFE",
    "\uD83E\uDDCE\uD83C\uDFFF‍♀️",
    "\uD83E\uDDCE\uD83C\uDFFF‍♂️",
    "\uD83E\uDDCE\uD83C\uDFFF",
    "\uD83E\uDDCE‍♀️",
    "\uD83E\uDDCE‍♂️",
    "\uD83E\uDDCE",
    "\uD83E\uDDCF\uD83C\uDFFB‍♀️",
    "\uD83E\uDDCF\uD83C\uDFFB‍♂️",
    "\uD83E\uDDCF\uD83C\uDFFB",
    "\uD83E\uDDCF\uD83C\uDFFC‍♀️",
    "\uD83E\uDDCF\uD83C\uDFFC‍♂️",
    "\uD83E\uDDCF\uD83C\uDFFC",
    "\uD83E\uDDCF\uD83C\uDFFD‍♀️",
    "\uD83E\uDDCF\uD83C\uDFFD‍♂️",
    "\uD83E\uDDCF\uD83C\uDFFD",
    "\uD83E\uDDCF\uD83C\uDFFE‍♀️",
    "\uD83E\uDDCF\uD83C\uDFFE‍♂️",
    "\uD83E\uDDCF\uD83C\uDFFE",
    "\uD83E\uDDCF\uD83C\uDFFF‍♀️",
    "\uD83E\uDDCF\uD83C\uDFFF‍♂️",
    "\uD83E\uDDCF\uD83C\uDFFF",
    "\uD83E\uDDCF‍♀️",
    "\uD83E\uDDCF‍♂️",
    "\uD83E\uDDCF",
    "\uD83E\uDDD0",
    "\uD83E\uDDD1\uD83C\uDFFB‍\uD83E\uDD1D‍\uD83E\uDDD1\uD83C\uDFFB",
    "\uD83E\uDDD1\uD83C\uDFFB",
    "\uD83E\uDDD1\uD83C\uDFFC‍\uD83E\uDD1D‍\uD83E\uDDD1\uD83C\uDFFB",
    "\uD83E\uDDD1\uD83C\uDFFC‍\uD83E\uDD1D‍\uD83E\uDDD1\uD83C\uDFFC",
    "\uD83E\uDDD1\uD83C\uDFFC",
    "\uD83E\uDDD1\uD83C\uDFFD‍\uD83E\uDD1D‍\uD83E\uDDD1\uD83C\uDFFB",
    "\uD83E\uDDD1\uD83C\uDFFD‍\uD83E\uDD1D‍\uD83E\uDDD1\uD83C\uDFFC",
    "\uD83E\uDDD1\uD83C\uDFFD‍\uD83E\uDD1D‍\uD83E\uDDD1\uD83C\uDFFD",
    "\uD83E\uDDD1\uD83C\uDFFD",
    "\uD83E\uDDD1\uD83C\uDFFE‍\uD83E\uDD1D‍\uD83E\uDDD1\uD83C\uDFFB",
    "\uD83E\uDDD1\uD83C\uDFFE‍\uD83E\uDD1D‍\uD83E\uDDD1\uD83C\uDFFC",
    "\uD83E\uDDD1\uD83C\uDFFE‍\uD83E\uDD1D‍\uD83E\uDDD1\uD83C\uDFFD",
    "\uD83E\uDDD1\uD83C\uDFFE‍\uD83E\uDD1D‍\uD83E\uDDD1\uD83C\uDFFE",
    "\uD83E\uDDD1\uD83C\uDFFE",
    "\uD83E\uDDD1\uD83C\uDFFF‍\uD83E\uDD1D‍\uD83E\uDDD1\uD83C\uDFFB",
    "\uD83E\uDDD1\uD83C\uDFFF‍\uD83E\uDD1D‍\uD83E\uDDD1\uD83C\uDFFC",
    "\uD83E\uDDD1\uD83C\uDFFF‍\uD83E\uDD1D‍\uD83E\uDDD1\uD83C\uDFFD",
    "\uD83E\uDDD1\uD83C\uDFFF‍\uD83E\uDD1D‍\uD83E\uDDD1\uD83C\uDFFE",
    "\uD83E\uDDD1\uD83C\uDFFF‍\uD83E\uDD1D‍\uD83E\uDDD1\uD83C\uDFFF",
    "\uD83E\uDDD1\uD83C\uDFFF",
    "\uD83E\uDDD1‍\uD83E\uDD1D‍\uD83E\uDDD1",
    "\uD83E\uDDD1",
    "\uD83E\uDDD2\uD83C\uDFFB",
    "\uD83E\uDDD2\uD83C\uDFFC",
    "\uD83E\uDDD2\uD83C\uDFFD",
    "\uD83E\uDDD2\uD83C\uDFFE",
    "\uD83E\uDDD2\uD83C\uDFFF",
    "\uD83E\uDDD2",
    "\uD83E\uDDD3\uD83C\uDFFB",
    "\uD83E\uDDD3\uD83C\uDFFC",
    "\uD83E\uDDD3\uD83C\uDFFD",
    "\uD83E\uDDD3\uD83C\uDFFE",
    "\uD83E\uDDD3\uD83C\uDFFF",
    "\uD83E\uDDD3",
    "\uD83E\uDDD4\uD83C\uDFFB",
    "\uD83E\uDDD4\uD83C\uDFFC",
    "\uD83E\uDDD4\uD83C\uDFFD",
    "\uD83E\uDDD4\uD83C\uDFFE",
    "\uD83E\uDDD4\uD83C\uDFFF",
    "\uD83E\uDDD4",
    "\uD83E\uDDD5\uD83C\uDFFB",
    "\uD83E\uDDD5\uD83C\uDFFC",
    "\uD83E\uDDD5\uD83C\uDFFD",
    "\uD83E\uDDD5\uD83C\uDFFE",
    "\uD83E\uDDD5\uD83C\uDFFF",
    "\uD83E\uDDD5",
    "\uD83E\uDDD6\uD83C\uDFFB‍♀️",
    "\uD83E\uDDD6\uD83C\uDFFB‍♂️",
    "\uD83E\uDDD6\uD83C\uDFFB",
    "\uD83E\uDDD6\uD83C\uDFFC‍♀️",
    "\uD83E\uDDD6\uD83C\uDFFC‍♂️",
    "\uD83E\uDDD6\uD83C\uDFFC",
    "\uD83E\uDDD6\uD83C\uDFFD‍♀️",
    "\uD83E\uDDD6\uD83C\uDFFD‍♂️",
    "\uD83E\uDDD6\uD83C\uDFFD",
    "\uD83E\uDDD6\uD83C\uDFFE‍♀️",
    "\uD83E\uDDD6\uD83C\uDFFE‍♂️",
    "\uD83E\uDDD6\uD83C\uDFFE",
    "\uD83E\uDDD6\uD83C\uDFFF‍♀️",
    "\uD83E\uDDD6\uD83C\uDFFF‍♂️",
    "\uD83E\uDDD6\uD83C\uDFFF",
    "\uD83E\uDDD6‍♀️",
    "\uD83E\uDDD6‍♂️",
    "\uD83E\uDDD6",
    "\uD83E\uDDD7\uD83C\uDFFB‍♀️",
    "\uD83E\uDDD7\uD83C\uDFFB‍♂️",
    "\uD83E\uDDD7\uD83C\uDFFB",
    "\uD83E\uDDD7\uD83C\uDFFC‍♀️",
    "\uD83E\uDDD7\uD83C\uDFFC‍♂️",
    "\uD83E\uDDD7\uD83C\uDFFC",
    "\uD83E\uDDD7\uD83C\uDFFD‍♀️",
    "\uD83E\uDDD7\uD83C\uDFFD‍♂️",
    "\uD83E\uDDD7\uD83C\uDFFD",
    "\uD83E\uDDD7\uD83C\uDFFE‍♀️",
    "\uD83E\uDDD7\uD83C\uDFFE‍♂️",
    "\uD83E\uDDD7\uD83C\uDFFE",
    "\uD83E\uDDD7\uD83C\uDFFF‍♀️",
    "\uD83E\uDDD7\uD83C\uDFFF‍♂️",
    "\uD83E\uDDD7\uD83C\uDFFF",
    "\uD83E\uDDD7‍♀️",
    "\uD83E\uDDD7‍♂️",
    "\uD83E\uDDD7",
    "\uD83E\uDDD8\uD83C\uDFFB‍♀️",
    "\uD83E\uDDD8\uD83C\uDFFB‍♂️",
    "\uD83E\uDDD8\uD83C\uDFFB",
    "\uD83E\uDDD8\uD83C\uDFFC‍♀️",
    "\uD83E\uDDD8\uD83C\uDFFC‍♂️",
    "\uD83E\uDDD8\uD83C\uDFFC",
    "\uD83E\uDDD8\uD83C\uDFFD‍♀️",
    "\uD83E\uDDD8\uD83C\uDFFD‍♂️",
    "\uD83E\uDDD8\uD83C\uDFFD",
    "\uD83E\uDDD8\uD83C\uDFFE‍♀️",
    "\uD83E\uDDD8\uD83C\uDFFE‍♂️",
    "\uD83E\uDDD8\uD83C\uDFFE",
    "\uD83E\uDDD8\uD83C\uDFFF‍♀️",
    "\uD83E\uDDD8\uD83C\uDFFF‍♂️",
    "\uD83E\uDDD8\uD83C\uDFFF",
    "\uD83E\uDDD8‍♀️",
    "\uD83E\uDDD8‍♂️",
    "\uD83E\uDDD8",
    "\uD83E\uDDD9\uD83C\uDFFB‍♀️",
    "\uD83E\uDDD9\uD83C\uDFFB‍♂️",
    "\uD83E\uDDD9\uD83C\uDFFB",
    "\uD83E\uDDD9\uD83C\uDFFC‍♀️",
    "\uD83E\uDDD9\uD83C\uDFFC‍♂️",
    "\uD83E\uDDD9\uD83C\uDFFC",
    "\uD83E\uDDD9\uD83C\uDFFD‍♀️",
    "\uD83E\uDDD9\uD83C\uDFFD‍♂️",
    "\uD83E\uDDD9\uD83C\uDFFD",
    "\uD83E\uDDD9\uD83C\uDFFE‍♀️",
    "\uD83E\uDDD9\uD83C\uDFFE‍♂️",
    "\uD83E\uDDD9\uD83C\uDFFE",
    "\uD83E\uDDD9\uD83C\uDFFF‍♀️",
    "\uD83E\uDDD9\uD83C\uDFFF‍♂️",
    "\uD83E\uDDD9\uD83C\uDFFF",
    "\uD83E\uDDD9‍♀️",
    "\uD83E\uDDD9‍♂️",
    "\uD83E\uDDD9",
    "\uD83E\uDDDA\uD83C\uDFFB‍♀️",
    "\uD83E\uDDDA\uD83C\uDFFB‍♂️",
    "\uD83E\uDDDA\uD83C\uDFFB",
    "\uD83E\uDDDA\uD83C\uDFFC‍♀️",
    "\uD83E\uDDDA\uD83C\uDFFC‍♂️",
    "\uD83E\uDDDA\uD83C\uDFFC",
    "\uD83E\uDDDA\uD83C\uDFFD‍♀️",
    "\uD83E\uDDDA\uD83C\uDFFD‍♂️",
    "\uD83E\uDDDA\uD83C\uDFFD",
    "\uD83E\uDDDA\uD83C\uDFFE‍♀️",
    "\uD83E\uDDDA\uD83C\uDFFE‍♂️",
    "\uD83E\uDDDA\uD83C\uDFFE",
    "\uD83E\uDDDA\uD83C\uDFFF‍♀️",
    "\uD83E\uDDDA\uD83C\uDFFF‍♂️",
    "\uD83E\uDDDA\uD83C\uDFFF",
    "\uD83E\uDDDA‍♀️",
    "\uD83E\uDDDA‍♂️",
    "\uD83E\uDDDA",
    "\uD83E\uDDDB\uD83C\uDFFB‍♀️",
    "\uD83E\uDDDB\uD83C\uDFFB‍♂️",
    "\uD83E\uDDDB\uD83C\uDFFB",
    "\uD83E\uDDDB\uD83C\uDFFC‍♀️",
    "\uD83E\uDDDB\uD83C\uDFFC‍♂️",
    "\uD83E\uDDDB\uD83C\uDFFC",
    "\uD83E\uDDDB\uD83C\uDFFD‍♀️",
    "\uD83E\uDDDB\uD83C\uDFFD‍♂️",
    "\uD83E\uDDDB\uD83C\uDFFD",
    "\uD83E\uDDDB\uD83C\uDFFE‍♀️",
    "\uD83E\uDDDB\uD83C\uDFFE‍♂️",
    "\uD83E\uDDDB\uD83C\uDFFE",
    "\uD83E\uDDDB\uD83C\uDFFF‍♀️",
    "\uD83E\uDDDB\uD83C\uDFFF‍♂️",
    "\uD83E\uDDDB\uD83C\uDFFF",
    "\uD83E\uDDDB‍♀️",
    "\uD83E\uDDDB‍♂️",
    "\uD83E\uDDDB",
    "\uD83E\uDDDC\uD83C\uDFFB‍♀️",
    "\uD83E\uDDDC\uD83C\uDFFB‍♂️",
    "\uD83E\uDDDC\uD83C\uDFFB",
    "\uD83E\uDDDC\uD83C\uDFFC‍♀️",
    "\uD83E\uDDDC\uD83C\uDFFC‍♂️",
    "\uD83E\uDDDC\uD83C\uDFFC",
    "\uD83E\uDDDC\uD83C\uDFFD‍♀️",
    "\uD83E\uDDDC\uD83C\uDFFD‍♂️",
    "\uD83E\uDDDC\uD83C\uDFFD",
    "\uD83E\uDDDC\uD83C\uDFFE‍♀️",
    "\uD83E\uDDDC\uD83C\uDFFE‍♂️",
    "\uD83E\uDDDC\uD83C\uDFFE",
    "\uD83E\uDDDC\uD83C\uDFFF‍♀️",
    "\uD83E\uDDDC\uD83C\uDFFF‍♂️",
    "\uD83E\uDDDC\uD83C\uDFFF",
    "\uD83E\uDDDC‍♀️",
    "\uD83E\uDDDC‍♂️",
    "\uD83E\uDDDC",
    "\uD83E\uDDDD\uD83C\uDFFB‍♀️",
    "\uD83E\uDDDD\uD83C\uDFFB‍♂️",
    "\uD83E\uDDDD\uD83C\uDFFB",
    "\uD83E\uDDDD\uD83C\uDFFC‍♀️",
    "\uD83E\uDDDD\uD83C\uDFFC‍♂️",
    "\uD83E\uDDDD\uD83C\uDFFC",
    "\uD83E\uDDDD\uD83C\uDFFD‍♀️",
    "\uD83E\uDDDD\uD83C\uDFFD‍♂️",
    "\uD83E\uDDDD\uD83C\uDFFD",
    "\uD83E\uDDDD\uD83C\uDFFE‍♀️",
    "\uD83E\uDDDD\uD83C\uDFFE‍♂️",
    "\uD83E\uDDDD\uD83C\uDFFE",
    "\uD83E\uDDDD\uD83C\uDFFF‍♀️",
    "\uD83E\uDDDD\uD83C\uDFFF‍♂️",
    "\uD83E\uDDDD\uD83C\uDFFF",
    "\uD83E\uDDDD‍♀️",
    "\uD83E\uDDDD‍♂️",
    "\uD83E\uDDDD",
    "\uD83E\uDDDE‍♀️",
    "\uD83E\uDDDE‍♂️",
    "\uD83E\uDDDE",
    "\uD83E\uDDDF‍♀️",
    "\uD83E\uDDDF‍♂️",
    "\uD83E\uDDDF",
    "\uD83E\uDDE0",
    "\uD83E\uDDE1",
    "\uD83E\uDDE2",
    "\uD83E\uDDE3",
    "\uD83E\uDDE4",
    "\uD83E\uDDE5",
    "\uD83E\uDDE6",
    "\uD83E\uDDE7",
    "\uD83E\uDDE8",
    "\uD83E\uDDE9",
    "\uD83E\uDDEA",
    "\uD83E\uDDEB",
    "\uD83E\uDDEC",
    "\uD83E\uDDED",
    "\uD83E\uDDEE",
    "\uD83E\uDDEF",
    "\uD83E\uDDF0",
    "\uD83E\uDDF1",
    "\uD83E\uDDF2",
    "\uD83E\uDDF3",
    "\uD83E\uDDF4",
    "\uD83E\uDDF5",
    "\uD83E\uDDF6",
    "\uD83E\uDDF7",
    "\uD83E\uDDF8",
    "\uD83E\uDDF9",
    "\uD83E\uDDFA",
    "\uD83E\uDDFB",
    "\uD83E\uDDFC",
    "\uD83E\uDDFD",
    "\uD83E\uDDFE",
    "\uD83E\uDDFF",
    "\uD83E\uDE70",
    "\uD83E\uDE71",
    "\uD83E\uDE72",
    "\uD83E\uDE73",
    "\uD83E\uDE78",
    "\uD83E\uDE79",
    "\uD83E\uDE7A",
    "\uD83E\uDE80",
    "\uD83E\uDE81",
    "\uD83E\uDE82",
    "\uD83E\uDE90",
    "\uD83E\uDE91",
    "\uD83E\uDE92",
    "\uD83E\uDE93",
    "\uD83E\uDE94",
    "\uD83E\uDE95",
    "‼️",
    "⁉️",
    "™️",
    "ℹ️",
    "↔️",
    "↕️",
    "↖️",
    "↗️",
    "↘️",
    "↙️",
    "↩️",
    "↪️",
    "#⃣",
    "⌚️",
    "⌛️",
    "⌨️",
    "⏏️",
    "⏩",
    "⏪",
    "⏫",
    "⏬",
    "⏭️",
    "⏮️",
    "⏯️",
    "⏰",
    "⏱️",
    "⏲️",
    "⏳",
    "⏸️",
    "⏹️",
    "⏺️",
    "Ⓜ️",
    "▪️",
    "▫️",
    "▶️",
    "◀️",
    "◻️",
    "◼️",
    "◽️",
    "◾️",
    "☀️",
    "☁️",
    "☂️",
    "☃️",
    "☄️",
    "☎️",
    "☑️",
    "☔️",
    "☕️",
    "☘️",
    "☝\uD83C\uDFFB",
    "☝\uD83C\uDFFC",
    "☝\uD83C\uDFFD",
    "☝\uD83C\uDFFE",
    "☝\uD83C\uDFFF",
    "☝️",
    "☠️",
    "☢️",
    "☣️",
    "☦️",
    "☪️",
    "☮️",
    "☯️",
    "☸️",
    "☹️",
    "☺️",
    "♀️",
    "♂️",
    "♈️",
    "♉️",
    "♊️",
    "♋️",
    "♌️",
    "♍️",
    "♎️",
    "♏️",
    "♐️",
    "♑️",
    "♒️",
    "♓️",
    "♟️",
    "♠️",
    "♣️",
    "♥️",
    "♦️",
    "♨️",
    "♻️",
    "♾",
    "♿️",
    "⚒️",
    "⚓️",
    "⚔️",
    "⚕️",
    "⚖️",
    "⚗️",
    "⚙️",
    "⚛️",
    "⚜️",
    "⚠️",
    "⚡️",
    "⚪️",
    "⚫️",
    "⚰️",
    "⚱️",
    "⚽️",
    "⚾️",
    "⛄️",
    "⛅️",
    "⛈️",
    "⛎",
    "⛏️",
    "⛑️",
    "⛓️",
    "⛔️",
    "⛩️",
    "⛪️",
    "⛰️",
    "⛱️",
    "⛲️",
    "⛳️",
    "⛴️",
    "⛵️",
    "⛷\uD83C\uDFFB",
    "⛷\uD83C\uDFFC",
    "⛷\uD83C\uDFFD",
    "⛷\uD83C\uDFFE",
    "⛷\uD83C\uDFFF",
    "⛷️",
    "⛸️",
    "⛹\uD83C\uDFFB‍♀️",
    "⛹\uD83C\uDFFB‍♂️",
    "⛹\uD83C\uDFFB",
    "⛹\uD83C\uDFFC‍♀️",
    "⛹\uD83C\uDFFC‍♂️",
    "⛹\uD83C\uDFFC",
    "⛹\uD83C\uDFFD‍♀️",
    "⛹\uD83C\uDFFD‍♂️",
    "⛹\uD83C\uDFFD",
    "⛹\uD83C\uDFFE‍♀️",
    "⛹\uD83C\uDFFE‍♂️",
    "⛹\uD83C\uDFFE",
    "⛹\uD83C\uDFFF‍♀️",
    "⛹\uD83C\uDFFF‍♂️",
    "⛹\uD83C\uDFFF",
    "⛹️‍♀️",
    "⛹️‍♂️",
    "⛹️",
    "⛺️",
    "⛽️",
    "✂️",
    "✅",
    "✈️",
    "✉️",
    "✊\uD83C\uDFFB",
    "✊\uD83C\uDFFC",
    "✊\uD83C\uDFFD",
    "✊\uD83C\uDFFE",
    "✊\uD83C\uDFFF",
    "✊",
    "✋\uD83C\uDFFB",
    "✋\uD83C\uDFFC",
    "✋\uD83C\uDFFD",
    "✋\uD83C\uDFFE",
    "✋\uD83C\uDFFF",
    "✋",
    "✌\uD83C\uDFFB",
    "✌\uD83C\uDFFC",
    "✌\uD83C\uDFFD",
    "✌\uD83C\uDFFE",
    "✌\uD83C\uDFFF",
    "✌️",
    "✍\uD83C\uDFFB",
    "✍\uD83C\uDFFC",
    "✍\uD83C\uDFFD",
    "✍\uD83C\uDFFE",
    "✍\uD83C\uDFFF",
    "✍️",
    "✏️",
    "✒️",
    "✔️",
    "✖️",
    "✝️",
    "✡️",
    "✨",
    "✳️",
    "✴️",
    "❄️",
    "❇️",
    "❌",
    "❎",
    "❓",
    "❔",
    "❕",
    "❗️",
    "❣️",
    "❤️",
    "➕",
    "➖",
    "➗",
    "➡️",
    "➰",
    "➿",
    "⤴️",
    "⤵️",
    "*⃣",
    "⬅️",
    "⬆️",
    "⬇️",
    "⬛️",
    "⬜️",
    "⭐️",
    "⭕️",
    "0⃣",
    "〰️",
    "〽️",
    "1⃣",
    "2⃣",
    "㊗️",
    "㊙️",
    "3⃣",
    "4⃣",
    "5⃣",
    "6⃣",
    "7⃣",
    "8⃣",
    "9⃣",
    "\xa9️",
    "\xae️",
    ""
];

var path = require$$0__default["default"];
var emojisList = emojisList$1;
var getHashDigest$1 = getHashDigest_1;
var emojiRegex = /[\uD800-\uDFFF]./;
var emojiList = emojisList.filter(function(emoji) {
    return emojiRegex.test(emoji);
});
var emojiCache = {};
function encodeStringToEmoji(content, length) {
    if (emojiCache[content]) {
        return emojiCache[content];
    }
    length = length || 1;
    var emojis = [];
    do {
        if (!emojiList.length) {
            throw new Error("Ran out of emoji");
        }
        var index = Math.floor(Math.random() * emojiList.length);
        emojis.push(emojiList[index]);
        emojiList.splice(index, 1);
    }while (--length > 0);
    var emojiEncoding = emojis.join("");
    emojiCache[content] = emojiEncoding;
    return emojiEncoding;
}
function interpolateName$1(loaderContext, name, options) {
    var filename;
    var hasQuery = loaderContext.resourceQuery && loaderContext.resourceQuery.length > 1;
    if (typeof name === "function") {
        filename = name(loaderContext.resourcePath, hasQuery ? loaderContext.resourceQuery : undefined);
    } else {
        filename = name || "[hash].[ext]";
    }
    var context = options.context;
    var content = options.content;
    var regExp = options.regExp;
    var ext = "bin";
    var basename = "file";
    var directory = "";
    var folder = "";
    var query = "";
    if (loaderContext.resourcePath) {
        var parsed = path.parse(loaderContext.resourcePath);
        var resourcePath = loaderContext.resourcePath;
        if (parsed.ext) {
            ext = parsed.ext.substr(1);
        }
        if (parsed.dir) {
            basename = parsed.name;
            resourcePath = parsed.dir + path.sep;
        }
        if (typeof context !== "undefined") {
            directory = path.relative(context, resourcePath + "_").replace(/\\/g, "/").replace(/\.\.(\/)?/g, "_$1");
            directory = directory.substr(0, directory.length - 1);
        } else {
            directory = resourcePath.replace(/\\/g, "/").replace(/\.\.(\/)?/g, "_$1");
        }
        if (directory.length === 1) {
            directory = "";
        } else if (directory.length > 1) {
            folder = path.basename(directory);
        }
    }
    if (loaderContext.resourceQuery && loaderContext.resourceQuery.length > 1) {
        query = loaderContext.resourceQuery;
        var hashIdx = query.indexOf("#");
        if (hashIdx >= 0) {
            query = query.substr(0, hashIdx);
        }
    }
    var url = filename;
    if (content) {
        // Match hash template
        url = url// `hash` and `contenthash` are same in `loader-utils` context
        // let's keep `hash` for backward compatibility
        .replace(/\[(?:([^:\]]+):)?(?:hash|contenthash)(?::([a-z]+\d*))?(?::(\d+))?\]/gi, function(all, hashType, digestType, maxLength) {
            return getHashDigest$1(content, hashType, digestType, parseInt(maxLength, 10));
        }).replace(/\[emoji(?::(\d+))?\]/gi, function(all, length) {
            return encodeStringToEmoji(content, parseInt(length, 10));
        });
    }
    url = url.replace(/\[ext\]/gi, function() {
        return ext;
    }).replace(/\[name\]/gi, function() {
        return basename;
    }).replace(/\[path\]/gi, function() {
        return directory;
    }).replace(/\[folder\]/gi, function() {
        return folder;
    }).replace(/\[query\]/gi, function() {
        return query;
    });
    if (regExp && loaderContext.resourcePath) {
        var match = loaderContext.resourcePath.match(new RegExp(regExp));
        match && match.forEach(function(matched, i) {
            url = url.replace(new RegExp("\\[" + i + "\\]", "ig"), matched);
        });
    }
    if (typeof loaderContext.options === "object" && typeof loaderContext.options.customInterpolateName === "function") {
        url = loaderContext.options.customInterpolateName.call(loaderContext, url, name, options);
    }
    return url;
}
var interpolateName_1 = interpolateName$1;

var getOptions = getOptions_1;
var parseQuery = parseQuery_1;
var stringifyRequest = stringifyRequest_1;
var getRemainingRequest = getRemainingRequest_1;
var getCurrentRequest = getCurrentRequest_1;
var isUrlRequest = isUrlRequest_1;
var urlToRequest = urlToRequest_1;
var parseString = parseString_1;
var getHashDigest = getHashDigest_1;
var interpolateName = interpolateName_1;
lib$1.getOptions = getOptions;
lib$1.parseQuery = parseQuery;
lib$1.stringifyRequest = stringifyRequest;
lib$1.getRemainingRequest = getRemainingRequest;
lib$1.getCurrentRequest = getCurrentRequest;
lib$1.isUrlRequest = isUrlRequest;
lib$1.urlToRequest = urlToRequest;
lib$1.parseString = parseString;
lib$1.getHashDigest = getHashDigest;
lib$1.interpolateName = interpolateName;

var types = [
    "scoped",
    "global",
    "resolve"
];
function webpack(content) {
    if (this.cacheable) this.cacheable();
    this.addDependency(this.resourcePath);
    var options = Object.assign({}, lib$1.getOptions(this));
    if (!options.type) {
        options.type = "scoped";
    }
    // Calls type with the current file name.
    if (typeof options.type === "function") {
        options.type = options.type(this.resourcePath, {
            query: lib$1.parseQuery(this.resourceQuery || "?") || {}
        });
    }
    if (!types.includes(options.type)) {
        return this.callback("The given `type` option is invalid. \n\n" + "Expected:\n One of scoped|global|resolve \n\n" + "Actual:\n " + options.type);
    }
    // Allows to define the type for each individual file using a CSS comment.
    var commentType = content.match(/\/*\s*@styled-jsx=(scoped|global|resolve)/);
    if (commentType) {
        options.type = commentType[1];
    }
    var output = "import css from 'styled-jsx/css';\n\nconst styles = css";
    if (options.type === "global") {
        // css.global``
        output += ".global";
    } else if (options.type === "resolve") {
        // css.resolve``
        output += ".resolve";
    }
    // default css``
    // Escape backticks and backslashes: “`” ⇒ “\`”, “\” ⇒ “\\”
    // (c) https://github.com/coox/styled-jsx-css-loader/blob/97a38e90dddf2c4b066e9247db0612c8f95302de/index.js#L6
    output += "`" + content.replace(/[`\\]/g, function(match) {
        return "\\" + match;
    }) + "`;\n\nexport default styles;";
    this.callback(null, output);
}

exports["default"] = webpack;
