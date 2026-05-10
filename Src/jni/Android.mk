LOCAL_PATH := $(call my-dir)


include $(CLEAR_VARS)
LOCAL_MODULE := libcrypto
LOCAL_SRC_FILES := include/lib/$(TARGET_ARCH_ABI)/openssl/libcrypto.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := liboo2core
LOCAL_SRC_FILES := include/lib/$(TARGET_ARCH_ABI)/oodle/liboo2core.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libzlib
LOCAL_SRC_FILES := include/lib/$(TARGET_ARCH_ABI)/zlib/libzlib.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libzstd
LOCAL_SRC_FILES := include/lib/$(TARGET_ARCH_ABI)/zstd/libzstd.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := PRPakTool

LOCAL_C_INCLUDES += $(LOCAL_PATH)/include
LOCAL_C_INCLUDES += $(LOCAL_PATH)/include/lib/$(TARGET_ARCH_ABI)
LOCAL_CPP_INCLUDES += $(LOCAL_PATH)/include

LOCAL_CPPFLAGS += -pie -fPIE -ffunction-sections -fdata-sections -fvisibility=hidden -std=c++14
LOCAL_LDFLAGS += -pie -fPIE -Wl,--gc-sections
LOCAL_CFLAGS := -Wno-error=format-security -fpermissive
LOCAL_CFLAGS += -ffunction-sections -fdata-sections -fvisibility=hidden
LOCAL_CFLAGS += -DNDEBUG
LOCAL_LDLIBS += -L$(SYSROOT)/usr/lib -lz -llog
LOCAL_ARM_MODE := arm



FILE_LIST := $(wildcard $(LOCAL_PATH)/*.cpp)
FILE_LIST += $(wildcard $(LOCAL_PATH)/**/*.cpp)
FILE_LIST += $(wildcard $(LOCAL_PATH)/**/**/*.cpp)
LOCAL_SRC_FILES := $(FILE_LIST:$(LOCAL_PATH)/%=%)



LOCAL_STATIC_LIBRARIES := libcrypto liboo2core libzlib libzstd

include $(BUILD_EXECUTABLE)




