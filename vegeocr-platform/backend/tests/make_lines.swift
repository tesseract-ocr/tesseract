// swift tests/make_lines.swift OUT_DIR  -> line images + labels.tsv for the training smoke test
import AppKit
let out = CommandLine.arguments[1]
try? FileManager.default.createDirectory(atPath: out, withIntermediateDirectories: true)
let texts = ["ワルドデリカ株式会社 納品書", "請求書 合計金額 9,800円", "野菜協同組合 本社", "注文番号 20261026-001", "お支払い期限 2026年11月30日",
             "東京都千代田区丸の内1-1-1", "税込合計 12,345円", "ご利用ありがとうございます", "担当者 山田 太郎", "商品名 トマト 5kg 箱入り", "数量 24 単価 480円", "備考 冷蔵便にて配送"]
let fonts = ["HiraginoSans-W3", "HiraginoSans-W6", "HiraMinProN-W3", "HiraMaruProN-W4", "YuGothic-Medium"]
var labels = ""
var n = 0
for (ti, t) in texts.enumerated() { for (fi, fn) in fonts.enumerated() {
    guard let f = NSFont(name: fn, size: 40) else { continue }
    let at: [NSAttributedString.Key: Any] = [.font: f, .foregroundColor: NSColor.black]
    let s = NSAttributedString(string: t, attributes: at)
    let sz = s.size()
    let w = Int(sz.width) + 40, h = Int(sz.height) + 24
    let img = NSImage(size: NSSize(width: w, height: h)); img.lockFocus()
    NSColor.white.setFill(); NSRect(x: 0, y: 0, width: w, height: h).fill()
    s.draw(at: NSPoint(x: 20, y: 12)); img.unlockFocus()
    let r = NSBitmapImageRep(data: img.tiffRepresentation!)!
    let name = String(format: "l%02d_%d.png", ti, fi)
    try! r.representation(using: .png, properties: [:])!.write(to: URL(fileURLWithPath: "\(out)/\(name)"))
    labels += "\(name)\t\(t)\n"; n += 1
}}
try! labels.write(toFile: "\(out)/labels.tsv", atomically: true, encoding: .utf8)
print("wrote \(n) lines")
