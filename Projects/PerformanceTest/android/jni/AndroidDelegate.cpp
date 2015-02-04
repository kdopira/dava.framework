/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/


#include "AndroidDelegate.h"
#include "FileSystem/Logger.h"

AndroidDelegate::AndroidDelegate(JNIEnv* env)
{
	environment = env;
	classActivity = NULL;
	classApplication = NULL;
    
    renderBuffer = 0;
    frameBuffer = 0;
}

void AndroidDelegate::SetApplication(jobject app, char *packageName)
{
	classApplication = app;

	strcpy(activityName, packageName);
	strcat(activityName, ".JNIActivity");

	strcpy(httpDownloaderName, packageName);
	strcat(httpDownloaderName, ".JNIHttpDownloader");
}

void AndroidDelegate::SetActivity(jobject activity)
{
	classActivity = activity;
}

void AndroidDelegate::ShowKeyboard()
{
	if(environment)
	{
		jclass cls = environment->FindClass(activityName);
		jmethodID mid = environment->GetMethodID(cls, "ShowKeyboard", "()V");
		if(mid && classActivity)
		{
			environment->CallVoidMethod(classActivity, mid);
		}
		else
		{
			DAVA::Logger::Error("[AndroidDelegate::ShowKeyboard] Can't find method");
		}

		environment->DeleteLocalRef(cls);
	}
}

void AndroidDelegate::HideKeyboard()
{
	if(environment)
	{
		jclass cls = environment->FindClass(activityName);
		jmethodID mid = environment->GetMethodID(cls, "HideKeyboard", "()V");

		if(mid && classActivity)
		{
			environment->CallVoidMethod(classActivity, mid);
		}
		else
		{
			DAVA::Logger::Error("[AndroidDelegate::HideKeyboard] Can't find method");
		}

		environment->DeleteLocalRef(cls);	
	}
}

bool AndroidDelegate::DownloadHttpFile(const DAVA::String & url, const DAVA::String & documentsPathname)
{
	DAVA::Logger::Debug("[AndroidDelegate::DownloadHttpFile] url=%s", url.c_str());
	DAVA::Logger::Debug("[AndroidDelegate::DownloadHttpFile] docpath=%s", documentsPathname.c_str());

	bool retValue = false;
	if(environment)
	{
		jclass cls = environment->FindClass(httpDownloaderName);
		if(cls)
		{
			jmethodID mid = environment->GetStaticMethodID(cls, "DownloadFileFromUrl", "(Ljava/lang/String;Ljava/lang/String;)Z");

			if(mid)
			{
				jstring jstrUrl = environment->NewStringUTF(url.c_str());
				jstring jstrPath = environment->NewStringUTF(documentsPathname.c_str());

				retValue = environment->CallStaticBooleanMethod(cls, mid, jstrUrl, jstrPath);
			}
			else
			{
				DAVA::Logger::Error("[AndroidDelegate::DownloadHttpFile] Can't find method");
			}

			environment->DeleteLocalRef(cls);	
		}
	}
	return retValue;
}

void AndroidDelegate::SetBuffers(GLint newFrameBuffer, GLint newRenderBuffer)
{
    renderBuffer = newRenderBuffer;
    frameBuffer = newFrameBuffer;
}

GLint AndroidDelegate::RenderBuffer()
{
    return renderBuffer;
}

GLint AndroidDelegate::FrameBuffer()
{
    return frameBuffer;
}
