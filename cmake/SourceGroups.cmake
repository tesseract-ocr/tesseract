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
source_group("ccmain"       "${SSRC}/ccmain/${H_CPP}")
source_group("ccstruct"     "${SSRC}/ccstruct/${H_CPP}")
source_group("ccutil"       "${SSRC}/ccutil/${H_CPP}")
source_group("classify"     "${SSRC}/classify/${H_CPP}")
source_group("cube"         "${SSRC}/cube/${H_CPP}")
source_group("cutil"        "${SSRC}/cutil/${H_CPP}")
source_group("dict"         "${SSRC}/dict/${H_CPP}")
source_group("neural"       "${SSRC}/neural_networks/runtime/${H_CPP}")
source_group("opencl"       "${SSRC}/opencl/${H_CPP}")
source_group("textord"      "${SSRC}/textord/${H_CPP}")
source_group("viewer"       "${SSRC}/viewer/${H_CPP}")
source_group("port"         "${SSRC}/vs2010/port/${H_CPP}")
source_group("wordrec"      "${SSRC}/wordrec/${H_CPP}")
