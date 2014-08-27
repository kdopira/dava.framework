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


#ifndef __VIEW_SCENE_SCREEN_H__
#define __VIEW_SCENE_SCREEN_H__

#include "BaseScreen.h"

using namespace DAVA;

class ViewSceneScreen: public BaseScreen
{
protected:
    virtual ~ViewSceneScreen(){}
public:
	ViewSceneScreen();

	virtual void LoadResources();
	virtual void UnloadResources();

    virtual void Draw(const UIGeometricData &geometricData);
    virtual void Update(float32 timeElapsed);

    virtual void DidAppear();

protected:
  
    void OnBack(BaseObject *caller, void *param, void *callerData);
    
    void UpdateCamera(float32 timeElapsed);
    void UpdateInfo(float32 timeElapsed);
    
    Camera *camera;
    
    UIJoypad *moveJoyPAD;
    UIJoypad *viewJoyPAD;
    
    UIStaticText *info;
    
    Matrix4 aimUser;
    float32 viewXAngle, viewYAngle;
    Vector3 targetPosition;
    
    float32 framesTime;
    uint32 frameCounter;
    
    
    uint64 drawTime;
    uint64 updateTime;
};

#endif //__VIEW_SCENE_SCREEN_H__