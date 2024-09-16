let argv = process.argv || [],
	env = process.env
let isColorSupported =
	!("NO_COLOR" in env || argv.includes("--no-color")) &&
	("FORCE_COLOR" in env ||
		argv.includes("--color") ||
		process.platform === "win32" ||
		(require != null && require("tty").isatty(1) && env.TERM !== "dumb") ||
		"CI" in env)

let formatter =
	(open, close, replace = open) =>
	input => {
		let string = "" + input
		let index = string.indexOf(close, open.length)
		return ~index
			? open + replaceClose(string, close, replace, index) + close
			: open + string + close
	}

let replaceClose = (string, close, replace, index) => {
	let result = ""
	let cursor = 0
	do {
		result += string.substring(cursor, index) + replace
		cursor = index + close.length
		index = string.indexOf(close, cursor)
	} while (~index)
	return result + string.substring(cursor)
}

let createColors = (enabled = isColorSupported) => {
	let init = enabled ? formatter : () => String
	return {
		isColorSupported: enabled,
		reset: init("\x1b[0m", "\x1b[0m"),
		bold: init("\x1b[1m", "\x1b[22m", "\x1b[22m\x1b[1m"),
		dim: init("\x1b[2m", "\x1b[22m", "\x1b[22m\x1b[2m"),
		italic: init("\x1b[3m", "\x1b[23m"),
		underline: init("\x1b[4m", "\x1b[24m"),
		inverse: init("\x1b[7m", "\x1b[27m"),
		hidden: init("\x1b[8m", "\x1b[28m"),
		strikethrough: init("\x1b[9m", "\x1b[29m"),

		black: init("\x1b[30m", "\x1b[39m"),
		red: init("\x1b[31m", "\x1b[39m"),
		green: init("\x1b[32m", "\x1b[39m"),
		yellow: init("\x1b[33m", "\x1b[39m"),
		blue: init("\x1b[34m", "\x1b[39m"),
		magenta: init("\x1b[35m", "\x1b[39m"),
		cyan: init("\x1b[36m", "\x1b[39m"),
		white: init("\x1b[37m", "\x1b[39m"),
		gray: init("\x1b[90m", "\x1b[39m"),

		bgBlack: init("\x1b[40m", "\x1b[49m"),
		bgRed: init("\x1b[41m", "\x1b[49m"),
		bgGreen: init("\x1b[42m", "\x1b[49m"),
		bgYellow: init("\x1b[43m", "\x1b[49m"),
		bgBlue: init("\x1b[44m", "\x1b[49m"),
		bgMagenta: init("\x1b[45m", "\x1b[49m"),
		bgCyan: init("\x1b[46m", "\x1b[49m"),
		bgWhite: init("\x1b[47m", "\x1b[49m"),

		blackBright: init("\x1b[90m", "\x1b[39m"),
		redBright: init("\x1b[91m", "\x1b[39m"),
		greenBright: init("\x1b[92m", "\x1b[39m"),
		yellowBright: init("\x1b[93m", "\x1b[39m"),
		blueBright: init("\x1b[94m", "\x1b[39m"),
		magentaBright: init("\x1b[95m", "\x1b[39m"),
		cyanBright: init("\x1b[96m", "\x1b[39m"),
		whiteBright: init("\x1b[97m", "\x1b[39m"),

		bgBlackBright: init("\x1b[100m","\x1b[49m"),
		bgRedBright: init("\x1b[101m","\x1b[49m"),
		bgGreenBright: init("\x1b[102m","\x1b[49m"),
		bgYellowBright: init("\x1b[103m","\x1b[49m"),
		bgBlueBright: init("\x1b[104m","\x1b[49m"),
		bgMagentaBright: init("\x1b[105m","\x1b[49m"),
		bgCyanBright: init("\x1b[106m","\x1b[49m"),
		bgWhiteBright: init("\x1b[107m","\x1b[49m"),
	}
}

module.exports = createColors()
module.exports.createColors = createColors
