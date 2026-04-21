#!/bin/bash
#
# File:         generate_manpages.sh
# Description:  Converts .asc files into man pages, etc. for Tesseract.
# Author:       eger@google.com (David Eger)
# Created:      9 Feb 2012
#
# (C) Copyright 2012 Google Inc.
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
# http://www.apache.org/licenses/LICENSE-2.0
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

asciidoctor=$(which asciidoctor)
asciidoctor_pdf=$(which asciidoctor-pdf)
if [[ -z "${asciidoctor}" ]]; then
  echo "Please make sure asciidoctor is installed."
  exit 1
else
  for src in *.asc; do
    pagename=${src/.asc/}
    (${asciidoctor} -b manpage "${src}" -o "${pagename}" &&
     ${asciidoctor} -b html5 "${src}" -o "${pagename}".html &&
     { [[ -z "${asciidoctor_pdf}" ]] || ${asciidoctor_pdf} "${src}" -o "${pagename}".pdf; }) ||
       echo "Error generating ${pagename}"
  done
fi
exit 0
