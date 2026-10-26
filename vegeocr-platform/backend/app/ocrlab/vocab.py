"""Snap a noisy reading of a handwritten item name onto a known vocabulary, or refuse.

Handwritten Japanese is beyond the engines available here, but the same few dozen items are ordered again and again, so a
reading only has to be close to one of them. Close-but-not-sure results come back flagged; far ones come back unmatched."""
from __future__ import annotations

import unicodedata
from difflib import SequenceMatcher
from typing import Iterable, List, Optional, Tuple

ACCEPT = 0.72      # similarity needed to propose a name
MARGIN = 0.08      # the best must beat the runner-up by this much to be trusted without review

SEED = """花穂 青ゆず エリンギ キャベツ 長ねぎ ピーマン 玉ねぎ しめじ えのき 絹さや かぶ もやし じゃがいも メロン
胡瓜 納豆 絹豆腐 木綿豆腐 やっこネギ みょうが しいたけ 水菜 ほうれん草 みつば 白菜 巨峰 パパイヤ パプリカ 鶏卵 花ぼじそ
大葉 ヤングコーン たでの葉 サクランボ アスパラ クレソン エディブルフラワー すだち 水ナス 大根 ミニオクラ 生姜 サニーレタス
グリーンカール 豆苗 ししとう アーリーレッド 人参 きゅうり 長葱 ミント レタス 茄子 ローズマリー りんご にんにく 小松菜 レモン トマト
ブロッコリー バジル パセリ タイム セロリ 万能ねぎ 春菊 キウイ オレンジ パイナップル バナナ アボカド ゴーヤ いちご ブルーベリー ラズベリー セルフィーユ ベビーリーフ フルーツトマト 木綿豆腐 グリーンアスパラ 生姜 春菊 ごぼう サツマイモ カボチャ バターナッツ ベルギーエシャロット ナス レンコン イエロービーツ イエローミニトマト ズッキーニ カリフラワー ラディッシュ 国産レモン 金柑""".split()


def norm(s: str) -> str:
    s = unicodedata.normalize("NFKC", s)
    s = "".join(ch for ch in s if not ch.isspace() and ch not in "・･()（）[]【】")
    return "".join(chr(ord(c) - 0x60) if "ァ" <= c <= "ン" else c for c in s)       # katakana -> hiragana


class Vocabulary:
    def __init__(self, words: Iterable[str] = SEED):
        self.words: List[str] = list(dict.fromkeys(words))
        self._n = [norm(w) for w in self.words]

    def add(self, word: str):
        if word and word not in self.words:
            self.words.append(word)
            self._n.append(norm(word))

    def match(self, text: str) -> Tuple[Optional[str], float, bool]:
        """(word|None, score 0-1, needs_review). None when nothing is close enough."""
        q = norm(text)
        if not q:
            return None, 0.0, True
        scores = sorted(((SequenceMatcher(None, q, n).ratio(), w) for n, w in zip(self._n, self.words)), reverse=True)
        best, word = scores[0]
        if best < ACCEPT:
            return None, round(best, 2), True
        second = scores[1][0] if len(scores) > 1 else 0.0
        return word, round(best, 2), (best - second) < MARGIN or best < 0.85
