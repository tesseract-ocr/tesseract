/**************************************************************************
 ** Licensed under the Apache License, Version 2.0 (the "License");
 ** you may not use this file except in compliance with the License.
 ** You may obtain a copy of the License at
 ** http://www.apache.org/licenses/LICENSE-2.0
 ** Unless required by applicable law or agreed to in writing, software
 ** distributed under the License is distributed on an "AS IS" BASIS,
 ** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 ** See the License for the specific language governing permissions and
 ** limitations under the License.
**************************************************************************/
#include <string.h>
#include <ctype.h>

/*chartoname(name,c,dir) converts c into a useful filename*/
void chartoname(register char *name,  /*result */
                char c,               /*char to convert */
                const char *dir) {    /*directory to use */
  char file[3];                  /*filename */
  int index;                     /*index of namelist */
  static const char *namelist[] = {
    "!bang",
    "\"doubleq",
    "#hash",
    "$dollar",
    "%percent",
    "&and",
    "'quote",
    "(lround",
    ")rround",
    "*asterisk",
    "+plus",
    ",comma",
    "-minus",
    ".dot",
    "/slash",
    ":colon",
    ";semic",
    "<less",
    "=equal",
    ">greater",
    "?question",
    "@at",
    "[lsquare",
    "\\backsl",
    "]rsquare",
    "^uparr",
    "_unders",
    "`grave",
    "{lbrace",
    "|bar",
    "}rbrace",
    "~tilde"
  };

  strcpy(name, dir);  /*add specific directory */
  for (index = 0; index < sizeof namelist / sizeof (char *)
    && c != namelist[index][0]; index++);
  if (index < sizeof namelist / sizeof (char *))
                                 /*add text name */
    strcat (name, &namelist[index][1]);
  else {
    if (isupper (c)) {
      file[0] = 'c';             /*direct a-z or A-Z */
      file[1] = c;               /*direct a-z or A-Z */
      file[2] = '\0';
    }
    else {
      file[0] = c;               /*direct a-z or A-Z */
      file[1] = '\0';
    }
    strcat(name, file);  /*append filename */
  }
}
