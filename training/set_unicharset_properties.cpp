// This program reads a unicharset file, puts the result in a UNICHARSET
// object, fills it with properties about the unichars it contains and writes
// the result back to a file.

#include <stdlib.h>
#include <string.h>
#include <string>

#include "commandlineflags.h"
#include "fileio.h"
#include "genericvector.h"
#include "icuerrorcode.h"
#include "normstrngs.h"
#include "strngs.h"
#include "unicharset.h"
#include "unicode/uchar.h"    // from libicu
#include "unicode/uscript.h"  // from libicu

// The directory that is searched for universal script unicharsets.
STRING_PARAM_FLAG(script_dir, "",
                  "Directory name for input script unicharsets/xheights");

// Flags from commontraining.cpp
DECLARE_STRING_PARAM_FLAG(U);
DECLARE_STRING_PARAM_FLAG(O);
DECLARE_STRING_PARAM_FLAG(X);

namespace tesseract {

// Helper sets the character attribute properties and sets up the script table.
// Does not set tops and bottoms.
static void SetupBasicProperties(UNICHARSET* unicharset) {
  for (int unichar_id = 0; unichar_id < unicharset->size(); ++unichar_id) {
    // Convert any custom ligatures.
    const char* unichar_str = unicharset->id_to_unichar(unichar_id);
    for (int i = 0; UNICHARSET::kCustomLigatures[i][0] != NULL; ++i) {
      if (!strcmp(UNICHARSET::kCustomLigatures[i][1], unichar_str)) {
        unichar_str = UNICHARSET::kCustomLigatures[i][0];
        break;
      }
    }

    // Convert the unichar to UTF32 representation
    GenericVector<char32> uni_vector;
    tesseract::UTF8ToUTF32(unichar_str, &uni_vector);

    // Assume that if the property is true for any character in the string,
    // then it holds for the whole "character".
    bool unichar_isalpha = false;
    bool unichar_islower = false;
    bool unichar_isupper = false;
    bool unichar_isdigit = false;
    bool unichar_ispunct = false;

    for (int i = 0; i < uni_vector.size(); ++i) {
      if (u_isalpha(uni_vector[i]))
        unichar_isalpha = true;
      if (u_islower(uni_vector[i]))
        unichar_islower = true;
      if (u_isupper(uni_vector[i]))
        unichar_isupper = true;
      if (u_isdigit(uni_vector[i]))
        unichar_isdigit = true;
      if (u_ispunct(uni_vector[i]))
        unichar_ispunct = true;
    }

    unicharset->set_isalpha(unichar_id, unichar_isalpha);
    unicharset->set_islower(unichar_id, unichar_islower);
    unicharset->set_isupper(unichar_id, unichar_isupper);
    unicharset->set_isdigit(unichar_id, unichar_isdigit);
    unicharset->set_ispunctuation(unichar_id, unichar_ispunct);

    tesseract::IcuErrorCode err;
    unicharset->set_script(unichar_id, uscript_getName(
        uscript_getScript(uni_vector[0], err)));

    const int num_code_points = uni_vector.size();
    // Obtain the lower/upper case if needed and record it in the properties.
    unicharset->set_other_case(unichar_id, unichar_id);
    if (unichar_islower || unichar_isupper) {
      GenericVector<char32> other_case(num_code_points, 0);
      for (int i = 0; i < num_code_points; ++i) {
        // TODO(daria): Ideally u_strToLower()/ustrToUpper() should be used.
        // However since they deal with UChars (so need a conversion function
        // from char32 or UTF8string) and require a meaningful locale string,
        // for now u_tolower()/u_toupper() are used.
        other_case[i] = unichar_islower ? u_toupper(uni_vector[i]) :
          u_tolower(uni_vector[i]);
      }
      STRING other_case_uch;
      tesseract::UTF32ToUTF8(other_case, &other_case_uch);
      UNICHAR_ID other_case_id =
          unicharset->unichar_to_id(other_case_uch.c_str());
      if (other_case_id != INVALID_UNICHAR_ID) {
        unicharset->set_other_case(unichar_id, other_case_id);
      } else {
        tprintf("Other case %s of %s is not in unicharset",
                other_case_uch.c_str(), unichar_str);
      }
    }

    // Set RTL property and obtain mirror unichar ID from ICU.
    GenericVector<char32> mirrors(num_code_points, 0);
    for (int i = 0; i < num_code_points; ++i) {
      mirrors[i] = u_charMirror(uni_vector[i]);
      if (i == 0) {  // set directionality to that of the 1st code point
        unicharset->set_direction(unichar_id,
                                  static_cast<UNICHARSET::Direction>(
                                      u_charDirection(uni_vector[i])));
      }
    }
    STRING mirror_uch;
    tesseract::UTF32ToUTF8(mirrors, &mirror_uch);
    UNICHAR_ID mirror_uch_id = unicharset->unichar_to_id(mirror_uch.c_str());
    if (mirror_uch_id != INVALID_UNICHAR_ID) {
      unicharset->set_mirror(unichar_id, mirror_uch_id);
    } else {
      tprintf("Mirror %s of %s is not in unicharset\n",
              mirror_uch.c_str(), unichar_str);
    }

    // Record normalized version of this unichar.
    STRING normed_str = tesseract::NormalizeUTF8String(unichar_str);
    if (unichar_id != 0 && normed_str.length() > 0) {
      unicharset->set_normed(unichar_id, normed_str.c_str());
    } else {
      unicharset->set_normed(unichar_id, unichar_str);
    }
  }
  unicharset->post_load_setup();
}

// Helper to set the properties for an input unicharset file, writes to the
// output file. If an appropriate script unicharset can be found in the
// script_dir directory, then the tops and bottoms are expanded using the
// script unicharset.
// If non-empty, xheight data for the fonts are written to the xheights_file.
static void SetPropertiesForInputFile(const string& script_dir,
                                      const string& input_unicharset_file,
                                      const string& output_unicharset_file,
                                      const string& output_xheights_file) {
  UNICHARSET unicharset;

  // Load the input unicharset
  unicharset.load_from_file(input_unicharset_file.c_str());
  tprintf("Loaded unicharset of size %d from file %s", unicharset.size(),
          input_unicharset_file.c_str());

  // Set unichar properties
  tprintf("Setting unichar properties");
  SetupBasicProperties(&unicharset);
  string xheights_str;
  for (int s = 0; s < unicharset.get_script_table_size(); ++s) {
    // Load the unicharset for the script if available.
    string filename = script_dir + "/" +
        unicharset.get_script_from_script_id(s) + ".unicharset";
    UNICHARSET script_set;
    if (script_set.load_from_file(filename.c_str())) {
      unicharset.SetPropertiesFromOther(script_set);
    }
    // Load the xheights for the script if available.
    filename = script_dir + "/" + unicharset.get_script_from_script_id(s) +
        ".xheights";
    string script_heights;
    if (File::ReadFileToString(filename, &script_heights))
      xheights_str += script_heights;
  }
  if (!output_xheights_file.empty())
    File::WriteStringToFileOrDie(xheights_str, output_xheights_file);

  // Write the output unicharset
  tprintf("Writing unicharset to file %s", output_unicharset_file.c_str());
  unicharset.save_to_file(output_unicharset_file.c_str());
}
}  // namespace tesseract


int main(int argc, char** argv) {
  tesseract::ParseCommandLineFlags(argv[0], &argc, &argv, true);

  // Check validity of input flags.
  if (FLAGS_U.empty() || FLAGS_O.empty()) {
    tprintf("Specify both input and output unicharsets!");
    exit(1);
  }
  if (FLAGS_script_dir.empty()) {
    tprintf("Must specify a script_dir!");
    exit(1);
  }

  tesseract::SetPropertiesForInputFile(FLAGS_script_dir.c_str(),
                                       FLAGS_U.c_str(), FLAGS_O.c_str(),
                                       FLAGS_X.c_str());
  return 0;
}
