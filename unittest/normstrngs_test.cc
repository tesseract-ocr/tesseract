// (C) Copyright 2017, Google Inc.
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "absl/strings/str_format.h"    // for absl::StrFormat
#include "include_gunit.h"
#include "normstrngs.h"
#include "normstrngs_test.h"
#include "strngs.h"
#include "unichar.h"
#if defined(HAS_UNILIB_H)
#include "unilib.h"
#endif

#include "include_gunit.h"

namespace tesseract {
namespace {

#if defined(HAS_UNILIB_H)
static std::string EncodeAsUTF8(const char32 ch32) {
  UNICHAR uni_ch(ch32);
  return std::string(uni_ch.utf8(), uni_ch.utf8_len());
}
#endif

TEST(NormstrngsTest, BasicText) {
  const char* kBasicText = "AbCd Ef";
  std::string result;
  EXPECT_TRUE(NormalizeUTF8String(UnicodeNormMode::kNFKC, OCRNorm::kNormalize,
                                  GraphemeNorm::kNormalize, kBasicText,
                                  &result));
  EXPECT_STREQ(kBasicText, result.c_str());
}

TEST(NormstrngsTest, LigatureText) {
  const char* kTwoByteLigText = "ĳ";  // U+0133 (ĳ) -> ij
  std::string result;
  EXPECT_TRUE(NormalizeUTF8String(UnicodeNormMode::kNFKC, OCRNorm::kNormalize,
                                  GraphemeNorm::kNormalize, kTwoByteLigText,
                                  &result));
  EXPECT_STREQ("ij", result.c_str());

  const char* kThreeByteLigText = "ﬁnds";  // U+FB01 (ﬁ) -> fi
  EXPECT_TRUE(NormalizeUTF8String(UnicodeNormMode::kNFKC, OCRNorm::kNormalize,
                                  GraphemeNorm::kNormalize, kThreeByteLigText,
                                  &result));
  EXPECT_STREQ("finds", result.c_str());
}

TEST(NormstrngsTest, OcrSpecificNormalization) {
  const char* kSingleQuoteText = "‘Hi";  // U+2018 (‘) -> U+027 (')
  std::string result;
  EXPECT_TRUE(NormalizeUTF8String(UnicodeNormMode::kNFKC, OCRNorm::kNormalize,
                                  GraphemeNorm::kNormalize, kSingleQuoteText,
                                  &result));
  EXPECT_STREQ("'Hi", result.c_str());

  const char* kDoubleQuoteText = "“Hi";  // U+201C (“) -> U+022 (")
  EXPECT_TRUE(NormalizeUTF8String(UnicodeNormMode::kNFKC, OCRNorm::kNormalize,
                                  GraphemeNorm::kNormalize, kDoubleQuoteText,
                                  &result));
  EXPECT_STREQ("\"Hi", result.c_str());

  const char* kEmDash = "Hi—";  // U+2014 (—) -> U+02D (-)
  EXPECT_TRUE(NormalizeUTF8String(UnicodeNormMode::kNFKC, OCRNorm::kNormalize,
                                  GraphemeNorm::kNormalize, kEmDash, &result));
  EXPECT_STREQ("Hi-", result.c_str());
  // Without the ocr normalization, these changes are not made.
  EXPECT_TRUE(NormalizeUTF8String(UnicodeNormMode::kNFKC, OCRNorm::kNone,
                                  GraphemeNorm::kNormalize, kSingleQuoteText,
                                  &result));
  EXPECT_STREQ(kSingleQuoteText, result.c_str());
  EXPECT_TRUE(NormalizeUTF8String(UnicodeNormMode::kNFKC, OCRNorm::kNone,
                                  GraphemeNorm::kNormalize, kDoubleQuoteText,
                                  &result));
  EXPECT_STREQ(kDoubleQuoteText, result.c_str());
  EXPECT_TRUE(NormalizeUTF8String(UnicodeNormMode::kNFKC, OCRNorm::kNone,
                                  GraphemeNorm::kNormalize, kEmDash, &result));
  EXPECT_STREQ(kEmDash, result.c_str());
}

// Sample text used in tests.
const char kEngText[] = "the quick brown fox jumps over the lazy dog";
const char kHinText[] = "पिताने विवाह की | हो गई उद्विग्न वह सोचा";
const char kKorText[] = "이는 것으로";
// Hindi words containing illegal vowel sequences.
const char* kBadlyFormedHinWords[] = {"उपयोक्ताो", "नहीें",     "प्रंात",
                                      "कहीअे",     "पत्रिाका", "छह्णाीस"};
// Thai illegal sequences.
const char* kBadlyFormedThaiWords[] = {"ฤิ", "กา้ํ", "กิำ", "นำ้", "เเก"};

TEST(NormstrngsTest, DetectsCorrectText) {
  std::string chars;
  EXPECT_TRUE(NormalizeUTF8String(UnicodeNormMode::kNFKC, OCRNorm::kNone,
                                  GraphemeNorm::kNormalize, kEngText, &chars));
  EXPECT_STREQ(kEngText, chars.c_str());

  EXPECT_TRUE(NormalizeUTF8String(UnicodeNormMode::kNFKC, OCRNorm::kNone,
                                  GraphemeNorm::kNormalize, kHinText, &chars))
      << "Incorrect text: '" << kHinText << "'";
  EXPECT_STREQ(kHinText, chars.c_str());

  EXPECT_TRUE(NormalizeUTF8String(UnicodeNormMode::kNFKC, OCRNorm::kNone,
                                  GraphemeNorm::kNormalize, kKorText, &chars));
  EXPECT_STREQ(kKorText, chars.c_str());
}

TEST(NormstrngsTest, DetectsIncorrectText) {
  for (size_t i = 0; i < ARRAYSIZE(kBadlyFormedHinWords); ++i) {
    EXPECT_FALSE(NormalizeUTF8String(UnicodeNormMode::kNFKC, OCRNorm::kNone,
                                     GraphemeNorm::kNormalize,
                                     kBadlyFormedHinWords[i], nullptr))
        << kBadlyFormedHinWords[i];
  }
  for (size_t i = 0; i < ARRAYSIZE(kBadlyFormedThaiWords); ++i) {
    EXPECT_FALSE(NormalizeUTF8String(UnicodeNormMode::kNFKC, OCRNorm::kNone,
                                     GraphemeNorm::kNormalize,
                                     kBadlyFormedThaiWords[i], nullptr))
        << kBadlyFormedThaiWords[i];
  }
}

TEST(NormstrngsTest, NonIndicTextDoesntBreakIndicRules) {
  std::string nonindic = "Here's some latin text.";
  std::string dest;
  EXPECT_TRUE(NormalizeUTF8String(UnicodeNormMode::kNFC, OCRNorm::kNone,
                                  GraphemeNorm::kNormalize, nonindic.c_str(),
                                  &dest))
      << PrintString32WithUnicodes(nonindic);
  EXPECT_EQ(dest, nonindic);
}

TEST(NormstrngsTest, NoLonelyJoiners) {
  std::string str = "x\u200d\u0d06\u0d34\u0d02";
  std::vector<std::string> glyphs;
  // Returns true, but the joiner is gone.
  EXPECT_TRUE(NormalizeCleanAndSegmentUTF8(
      UnicodeNormMode::kNFC, OCRNorm::kNone, GraphemeNormMode::kCombined, true,
      str.c_str(), &glyphs))
      << PrintString32WithUnicodes(str);
  EXPECT_EQ(glyphs.size(), 3);
  EXPECT_EQ(glyphs[0], std::string("x"));
  EXPECT_EQ(glyphs[1], std::string("\u0d06"));
  EXPECT_EQ(glyphs[2], std::string("\u0d34\u0d02"));
}

TEST(NormstrngsTest, NoLonelyJoinersPlus) {
  std::string str = "\u0d2a\u200d+\u0d2a\u0d4b";
  std::vector<std::string> glyphs;
  // Returns true, but the joiner is gone.
  EXPECT_TRUE(NormalizeCleanAndSegmentUTF8(
      UnicodeNormMode::kNFC, OCRNorm::kNone, GraphemeNormMode::kCombined, true,
      str.c_str(), &glyphs))
      << PrintString32WithUnicodes(str);
  EXPECT_EQ(glyphs.size(), 3);
  EXPECT_EQ(glyphs[0], std::string("\u0d2a"));
  EXPECT_EQ(glyphs[1], std::string("+"));
  EXPECT_EQ(glyphs[2], std::string("\u0d2a\u0d4b"));
}

TEST(NormstrngsTest, NoLonelyJoinersNonAlpha) {
  std::string str = "\u200d+\u200c\u200d";
  // Returns true, but the joiners are gone.
  ExpectGraphemeModeResults(str, UnicodeNormMode::kNFC, 1, 1, 1, std::string("+"));
  str = "\u200d\u200c\u200d";
  // Without the plus, the string is invalid.
  std::string result;
  EXPECT_FALSE(NormalizeUTF8String(UnicodeNormMode::kNFC, OCRNorm::kNone,
                                   GraphemeNorm::kNormalize, str.c_str(),
                                   &result))
      << PrintString32WithUnicodes(result);
}

TEST(NormstrngsTest, JoinersStayInArabic) {
  std::string str = "\u0628\u200c\u0628\u200d\u0628";
  // Returns true, string untouched.
  ExpectGraphemeModeResults(str, UnicodeNormMode::kNFC, 5, 5, 2, str);
}

TEST(NormstrngsTest, DigitOK) {
  std::string str = "\u0cea";  // Digit 4.
  ExpectGraphemeModeResults(str, UnicodeNormMode::kNFC, 1, 1, 1, str);
}

TEST(NormstrngsTest, DandaOK) {
  std::string str = "\u0964";  // Single danda.
  ExpectGraphemeModeResults(str, UnicodeNormMode::kNFC, 1, 1, 1, str);
  str = "\u0965";  // Double danda.
  ExpectGraphemeModeResults(str, UnicodeNormMode::kNFC, 1, 1, 1, str);
}

TEST(NormstrngsTest, AllScriptsRegtest) {
  // Tests some valid text in a large number of scripts, some of which were
  // found to be rejected by an earlier version.
  const std::vector<std::pair<std::string, std::string>> kScriptText(
      {{"Arabic",
        " فكان منهم علقمة بن قيس ، و إبراهيم النخعي ، و الأسود بن"
        "توفي بالمدينة في هذه السنة وهو ابن مائة وعشرين سنة "
        "مجموعه هیچ اثری در فنون هنر و ادب و ترجمه، تقدیم پیشگاه ارجمند "
        "سازنده تاریخ نگاه میکرد و به اصطلاح انسان و فطرت انسانی را زیربنای"},
       {"Armenian",
        "անտիկ աշխարհի փիլիսոփաների կենսագրությունը, թե′ նրանց ուս-"
        "պատրաստւում է դալ (բուլամա): Կովկասում կաթից նաև պատ-"
        "Հոգաբարձութեան յղել այդ անձին յիմարութիւնը հաստա-"
        "գծերը եւ միջագծերը կը համրուին վարէն վեր:"},
       {"Bengali",
        "এসে দাঁড়ায় দাও খানি উঁচিয়ে নিয়ে । ঝরনার স্বচ্ছ জলে প্রতিবিম্বিত "
        "পাঠিয়ে, গোবিন্দ স্মরণ করে, নির্ভয়ে রওনা হয়েছিল। তাতে সে "
        "সুলতার। মনে পড়ে বিয়ের সময় বাবা এদের বাড়ি থেকে ঘুরে "
        "কিন্তু তারপর মাতৃহৃদয় কেমন করে আছে? কী"},
       {"Cyrillic",
        "достей, є ще нагороди й почесті, є хай і сумнівна, але слава, "
        "вып., 96б). Параўн. найсвятший у 1 знач., насвятейший у 1 знач., "
        "»Правді«, — гітлерівські окупанти винищували нижчі раси, після дру- "
        "І знов майдан зачорнів од народу. Всередині чоло-"},
       {"Devanagari",
        "डा॰ नै हात्तीमाथि चढेर त्यो भएनेर आइपुगे। राजालाई देखी "
        "बाबतीत लिहिणे ही  एक मोठीच जबाबदारी आहे. काकासाहेबांच्या कार्याचा "
        "प्रबंध, आधोगिक प्रबंध तथा बैंकिंग  एवम वाणिज्य आदि विषयों में "
        "चित्रकृती दिल्या. शंभराहून अधिक देश आज आपापले चित्रपट निर्माण करीत"},
       {"Greek",
        "Μέσα ένα τετράδιο είχα στριμώξει το πρώτο "
        "νον αξίως τού ευαγγελίου τού χριστού πολιτεύεσθε, ίνα "
        "οὐδεμία ὑπ' αὐτοῦ μνεία γίνεται τῶν οἰκείων χωρίων. "
        "είτα την φάσιν αυτήν ην ούτος εποιήσατο κατά του Μίκω-"},
       {"Gujarati",
        "ઉપહારગૃહે ને નાટ્યસ્થળે આ એ જ તેલ કડકડતું "
        "શકી. ભાવવધારો અટકાવી નથી શકી અને બેકારીને "
        "ત્યાં વાંકુથી પાછે  આવ્યો, ચોરીનો માલ સોંપવા ! "
        "કહી. એણે રેશમના કપડામાં વીંટી રાખેલ કુંવરીની છબી"},
       {"Gurmukhi",
        "ਯਾਦ ਰਹੇ ਕਿ ‘ਨਫਰਤ ’ ਦਾ ਵਿਸ਼ਾ ਕ੍ਰਾਤੀ ਨਹੀ ਹੈ ਅਤੇ ਕਵੀ ਦੀ ਇਹ "
        "ਮਹਾਂ ਨੰਦਾ ਕੋਲ ਇਕ ਚੀਜ਼ ਸੀ ਉਹ ਸੀ ਸਚ, ਕੋਰਾ ਸਚ, ਬੇਧਤ੍ਰਕ ਕਹਿੳ "
        "ਭੂਰਾ  ਸਾਨੂੰ  ਥੜਾ  ਚੰਗਾ  ਲਗਦਾ  ਸੀ ।  ਉਸ  ਦਾ  ਇਕ  ਪੈਰ  ਜਨਮ ਤੋ "
        "ਨੂੰ ਇਹ ਅਧਿਕਾਰ ਦਿੱਤਾ ਕਿ ਉਹ ਸਿੱਖ ਵਿਰੋਧ ਦਾ ਸੰਗਠਨ ਕਰੇ ਅਤੇ 3 ਸਤੰਬਰ,"},
       {"Hangul",
        "로 들어갔다. 이대통령은 아이젠하워 대통령의 뒷모습을 보면서 "
        "그것뿐인 줄 아요? 노름도 했다 캅니다. 빌어묵을 놈이 그러 "
        "의 가장 과학적 태도이며, 우리 역사를 가장 정확하게 학습할 수 있는 "
        "마르크스 레"
        "각하는 그는 그들의 식사보장을 위해 때때로 집에"},
       {"HanS",
        "大凡世界上的先生可 分 三 种： 第一种只会教书， 只会拿一 "
        "书像是探宝一样，在茶叶店里我买过西湖龙井﹑黄山毛峰﹑福建的铁观音﹑大红"
        " "
        "持 “左” 倾冒险主义的干部，便扣上 “富农 "
        "笑说：“我听说了，王总工程师也跟我说过了，只是工作忙，谁"},
       {"HanT",
        "叁、 銀行資產管理的群組分析模式 "
        "民國六十三年，申請就讀台灣大學歷史研究所，並從事著述，"
        "質言之﹐在社會結構中﹐性質﹑特徵﹑地位相類似的一羣人﹐由於 "
        "董橋，一九四二年生，福建晉江人，國立成功大學外"},
       {"Hebrew",
        " אֵ-לִי, אֵ-לִי, כֵּיַצד מְטַפְּסִים בְּקִירוֹת שֶׁל זְכוּכִי"
        " הראשון חוצה אותי שוב. אני בסיבוב הרביעי, הוא בטח מתחיל את"
        " ווערטער  געהאט,  אבער  דער  עיקר  איז  ניט  דאָס  וואָרט,  נאָר"
        " על גחלת היהדות המקורית בעירך, נתת צביון ואופי מיוחד"},
       {"Japanese",
        "は異民族とみなされていた。楚の荘王（前613〜前 "
        "を詳細に吟味する。実際の治療活動の領域は便宜上、(1)　障害者 "
        "困難性は多角企業の場合原則として部門別に判断されている.). "
        "☆ご希望の団体には見本をお送りします"},
       {"Kannada",
        "ಕೂಡ ಯುದ್ಧ ಮಾಡಿ ಜಯಪಡೆ. ನಂತರ ನಗರದೊಳಕ್ಕೆ ನಡೆ ಇದನ್ನು "
        "ಅಸಹ್ಯದೃಶ್ಯ ಯಾರಿಗಾದರೂ ನಾಚಿಕೆತರುವಂತಹದಾಗಿದೆ. ಆರೋಗ್ಯ ದೃಷ್ಟಿ "
        "ಯಾಗಲಿ, ಮೋಹನನಾಗಲಿ ಇಂಥ ಬಿಸಿಲಿನಲ್ಲಿ ಎಂದೂ ಬಹಳ ಹೊತ್ತು "
        "\"ಇದೆ...ಖಂಡಿತಾ ಇದೆ\" ಅಂದ ಮನಸ್ಸಿನಲ್ಲಿಯೇ ವಂದಿಸುತ್ತಾ,"},
       {"Khmer",
        "សិតសក់និងផ្លាស់សម្លៀកបំពាក់ពេលយប់ចេញ។ "
        "និយាយអំពីនគរនេះ ប្រាប់ដល់លោកទាំងមូលឲ្យដឹងច្បាស់លាស់អំពី "
        "កន្លះកាថាសម្រាប់ទន្ទេញឲ្យងាយចាំ បោះពុម្ពនៅក្នុងទ្រង់ទ្រាយបច្ចុប្បន្ន "
        "ឯកសារនេះបានផ្សព្វផ្សាយនៅក្នុងសន្និសីទ"},
       {"Lao",
        "ເອີຍ ! ຟັງສຽງຟ້າມັນຮ້ອງຮ່ວນ ມັນດັງໄກໆ ເອີຍ "
        "ໄດລຽງດູລາວມາດວບຄວາມລາບາກຫລາຍ; "
        "ບາງໄດ້ ເຈົ້າລອງສູ້ບໍ່ໄດ້ຈຶ່ງຫນີລົງມາວຽງຈັນ. "
        "ລົບອອກຈາກ 3 ເຫລືອ 1, ຂ້ອຍຂຽນ 1 (1)"},
       {"Latin",
        "režisoru, palīdzēja to manu domīgo, kluso Dzejas metru ielikt "
        "Ešte nedávno sa chcel mladý Novomeský „liečiť” "
        "tiivisia kysymyksiä, mistä seuraa, että spekula-   |   don luonteesta "
        "Grabiel Sanchez, yang bertani selama 120 tahun meninggal"},
       {"Malayalam",
        "അമൂർത്തചിത്രമായിരിക്കും.  ഛേ! ആ വീട്ടിലേക്ക്  അവളൊന്നിച്ച്  പോകേണ്ടതാ "
        "മൃഗങ്ങൾക്ക് എന്തെക്കിലും പറ്റിയാൽ മാത്രം ഞാനതു "
        "വെലക്ക് വേണമെങ്കിൽ തരാം. എന്തോ തരും?  പറ. "
        "എല്ലാം കഴിഞ്ഞ് സീനിയറിന്റെ അടുത്തു ചെന്ന് കാൽതൊട്ട"},
       {"Tamil",
        "பொருத்தமாகப் பாடினாள் நம் ஔவைப் பாட்டி. காவிரி "
        "உள்ளடக்கி  நிற்பது  விநோத  வார்த்தையின் அஃறிணை "
        "சூரிய   கிரஹண   சமயத்தில்   குருக்ஷேத்திரம்   செல்வது "
        "காலங்களில் வெளியே போகும்பொழுது, 'ஸார்', 'ஸார்',"},
       {"Telugu",
        "1892లో ఆమె 10వ సంవత్సరంలో గుంటూరు తాలూకా వేములాపాడు "
        "ఫండ్స్ చట్టము'నందు చేయబడెను. తరువాత క్రీ. శ. "
        "సంచారము చేయును.  మీరు ఇప్పుడే కాళకాలయమునకు "
        "ఎంతటి  సరళమైన  భాషలో  వ్రాశాడో  విశదమవుతుంది.   పైగా  ఆనాటి   భాష"},
       {"Thai",
        "อ้อ! กับนัง....แม่ยอดพระกลิ่น นั่นเอง ! หรับก็ย่อมจะรู้โดยชัดเจนว่า "
        "ถ้าตราบใดยังมีเรือปืนอยู่ใกล้ ๆ แล้ว  ตราบนั้น "
        "พระดำรินี้ ที่มีคตีทำกรวยหมากและธูปเทียน "
        "อันยานมีเรือเปนต้นฃ้ามยาก ฯ เพราะว่าแม่น้ำนั่นมีน้ำใสยิ่ง แม้เพียง"},
       {"Vietnamese",
        "vợ đến tai mụ hung thần Xăng-tô- mê-a. Mụ vô cùng "
        "chiếc xe con gấu chạy qua nhà. Nhưng thỉnh thoảng "
        "hòa hoãn với người Pháp để cho họ được dựng một ngôi nhà thờ nhỏ bằng "
        "Cặp câu đói súc tích mà sâu sắc, là lời chúc lời"}});

  for (const auto& p : kScriptText) {
    std::string normalized;
    EXPECT_TRUE(tesseract::NormalizeUTF8String(
        tesseract::UnicodeNormMode::kNFKC, tesseract::OCRNorm::kNormalize,
        tesseract::GraphemeNorm::kNormalize, p.second.c_str(), &normalized))
        << "Script=" << p.first << " text=" << p.second;
  }
}

TEST(NormstrngsTest, IsWhitespace) {
  // U+0020 is whitespace
  EXPECT_TRUE(IsWhitespace(' '));
  EXPECT_TRUE(IsWhitespace('\t'));
  EXPECT_TRUE(IsWhitespace('\r'));
  EXPECT_TRUE(IsWhitespace('\n'));
  // U+2000 through U+200A
  for (char32 ch = 0x2000; ch <= 0x200A; ++ch) {
    SCOPED_TRACE(absl::StrFormat("Failed at U+%x", ch));
    EXPECT_TRUE(IsWhitespace(ch));
  }
  // U+3000 is whitespace
  EXPECT_TRUE(IsWhitespace(0x3000));
  // ZWNBSP is not considered a space.
  EXPECT_FALSE(IsWhitespace(0xFEFF));
}

TEST(NormstrngsTest, SpanUTF8Whitespace) {
  EXPECT_EQ(4, SpanUTF8Whitespace(" \t\r\n"));
  EXPECT_EQ(4, SpanUTF8Whitespace(" \t\r\nabc"));
  EXPECT_EQ(0, SpanUTF8Whitespace("abc \t\r\nabc"));
  EXPECT_EQ(0, SpanUTF8Whitespace(""));
}

TEST(NormstrngsTest, SpanUTF8NotWhitespace) {
  const char kHinText[] = "पिताने विवाह";
  const char kKorText[] = "이는 것으로 다시 넣을";
  const char kMixedText[] = "والفكر 123 والصراع abc";

  EXPECT_EQ(0, SpanUTF8NotWhitespace(""));
  EXPECT_EQ(0, SpanUTF8NotWhitespace(" abc"));
  EXPECT_EQ(0, SpanUTF8NotWhitespace("\rabc"));
  EXPECT_EQ(0, SpanUTF8NotWhitespace("\tabc"));
  EXPECT_EQ(0, SpanUTF8NotWhitespace("\nabc"));
  EXPECT_EQ(3, SpanUTF8NotWhitespace("abc def"));
  EXPECT_EQ(18, SpanUTF8NotWhitespace(kHinText));
  EXPECT_EQ(6, SpanUTF8NotWhitespace(kKorText));
  EXPECT_EQ(12, SpanUTF8NotWhitespace(kMixedText));
}

#if defined(HAS_UNILIB_H)
// Test that the method clones the util/utf8/public/unilib definition of
// interchange validity.
TEST(NormstrngsTest, IsInterchangeValid) {
  const int32_t kMinUnicodeValue = 33;
  const int32_t kMaxUnicodeValue = 0x10FFFF;
  for (int32_t ch = kMinUnicodeValue; ch <= kMaxUnicodeValue; ++ch) {
    SCOPED_TRACE(absl::StrFormat("Failed at U+%x", ch));
    EXPECT_EQ(UniLib::IsInterchangeValid(ch), IsInterchangeValid(ch));
  }
}
#endif

#if defined(HAS_UNILIB_H)
// Test that the method clones the util/utf8/public/unilib definition of
// 7-bit ASCII interchange validity.
TEST(NormstrngsTest, IsInterchangeValid7BitAscii) {
  const int32_t kMinUnicodeValue = 33;
  const int32_t kMaxUnicodeValue = 0x10FFFF;
  for (int32_t ch = kMinUnicodeValue; ch <= kMaxUnicodeValue; ++ch) {
    SCOPED_TRACE(absl::StrFormat("Failed at U+%x", ch));
    std::string str = EncodeAsUTF8(ch);
    EXPECT_EQ(UniLib::IsInterchangeValid7BitAscii(str),
              IsInterchangeValid7BitAscii(ch));
  }
}
#endif

// Test that the method clones the util/utf8/public/unilib definition of
// fullwidth-halfwidth .
TEST(NormstrngsTest, FullwidthToHalfwidth) {
  // U+FF21 -> U+0041 (Latin capital letter A)
  EXPECT_EQ('A', FullwidthToHalfwidth(0xFF21));
  // U+FF05 -> U+0025 (percent sign)
  EXPECT_EQ('%', FullwidthToHalfwidth(0xFF05));
  // U+FFE6 -> U+20A9 (won sign)
  EXPECT_EQ(0x20A9, FullwidthToHalfwidth(0xFFE6));

#if defined(HAS_UNILIB_H)
  const int32_t kMinUnicodeValue = 33;
  const int32_t kMaxUnicodeValue = 0x10FFFF;
  for (int32_t ch = kMinUnicodeValue; ch <= kMaxUnicodeValue; ++ch) {
    if (!IsValidCodepoint(ch)) continue;
    SCOPED_TRACE(absl::StrFormat("Failed at U+%x", ch));
    std::string str = EncodeAsUTF8(ch);
    const std::string expected_half_str =
        UniLib::FullwidthToHalfwidth(str.c_str(), str.length(), true);
    EXPECT_EQ(expected_half_str, EncodeAsUTF8(FullwidthToHalfwidth(ch)));
  }
#endif
}

}  // namespace
}  // namespace tesseract
