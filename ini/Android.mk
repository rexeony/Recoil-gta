LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE    := WeaponFireRecoil
LOCAL_SRC_FILES := ../main.cpp
# Sesuaikan AML_DIR ke folder AndroidModLoader-main milikmu
AML_DIR         := $(LOCAL_PATH)/../../AndroidModLoader-main
LOCAL_C_INCLUDES += $(AML_DIR)/include $(AML_DIR)
LOCAL_SRC_FILES += ../../AndroidModLoader-main/mod/config.cpp ../../AndroidModLoader-main/mod/logger.cpp
LOCAL_CPPFLAGS  += -O2 -std=c++17 -fexceptions -DNDEBUG
ifeq ($(TARGET_ARCH_ABI),armeabi-v7a)
LOCAL_CPPFLAGS  += -mthumb -mfloat-abi=softfp
endif
LOCAL_LDLIBS    += -llog
include $(BUILD_SHARED_LIBRARY)
