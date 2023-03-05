void build(Solution &s)
{
    auto &tess = s.addProject("google.tesseract", "main");
    tess += Git("https://github.com/tesseract-ocr/tesseract", "", "{v}");

    auto cppstd = cpp17;

    auto &libtesseract = tess.addTarget<LibraryTarget>("libtesseract");
    {
        libtesseract.setChecks("libtesseract");

        libtesseract.PackageDefinitions = true;

        libtesseract += cppstd;

        libtesseract += "TESS_API"_api;
        libtesseract += "include/.*"_rr;
        libtesseract += "src/.+/.*"_rr;
        libtesseract -= "src/lstm/.*\\.cc"_rr;
        libtesseract -= "src/training/.*"_rr;

        libtesseract.Public += "include"_idir;
        libtesseract.Protected +=
            "src/opencl"_id,
            "src/ccmain"_id,
            "src/api"_id,
            "src/dict"_id,
            "src/viewer"_id,
            "src/wordrec"_id,
            "src/ccstruct"_id,
            "src/cutil"_id,
            "src/textord"_id,
            "src/ccutil"_id,
            "src/lstm"_id,
            "src/classify"_id,
            "src/arch"_id,
            "src/training"_id;

        if (libtesseract.getCompilerType() == CompilerType::MSVC ||
            libtesseract.getCompilerType() == CompilerType::ClangCl)
        {
            libtesseract += "__SSE4_1__"_def;
            libtesseract.CompileOptions.push_back("-arch:AVX2");

            // openmp
            //if (libtesseract.getOptions()["openmp"] == "true")
            if (0)
            {
                if (libtesseract.getCompilerType() == CompilerType::MSVC)
                    libtesseract.CompileOptions.push_back("-openmp");
                else
                    libtesseract.CompileOptions.push_back("-fopenmp");
                libtesseract += "_OPENMP=201107"_def;
                if (libtesseract.getBuildSettings().Native.ConfigurationType == ConfigurationType::Debug)
                    libtesseract += "vcompd.lib"_slib;
                else
                    libtesseract += "vcomp.lib"_slib;
            }
        }

        auto win_or_mingw =
            libtesseract.getBuildSettings().TargetOS.Type == OSType::Windows ||
            libtesseract.getBuildSettings().TargetOS.Type == OSType::Mingw
            ;

        // check fma flags
        libtesseract -= "src/arch/dotproductfma.cpp";
        // check arch (arm)
        libtesseract -= "src/arch/dotproductneon.cpp";

        if (libtesseract.getBuildSettings().TargetOS.Type != OSType::Windows &&
            libtesseract.getBuildSettings().TargetOS.Arch != ArchType::aarch64)
        {
            libtesseract["src/arch/dotproductavx.cpp"].args.push_back("-mavx");
            libtesseract["src/arch/dotproductavx512.cpp"].args.push_back("-mavx512f");
            libtesseract["src/arch/dotproductsse.cpp"].args.push_back("-msse4.1");
            libtesseract["src/arch/intsimdmatrixsse.cpp"].args.push_back("-msse4.1");
            libtesseract["src/arch/intsimdmatrixavx2.cpp"].args.push_back("-mavx2");
        }
        if (!win_or_mingw)
        {
#if SW_MODULE_ABI_VERSION > 29
            if (!libtesseract.getBuildSettings().TargetOS.Android)
#endif
                libtesseract += "pthread"_slib;
        }
        if (libtesseract.getBuildSettings().TargetOS.Arch == ArchType::aarch64)
        {
            libtesseract += "src/arch/dotproductneon.cpp";
        }

        libtesseract.Public += "HAVE_CONFIG_H"_d;
        libtesseract.Public += "_SILENCE_STDEXT_HASH_DEPRECATION_WARNINGS=1"_d;
        libtesseract.Public += "HAVE_LIBARCHIVE"_d;

        libtesseract.Public += "org.sw.demo.danbloomberg.leptonica"_dep;
        libtesseract.Public += "org.sw.demo.libarchive.libarchive"_dep;

        if (win_or_mingw)
        {
            libtesseract.Public += "ws2_32.lib"_slib;
            libtesseract.Protected += "NOMINMAX"_def;
        }

        if (libtesseract.getCompilerType() == CompilerType::MSVC)
            libtesseract.Protected.CompileOptions.push_back("-utf-8");

        libtesseract.Variables["TESSERACT_MAJOR_VERSION"] = libtesseract.Variables["PACKAGE_MAJOR_VERSION"];
        libtesseract.Variables["TESSERACT_MINOR_VERSION"] = libtesseract.Variables["PACKAGE_MINOR_VERSION"];
        libtesseract.Variables["TESSERACT_MICRO_VERSION"] = libtesseract.Variables["PACKAGE_PATCH_VERSION"];
        libtesseract.Variables["TESSERACT_VERSION_STR"] = "master";
        libtesseract.configureFile("include/tesseract/version.h.in", "tesseract/version.h");
    }

    //
    auto &tesseract = tess.addExecutable("tesseract");
    {
        tesseract += cppstd;
        tesseract += "src/tesseract.cpp";
        tesseract += libtesseract;
    }

    auto &svpaint = tess.addExecutable("svpaint");
    {
        svpaint += cppstd;
        svpaint += "src/svpaint.cpp";
        svpaint += libtesseract;
    }

    auto &training = tess.addDirectory("training");

    //
    auto &common_training = training.addLibrary("common_training");
    {
        common_training += "TESS_COMMON_TRAINING_API"_api;
        common_training += cppstd;
        common_training += "src/training/common/.*"_rr;
        common_training.Public += "src/training/common"_idir;
        common_training.Public += libtesseract;
    }

    //
    auto &unicharset_training = training.addLibrary("unicharset_training");
    {
        unicharset_training += "TESS_UNICHARSET_TRAINING_API"_api;
        unicharset_training += cppstd;
        unicharset_training += "src/training/unicharset/.*"_rr;
        unicharset_training.Public += "src/training/unicharset"_idir;
        unicharset_training.Public += common_training;
        unicharset_training.Public += "org.sw.demo.unicode.icu.i18n"_dep;

        auto win_or_mingw =
          unicharset_training.getBuildSettings().TargetOS.Type == OSType::Windows ||
          unicharset_training.getBuildSettings().TargetOS.Type == OSType::Mingw
          ;
        if (!win_or_mingw)
          unicharset_training += "pthread"_slib;
    }

    //
#define ADD_EXE(n, ...)                     \
    auto &n = training.addExecutable(#n);   \
    n += cppstd;                            \
    n += "src/training/" #n ".*"_rr;        \
    n.Public += __VA_ARGS__;                \
    n

    ADD_EXE(ambiguous_words, common_training);
    ADD_EXE(classifier_tester, common_training);
    ADD_EXE(combine_lang_model, unicharset_training);
    ADD_EXE(combine_tessdata, common_training);
    ADD_EXE(cntraining, common_training);
    ADD_EXE(dawg2wordlist, common_training);
    ADD_EXE(mftraining, common_training) += "src/training/mergenf.*"_rr;
    ADD_EXE(shapeclustering, common_training);
    ADD_EXE(unicharset_extractor, unicharset_training);
    ADD_EXE(wordlist2dawg, common_training);
    ADD_EXE(lstmeval, unicharset_training);
    ADD_EXE(lstmtraining, unicharset_training);
    ADD_EXE(set_unicharset_properties, unicharset_training);
    ADD_EXE(merge_unicharsets, common_training);

    //
    auto &pango_training = training.addLibrary("pango_training");
    {
        pango_training += "TESS_PANGO_TRAINING_API"_api;
        pango_training += cppstd;
        pango_training += "src/training/pango/.*"_rr;
        pango_training.Public += "src/training/pango"_idir;
        pango_training.Public += unicharset_training;
        pango_training.Public += "org.sw.demo.gnome.pango.pangocairo"_dep;
    }

    ADD_EXE(text2image, pango_training);
    {
        text2image += cppstd;
        text2image +=
            "src/training/degradeimage.cpp",
            "src/training/degradeimage.h",
            "src/training/text2image.cpp"
            ;
    }

    if (!s.getExternalVariables()["with-tests"])
        return;

    // tests
    {
        auto &test = tess.addDirectory("test");
        test.Scope = TargetScope::Test;

        String skipped_tests_str;
        if (s.getExternalVariables()["skip-tests"])
            skipped_tests_str = s.getExternalVariables()["skip-tests"].getValue();
        auto skipped_tests = split_string(skipped_tests_str, ",");

        auto add_test = [&test, &s, &cppstd, &libtesseract, &pango_training, &skipped_tests](const String &name) -> decltype(auto)
        {
            auto &t = test.addTarget<ExecutableTarget>(name);
            t += cppstd;
            t += FileRegex("unittest", name + "_test.*", false);
            t += "unittest"_idir;

            t += "SW_TESTING"_def;

            auto datadir = test.SourceDir / "tessdata_unittest";
            if (s.getExternalVariables()["test-data-dir"])
                datadir = fs::current_path() / s.getExternalVariables()["test-data-dir"].getValue();
            t += Definition("TESSBIN_DIR=\"" + ""s + "\"");

            t += Definition("TESTING_DIR=\"" + to_printable_string(normalize_path(test.SourceDir / "test/testing")) + "\"");
            t += Definition("TESTDATA_DIR=\"" + to_printable_string(normalize_path(test.SourceDir / "test/testdata")) + "\"");

            t += Definition("LANGDATA_DIR=\"" + to_printable_string(normalize_path(datadir / "langdata_lstm")) + "\"");
            t += Definition("TESSDATA_DIR=\"" + to_printable_string(normalize_path(datadir / "tessdata")) + "\"");
            t += Definition("TESSDATA_BEST_DIR=\"" + to_printable_string(normalize_path(datadir / "tessdata_best")) + "\"");

            // we push all deps to all tests simplify things
            t += pango_training;
            t += "org.sw.demo.google.googletest.gmock.main"_dep;
            t += "org.sw.demo.google.googletest.gtest.main"_dep;

            if (t.getCompilerType() == CompilerType::MSVC)
                t.CompileOptions.push_back("-utf-8");

            auto win_or_mingw =
              t.getBuildSettings().TargetOS.Type == OSType::Windows ||
              t.getBuildSettings().TargetOS.Type == OSType::Mingw
              ;
            if (!win_or_mingw)
              t += "pthread"_slib;

            auto tst = libtesseract.addTest(t, name);
            for (auto &st : skipped_tests)
            {
                std::regex r(st);
                if (std::regex_match(name, r))
                {
                    tst.skip(true);
                    break;
                }
            }

            return t;
        };

        Strings tests
        {
            "apiexample",
            "applybox",
            "baseapi",
            "baseapi_thread",
            "bitvector",
            "capiexample",
            "capiexample_c",
            "cleanapi",
            "colpartition",
            "commandlineflags",
            "denorm",
            "equationdetect",
            "fileio",
            "heap",
            "imagedata",
            "indexmapbidi",
            "intfeaturemap",
            "intsimdmatrix",
            "lang_model",
            "layout",
            "ligature_table",
            "linlsq",
            "list",
            "lstm_recode",
            "lstm_squashed",
            "lstm",
            "lstmtrainer",
            "loadlang",
            "mastertrainer",
            "matrix",
            "networkio",
            "normstrngs",
            "nthitem",
            "osd",
            "pagesegmode",
            "pango_font_info",
            "paragraphs",
            "params_model",
            "progress",
            "qrsequence",
            "recodebeam",
            "rect",
            "resultiterator",
            "scanutils",
            "shapetable",
            "stats",
            "stringrenderer",
            "stridemap",
            "tablefind",
            "tablerecog",
            "tabvector",
            "textlineprojection",
            "tfile",
            "unichar",
            "unicharcompress",
            "unicharset",
            "validate_grapheme",
            "validate_indic",
            "validate_khmer",
            "validate_myanmar",
            "validator",
        };
        for (auto t : tests)
            add_test(t);
        auto &dt = add_test("dawg");
        dt += Definition("wordlist2dawg_prog=\"" + to_printable_string(normalize_path(wordlist2dawg.getOutputFile())) + "\"");
        dt += Definition("dawg2wordlist_prog=\"" + to_printable_string(normalize_path(dawg2wordlist.getOutputFile())) + "\"");

        auto &tw = add_test("tatweel");
        tw += "unittest/util/.*"_rr;
        tw += "unittest/third_party/.*"_rr;
        tw -= "unittest/third_party/googletest/.*"_rr;
    }
}

void check(Checker &c)
{
    auto &s = c.addSet("libtesseract");
    s.checkFunctionExists("getline");
    s.checkIncludeExists("dlfcn.h");
    s.checkIncludeExists("inttypes.h");
    s.checkIncludeExists("memory.h");
    s.checkIncludeExists("stdint.h");
    s.checkIncludeExists("stdlib.h");
    s.checkIncludeExists("string.h");
    s.checkIncludeExists("sys/stat.h");
    s.checkIncludeExists("sys/types.h");
    s.checkIncludeExists("tiffio.h");
    s.checkIncludeExists("unistd.h");
    s.checkTypeSize("long long int");
    s.checkTypeSize("size_t");
    s.checkTypeSize("void *");
    s.checkTypeSize("wchar_t");
    {
        auto &c = s.checkSymbolExists("snprintf");
        c.Parameters.Includes.push_back("stdio.h");
    }
}

