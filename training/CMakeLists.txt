#
# tesseract
#

########################################
# LIBRARY tessopt
########################################

add_library             (tessopt tessopt.cpp tessopt.h)
project_group           (tessopt "Training Tools")


########################################
# LIBRARY training
########################################

set(training_src
    boxchar.cpp commandlineflags.cpp commontraining.cpp degradeimage.cpp
    fileio.cpp ligature_table.cpp normstrngs.cpp pango_font_info.cpp
    stringrenderer.cpp tlog.cpp unicharset_training_utils.cpp
)
set(training_hdr
    boxchar.h commandlineflags.h commontraining.h degradeimage.h
    fileio.h icuerrorcode.h ligature_table.h normstrngs.h
    mergenf.h pango_font_info.h stringrenderer.h
    tessopt.h tlog.h unicharset_training_utils.h util.h
)
add_library             (training ${training_src} ${training_hdr})
#target_link_libraries   (training port)
project_group           (training "Training Tools")


########################################
# EXECUTABLE ambiguous_words
########################################

add_executable          (ambiguous_words ambiguous_words.cpp)
target_link_libraries   (ambiguous_words tesseract training tessopt)
project_group           (ambiguous_words "Training Tools")


########################################
# EXECUTABLE classifier_tester
########################################

add_executable          (classifier_tester classifier_tester.cpp)
target_link_libraries   (classifier_tester tesseract training tessopt)
project_group           (classifier_tester "Training Tools")


########################################
# EXECUTABLE combine_tessdata
########################################

add_executable          (combine_tessdata combine_tessdata.cpp)
target_link_libraries   (combine_tessdata tesseract)
project_group           (combine_tessdata "Training Tools")


########################################
# EXECUTABLE cntraining
########################################

add_executable          (cntraining cntraining.cpp)
target_link_libraries   (cntraining tesseract training tessopt)
project_group           (cntraining "Training Tools")


########################################
# EXECUTABLE dawg2wordlist
########################################

add_executable          (dawg2wordlist dawg2wordlist.cpp)
target_link_libraries   (dawg2wordlist tesseract training tessopt)
project_group           (dawg2wordlist "Training Tools")


########################################
# EXECUTABLE mftraining
########################################

add_executable          (mftraining mftraining.cpp mergenf.cpp)
target_link_libraries   (mftraining tesseract training tessopt)
project_group           (mftraining "Training Tools")


########################################
# EXECUTABLE set_unicharset_properties
########################################

add_executable          (set_unicharset_properties set_unicharset_properties.cpp)
target_link_libraries   (set_unicharset_properties tesseract training tessopt)
project_group           (set_unicharset_properties "Training Tools")


########################################
# EXECUTABLE shapeclustering
########################################

add_executable          (shapeclustering shapeclustering.cpp)
target_link_libraries   (shapeclustering tesseract training tessopt)
project_group           (shapeclustering "Training Tools")


########################################
# EXECUTABLE text2image
########################################

add_executable          (text2image text2image.cpp)
target_link_libraries   (text2image tesseract training tessopt)
project_group           (text2image "Training Tools")


########################################
# EXECUTABLE unicharset_extractor
########################################

add_executable          (unicharset_extractor unicharset_extractor.cpp)
target_link_libraries   (unicharset_extractor tesseract tessopt)
project_group           (unicharset_extractor "Training Tools")


########################################
# EXECUTABLE wordlist2dawg
########################################

add_executable          (wordlist2dawg wordlist2dawg.cpp)
target_link_libraries   (wordlist2dawg tesseract tessopt)
project_group           (wordlist2dawg "Training Tools")


###############################################################################
