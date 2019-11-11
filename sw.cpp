void build(Solution &s)
{
    auto &tess = s.addProject("google.tesseract", "master");
    tess += Git("https://github.com/tesseract-ocr/tesseract", "", "{v}");

    auto &libtesseract = tess.addTarget<LibraryTarget>("libtesseract");
    {
        libtesseract.setChecks("libtesseract");

        libtesseract.ExportAllSymbols = true;
        libtesseract.PackageDefinitions = true;
        libtesseract +=
            "src/api/.*\\.cpp"_rr,
            "src/api/.*\\.h"_rr,
            "src/api/tess_version.h.in",
            "src/arch/.*\\.cpp"_rr,
            "src/arch/.*\\.h"_rr,
            "src/ccmain/.*\\.cpp"_rr,
            "src/ccmain/.*\\.h"_rr,
            "src/ccstruct/.*\\.cpp"_rr,
            "src/ccstruct/.*\\.h"_rr,
            "src/ccutil/.*\\.cpp"_rr,
            "src/ccutil/.*\\.h"_rr,
            "src/classify/.*\\.cpp"_rr,
            "src/classify/.*\\.h"_rr,
            "src/cutil/.*\\.cpp"_rr,
            "src/cutil/.*\\.h"_rr,
            "src/dict/.*\\.cpp"_rr,
            "src/dict/.*\\.h"_rr,
            "src/lstm/.*\\.cpp"_rr,
            "src/lstm/.*\\.h"_rr,
            "src/opencl/.*\\.cpp"_rr,
            "src/opencl/.*\\.h"_rr,
            "src/textord/.*\\.cpp"_rr,
            "src/textord/.*\\.h"_rr,
            "src/viewer/.*\\.cpp"_rr,
            "src/viewer/.*\\.h"_rr,
            "src/wordrec/.*\\.cpp"_rr,
            "src/wordrec/.*\\.h"_rr;

        libtesseract += "src/training/.*\\.h"_rr;

        libtesseract -=
            "src/api/tesseractmain.cpp",
            "src/viewer/svpaint.cpp";

        libtesseract.Public +=
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

            libtesseract -=
                "src/arch/dotproductfma.cpp";
        }

        libtesseract.Public += "HAVE_CONFIG_H"_d;
        libtesseract.Public += "_SILENCE_STDEXT_HASH_DEPRECATION_WARNINGS=1"_d;
        libtesseract.Public += "HAVE_LIBARCHIVE"_d;
        libtesseract.Interface += sw::Shared, "TESS_IMPORTS"_d;
        libtesseract.Private += sw::Shared, "TESS_EXPORTS"_d;

        libtesseract.Public += "org.sw.demo.danbloomberg.leptonica"_dep;
        libtesseract.Public += "org.sw.demo.libarchive.libarchive"_dep;

        if (libtesseract.getBuildSettings().TargetOS.Type == OSType::Windows)
        {
            libtesseract.Public += "ws2_32.lib"_slib;
            libtesseract.Protected += "NOMINMAX"_def;
        }

        libtesseract.Variables["TESSERACT_MAJOR_VERSION"] = libtesseract.Variables["PACKAGE_MAJOR_VERSION"];
        libtesseract.Variables["TESSERACT_MINOR_VERSION"] = libtesseract.Variables["PACKAGE_MINOR_VERSION"];
        libtesseract.Variables["TESSERACT_MICRO_VERSION"] = libtesseract.Variables["PACKAGE_PATCH_VERSION"];
        libtesseract.Variables["TESSERACT_VERSION_STR"] = "master";
        libtesseract.configureFile("src/api/tess_version.h.in", "tess_version.h");

        // install
        if (!libtesseract.DryRun)
        {
            const Files files
            {
                // from api/makefile.am
                "src/api/apitypes.h",
                "src/api/baseapi.h",
                "src/api/capi.h",
                "src/api/renderer.h",
                "tess_version.h",

                //from ccmain/makefile.am
                "src/ccmain/thresholder.h",
                "src/ccmain/ltrresultiterator.h",
                "src/ccmain/pageiterator.h",
                "src/ccmain/resultiterator.h",
                "src/ccmain/osdetect.h",

                //from ccstruct/makefile.am
                "src/ccstruct/publictypes.h",

                //from ccutil/makefile.am
                "src/ccutil/genericvector.h",
                "src/ccutil/helpers.h",
                "src/ccutil/ocrclass.h",
                "src/ccutil/platform.h",
                "src/ccutil/serialis.h",
                "src/ccutil/strngs.h",
                "src/ccutil/unichar.h",
            };

            auto d = libtesseract.BinaryDir / "tesseract";
            fs::create_directories(d);
            for (auto f : files)
            {
                libtesseract.check_absolute(f);
                fs::copy_file(f, d / f.filename(), fs::copy_options::update_existing);
            }
        }
    }

    //
    auto &tesseract = tess.addExecutable("tesseract");
    tesseract += "src/api/tesseractmain.cpp";
    tesseract += libtesseract;

    //
    auto &tessopt = tess.addStaticLibrary("tessopt");
    tessopt += "src/training/tessopt.*"_rr;
    tessopt.Public += libtesseract;

    //
    auto &common_training = tess.addStaticLibrary("common_training");
    common_training +=
        "src/training/commandlineflags.cpp",
        "src/training/commandlineflags.h",
        "src/training/commontraining.cpp",
        "src/training/commontraining.h",
        "src/training/errorcounter.cpp",
        "src/training/errorcounter.h",
        "src/training/mastertrainer.cpp",
        "src/training/mastertrainer.h";
    common_training.Public += tessopt;

    //
    auto &unicharset_training = tess.addStaticLibrary("unicharset_training");
    unicharset_training +=
        "src/training/fileio.*"_rr,
        "src/training/icuerrorcode.*"_rr,
        "src/training/icuerrorcode.h",
        "src/training/lang_model_helpers.*"_rr,
        "src/training/lstmtester.*"_rr,
        "src/training/normstrngs.*"_rr,
        "src/training/unicharset_training_utils.*"_rr,
        "src/training/validat.*"_rr;
    unicharset_training.Public += common_training;
    unicharset_training.Public += "org.sw.demo.unicode.icu.i18n"_dep;

    //
#define ADD_EXE(n, ...)               \
    auto &n = tess.addExecutable(#n); \
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

    ADD_EXE(text2image, unicharset_training);
    text2image +=
        "src/training/boxchar.cpp",
        "src/training/boxchar.h",
        "src/training/degradeimage.cpp",
        "src/training/degradeimage.h",
        "src/training/icuerrorcode.h",
        "src/training/ligature_table.cpp",
        "src/training/ligature_table.h",
        "src/training/normstrngs.cpp",
        "src/training/normstrngs.h",
        "src/training/pango_font_info.cpp",
        "src/training/pango_font_info.h",
        "src/training/stringrenderer.cpp",
        "src/training/stringrenderer.h",
        "src/training/text2image.cpp",
        "src/training/tlog.cpp",
        "src/training/tlog.h",
        "src/training/util.h";
    text2image.Public += "org.sw.demo.gnome.pango.pangocairo"_dep;
}

void check(Checker &c)
{
    auto &s = c.addSet("libtesseract");
    s.checkFunctionExists("getline");
    s.checkIncludeExists("dlfcn.h");
    s.checkIncludeExists("inttypes.h");
    s.checkIncludeExists("limits.h");
    s.checkIncludeExists("malloc.h");
    s.checkIncludeExists("memory.h");
    s.checkIncludeExists("stdbool.h");
    s.checkIncludeExists("stdint.h");
    s.checkIncludeExists("stdlib.h");
    s.checkIncludeExists("string.h");
    s.checkIncludeExists("sys/ipc.h");
    s.checkIncludeExists("sys/shm.h");
    s.checkIncludeExists("sys/stat.h");
    s.checkIncludeExists("sys/types.h");
    s.checkIncludeExists("sys/wait.h");
    s.checkIncludeExists("tiffio.h");
    s.checkIncludeExists("unistd.h");
    s.checkTypeSize("long long int");
    s.checkTypeSize("mbstate_t");
    s.checkTypeSize("off_t");
    s.checkTypeSize("size_t");
    s.checkTypeSize("void *");
    s.checkTypeSize("wchar_t");
    s.checkTypeSize("_Bool");
    {
        auto &c = s.checkSymbolExists("snprintf");
        c.Parameters.Includes.push_back("stdio.h");
    }
}
