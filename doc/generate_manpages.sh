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

man_xslt=http://docbook.sourceforge.net/release/xsl/current/manpages/docbook.xsl
asciidoc=$(which asciidoc)
xsltproc=$(which xsltproc)
if [[ -z "${asciidoc}" ]] || [[ -z "${xsltproc}" ]]; then
  echo "Please make sure asciidoc and xsltproc are installed."
  exit 1
else
  for src in *.asc; do
    pagename=${src/.asc/}
    (${asciidoc} -d manpage ${src} &&
     ${asciidoc} -d manpage -b docbook ${src} &&
       ${xsltproc} --nonet ${man_xslt} ${pagename}.xml) ||
       echo "Error generating ${pagename}"
  done
fi
exit 0
