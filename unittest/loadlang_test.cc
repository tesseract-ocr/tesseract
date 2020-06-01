///////////////////////////////////////////////////////////////////////
// File:        loadlang_test.cc
// Description: Test loading of All languages and Scripts for Tesseract.
// Tests for All languages and scripts are Disabled by default.
// Force the disabled test to run if required by using the
// --gtest_also_run_disabled_tests argument. Author:      Shree Devi Kumar
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
///////////////////////////////////////////////////////////////////////

#include <memory>               // std::unique_ptr
#include <time.h>
#include <tesseract/baseapi.h>
#include "include_gunit.h"

namespace {

class QuickTest : public testing::Test {
 protected:
  virtual void SetUp() { start_time_ = time(nullptr); }
  virtual void TearDown() {
    const time_t end_time = time(nullptr);
    EXPECT_TRUE(end_time - start_time_ <= 25)
        << "The test took too long - "
        << ::testing::PrintToString(end_time - start_time_);
  }
  time_t start_time_;
};

void LangLoader(const char* lang, const char* tessdatadir) {
  std::unique_ptr<tesseract::TessBaseAPI> api(new tesseract::TessBaseAPI());
  ASSERT_FALSE(api->Init(tessdatadir, lang))
      << "Could not initialize tesseract for $lang.";
  api->End();
}

// For all languages

class LoadLanguage : public QuickTest,
                     public ::testing::WithParamInterface<const char*> {};

TEST_P(LoadLanguage, afr) { LangLoader("afr", GetParam()); }
TEST_P(LoadLanguage, amh) { LangLoader("amh", GetParam()); }
TEST_P(LoadLanguage, ara) { LangLoader("ara", GetParam()); }
TEST_P(LoadLanguage, asm) { LangLoader("asm", GetParam()); }
TEST_P(LoadLanguage, aze) { LangLoader("aze", GetParam()); }
TEST_P(LoadLanguage, aze_cyrl) { LangLoader("aze_cyrl", GetParam()); }
TEST_P(LoadLanguage, bel) { LangLoader("bel", GetParam()); }
TEST_P(LoadLanguage, ben) { LangLoader("ben", GetParam()); }
TEST_P(LoadLanguage, bod) { LangLoader("bod", GetParam()); }
TEST_P(LoadLanguage, bos) { LangLoader("bos", GetParam()); }
TEST_P(LoadLanguage, bre) { LangLoader("bre", GetParam()); }
TEST_P(LoadLanguage, bul) { LangLoader("bul", GetParam()); }
TEST_P(LoadLanguage, cat) { LangLoader("cat", GetParam()); }
TEST_P(LoadLanguage, ceb) { LangLoader("ceb", GetParam()); }
TEST_P(LoadLanguage, ces) { LangLoader("ces", GetParam()); }
TEST_P(LoadLanguage, chi_sim) { LangLoader("chi_sim", GetParam()); }
TEST_P(LoadLanguage, chi_sim_vert) { LangLoader("chi_sim_vert", GetParam()); }
TEST_P(LoadLanguage, chi_tra) { LangLoader("chi_tra", GetParam()); }
TEST_P(LoadLanguage, chi_tra_vert) { LangLoader("chi_tra_vert", GetParam()); }
TEST_P(LoadLanguage, chr) { LangLoader("chr", GetParam()); }
TEST_P(LoadLanguage, cos) { LangLoader("cos", GetParam()); }
TEST_P(LoadLanguage, cym) { LangLoader("cym", GetParam()); }
TEST_P(LoadLanguage, dan) { LangLoader("dan", GetParam()); }
TEST_P(LoadLanguage, deu) { LangLoader("deu", GetParam()); }
TEST_P(LoadLanguage, div) { LangLoader("div", GetParam()); }
TEST_P(LoadLanguage, dzo) { LangLoader("dzo", GetParam()); }
TEST_P(LoadLanguage, ell) { LangLoader("ell", GetParam()); }
TEST_P(LoadLanguage, eng) { LangLoader("eng", GetParam()); }
TEST_P(LoadLanguage, enm) { LangLoader("enm", GetParam()); }
TEST_P(LoadLanguage, epo) { LangLoader("epo", GetParam()); }
TEST_P(LoadLanguage, est) { LangLoader("est", GetParam()); }
TEST_P(LoadLanguage, eus) { LangLoader("eus", GetParam()); }
TEST_P(LoadLanguage, fao) { LangLoader("fao", GetParam()); }
TEST_P(LoadLanguage, fas) { LangLoader("fas", GetParam()); }
TEST_P(LoadLanguage, fil) { LangLoader("fil", GetParam()); }
TEST_P(LoadLanguage, fin) { LangLoader("fin", GetParam()); }
TEST_P(LoadLanguage, fra) { LangLoader("fra", GetParam()); }
TEST_P(LoadLanguage, frk) { LangLoader("frk", GetParam()); }
TEST_P(LoadLanguage, frm) { LangLoader("frm", GetParam()); }
TEST_P(LoadLanguage, fry) { LangLoader("fry", GetParam()); }
TEST_P(LoadLanguage, gla) { LangLoader("gla", GetParam()); }
TEST_P(LoadLanguage, gle) { LangLoader("gle", GetParam()); }
TEST_P(LoadLanguage, glg) { LangLoader("glg", GetParam()); }
TEST_P(LoadLanguage, grc) { LangLoader("grc", GetParam()); }
TEST_P(LoadLanguage, guj) { LangLoader("guj", GetParam()); }
TEST_P(LoadLanguage, hat) { LangLoader("hat", GetParam()); }
TEST_P(LoadLanguage, heb) { LangLoader("heb", GetParam()); }
TEST_P(LoadLanguage, hin) { LangLoader("hin", GetParam()); }
TEST_P(LoadLanguage, hrv) { LangLoader("hrv", GetParam()); }
TEST_P(LoadLanguage, hun) { LangLoader("hun", GetParam()); }
TEST_P(LoadLanguage, hye) { LangLoader("hye", GetParam()); }
TEST_P(LoadLanguage, iku) { LangLoader("iku", GetParam()); }
TEST_P(LoadLanguage, ind) { LangLoader("ind", GetParam()); }
TEST_P(LoadLanguage, isl) { LangLoader("isl", GetParam()); }
TEST_P(LoadLanguage, ita) { LangLoader("ita", GetParam()); }
TEST_P(LoadLanguage, ita_old) { LangLoader("ita_old", GetParam()); }
TEST_P(LoadLanguage, jav) { LangLoader("jav", GetParam()); }
TEST_P(LoadLanguage, jpn) { LangLoader("jpn", GetParam()); }
TEST_P(LoadLanguage, jpn_vert) { LangLoader("jpn_vert", GetParam()); }
TEST_P(LoadLanguage, kan) { LangLoader("kan", GetParam()); }
TEST_P(LoadLanguage, kat) { LangLoader("kat", GetParam()); }
TEST_P(LoadLanguage, kat_old) { LangLoader("kat_old", GetParam()); }
TEST_P(LoadLanguage, kaz) { LangLoader("kaz", GetParam()); }
TEST_P(LoadLanguage, khm) { LangLoader("khm", GetParam()); }
TEST_P(LoadLanguage, kir) { LangLoader("kir", GetParam()); }
//  TEST_P(LoadLanguage, kmr) {LangLoader("kmr" , GetParam());}
TEST_P(LoadLanguage, kor) { LangLoader("kor", GetParam()); }
TEST_P(LoadLanguage, kor_vert) { LangLoader("kor_vert", GetParam()); }
TEST_P(LoadLanguage, lao) { LangLoader("lao", GetParam()); }
TEST_P(LoadLanguage, lat) { LangLoader("lat", GetParam()); }
TEST_P(LoadLanguage, lav) { LangLoader("lav", GetParam()); }
TEST_P(LoadLanguage, lit) { LangLoader("lit", GetParam()); }
TEST_P(LoadLanguage, ltz) { LangLoader("ltz", GetParam()); }
TEST_P(LoadLanguage, mal) { LangLoader("mal", GetParam()); }
TEST_P(LoadLanguage, mar) { LangLoader("mar", GetParam()); }
TEST_P(LoadLanguage, mkd) { LangLoader("mkd", GetParam()); }
TEST_P(LoadLanguage, mlt) { LangLoader("mlt", GetParam()); }
TEST_P(LoadLanguage, mon) { LangLoader("mon", GetParam()); }
TEST_P(LoadLanguage, mri) { LangLoader("mri", GetParam()); }
TEST_P(LoadLanguage, msa) { LangLoader("msa", GetParam()); }
TEST_P(LoadLanguage, mya) { LangLoader("mya", GetParam()); }
TEST_P(LoadLanguage, nep) { LangLoader("nep", GetParam()); }
TEST_P(LoadLanguage, nld) { LangLoader("nld", GetParam()); }
TEST_P(LoadLanguage, nor) { LangLoader("nor", GetParam()); }
TEST_P(LoadLanguage, oci) { LangLoader("oci", GetParam()); }
TEST_P(LoadLanguage, ori) { LangLoader("ori", GetParam()); }
TEST_P(LoadLanguage, osd) { LangLoader("osd", GetParam()); }
TEST_P(LoadLanguage, pan) { LangLoader("pan", GetParam()); }
TEST_P(LoadLanguage, pol) { LangLoader("pol", GetParam()); }
TEST_P(LoadLanguage, por) { LangLoader("por", GetParam()); }
TEST_P(LoadLanguage, pus) { LangLoader("pus", GetParam()); }
TEST_P(LoadLanguage, que) { LangLoader("que", GetParam()); }
TEST_P(LoadLanguage, ron) { LangLoader("ron", GetParam()); }
TEST_P(LoadLanguage, rus) { LangLoader("rus", GetParam()); }
TEST_P(LoadLanguage, san) { LangLoader("san", GetParam()); }
TEST_P(LoadLanguage, sin) { LangLoader("sin", GetParam()); }
TEST_P(LoadLanguage, slk) { LangLoader("slk", GetParam()); }
TEST_P(LoadLanguage, slv) { LangLoader("slv", GetParam()); }
TEST_P(LoadLanguage, snd) { LangLoader("snd", GetParam()); }
TEST_P(LoadLanguage, spa) { LangLoader("spa", GetParam()); }
TEST_P(LoadLanguage, spa_old) { LangLoader("spa_old", GetParam()); }
TEST_P(LoadLanguage, sqi) { LangLoader("sqi", GetParam()); }
TEST_P(LoadLanguage, srp) { LangLoader("srp", GetParam()); }
TEST_P(LoadLanguage, srp_latn) { LangLoader("srp_latn", GetParam()); }
TEST_P(LoadLanguage, sun) { LangLoader("sun", GetParam()); }
TEST_P(LoadLanguage, swa) { LangLoader("swa", GetParam()); }
TEST_P(LoadLanguage, swe) { LangLoader("swe", GetParam()); }
TEST_P(LoadLanguage, syr) { LangLoader("syr", GetParam()); }
TEST_P(LoadLanguage, tam) { LangLoader("tam", GetParam()); }
TEST_P(LoadLanguage, tat) { LangLoader("tat", GetParam()); }
TEST_P(LoadLanguage, tel) { LangLoader("tel", GetParam()); }
TEST_P(LoadLanguage, tgk) { LangLoader("tgk", GetParam()); }
TEST_P(LoadLanguage, tha) { LangLoader("tha", GetParam()); }
TEST_P(LoadLanguage, tir) { LangLoader("tir", GetParam()); }
TEST_P(LoadLanguage, ton) { LangLoader("ton", GetParam()); }
TEST_P(LoadLanguage, tur) { LangLoader("tur", GetParam()); }
TEST_P(LoadLanguage, uig) { LangLoader("uig", GetParam()); }
TEST_P(LoadLanguage, ukr) { LangLoader("ukr", GetParam()); }
TEST_P(LoadLanguage, urd) { LangLoader("urd", GetParam()); }
TEST_P(LoadLanguage, uzb) { LangLoader("uzb", GetParam()); }
TEST_P(LoadLanguage, uzb_cyrl) { LangLoader("uzb_cyrl", GetParam()); }
TEST_P(LoadLanguage, vie) { LangLoader("vie", GetParam()); }
TEST_P(LoadLanguage, yid) { LangLoader("yid", GetParam()); }
TEST_P(LoadLanguage, yor) { LangLoader("yor", GetParam()); }

INSTANTIATE_TEST_CASE_P(DISABLED_Tessdata_fast, LoadLanguage,
                        ::testing::Values(TESSDATA_DIR "_fast"));
INSTANTIATE_TEST_CASE_P(DISABLED_Tessdata_best, LoadLanguage,
                        ::testing::Values(TESSDATA_DIR "_best"));
INSTANTIATE_TEST_CASE_P(DISABLED_Tessdata, LoadLanguage,
                        ::testing::Values(TESSDATA_DIR));

// For all scripts

class LoadScript : public QuickTest,
                   public ::testing::WithParamInterface<const char*> {};

TEST_P(LoadScript, Arabic) { LangLoader("script/Arabic", GetParam()); }
TEST_P(LoadScript, Armenian) { LangLoader("script/Armenian", GetParam()); }
TEST_P(LoadScript, Bengali) { LangLoader("script/Bengali", GetParam()); }
TEST_P(LoadScript, Canadian_Aboriginal) {
  LangLoader("script/Canadian_Aboriginal", GetParam());
}
TEST_P(LoadScript, Cherokee) { LangLoader("script/Cherokee", GetParam()); }
TEST_P(LoadScript, Cyrillic) { LangLoader("script/Cyrillic", GetParam()); }
TEST_P(LoadScript, Devanagari) { LangLoader("script/Devanagari", GetParam()); }
TEST_P(LoadScript, Ethiopic) { LangLoader("script/Ethiopic", GetParam()); }
TEST_P(LoadScript, Fraktur) { LangLoader("script/Fraktur", GetParam()); }
TEST_P(LoadScript, Georgian) { LangLoader("script/Georgian", GetParam()); }
TEST_P(LoadScript, Greek) { LangLoader("script/Greek", GetParam()); }
TEST_P(LoadScript, Gujarati) { LangLoader("script/Gujarati", GetParam()); }
TEST_P(LoadScript, Gurmukhi) { LangLoader("script/Gurmukhi", GetParam()); }
TEST_P(LoadScript, HanS) { LangLoader("script/HanS", GetParam()); }
TEST_P(LoadScript, HanS_vert) { LangLoader("script/HanS_vert", GetParam()); }
TEST_P(LoadScript, HanT) { LangLoader("script/HanT", GetParam()); }
TEST_P(LoadScript, HanT_vert) { LangLoader("script/HanT_vert", GetParam()); }
TEST_P(LoadScript, Hangul) { LangLoader("script/Hangul", GetParam()); }
TEST_P(LoadScript, Hangul_vert) {
  LangLoader("script/Hangul_vert", GetParam());
}
TEST_P(LoadScript, Hebrew) { LangLoader("script/Hebrew", GetParam()); }
TEST_P(LoadScript, Japanese) { LangLoader("script/Japanese", GetParam()); }
TEST_P(LoadScript, Japanese_vert) {
  LangLoader("script/Japanese_vert", GetParam());
}
TEST_P(LoadScript, Kannada) { LangLoader("script/Kannada", GetParam()); }
TEST_P(LoadScript, Khmer) { LangLoader("script/Khmer", GetParam()); }
TEST_P(LoadScript, Lao) { LangLoader("script/Lao", GetParam()); }
TEST_P(LoadScript, Latin) { LangLoader("script/Latin", GetParam()); }
TEST_P(LoadScript, Malayalam) { LangLoader("script/Malayalam", GetParam()); }
TEST_P(LoadScript, Myanmar) { LangLoader("script/Myanmar", GetParam()); }
TEST_P(LoadScript, Oriya) { LangLoader("script/Oriya", GetParam()); }
TEST_P(LoadScript, Sinhala) { LangLoader("script/Sinhala", GetParam()); }
TEST_P(LoadScript, Syriac) { LangLoader("script/Syriac", GetParam()); }
TEST_P(LoadScript, Tamil) { LangLoader("script/Tamil", GetParam()); }
TEST_P(LoadScript, Telugu) { LangLoader("script/Telugu", GetParam()); }
TEST_P(LoadScript, Thaana) { LangLoader("script/Thaana", GetParam()); }
TEST_P(LoadScript, Thai) { LangLoader("script/Thai", GetParam()); }
TEST_P(LoadScript, Tibetan) { LangLoader("script/Tibetan", GetParam()); }
TEST_P(LoadScript, Vietnamese) { LangLoader("script/Vietnamese", GetParam()); }

INSTANTIATE_TEST_CASE_P(DISABLED_Tessdata_fast, LoadScript,
                        ::testing::Values(TESSDATA_DIR "_fast"));
INSTANTIATE_TEST_CASE_P(DISABLED_Tessdata_best, LoadScript,
                        ::testing::Values(TESSDATA_DIR "_best"));
INSTANTIATE_TEST_CASE_P(DISABLED_Tessdata, LoadScript,
                        ::testing::Values(TESSDATA_DIR));

class LoadLang : public QuickTest {};

// Test Load of English here, as the parameterized tests are disabled by
// default.
TEST_F(LoadLang, engFast) { LangLoader("eng", TESSDATA_DIR "_fast"); }
TEST_F(LoadLang, engBest) { LangLoader("eng", TESSDATA_DIR "_best"); }
TEST_F(LoadLang, engBestInt) { LangLoader("eng", TESSDATA_DIR); }

// Use class LoadLang for languages which are NOT there in all three repos
TEST_F(LoadLang, kmrFast) { LangLoader("kmr", TESSDATA_DIR "_fast"); }
TEST_F(LoadLang, kmrBest) { LangLoader("kmr", TESSDATA_DIR "_best"); }
//  TEST_F(LoadLang, kmrBestInt) {LangLoader("kmr" , TESSDATA_DIR);}

}  // namespace
