#include "JavaObjects.h"

namespace Java {
	JarEntry::JarEntry(JNIEnv* env, jobject jar_entry)
		: env_(env), jar_entry_(jar_entry), name_cached_(false), is_directory_cached_(false)
	{
	}
	const std::string& JarEntry::get_name() const
	{
		if (!name_cached_)
		{
			jclass jar_entry_class = env_->GetObjectClass(jar_entry_);
			jmethodID get_name_method = env_->GetMethodID(jar_entry_class, "getName", "()Ljava/lang/String;");
			jstring name = (jstring)env_->CallObjectMethod(jar_entry_, get_name_method);
			const char* name_chars = env_->GetStringUTFChars(name, nullptr);
			cached_name_ = name_chars;
			env_->ReleaseStringUTFChars(name, name_chars);
			env_->DeleteLocalRef(name);
			env_->DeleteLocalRef(jar_entry_class);
			name_cached_ = true;
		}
		return cached_name_;
	}
	bool JarEntry::is_directory() const
	{
		if (!is_directory_cached_)
		{
			jclass jar_entry_class = env_->GetObjectClass(jar_entry_);
			jmethodID is_directory_method = env_->GetMethodID(jar_entry_class, "isDirectory", "()Z");
			cached_is_directory_ = env_->CallBooleanMethod(jar_entry_, is_directory_method);
			env_->DeleteLocalRef(jar_entry_class);
			is_directory_cached_ = true;
		}
		return cached_is_directory_;
	}
}