# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
# http://www.apache.org/licenses/LICENSE-2.0
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#include(SourceGroups)

set(SSRC ${CMAKE_SOURCE_DIR})
set(BSRC ${CMAKE_BINARY_DIR})

set(_CPP ".*\\.cpp")
set(CPP "${_CPP}$")

set(_H ".*\\.h")
set(H "${_H}$")

set(H_CPP "(${H}|${CPP})")

source_group("Resource files" ".*\\.(rc|ico)")

source_group("api"          "${SSRC}/api/${H_CPP}")
source_group("arch"         "${SSRC}/arch/${H_CPP}")
source_group("ccmain"       "${SSRC}/ccmain/${H_CPP}")
source_group("ccstruct"     "${SSRC}/ccstruct/${H_CPP}")
source_group("ccutil"       "${SSRC}/ccutil/${H_CPP}")
source_group("classify"     "${SSRC}/classify/${H_CPP}")
source_group("cutil"        "${SSRC}/cutil/${H_CPP}")
source_group("dict"         "${SSRC}/dict/${H_CPP}")
source_group("lstm"         "${SSRC}/lstm/${H_CPP}")
source_group("textord"      "${SSRC}/textord/${H_CPP}")
source_group("viewer"       "${SSRC}/viewer/${H_CPP}")
source_group("wordrec"      "${SSRC}/wordrec/${H_CPP}")
