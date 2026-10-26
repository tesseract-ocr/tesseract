"""Build the 丸五水産 (雅山GARDEN) form catalog from a clean photo:  python scripts/build_form_catalog.py <clean.jpg> <out_dir>"""
import sys
import cv2

from app.ocrlab import forms

T0 = ["10063 活〆　養殖ヒラメ 枚", "10409 生　キンキ　250g 本", "10621 活　穴子（丸）180g 本", "10264 活　すっぽん　800g（小サイズ） 匹", None,
      "60132 生　上がり　車エビ（10g） 本", "60145 生　上がり　車エビ（20g） 本", "60133 生　上がり　車エビ（30g） 本", None,
      "30003 生　殻付アサリ（大） kg", "30047 生　しじみ（並） kg", "30232 活　蝦夷アワビ（韓国）約150/159g ヶ", None,
      "60139 生　塩蔵ワカメ（500g）三陸 pc", "70245 生　もずく【沖縄】細 pc", "10640 生　助子（500g） pc", None,
      "10070 生　板ウニ（小） 枚", "10466 小川のウニ（150g） pc", "60008 生　本ズワイ蟹フレーク 260g pc", None, "30221 姫サザエ（50g） ヶ"]
T1 = ["41212 冷無頭バナメイ海老（16/20） 枚", "40076 冷　無頭BT　13/15 枚", "40291 冷　むきBT　61/70 枚", "40560 冷　ホワイト海老（13/15） 箱",
      "40586 冷　ロシア産ボタン海老（BM） kg", "40101 生冷　オマールテール（3/4オンス） 本", "40026 冷　桜エビ（台湾）500g pc", "40008 冷　スルメイカ《IQF》 ハイ", None,
      "40767 冷　本マグロ（大トロ）柵 柵", "70014 冷凍　ねぎトロ　500g pc", "40040 冷　青のり　150g pc", "40304 冷　釜揚げしらす kg", "70003 とび子（小） pc",
      "40037 冷　スライスモークトラウト 500g（真空） pc", "40127 冷　ホタテ貝柱（L）1kg入 pc", "40126 冷　ホタテ貝柱（M）1kg入 pc", None,
      "70314 サバ文化干し（16入/バラ） 枚", "70096 アジ干物（並）約130/150g 枚", "70388 うるめイワシ（丸干し） pc", "70137 たたみいわし pc", "70347 いわし丸干し45g×6尾 pc"]
T2 = ["80242 ひら湯葉（5枚入） pc", "80134 汲み上げ湯葉 pc", "80307 糸がき pc", "80223 紅白はじかみ（瓶・30入） 瓶", "80310 冷　生わさび（金印）800gサイズ pc",
      "80202 和田久 2番（500g） pc", "80204 和田久 出汁パック pc", "80240 焼き海苔　20入 pc", None, "70318 しいの　かつお酒盗280g 瓶",
      "80196 よもぎ麩 本", "80096 栗麩 本", None, None, "90047 【LINE特売】 本"]


def parse(rows):
    out = {}
    for i, r in enumerate(rows):
        if r:
            code, rest = r.split(" ", 1)
            name, unit = rest.rsplit(" ", 1)
            out[i] = {"code": code, "name": name, "unit": unit}
    return out


if __name__ == "__main__":
    src, dst = sys.argv[1:3]
    truth = {}
    for t, rows in enumerate((T0, T1, T2)):
        for i, info in parse(rows).items():
            truth[(t, i)] = info
    cat = forms.build_catalog(cv2.imread(src), truth, "丸五水産 発注書")
    cat.save(dst)
    print("catalog items:", len(cat.items), "->", dst)
