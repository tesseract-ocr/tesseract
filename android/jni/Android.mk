LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE := tesseract-$(APP_ABI)

LOCAL_STATIC_LIBRARIES := \
    base \
    leptonica-$(APP_ABI)

LOCAL_C_INCLUDES := $(APP_C_INCLUDES)

LOCAL_C_INCLUDES += \
  $(LOCAL_PATH)/../../api \
  $(LOCAL_PATH)/../../ccmain\
  $(LOCAL_PATH)/../../ccstruct\
  $(LOCAL_PATH)/../../ccutil\
  $(LOCAL_PATH)/../../classify\
  $(LOCAL_PATH)/../../cutil\
  $(LOCAL_PATH)/../../dict\
  $(LOCAL_PATH)/../../image\
  $(LOCAL_PATH)/../../textord\
  $(LOCAL_PATH)/../../third_party\
  $(LOCAL_PATH)/../../wordrec\
  $(LOCAL_PATH)/../../opencl\
  $(LOCAL_PATH)/../../viewer\
  $(LOCAL_PATH)/../../../leptonica/include

$(info local c includes=$(LOCAL_C_INCLUDES))
$(info local path=$(LOCAL_PATH))
LOCAL_SRC_FILES := $(wildcard $(LOCAL_PATH)/../../api/*.cpp $(LOCAL_PATH)/../../ccmain/*.cpp $(LOCAL_PATH)/../../ccstruct/*.cpp $(LOCAL_PATH)/../../ccutil/*.cpp $(LOCAL_PATH)/../../classify/*.cpp $(LOCAL_PATH)/../../cutil/*.cpp $(LOCAL_PATH)/../../dict/*.cpp $(LOCAL_PATH)/../../image/*.cpp $(LOCAL_PATH)/../../textord/*.cpp $(LOCAL_PATH)/../../viewer/*.cpp $(LOCAL_PATH)/../../wordrec/*.cpp)

EXPLICIT_SRC_EXCLUDES := \
  $(LOCAL_PATH)/../../api/pdfrenderer.cpp \
  $(LOCAL_PATH)/../../api/tesseractmain.cpp \

LOCAL_SRC_FILES := $(filter-out $(EXPLICIT_SRC_EXCLUDES), $(LOCAL_SRC_FILES))

LOCAL_SRC_FILES := $(LOCAL_SRC_FILES:$(LOCAL_PATH)/%=%)

$(info local src files  = $(LOCAL_SRC_FILES))

LOCAL_LDLIBS := -ldl -llog -ljnigraphics
LOCAL_CFLAGS := -DANDROID_BUILD -DGRAPHICS_DISABLED

include $(BUILD_SHARED_LIBRARY)

$(call import-module,base/port)
$(call import-module,mobile/util/hash)
$(call import-module,third_party/leptonica/android/jni)
