void build(Solution &s)
{
    auto &tess = s.addProject("google.tesseract", "master");
    tess += Git("https://github.com/tesseract-ocr/tesseract", "", "{v}");

    auto cppstd = cpp17;

    auto &libtesseract = tess.addTarget<LibraryTarget>("libtesseract");
    {
        libtesseract.setChecks("libtesseract");

        libtesseract.ExportAllSymbols = true;
        libtesseract.PackageDefinitions = true;

        libtesseract += cppstd;

        libtesseract += "include/.*"_rr;
        libtesseract += "src/.*"_rr;
        libtesseract -= "src/lstm/.*\\.cc"_rr;
        libtesseract -= "src/training/.*"_rr;

        libtesseract -=
            "src/api/tesseractmain.cpp",
            "src/viewer/svpaint.cpp";

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

        if (libtesseract.getBuildSettings().TargetOS.Type != OSType::Windows)
        {
            libtesseract["src/arch/dotproductavx.cpp"].args.push_back("-mavx");
            libtesseract["src/arch/dotproductsse.cpp"].args.push_back("-msse4.1");
            libtesseract["src/arch/intsimdmatrixsse.cpp"].args.push_back("-msse4.1");
            libtesseract["src/arch/intsimdmatrixavx2.cpp"].args.push_back("-mavx2");
        }
        if (!win_or_mingw)
            libtesseract += "pthread"_slib;

        libtesseract.Public += "HAVE_CONFIG_H"_d;
        libtesseract.Public += "_SILENCE_STDEXT_HASH_DEPRECATION_WARNINGS=1"_d;
        libtesseract.Public += "HAVE_LIBARCHIVE"_d;
        libtesseract.Interface += sw::Shared, "TESS_IMPORTS"_d;
        libtesseract.Private += sw::Shared, "TESS_EXPORTS"_d;

        libtesseract.Public += "org.sw.demo.danbloomberg.leptonica"_dep;
        libtesseract.Public += "org.sw.demo.libarchive.libarchive"_dep;

        if (win_or_mingw)
        {
            libtesseract.Public += "ws2_32.lib"_slib;
            libtesseract.Protected += "NOMINMAX"_def;
        }

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
        tesseract += "src/api/tesseractmain.cpp";
        tesseract += libtesseract;
    }

    //
    auto &tessopt = tess.addStaticLibrary("tessopt");
    {
        tessopt += cppstd;
        tessopt += "src/training/tessopt.*"_rr;
        tessopt.Public += libtesseract;
    }

    //
    auto &common_training = tess.addStaticLibrary("common_training");
    {
        common_training += cppstd;
        common_training +=
            "src/training/commandlineflags.cpp",
            "src/training/commandlineflags.h",
            "src/training/commontraining.cpp",
            "src/training/commontraining.h",
            "src/training/ctc.cpp",
            "src/training/ctc.h",
            "src/training/errorcounter.cpp",
            "src/training/errorcounter.h",
            "src/training/intfeaturedist.cpp",
            "src/training/intfeaturedist.h",
            "src/training/intfeaturemap.cpp",
            "src/training/intfeaturemap.h",
            "src/training/mastertrainer.cpp",
            "src/training/mastertrainer.h",
            "src/training/networkbuilder.cpp",
            "src/training/networkbuilder.h",
            "src/training/sampleiterator.cpp",
            "src/training/sampleiterator.h",
            "src/training/trainingsampleset.cpp",
            "src/training/trainingsampleset.h";
        common_training.Public += tessopt;
    }

    //
    auto &unicharset_training = tess.addStaticLibrary("unicharset_training");
    {
        unicharset_training += cppstd;
        unicharset_training +=
            "src/training/fileio.*"_rr,
            "src/training/icuerrorcode.*"_rr,
            "src/training/icuerrorcode.h",
            "src/training/lang_model_helpers.*"_rr,
            "src/training/lstmtester.*"_rr,
            "src/training/lstmtrainer.*"_rr,
            "src/training/normstrngs.*"_rr,
            "src/training/unicharset_training_utils.*"_rr,
            "src/training/validat.*"_rr;
        unicharset_training.Public += common_training;
        unicharset_training.Public += "org.sw.demo.unicode.icu.i18n"_dep;
    }

    //
#define ADD_EXE(n, ...)               \
    auto &n = tess.addExecutable(#n); \
    n += cppstd;                       \
    n += "src/training/" #n ".*"_rr;  \
    n.Public += __VA_ARGS__;          \
    n

    ADD_EXE(ambiguous_words, libtesseract);
    ADD_EXE(classifier_tester, common_training);
    ADD_EXE(combine_lang_model, unicharset_training);
    ADD_EXE(combine_tessdata, libtesseract);
    ADD_EXE(cntraining, common_training);
    ADD_EXE(dawg2wordlist, libtesseract);
    ADD_EXE(mftraining, common_training) += "src/training/mergenf.*"_rr;
    ADD_EXE(shapeclustering, common_training);
    ADD_EXE(unicharset_extractor, unicharset_training);
    ADD_EXE(wordlist2dawg, libtesseract);
    ADD_EXE(lstmeval, unicharset_training);
    ADD_EXE(lstmtraining, unicharset_training);
    ADD_EXE(set_unicharset_properties, unicharset_training);
    ADD_EXE(merge_unicharsets, tessopt);

    //
    auto &pango_training = tess.addStaticLibrary("pango_training");
    {
        pango_training += cppstd;
        pango_training +=
            "src/training/boxchar.cpp",
            "src/training/boxchar.h",
            "src/training/ligature_table.cpp",
            "src/training/ligature_table.h",
            "src/training/pango_font_info.cpp",
            "src/training/pango_font_info.h",
            "src/training/stringrenderer.cpp",
            "src/training/stringrenderer.h",
            "src/training/tlog.cpp",
            "src/training/tlog.h"
            ;
        pango_training.Public += unicharset_training;
        pango_training.Public += "org.sw.demo.gnome.pango.pangocairo"_dep;
    }

    ADD_EXE(text2image, pango_training);
    {
        text2image += cppstd;
        text2image +=
            "src/training/degradeimage.cpp",
            "src/training/degradeimage.h",
            "src/training/icuerrorcode.h",
            "src/training/normstrngs.cpp",
            "src/training/normstrngs.h",
            "src/training/text2image.cpp",
            "src/training/util.h"
            ;
    }

    auto &test = tess.addDirectory("test");
    test.Scope = TargetScope::Test;

    auto add_test = [&test, &s, &cppstd, &libtesseract, &pango_training](const String &name) -> decltype(auto)
    {
        auto &t = test.addTarget<ExecutableTarget>(name);
        t += cppstd;
        t += path("unittest/" + name + "_test.cc");

        auto datadir = test.SourceDir / "tessdata_unittest";
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
        t += "org.sw.demo.google.abseil"_dep;

        if (t.getCompilerType() == CompilerType::MSVC)
            t.CompileOptions.push_back("-utf-8");

        libtesseract.addTest(t, name);

        return t;
    };

    Strings tests{
        "apiexample",
        "applybox",
        "baseapi",
        "bitvector",
        "cleanapi",
        "colpartition",
        "commandlineflags",
        "dawg",
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
        "lstm_recode",
        "lstm_squashed",
        "lstm",
        "lstmtrainer",
        "loadlang",
        "mastertrainer",
        "matrix",
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

