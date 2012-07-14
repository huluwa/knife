#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "jvmti.h"
#include "agent.h"
#include "nativehelper.h"
jvmtiEnv *jvmti;
void initJvmti(JNIEnv * env){
	JavaVM *jvm = 0;
	int res;
	res = env->GetJavaVM(&jvm);
	if (res < 0 || jvm == 0) {
		throwException(env,"java/lang/RuntimeException","GetJavaVM fail");
	}
	res = jvm->GetEnv((void **)&jvmti, JVMTI_VERSION_1_0);
	if (res != JNI_OK || jvmti == 0) {
		throwException(env,"java/lang/RuntimeException","GetEnv fail");
	}
	jvmtiError error;
  	jvmtiCapabilities   capabilities;
  	error = jvmti->GetCapabilities(&capabilities);
  	capabilities.can_tag_objects = 1;
  	capabilities.can_generate_garbage_collection_events = 1;
  	error= jvmti->AddCapabilities(&capabilities);
}

void throwException(JNIEnv * env,char * clazz, char * message)
{
  	jclass exceptionClass = env->FindClass(clazz);
  	if (exceptionClass==NULL) 
  	{
		exceptionClass = env->FindClass("java/lang/RuntimeException");
     		if (exceptionClass==NULL) 
     		{
			fprintf (stderr,"Couldn't throw exception %s - %s\n",clazz,message);
     		}
 	}
  	env->ThrowNew(exceptionClass,message);
}


jvmtiIterationControl JNICALL iterate_markTag
    (jlong class_tag, jlong size, jlong* tag_ptr, void* user_data) 
{
	IteraOverObjectsControl * control = (IteraOverObjectsControl *) user_data;
    	*tag_ptr=1;
    	control->count++;
    	return JVMTI_ITERATION_CONTINUE;
}

jvmtiIterationControl JNICALL iterate_cleanTag
    (jlong class_tag, jlong size, jlong* tag_ptr, void* user_data)
{
	*tag_ptr=0;
   	return JVMTI_ITERATION_CONTINUE;   
}


void releaseTags()
{
  	jvmti->IterateOverHeap( JVMTI_HEAP_OBJECT_TAGGED,
                          &iterate_cleanTag, NULL);
}

jobject getBooleanField(JNIEnv * env,jobject obj,jfieldID fieldId){
	jboolean fieldValue=env->GetBooleanField(obj,fieldId);
	jclass clazz=env->FindClass("java/lang/Boolean");
	jobject r=env->NewObject(clazz,env->GetMethodID(clazz,"<init>","(Z)V"),fieldValue);
	return r;
}

jobject getByteField(JNIEnv * env,jobject obj,jfieldID fieldId){
	jbyte fieldValue=env->GetByteField(obj,fieldId);
	jclass clazz=env->FindClass("java/lang/Byte");
	jobject r=env->NewObject(clazz,env->GetMethodID(clazz,"<init>","(B)V"),fieldValue);
	return r;
}

jobject getCharField(JNIEnv * env,jobject obj,jfieldID fieldId){
	jchar fieldValue=env->GetCharField(obj,fieldId);
	jclass clazz=env->FindClass("java/lang/Character");
	jobject r=env->NewObject(clazz,env->GetMethodID(clazz,"<init>","(C)V"),fieldValue);
	return r;
}

jobject getShortField(JNIEnv * env,jobject obj,jfieldID fieldId){
	jshort fieldValue=env->GetShortField(obj,fieldId);
	jclass clazz=env->FindClass("java/lang/Short");
	jobject r=env->NewObject(clazz,env->GetMethodID(clazz,"<init>","(S)V"),fieldValue);
	return r;
}

jobject getIntField(JNIEnv * env,jobject obj,jfieldID fieldId){
	jint fieldValue=env->GetIntField(obj,fieldId);
	jclass clazz=env->FindClass("java/lang/Integer");
	jobject r=env->NewObject(clazz,env->GetMethodID(clazz,"<init>","(I)V"),fieldValue);
	return r;
}

jobject getLongField(JNIEnv * env,jobject obj,jfieldID fieldId){
	jlong fieldValue=env->GetLongField(obj,fieldId);
	jclass clazz=env->FindClass("java/lang/Long");
	jobject r=env->NewObject(clazz,env->GetMethodID(clazz,"<init>","(J)V"),fieldValue);
	return r;
}

jobject getFloatField(JNIEnv * env,jobject obj,jfieldID fieldId){
	jfloat fieldValue=env->GetFloatField(obj,fieldId);
	jclass clazz=env->FindClass("java/lang/Float");
	jobject r=env->NewObject(clazz,env->GetMethodID(clazz,"<init>","(F)V"),fieldValue);
	return r;
}

jobject getDoubleField(JNIEnv * env,jobject obj,jfieldID fieldId){
	jdouble fieldValue=env->GetDoubleField(obj,fieldId);
	jclass clazz=env->FindClass("java/lang/Double");
	jobject r=env->NewObject(clazz,env->GetMethodID(clazz,"<init>","(D)V"),fieldValue);
	return r;
}


jobject getObjectField(JNIEnv * env,jobject obj,jfieldID fieldId){
	jobject fieldValue=env->GetObjectField(obj,fieldId);
	return fieldValue;
}

jobject getFieldValue(JNIEnv * env,jobject obj,jclass fieldClass,jfieldID fieldId)
{
	

	char* signature;
	jvmti->GetClassSignature(fieldClass,&signature,NULL);
	//printf("bbb%s\n",signature);
	if(strcmp(signature,"Z")==0){
		return getBooleanField(env,obj,fieldId);
	}
	else if(strcmp(signature,"B")==0){
		return getByteField(env,obj,fieldId);
	}
	else if(strcmp(signature,"C")==0){
		return getCharField(env,obj,fieldId);
	}
	else if(strcmp(signature,"S")==0){
		return getShortField(env,obj,fieldId);
	}
	else if(strcmp(signature,"I")==0){
		return getIntField(env,obj,fieldId);
	}
	else if(strcmp(signature,"J")==0){
		return getLongField(env,obj,fieldId);
	}
	else if(strcmp(signature,"F")==0){
		return getFloatField(env,obj,fieldId);
	}
	else if(strcmp(signature,"D")==0){
		return getDoubleField(env,obj,fieldId);
	}
	else{
		return getObjectField(env,obj,fieldId);
	}
}


JNIEXPORT jobjectArray JNICALL Java_com_chenjw_knife_agent_NativeHelper_findInstancesByClass0
  (JNIEnv * env, jclass thisClass, jclass klass)
{ 
	initJvmti(env);
	jclass loadedObject = env->FindClass("java/lang/Object");
	IteraOverObjectsControl control;
  	control.size = 0;
  	control.maxsize = 0;
  	control.count=0;
  	jvmti->IterateOverInstancesOfClass(klass,JVMTI_HEAP_OBJECT_EITHER,iterate_markTag, &control);
  	jint countObjts=0;
  	jobject * objs;
  	jlong * tagResults;
  	jlong idToQuery=1;  
  	jvmti->GetObjectsWithTags(1,&idToQuery,&countObjts,&objs,&tagResults);
  	// Set the object array
  	jobjectArray arrayReturn = env->NewObjectArray(countObjts,loadedObject,0);
  	for (jint i=0;i<countObjts;i++) {
     		env->SetObjectArrayElement(arrayReturn,i, objs[i]);
  	}
	jvmti->Deallocate((unsigned char *)tagResults);  
  	jvmti->Deallocate((unsigned char *)objs);  
  	releaseTags();            
  	return arrayReturn;
}

 jobjectArray  findInstancesByClass0
  (JNIEnv * env, jclass thisClass, jclass klass)
{ 
	initJvmti(env);
	jclass loadedObject = env->FindClass("java/lang/Object");
	IteraOverObjectsControl control;
  	control.size = 0;
  	control.maxsize = 0;
  	control.count=0;
  	jvmti->IterateOverInstancesOfClass(klass,JVMTI_HEAP_OBJECT_EITHER,iterate_markTag, &control);
  	jint countObjts=0;
  	jobject * objs;
  	jlong * tagResults;
  	jlong idToQuery=1;  
  	jvmti->GetObjectsWithTags(1,&idToQuery,&countObjts,&objs,&tagResults);
  	// Set the object array
  	jobjectArray arrayReturn = env->NewObjectArray(countObjts,loadedObject,0);
  	for (jint i=0;i<countObjts;i++) {
     		env->SetObjectArrayElement(arrayReturn,i, objs[i]);
  	}
	jvmti->Deallocate((unsigned char *)tagResults);  
  	jvmti->Deallocate((unsigned char *)objs);  
  	releaseTags();            
  	return arrayReturn;
}

JNIEXPORT jobject JNICALL Java_com_chenjw_knife_agent_NativeHelper_getFieldValue0
  (JNIEnv * env, jclass thisClass, jobject obj,jstring fieldName,jclass fieldClass)
{
	char* fieldNameChars=(char*)env->GetStringUTFChars(fieldName,0);	
	//printf("123%s\n",fieldNameChars);	
	initJvmti(env);
	jclass klass=env->GetObjectClass(obj);
	
	//jfieldID fieldId=env->GetFieldID(klass,fieldNameChars,fieldNameChars);
	jint count=0;
	jfieldID* fieldIds;
	jvmti->GetClassFields(klass,&count,&fieldIds);
	for(int i=0;i<count;i++){
		char* tFieldName;
		jvmti->GetFieldName(klass,fieldIds[i],&tFieldName,0,0);
		
		//printf("%s\n",fieldNameChars);
		if(strcmp(tFieldName,fieldNameChars)==0){
			//printf("123%s\n",tFieldName);
			//printf("aaa\n");
			jobject result=getFieldValue(env,obj,fieldClass,fieldIds[i]);
			jvmti->Deallocate((unsigned char *)tFieldName);
			jvmti->Deallocate((unsigned char *)fieldIds);
			env->ReleaseStringUTFChars(fieldName,fieldNameChars);  	
			return result;
		}
	}
	//char* errorStr=strcat("field not found ",fieldNameChars);
	throwException(env,NULL,"field not found");
	jvmti->Deallocate((unsigned char *)fieldIds);  
	env->ReleaseStringUTFChars(fieldName,fieldNameChars);
	return NULL;
}

