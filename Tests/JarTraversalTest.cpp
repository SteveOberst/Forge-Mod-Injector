#include "gtest/gtest.h"
#include "JarTraversal.h"
#include "jni.h"
#include "jvmti.h"

// not fully implemented yet
TEST(Tests, JarTraversalTest) {
    JavaVM* jvm;
    JNIEnv* env;

    JavaVMInitArgs vm_args;
    memset(&vm_args, 0, sizeof(vm_args));
    vm_args.version = JNI_VERSION_1_8;
    vm_args.nOptions = 0;
    vm_args.options = NULL;
    vm_args.ignoreUnrecognized = JNI_FALSE;

	ASSERT_TRUE(JNI_CreateJavaVM(&jvm, (void**)&env, &vm_args) == JNI_OK);

	JarTraversalContext context;
	
	jar_traversal_init(nullptr, nullptr);
}