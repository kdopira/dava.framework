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

#include "UI/UIControl.h"
#include "UI/UIControlSystem.h"
#include "UI/UIYamlLoader.h"
#include "UI/UIControlHelpers.h"
#include "Animation/LinearAnimation.h"
#include "Animation/AnimationManager.h"
#include "Debug/DVAssert.h"
#include "FileSystem/YamlNode.h"
#include "Input/InputSystem.h"
#include "Render/RenderHelper.h"
#include "Render/RenderManager.h"
#include "Utils/StringFormat.h"
#include "Render/2D/Systems/RenderSystem2D.h"
#include "Render/2D/Systems/VirtualCoordinatesSystem.h"

#include "Components/UIComponent.h"

namespace DAVA
{

    UIControl::UIControl(const Rect &rect, bool rectInAbsoluteCoordinates/* = false*/)
    {
        UpdateFamily();

        parent = NULL;
        controlState = STATE_NORMAL;
        visible = true;
        visibleForUIEditor = true;
        /*
            VB:
            please do not change anymore to false, it no make any sense to make all controls untouchable by default.
            for particular controls it can be changed, but no make sense to make that for all controls.
         */
        inputEnabled = true;
        inputProcessorsCount = 1;
        focusEnabled = true;

        background = new UIControlBackground();
        eventDispatcher = NULL;
        clipContents = false;

        debugDrawEnabled = false;
        debugDrawColor = Color(1.0f, 0.0f, 0.0f, 1.0f);

        drawPivotPointMode = DRAW_NEVER;

        pivotPoint = Vector2(0, 0);
        scale = Vector2(1.0f, 1.0f);
        angle = 0;

        leftAlign = 0;
        hcenterAlign = 0;
        rightAlign = 0;
        topAlign = 0;
        vcenterAlign = 0;
        bottomAlign = 0;

        leftAlignEnabled = false;
        hcenterAlignEnabled = false;
        rightAlignEnabled = false;
        topAlignEnabled = false;
        vcenterAlignEnabled = false;
        bottomAlignEnabled = false;

        tag = 0;

        multiInput = false;
        exclusiveInput = false;
        currentInputID = 0;
        touchesInside = 0;
        totalTouches = 0;


        SetRect(rect, rectInAbsoluteCoordinates);

        initialState = STATE_NORMAL;
    }

    UIControl::~UIControl()
    {
        UIControlSystem::Instance()->CancelInputs(this);
        SafeRelease(background);
        SafeRelease(eventDispatcher);
        RemoveAllControls();
    }

    void UIControl::SetParent(UIControl *newParent)
    {
        if (parent)
        {
            parent->UnregisterInputProcessors(inputProcessorsCount);
        }
        parent = newParent;
        if (parent)
        {
            parent->RegisterInputProcessors(inputProcessorsCount);
        }
    }
    UIControl *UIControl::GetParent() const
    {
        return parent;
    }

    void UIControl::SetExclusiveInput(bool isExclusiveInput, bool hierarchic/* = true*/)
    {
        exclusiveInput = isExclusiveInput;

        if(hierarchic)
        {
            List<UIControl*>::iterator it = childs.begin();
            for(; it != childs.end(); ++it)
            {
                (*it)->SetExclusiveInput(isExclusiveInput, hierarchic);
            }
        }
    }

    void UIControl::SetMultiInput(bool isMultiInput, bool hierarchic/* = true*/)
    {
        multiInput = isMultiInput;

        if(hierarchic)
        {
            List<UIControl*>::iterator it = childs.begin();
            for(; it != childs.end(); ++it)
            {
                (*it)->SetMultiInput(isMultiInput, hierarchic);
            }
        }
    }


    void UIControl::AddEvent(int32 eventType, const Message &msg)
    {
        if(!eventDispatcher)
        {
            eventDispatcher = new EventDispatcher();
        }
        eventDispatcher->AddEvent(eventType, msg);
    }
    bool UIControl::RemoveEvent(int32 eventType, const Message &msg)
    {
        if(eventDispatcher)
        {
            return eventDispatcher->RemoveEvent(eventType, msg);
        }
        return false;
    }

    bool UIControl::RemoveAllEvents()
    {
        if(eventDispatcher)
        {
            return eventDispatcher->RemoveAllEvents();
        }
        return false;
    }

    void UIControl::PerformEvent(int32 eventType)
    {
        if(eventDispatcher)
        {
            eventDispatcher->PerformEvent(eventType, this);
        }
    }

    void UIControl::PerformEventWithData(int32 eventType, void *callerData)
    {
        if(eventDispatcher)
        {
            eventDispatcher->PerformEventWithData(eventType, this, callerData);
        }
    }


    const List<UIControl*> & UIControl::GetChildren() const
    {
        return childs;
    }

    List<UIControl* >& UIControl::GetRealChildren()
    {
        realChilds.clear();
        realChilds = childs;

        return realChilds;
    }

    List<UIControl* > UIControl::GetSubcontrols()
    {
        // Default list of Subcontrols is empty. To be overriden in the derived
        // controls.
        return List<UIControl*>();
    }

    bool UIControl::IsSubcontrol()
    {
        if (!this->GetParent())
        {
            return false;
        }

        const List<UIControl*>& parentSubcontrols = parent->GetSubcontrols();
        if (parentSubcontrols.empty())
        {
            return false;
        }

        bool isSubcontrol = (std::find(parentSubcontrols.begin(), parentSubcontrols.end(), this) != parentSubcontrols.end());
        return isSubcontrol;
    }

    bool UIControl::AddControlToList(List<UIControl*>& controlsList, const String& controlName, bool isRecursive)
    {
        UIControl* control = FindByName(controlName, isRecursive);
        if (control)
        {
            controlsList.push_back(control);
            return true;
        }

        return false;
    }

    void UIControl::SetName(const String & _name)
    {
        name = _name;
    }

    void UIControl::SetTag(int32 _tag)
    {
        tag = _tag;
    }

    // return first control with given name
    UIControl * UIControl::FindByName(const String & name, bool recursive) const
    {
        List<UIControl*>::const_iterator it = childs.begin();
        for(; it != childs.end(); ++it)
        {
            UIControl * c = (*it);
            if (c->name == name)return c;

            if (recursive)
            {
                UIControl * inChilds = c->FindByName(name);
                if (inChilds)return inChilds;
            }
        }
        return 0;
    }

    UIControl * UIControl::FindByPath(const String &path) const
    {
        return UIControlHelpers::GetControlByPath(path, this);
    }

    void UIControl::SetState(int32 state)
    {
        controlState = state;
    }

    Sprite* UIControl::GetSprite() const
    {
        return background->GetSprite();
    }

    int32 UIControl::GetFrame() const
    {
        return background->GetFrame();
    }

    UIControlBackground::eDrawType UIControl::GetSpriteDrawType() const
    {
        return background->GetDrawType();
    }
    int32 UIControl::GetSpriteAlign() const
    {
        return background->GetAlign();
    }
    void UIControl::SetSprite(const FilePath &spriteName, int32 spriteFrame)
    {
        background->SetSprite(spriteName, spriteFrame);
    }
    void UIControl::SetSprite(Sprite *newSprite, int32 spriteFrame)
    {
        background->SetSprite(newSprite, spriteFrame);
    }
    void UIControl::SetSpriteFrame(int32 spriteFrame)
    {
        background->SetFrame(spriteFrame);
    }
	void UIControl::SetSpriteFrame(const FastName& frameName)
	{
		background->SetFrame(frameName);
	}
    void UIControl::SetSpriteDrawType(UIControlBackground::eDrawType drawType)
    {
        background->SetDrawType(drawType);
    }
    void UIControl::SetSpriteAlign(int32 align)
    {
        background->SetAlign(align);
    }

    void UIControl::SetLeftAlign(float32 align, bool applyAlign/* = true*/)
    {
        // Set a property value
        leftAlign = align;

        if (leftAlignEnabled && applyAlign)
        {
            ApplyAlignSettings();
        }
    }

    float32 UIControl::GetLeftAlign() const
    {
        return leftAlign;
    }

    void UIControl::SetHCenterAlign(float32 align, bool applyAlign/* = true*/)
    {
        hcenterAlign = align;

        if (hcenterAlignEnabled && applyAlign)
        {
            ApplyAlignSettings();
        }
    }

    float32 UIControl::GetHCenterAlign() const
    {
        return hcenterAlign;
    }

    void UIControl::SetRightAlign(float32 align, bool applyAlign/* = true*/)
    {
        rightAlign = align;

        if (rightAlignEnabled && applyAlign)
        {
            ApplyAlignSettings();
        }
    }

    float32 UIControl::GetRightAlign() const
    {
        return rightAlign;
    }

    void UIControl::SetTopAlign(float32 align, bool applyAlign/* = true*/)
    {
        topAlign = align;

        if (topAlignEnabled && applyAlign)
        {
            ApplyAlignSettings();
        }
    }

    float32 UIControl::GetTopAlign() const
    {
        return topAlign;
    }

    void UIControl::SetVCenterAlign(float32 align, bool applyAlign/* = true*/)
    {
        vcenterAlign = align;

        if (vcenterAlignEnabled && applyAlign)
        {
            ApplyAlignSettings();
        }
    }

    float32 UIControl::GetVCenterAlign() const
    {
        return vcenterAlign;
    }

    void UIControl::SetBottomAlign(float32 align, bool applyAlign/* = true*/)
    {
        bottomAlign = align;

        if (bottomAlignEnabled && applyAlign)
        {
            ApplyAlignSettings();
        }
    }

    float32 UIControl::GetBottomAlign() const
    {
        return bottomAlign;
    }

    // Enable align options methods
    void UIControl::SetLeftAlignEnabled(bool isEnabled, bool applyAlign/* = true*/)
    {
        leftAlignEnabled = isEnabled;

        if (applyAlign)
        {
            ApplyAlignSettings();
        }
    }

    bool UIControl::GetLeftAlignEnabled() const
    {
        return leftAlignEnabled;
    }

    void UIControl::SetHCenterAlignEnabled(bool isEnabled, bool applyAlign/* = true*/)
    {
        hcenterAlignEnabled = isEnabled;

        if (applyAlign)
        {
            ApplyAlignSettings();
        }
    }

    bool UIControl::GetHCenterAlignEnabled() const
    {
        return hcenterAlignEnabled;
    }

    void UIControl::SetRightAlignEnabled(bool isEnabled, bool applyAlign/* = true*/)
    {
        rightAlignEnabled = isEnabled;

        if (applyAlign)
        {
            ApplyAlignSettings();
        }
    }

    bool UIControl::GetRightAlignEnabled() const
    {
        return rightAlignEnabled;
    }

    void UIControl::SetTopAlignEnabled(bool isEnabled, bool applyAlign/* = true*/)
    {
        topAlignEnabled = isEnabled;

        if (applyAlign)
        {
            ApplyAlignSettings();
        }
    }

    bool UIControl::GetTopAlignEnabled() const
    {
        return topAlignEnabled;
    }

    void UIControl::SetVCenterAlignEnabled(bool isEnabled, bool applyAlign/* = true*/)
    {
        vcenterAlignEnabled = isEnabled;

        if (applyAlign)
        {
            ApplyAlignSettings();
        }
    }

    bool UIControl::GetVCenterAlignEnabled() const
    {
        return vcenterAlignEnabled;
    }

    void UIControl::SetBottomAlignEnabled(bool isEnabled, bool applyAlign/* = true*/)
    {
        bottomAlignEnabled = isEnabled;

        if (applyAlign)
        {
            ApplyAlignSettings();
        }
    }

    bool UIControl::GetBottomAlignEnabled() const
    {
        return bottomAlignEnabled;
    }

    void UIControl::SetBackground(UIControlBackground *newBg)
    {
        DVASSERT(newBg);
        SafeRelease(background);
        background = newBg->Clone();
    }

    UIControlBackground *UIControl::GetBackground() const
    {
        return background;
    }

    const UIGeometricData &UIControl::GetGeometricData() const
    {
        tempGeometricData.position = relativePosition;
        tempGeometricData.size = size;
        tempGeometricData.pivotPoint = pivotPoint;
        tempGeometricData.scale = scale;
        tempGeometricData.angle = angle;
        tempGeometricData.unrotatedRect.x = relativePosition.x - relativePosition.x * scale.x;
        tempGeometricData.unrotatedRect.y = relativePosition.y - pivotPoint.y * scale.y;
        tempGeometricData.unrotatedRect.dx = size.x * scale.x;
        tempGeometricData.unrotatedRect.dy = size.y * scale.y;

        if(!parent)
        {
            tempGeometricData.AddGeometricData(UIControlSystem::Instance()->GetBaseGeometricData());
            return tempGeometricData;
        }
        tempGeometricData.AddGeometricData(parent->GetGeometricData());
        return tempGeometricData;
    }

    UIGeometricData UIControl::GetLocalGeometricData() const
    {
        UIGeometricData drawData;
        drawData.position = relativePosition;
        drawData.size = size;
        drawData.pivotPoint = pivotPoint;
        drawData.scale = scale;
        drawData.angle = angle;

        return drawData;
    }

    Vector2 UIControl::GetPosition(bool absoluteCoordinates)
    {
        if(!absoluteCoordinates || !parent)
        {
            return GetPosition();
        }

        return GetAbsolutePosition();
    }

    Vector2 UIControl::GetAbsolutePosition()
    {
        return GetGeometricData().position;
    }

    void UIControl::SetPosition(const Vector2 &position)
    {
        relativePosition = position;
    }

    void UIControl::SetPosition(const Vector2 &position, bool positionInAbsoluteCoordinates)
    {
        if(!positionInAbsoluteCoordinates)
        {
            SetPosition(position);
        }
        else
        {
            SetAbsolutePosition(position);
        }
    }

    void UIControl::SetAbsolutePosition(const Vector2 &position)
    {
        if(parent)
        {
            const UIGeometricData &parentGD = parent->GetGeometricData();
            SetPosition(position - parentGD.position + parentGD.pivotPoint);
        }
        else
        {
            SetPosition(position);
        }
    }

    void UIControl::SetSize(const Vector2 &newSize)
    {
        if (size == newSize)
            return;

        Vector2 oldPivot = GetPivot();
        size = newSize;
        SetPivot(oldPivot);
        // Update size and align of childs
        UpdateChildrenLayout();
    }

    void UIControl::SetAngle(float32 angleInRad)
    {
        angle = angleInRad;
    }
    
    void UIControl::SetAngleInDegrees(float32 angleInDeg)
    {
        SetAngle(DegToRad(angleInDeg));
    }

    Rect UIControl::GetRect(bool absoluteCoordinates)
    {
        if(!absoluteCoordinates)
            return GetRect();

        return GetAbsoluteRect();
    }

    Rect UIControl::GetAbsoluteRect()
    {
        return Rect(GetAbsolutePosition() - GetPivotPoint(), size);
    }

    void UIControl::SetRect(const Rect &rect)
    {
        SetSize(rect.GetSize());
        SetPosition(rect.GetPosition() + GetPivotPoint());

        // Update aligns if control was resized manually
        CalculateAlignSettings();
    }

    void UIControl::SetAbsoluteRect(const Rect &rect)
    {
        if (!parent)
        {
            SetRect(rect);
            return;
        }

        Rect localRect = rect;
        const UIGeometricData &parentGD = parent->GetGeometricData();
        localRect.SetPosition(rect.GetPosition() - parentGD.position + parentGD.pivotPoint);
        SetRect(localRect);
    }

    void UIControl::SetRect(const Rect &rect, bool rectInAbsoluteCoordinates/* = false*/)
    {
        if (!rectInAbsoluteCoordinates)
        {
            SetRect(rect);
        }
        else
        {
            SetAbsoluteRect(rect);
        }
    }

    void UIControl::SetScaledRect(const Rect &rect, bool rectInAbsoluteCoordinates/* = false*/)
    {
        if(!rectInAbsoluteCoordinates || !parent)
        {
            scale.x = rect.dx / size.x;
            scale.y = rect.dy / size.y;
            SetPosition(Vector2(rect.x + pivotPoint.x * scale.x, rect.y + pivotPoint.y * scale.y), rectInAbsoluteCoordinates);
        }
        else
        {
            const UIGeometricData &gd = parent->GetGeometricData();
            scale.x = rect.dx / (size.x * gd.scale.x);
            scale.y = rect.dy / (size.y * gd.scale.y);
            SetPosition(Vector2(rect.x + pivotPoint.x * scale.x, rect.y + pivotPoint.y * scale.y), rectInAbsoluteCoordinates);
        }
    }

    void UIControl::SetVisible(bool isVisible)
    {
        if (visible == isVisible)
        {
            return;
        }

        bool oldSystemVisible = GetSystemVisible();
        visible = isVisible;
        if (GetSystemVisible() == oldSystemVisible)
        {
            return;
        }

        SystemNotifyVisibilityChanged();
    }

    void UIControl::SetVisibleForUIEditor(bool value)
    {
        if (visibleForUIEditor == value)
        {
            return;
        }

        bool oldSystemVisible = GetSystemVisible();
        visibleForUIEditor = value;
        if (GetSystemVisible() == oldSystemVisible)
        {
            return;
        }

        SystemNotifyVisibilityChanged();
    }

    void UIControl::SystemNotifyVisibilityChanged()
    {
        if (parent && parent->IsOnScreen())
        {
            if (GetSystemVisible())
            {
                SystemWillBecomeVisible();
            }
            else
            {
                SystemWillBecomeInvisible();
            }
        }
    }

    void UIControl::SetInputEnabled(bool isEnabled, bool hierarchic/* = true*/)
    {
        if (isEnabled != inputEnabled)
        {
            inputEnabled = isEnabled;
            if (inputEnabled)
            {
                RegisterInputProcessor();
            }
            else
            {
                UnregisterInputProcessor();
            }
        }
        if(hierarchic)
        {
            List<UIControl*>::iterator it = childs.begin();
            for(; it != childs.end(); ++it)
            {
                (*it)->SetInputEnabled(isEnabled, hierarchic);
            }
        }
    }

    void UIControl::SetFocusEnabled(bool isEnabled)
    {
        focusEnabled = isEnabled;
    }

    bool UIControl::GetDisabled() const
    {
        return ((controlState & STATE_DISABLED) != 0);
    }

    void UIControl::SetDisabled(bool isDisabled, bool hierarchic/* = true*/)
    {
        if(isDisabled)
        {
            controlState |= STATE_DISABLED;

            // Cancel all inputs because of DF-2943.
            UIControlSystem::Instance()->CancelInputs(this);
        }
        else
        {
            controlState &= ~STATE_DISABLED;
        }

        if(hierarchic)
        {
            List<UIControl*>::iterator it = childs.begin();
            for(; it != childs.end(); ++it)
            {
                (*it)->SetDisabled(isDisabled, hierarchic);
            }
        }
    }

    bool UIControl::GetSelected() const
    {
        return ((controlState & STATE_SELECTED) != 0);
    }

    void UIControl::SetSelected(bool isSelected, bool hierarchic/* = true*/)
    {
        if(isSelected)
        {
            controlState |= STATE_SELECTED;
        }
        else
        {
            controlState &= ~STATE_SELECTED;
        }

        if(hierarchic)
        {
            List<UIControl*>::iterator it = childs.begin();
            for(; it != childs.end(); ++it)
            {
                (*it)->SetSelected(isSelected, hierarchic);
            }
        }
    }

    void UIControl::SetClipContents(bool isNeedToClipContents)
    {
        clipContents = isNeedToClipContents;
    }

    bool UIControl::GetHover() const
    {
        return (controlState & STATE_HOVER) != 0;
    }

    void UIControl::AddControl(UIControl *control)
    {
        control->Retain();
        control->RemoveFromParent();

        bool inHierarchy = InViewHierarchy();
        if (inHierarchy)
        {
            control->SystemWillAppear();
        }
        control->isUpdated = false;
        control->SetParent(this);
        childs.push_back(control);
        if (inHierarchy)
        {
            control->SystemDidAppear();
        }

        if (IsOnScreen() && control->GetSystemVisible())
            control->SystemWillBecomeVisible();

        isIteratorCorrupted = true;
    }
    
    void UIControl::RemoveControl(UIControl *control)
    {
        if (NULL == control)
        {
            return;
        }

        List<UIControl*>::iterator it = childs.begin();
        for(; it != childs.end(); ++it)
        {
            if((*it) == control)
            {
                if (IsOnScreen() && control->GetSystemVisible())
                    control->SystemWillBecomeInvisible();

                bool inHierarchy = InViewHierarchy();
                if (inHierarchy)
                {
                    control->SystemWillDisappear();
                }
                control->SetParent(NULL);
                childs.erase(it);
                if (inHierarchy)
                {
                    control->SystemDidDisappear();
                }
                control->Release();
                isIteratorCorrupted = true;
                return;
            }
        }
    }

    void UIControl::RemoveFromParent()
    {
        UIControl* parentControl = this->GetParent();
        if (parentControl)
        {
            parentControl->RemoveControl(this);
        }
    }

    void UIControl::RemoveAllControls()
    {
        while(!childs.empty())
        {
            RemoveControl(childs.front());
        }
    }
    void UIControl::BringChildFront(UIControl *_control)
    {
        List<UIControl*>::iterator it = childs.begin();
        for(; it != childs.end(); ++it)
        {
            if((*it) == _control)
            {
                childs.erase(it);
                childs.push_back(_control);
                isIteratorCorrupted = true;
                return;
            }
        }
    }
    void UIControl::BringChildBack(UIControl *_control)
    {
        List<UIControl*>::iterator it = childs.begin();
        for(; it != childs.end(); ++it)
        {
            if((*it) == _control)
            {
                childs.erase(it);
                childs.push_front(_control);
                isIteratorCorrupted = true;
                return;
            }
        }
    }

    void UIControl::InsertChildBelow(UIControl * control, UIControl * _belowThisChild)
    {
        List<UIControl*>::iterator it = childs.begin();
        for(; it != childs.end(); ++it)
        {
            if((*it) == _belowThisChild)
            {
                control->Retain();
                control->RemoveFromParent();

                bool inHierarchy = InViewHierarchy();
                if(inHierarchy)
                {
                    control->SystemWillAppear();
                }
                childs.insert(it, control);
                control->SetParent(this);
                if(inHierarchy)
                {
                    control->SystemDidAppear();
                }

                if (IsOnScreen() && control->GetSystemVisible())
                    control->SystemWillBecomeVisible();

                isIteratorCorrupted = true;
                return;
            }
        }

        AddControl(control);
    }
    void UIControl::InsertChildAbove(UIControl * control, UIControl * _aboveThisChild)
    {
        List<UIControl*>::iterator it = childs.begin();
        for(; it != childs.end(); ++it)
        {
            if((*it) == _aboveThisChild)
            {
                control->Retain();
                control->RemoveFromParent();

                bool inHierarchy = InViewHierarchy();
                if(inHierarchy)
                {
                    control->SystemWillAppear();
                }
                childs.insert(++it, control);
                control->SetParent(this);
                if(inHierarchy)
                {
                    control->SystemDidAppear();
                }

                if (IsOnScreen() && control->GetSystemVisible())
                    control->SystemWillBecomeVisible();

                isIteratorCorrupted = true;
                return;
            }
        }

        AddControl(control);
    }

    void UIControl::SendChildBelow(UIControl * _control, UIControl * _belowThisChild)
    {
        //TODO: Fix situation when controls not from this hierarchy

        // firstly find control in list and erase it
        List<UIControl*>::iterator it = childs.begin();
        for(; it != childs.end(); ++it)
        {
            if((*it) == _control)
            {
                childs.erase(it);
                isIteratorCorrupted = true;
                break;
            }
        }
        // after that find place where we should put the control and do that
        it = childs.begin();
        for(; it != childs.end(); ++it)
        {
            if((*it) == _belowThisChild)
            {
                childs.insert(it, _control);
                isIteratorCorrupted = true;
                return;
            }
        }
        DVASSERT_MSG(0, "Control _belowThisChild not found");
    }

    void UIControl::SendChildAbove(UIControl * _control, UIControl * _aboveThisChild)
    {
        //TODO: Fix situation when controls not from this hierarhy

        // firstly find control in list and erase it
        List<UIControl*>::iterator it = childs.begin();
        for(; it != childs.end(); ++it)
        {
            if((*it) == _control)
            {
                childs.erase(it);
                isIteratorCorrupted = true;
                break;
            }
        }
        // after that find place where we should put the control and do that
        it = childs.begin();
        for(; it != childs.end(); ++it)
        {
            if((*it) == _aboveThisChild)
            {
                childs.insert(++it, _control);
                isIteratorCorrupted = true;
                return;
            }
        }

        DVASSERT_MSG(0, "Control _aboveThisChild not found");
    }

    UIControl *UIControl::Clone()
    {
        UIControl *c = new UIControl(Rect(relativePosition.x, relativePosition.y, size.x, size.y));
        c->CopyDataFrom(this);
        return c;
    }

    void UIControl::CopyDataFrom(UIControl *srcControl)
    {
        relativePosition = srcControl->relativePosition;
        size = srcControl->size;
        pivotPoint = srcControl->pivotPoint;
        scale = srcControl->scale;
        angle = srcControl->angle;
        SafeRelease(background);
        background = srcControl->background->Clone();

        leftAlign = srcControl->leftAlign;
        hcenterAlign = srcControl->hcenterAlign;
        rightAlign = srcControl->rightAlign;
        topAlign = srcControl->topAlign;
        vcenterAlign = srcControl->vcenterAlign;
        bottomAlign = srcControl->bottomAlign;

        leftAlignEnabled = srcControl->leftAlignEnabled;
        hcenterAlignEnabled = srcControl->hcenterAlignEnabled;
        rightAlignEnabled = srcControl->rightAlignEnabled;
        topAlignEnabled = srcControl->topAlignEnabled;
        vcenterAlignEnabled = srcControl->vcenterAlignEnabled;
        bottomAlignEnabled = srcControl->bottomAlignEnabled;

        tag = srcControl->GetTag();
        name = srcControl->name;

        controlState = srcControl->controlState;
        visible = srcControl->visible;
        visibleForUIEditor = srcControl->visibleForUIEditor;
        inputEnabled = srcControl->inputEnabled;
        clipContents = srcControl->clipContents;

        customControlType = srcControl->GetCustomControlClassName();
        initialState = srcControl->GetInitialState();
        drawPivotPointMode = srcControl->drawPivotPointMode;
        debugDrawColor = srcControl->debugDrawColor;
        debugDrawEnabled = srcControl->debugDrawEnabled;

        SafeRelease(eventDispatcher);
        if(srcControl->eventDispatcher)
        {
            eventDispatcher = new EventDispatcher();
            eventDispatcher->CopyDataFrom(srcControl->eventDispatcher);
        }

        RemoveAllControls();
        if (inputEnabled)
        {
            inputProcessorsCount = 1;
        }
        else
        {
            inputProcessorsCount = 0;
        }

        // Yuri Coder, 2012/11/30. Use Real Children List to avoid copying
        // unnecessary children we have on the for example UIButton.
        const List<UIControl*>& realChildren = srcControl->GetRealChildren();
        List<UIControl*>::const_iterator it = realChildren.begin();
        for(; it != realChildren.end(); ++it)
        {

            UIControl *c = (*it)->Clone();
            AddControl(c);
            c->Release();
        }
    }


    bool UIControl::InViewHierarchy() const
    {
        if (UIControlSystem::Instance()->GetScreen() == this ||
            UIControlSystem::Instance()->GetPopupContainer() == this)
        {
            return true;
        }

        if (parent)
            return parent->InViewHierarchy();

        return false;
    }


    bool UIControl::IsOnScreen() const
    {
        if(UIControlSystem::Instance()->GetScreen() == this ||
           UIControlSystem::Instance()->GetPopupContainer() == this)
        {
            return GetSystemVisible();
        }

        if( !GetSystemVisible() || !parent )
            return false;

        return parent->IsOnScreen();
    }


    void UIControl::SystemWillAppear()
    {
        WillAppear();

        List<UIControl*>::iterator it = childs.begin();
        while(it != childs.end())
        {
            isIteratorCorrupted = false;
            UIControl *current = *it;
            current->Retain();
            current->SystemWillAppear();
            current->Release();
            if(isIteratorCorrupted)
            {
                it = childs.begin();
                continue;
            }
            ++it;
        }
    }

    void UIControl::SystemWillDisappear()
    {
        List<UIControl*>::iterator it = childs.begin();
        while(it != childs.end())
        {
            isIteratorCorrupted = false;
            UIControl *current = *it;
            current->Retain();
            current->SystemWillDisappear();
            current->Release();
            if(isIteratorCorrupted)
            {
                it = childs.begin();
                continue;
            }
            ++it;
        }

        WillDisappear();
    }

    void UIControl::SystemDidAppear()
    {
        DidAppear();

        List<UIControl*>::iterator it = childs.begin();
        while(it != childs.end())
        {
            isIteratorCorrupted = false;
            UIControl *current = *it;
            current->Retain();
            current->SystemDidAppear();
            current->Release();
            if(isIteratorCorrupted)
            {
                it = childs.begin();
                continue;
            }
            ++it;
        }
    }

    void UIControl::SystemDidDisappear()
    {
        DidDisappear();

        List<UIControl*>::iterator it = childs.begin();
        while(it != childs.end())
        {
            isIteratorCorrupted = false;
            UIControl *current = *it;
            current->Retain();
            current->SystemDidDisappear();
            current->Release();
            if(isIteratorCorrupted)
            {
                it = childs.begin();
                continue;
            }
            ++it;
        }
    }

    void UIControl::SystemScreenSizeDidChanged(const Rect &newFullScreenRect)
    {
        ScreenSizeDidChanged(newFullScreenRect);

        List<UIControl*>::iterator it = childs.begin();
        while(it != childs.end())
        {
            isIteratorCorrupted = false;
            UIControl *current = *it;
            current->Retain();
            current->SystemScreenSizeDidChanged(newFullScreenRect);
            current->Release();
            if(isIteratorCorrupted)
            {
                it = childs.begin();
                continue;
            }
            ++it;
        }
    }




    void UIControl::WillAppear()
    {

    }
    void UIControl::WillDisappear()
    {

    }
    void UIControl::DidAppear()
    {

    }
    void UIControl::DidDisappear()
    {

    }
    void UIControl::ScreenSizeDidChanged(const Rect &newFullScreenRect)
    {

    }



    void UIControl::SystemUpdate(float32 timeElapsed)
    {
        if(!GetSystemVisible())
        {
            return;
        }
        
        UIControlSystem::Instance()->updateCounter++;
        Update(timeElapsed);
        isUpdated = true;
        List<UIControl*>::iterator it = childs.begin();
        for(; it != childs.end(); ++it)
        {
            (*it)->isUpdated = false;
        }

        it = childs.begin();
        while(it != childs.end())
        {
            isIteratorCorrupted = false;
            UIControl *current = *it;
            if(!current->isUpdated)
            {
                current->Retain();
                current->SystemUpdate(timeElapsed);
                current->Release();
                if(isIteratorCorrupted)
                {
                    it = childs.begin();
                    continue;
                }
            }
            ++it;
        }
    }

    void UIControl::SystemDraw(const UIGeometricData &geometricData)
    {
        if (!GetSystemVisible())
            return;

        UIControlSystem::Instance()->drawCounter++;
        UIGeometricData drawData = GetLocalGeometricData();
        drawData.AddGeometricData(geometricData);

        const Color &parentColor = parent ? parent->GetBackground()->GetDrawColor() : Color::White;

        SetParentColor(parentColor);

        const Rect& unrotatedRect = drawData.GetUnrotatedRect();

        if(clipContents)
        {//WARNING: for now clip contents don't work for rotating controls if you have any ideas you are welcome
            RenderSystem2D::Instance()->PushClip();
            RenderSystem2D::Instance()->IntersectClipRect(drawData.GetAABBox());
        }

        Draw(drawData);

        isIteratorCorrupted = false;
        List<UIControl*>::iterator it = childs.begin();
        List<UIControl*>::iterator itEnd = childs.end();
        for(; it != itEnd; ++it)
        {
            (*it)->SystemDraw(drawData);
            DVASSERT(!isIteratorCorrupted);
        }

        DrawAfterChilds(drawData);

        if(clipContents)
        {
            RenderSystem2D::Instance()->PopClip();
        }

        if(debugDrawEnabled)
        {
            RenderSystem2D::Instance()->PushClip();
            RenderSystem2D::Instance()->RemoveClip();
            DrawDebugRect(drawData, false);
            DrawPivotPoint(unrotatedRect);
            RenderSystem2D::Instance()->PopClip();
        }
    }

    void UIControl::SetParentColor( const Color &parentColor )
    {
        GetBackground()->SetParentColor(parentColor);
    }

    void UIControl::DrawDebugRect(const UIGeometricData &gd, bool useAlpha)
    {
        Color oldColor = RenderManager::Instance()->GetColor();
        RenderSystem2D::Instance()->PushClip();

        if (useAlpha)
        {
            Color drawColor = debugDrawColor;
            drawColor.a = 0.4f;
            RenderManager::Instance()->SetColor(drawColor);
        }
        else
        {
            RenderManager::Instance()->SetColor(debugDrawColor);
        }

        if( gd.angle != 0.0f )
        {
            Polygon2 poly;
            gd.GetPolygon( poly );

            RenderHelper::Instance()->DrawPolygon( poly, true, RenderState::RENDERSTATE_2D_BLEND );
        }
        else
        {
            RenderHelper::Instance()->DrawRect( gd.GetUnrotatedRect(), RenderState::RENDERSTATE_2D_BLEND );
        }

        RenderSystem2D::Instance()->PopClip();
        RenderManager::Instance()->SetColor(oldColor);
    }

    void UIControl::DrawPivotPoint(const Rect &drawRect)
    {
        if (drawPivotPointMode == DRAW_NEVER)
        {
            return;
        }

        if (drawPivotPointMode == DRAW_ONLY_IF_NONZERO && GetPivotPoint().IsZero())
        {
            return;
        }

        static const float32 PIVOT_POINT_MARK_RADIUS = 10.0f;
        static const float32 PIVOT_POINT_MARK_HALF_LINE_LENGTH = 13.0f;

        Color oldColor = RenderManager::Instance()->GetColor();
        RenderSystem2D::Instance()->PushClip();
        RenderManager::Instance()->SetColor(Color(1.0f, 0.0f, 0.0f, 1.0f));

        Vector2 pivotPointCenter = drawRect.GetPosition() + GetPivotPoint();
        RenderHelper::Instance()->DrawCircle(pivotPointCenter, PIVOT_POINT_MARK_RADIUS, RenderState::RENDERSTATE_2D_BLEND);

        // Draw the cross mark.
        Vector2 lineStartPoint = pivotPointCenter;
        Vector2 lineEndPoint = pivotPointCenter;
        lineStartPoint.y -= PIVOT_POINT_MARK_HALF_LINE_LENGTH;
        lineEndPoint.y += PIVOT_POINT_MARK_HALF_LINE_LENGTH;
        RenderHelper::Instance()->DrawLine(lineStartPoint, lineEndPoint, RenderState::RENDERSTATE_2D_BLEND);

        lineStartPoint = pivotPointCenter;
        lineEndPoint = pivotPointCenter;
        lineStartPoint.x -= PIVOT_POINT_MARK_HALF_LINE_LENGTH;
        lineEndPoint.x += PIVOT_POINT_MARK_HALF_LINE_LENGTH;
        RenderHelper::Instance()->DrawLine(lineStartPoint, lineEndPoint, RenderState::RENDERSTATE_2D_BLEND);

        RenderSystem2D::Instance()->PopClip();
        RenderManager::Instance()->SetColor(oldColor);
    }

    bool UIControl::IsPointInside(const Vector2 &_point, bool expandWithFocus/* = false*/) const
    {
        Vector2 point = _point;

        if(InputSystem::Instance()->IsCursorPining())
        {
            point.x = VirtualCoordinatesSystem::Instance()->GetVirtualScreenSize().dx / 2.f;
            point.y = VirtualCoordinatesSystem::Instance()->GetVirtualScreenSize().dx / 2.f;
        }

        const UIGeometricData &gd = GetGeometricData();
        Rect rect = gd.GetUnrotatedRect();
        if(expandWithFocus)
        {
            rect.dx += CONTROL_TOUCH_AREA*2;
            rect.dy += CONTROL_TOUCH_AREA*2;
            rect.x -= CONTROL_TOUCH_AREA;
            rect.y -= CONTROL_TOUCH_AREA;
        }
        if( gd.angle != 0 )
        {
            Vector2 testPoint;
            testPoint.x = (point.x - gd.position.x) * gd.cosA  + (gd.position.y - point.y) * -gd.sinA + gd.position.x;
            testPoint.y = (point.x - gd.position.x) * -gd.sinA  + (point.y - gd.position.y) * gd.cosA + gd.position.y;
            return rect.PointInside(testPoint);
        }

        return rect.PointInside(point);
    }

    bool UIControl::SystemProcessInput(UIEvent *currentInput)
    {
        if(!inputEnabled || !GetSystemVisible() || controlState & STATE_DISABLED)
        {
            return false;
        }
        if(UIControlSystem::Instance()->GetExclusiveInputLocker()
           && UIControlSystem::Instance()->GetExclusiveInputLocker() != this)
        {
            return false;
        }
        if (customSystemProcessInput != 0 && customSystemProcessInput(this, currentInput))
        {
        	return true;
        }

        switch (currentInput->phase)
        {
#if !defined(__DAVAENGINE_IPHONE__) && !defined(__DAVAENGINE_ANDROID__)
            case UIEvent::PHASE_KEYCHAR:
            {
                    Input(currentInput);
            }
            break;
            case UIEvent::PHASE_MOVE:
            {
                if (!currentInput->touchLocker && IsPointInside(currentInput->point))
                {
                    UIControlSystem::Instance()->SetHoveredControl(this);
                    Input(currentInput);
                    return true;
                }
            }
            break;
            case UIEvent::PHASE_WHEEL:
            {
                 Input(currentInput);
            }
            break;
#endif
            case UIEvent::PHASE_BEGAN:
            {
                if (!currentInput->touchLocker && IsPointInside(currentInput->point))
                {
                    if(multiInput || !currentInputID)
                    {

                        controlState |= STATE_PRESSED_INSIDE;
                        controlState &= ~STATE_NORMAL;
                        ++touchesInside;
                        ++totalTouches;
                        currentInput->controlState = UIEvent::CONTROL_STATE_INSIDE;

                        // Yuri Coder, 2013/12/18. Set the touch lockers before the EVENT_TOUCH_DOWN handler
                        // to have possibility disable control inside the EVENT_TOUCH_DOWN. See also DF-2943.
                        currentInput->touchLocker = this;
                        if(exclusiveInput)
                        {
                            UIControlSystem::Instance()->SetExclusiveInputLocker(this, currentInput->tid);
                        }

                        PerformEventWithData(EVENT_TOUCH_DOWN, currentInput);

                        if(!multiInput)
                        {
                            currentInputID = currentInput->tid;
                        }

                        Input(currentInput);
                        return true;
                    }
                    else
                    {
                        currentInput->touchLocker = this;
                        return true;
                    }

                }
            }
                break;
            case UIEvent::PHASE_DRAG:
            {
                if(currentInput->touchLocker == this)
                {
                    if(multiInput || currentInputID == currentInput->tid)
                    {
                        if(controlState & STATE_PRESSED_INSIDE || controlState & STATE_PRESSED_OUTSIDE)
                        {
                            if (IsPointInside(currentInput->point, true))
                            {
                                if(currentInput->controlState == UIEvent::CONTROL_STATE_OUTSIDE)
                                {
                                    currentInput->controlState = UIEvent::CONTROL_STATE_INSIDE;
                                    ++touchesInside;
                                    if(touchesInside > 0)
                                    {
                                        controlState |= STATE_PRESSED_INSIDE;
                                        controlState &= ~STATE_PRESSED_OUTSIDE;
#if !defined(__DAVAENGINE_IPHONE__) && !defined(__DAVAENGINE_ANDROID__)
                                        controlState |= STATE_HOVER;
#endif
                                    }
                                }
                            }
                            else
                            {
                                if(currentInput->controlState == UIEvent::CONTROL_STATE_INSIDE)
                                {
                                    currentInput->controlState = UIEvent::CONTROL_STATE_OUTSIDE;
                                    --touchesInside;
                                    if(touchesInside == 0)
                                    {
                                        controlState |= STATE_PRESSED_OUTSIDE;
                                        controlState &= ~STATE_PRESSED_INSIDE;
                                    }
                                }
                            }
                        }
                        Input(currentInput);
                    }
                    return true;
                }
            }
                break;
            case UIEvent::PHASE_ENDED:
            {
                if(currentInput->touchLocker == this)
                {
                    if(multiInput || currentInputID == currentInput->tid)
                    {
                        Input(currentInput);
                        if(currentInput->tid == currentInputID)
                        {
                            currentInputID = 0;
                        }
                        if(totalTouches > 0)
                        {
                            --totalTouches;
                            if(currentInput->controlState == UIEvent::CONTROL_STATE_INSIDE)
                            {
                                --touchesInside;
#if !defined(__DAVAENGINE_IPHONE__) && !defined(__DAVAENGINE_ANDROID__)
                                if(totalTouches == 0)
                                {
                                    controlState |= STATE_HOVER;
                                }
#endif
                            }

                            currentInput->controlState = UIEvent::CONTROL_STATE_RELEASED;

                            if(totalTouches == 0)
                            {
                                if (IsPointInside(currentInput->point, true))
                                {
                                    if (UIControlSystem::Instance()->GetFocusedControl() != this && focusEnabled)
                                    {
                                        UIControlSystem::Instance()->SetFocusedControl(this, false);
                                    }
                                    PerformEventWithData(EVENT_TOUCH_UP_INSIDE, currentInput);
                                }
                                else
                                {
                                    PerformEventWithData(EVENT_TOUCH_UP_OUTSIDE, currentInput);
                                }
                                controlState &= ~STATE_PRESSED_INSIDE;
                                controlState &= ~STATE_PRESSED_OUTSIDE;
                                controlState |= STATE_NORMAL;
                                if(UIControlSystem::Instance()->GetExclusiveInputLocker() == this)
                                {
                                    UIControlSystem::Instance()->SetExclusiveInputLocker(NULL, -1);
                                }
                            }
                            else if(touchesInside <= 0)
                            {
                                controlState |= STATE_PRESSED_OUTSIDE;
                                controlState &= ~STATE_PRESSED_INSIDE;
#if !defined(__DAVAENGINE_IPHONE__) && !defined(__DAVAENGINE_ANDROID__)
                                controlState &= ~STATE_HOVER;
#endif
                            }
                        }
                    }

                    currentInput->touchLocker = NULL;
                    return true;
                }
            }
                break;
            case UIEvent::PHASE_JOYSTICK:
            {
                Input(currentInput);
            }
        }

        return false;
    }

    bool UIControl::SystemInput(UIEvent *currentInput)
    {
        UIControlSystem::Instance()->inputCounter++;
        isUpdated = true;

        if( !GetSystemVisible() )
            return false;

        //if(currentInput->touchLocker != this)
        {
            if(clipContents
               && (currentInput->phase != UIEvent::PHASE_DRAG
                   && currentInput->phase != UIEvent::PHASE_ENDED
                   && currentInput->phase != UIEvent::PHASE_KEYCHAR
                   && currentInput->phase != UIEvent::PHASE_JOYSTICK))
            {
                if(!IsPointInside(currentInput->point))
                {
                    return false;
                }
            }

            List<UIControl*>::reverse_iterator it = childs.rbegin();
            List<UIControl*>::reverse_iterator itEnd = childs.rend();
            for(; it != itEnd; ++it)
            {
                (*it)->isUpdated = false;
            }

            it = childs.rbegin();
            itEnd = childs.rend();
            while(it != itEnd)
            {
                isIteratorCorrupted = false;
                UIControl *current = *it;
                if(!current->isUpdated)
                {
                    current->Retain();
                    if(current->inputProcessorsCount > 0 && current->SystemInput(currentInput))
                    {
                        current->Release();
                        return true;
                    }
                    current->Release();
                    if(isIteratorCorrupted)
                    {
                        it = childs.rbegin();
                        continue;
                    }
                }
                ++it;
            }
        }
        return SystemProcessInput(currentInput);
    }

    void UIControl::SystemInputCancelled(UIEvent *currentInput)
    {
        if(currentInput->controlState != UIEvent::CONTROL_STATE_RELEASED)
        {
            --totalTouches;
        }
        if(currentInput->controlState == UIEvent::CONTROL_STATE_INSIDE)
        {
            --touchesInside;
        }

        if(touchesInside == 0)
        {
            controlState &= ~STATE_PRESSED_INSIDE;
            controlState &= ~STATE_PRESSED_OUTSIDE;
            controlState |= STATE_NORMAL;
            if(UIControlSystem::Instance()->GetExclusiveInputLocker() == this)
            {
                UIControlSystem::Instance()->SetExclusiveInputLocker(NULL, -1);
            }
        }

        currentInput->controlState = UIEvent::CONTROL_STATE_RELEASED;
        if(currentInput->tid == currentInputID)
        {
            currentInputID = 0;
        }
        currentInput->touchLocker = NULL;


        InputCancelled(currentInput);
    }

    void UIControl::SystemDidSetHovered()
    {
        controlState |= STATE_HOVER;
        PerformEventWithData(EVENT_HOVERED_SET, NULL);
        DidSetHovered();
    }

    void UIControl::SystemDidRemoveHovered()
    {
        PerformEventWithData(EVENT_HOVERED_REMOVED, NULL);
        controlState &= ~STATE_HOVER;
        DidRemoveHovered();
    }

    void UIControl::DidSetHovered()
    {
    }

    void UIControl::DidRemoveHovered()
    {
    }

    void UIControl::Input(UIEvent *currentInput)
    {
        currentInput->SetInputHandledType(UIEvent::INPUT_NOT_HANDLED);
    }

    void UIControl::InputCancelled(UIEvent *currentInput)
    {
    }

    void UIControl::Update(float32 timeElapsed)
    {

    }
    void UIControl::Draw(const UIGeometricData &geometricData)
    {
        background->Draw(geometricData);
    }
    void UIControl::DrawAfterChilds(const UIGeometricData &geometricData)
    {

    }

    void UIControl::SystemWillBecomeVisible()
    {
        WillBecomeVisible();

        List<UIControl*>::const_iterator it = childs.begin();
        List<UIControl*>::const_iterator end = childs.end();
        for (; it != end; ++it)
        {
            if ((*it)->GetSystemVisible())
                (*it)->SystemWillBecomeVisible();
        }
    }

    void UIControl::SystemWillBecomeInvisible()
    {
        if (GetHover())
        {
            UIControlSystem::Instance()->SetHoveredControl(NULL);
        }
        if (UIControlSystem::Instance()->GetFocusedControl() == this)
        {
            UIControlSystem::Instance()->SetFocusedControl(NULL, true);
        }
        if (GetInputEnabled())
        {
            UIControlSystem::Instance()->CancelInputs(this, false);
        }

        List<UIControl*>::const_iterator it = childs.begin();
        List<UIControl*>::const_iterator end = childs.end();
        for (; it != end; ++it)
        {
            if ((*it)->GetSystemVisible())
                (*it)->SystemWillBecomeInvisible();
        }

        WillBecomeInvisible();
    }

    void UIControl::WillBecomeVisible()
    {
    }

    void UIControl::WillBecomeInvisible()
    {
    }

    YamlNode* UIControl::SaveToYamlNode(UIYamlLoader * loader)
    {
        // Return node
        YamlNode *node = YamlNode::CreateMapNode(false);
        // Model UIControl to be used in comparing
        ScopedPtr<UIControl> baseControl(new UIControl());

        // Control name
        SetPreferredNodeType(node, GetControlClassName());

        // Transform data
        // Position
        const Vector2 &position = GetPosition();
        if (baseControl->GetPosition() != position)
        {
            node->Set("position", position);
        }
        // Size
        const Vector2 &size = GetSize();
        if (baseControl->GetSize() != size)
        {
            node->Set("size", size);
        }
        // Pivot
        if (baseControl->GetPivotPoint() != GetPivotPoint())
        {
            node->Set("pivot", GetPivotPoint());
        }
        // Angle
        if (baseControl->GetAngle() != GetAngle())
        {
            node->Set("angle", GetAngle());
        }
        // Visible
        if (baseControl->GetVisible() != GetVisible())
        {
            node->Set("visible", GetVisible());
        }
        // Enabled
        if (baseControl->GetDisabled() != GetDisabled())
        {
            node->Set("enabled", !GetDisabled());
        }
        // Clip contents
        if (baseControl->GetClipContents() != GetClipContents())
        {
            node->Set("clip", GetClipContents());
        }
        // Input
        if (baseControl->GetInputEnabled() != GetInputEnabled())
        {
            node->Set("noInput", !GetInputEnabled());
        }
        // Tag
        if (baseControl->GetTag() != GetTag())
        {
            node->Set("tag", GetTag());
        }
        // Initial state.
        if (baseControl->GetInitialState() != GetInitialState())
        {
            node->Set("initialState", GetInitialState());
        }

        // Anchor data
        // Left Align
        if (GetLeftAlignEnabled())
        {
            node->Set("leftAlign", GetLeftAlign());
        }
        // Horizontal Center Align
        if (GetHCenterAlignEnabled())
        {
            node->Set("hcenterAlign", GetHCenterAlign());
        }
        // Right Align
        if (GetRightAlignEnabled())
        {
            node->Set("rightAlign", GetRightAlign());
        }
        // Top Align
        if (GetTopAlignEnabled())
        {
            node->Set("topAlign", GetTopAlign());
        }
        // Vertical Center Align
        if (GetVCenterAlignEnabled())
        {
            node->Set("vcenterAlign", GetVCenterAlign());
        }
        // Bottom Align
        if (GetBottomAlignEnabled())
        {
            node->Set("bottomAlign", GetBottomAlign());
        }
        // Anchor data

        UIControlBackground *baseBackground = baseControl->GetBackground();
        if (!baseBackground->IsEqualTo(GetBackground()))
        {
            // Draw type, obligatory for UI controls.
            UIControlBackground::eDrawType drawType =  GetBackground()->GetDrawType();
            if (baseBackground->GetDrawType() != drawType)
            {
                node->Set("drawType", loader->GetDrawTypeNodeValue(drawType));
            }
            // Sprite
            String spritePath = Sprite::GetPathString(GetBackground()->GetSprite());
            if (Sprite::GetPathString(baseBackground->GetSprite()) != spritePath)
            {
                node->Set("sprite", spritePath);
            }
            // Frame
            if (baseBackground->GetFrame() != GetBackground()->GetFrame())
            {
                node->Set("frame", GetFrame());
            }
            // Color
            const Color &color =  GetBackground()->GetColor();
            if (baseBackground->GetColor() != color)
            {
                node->Set("color", VariantType(color));
            }
            // Color inherit
            UIControlBackground::eColorInheritType colorInheritType =  GetBackground()->GetColorInheritType();
            if (baseBackground->GetColorInheritType() != colorInheritType)
            {
                node->Set("colorInherit", loader->GetColorInheritTypeNodeValue(colorInheritType));
            }
            // Per pixel accuracy
            UIControlBackground::ePerPixelAccuracyType perPixelAccuracyType = GetBackground()->GetPerPixelAccuracyType();
            if (baseBackground->GetPerPixelAccuracyType() != perPixelAccuracyType)
            {
                node->Set("perPixelAccuracy", loader->GetPerPixelAccuracyTypeNodeValue(perPixelAccuracyType));
            }
            // Align
            int32 align = GetBackground()->GetAlign();
            if (baseBackground->GetAlign() != align)
            {
                node->AddNodeToMap("align", loader->GetAlignNodeValue(align));
            }
            // LeftRightStretchCapNode
            if (baseBackground->GetLeftRightStretchCap() != GetBackground()->GetLeftRightStretchCap())
            {
                node->Set("leftRightStretchCap", GetBackground()->GetLeftRightStretchCap());
            }
            // topBottomStretchCap
            if (baseBackground->GetTopBottomStretchCap() != GetBackground()->GetTopBottomStretchCap())
            {
                node->Set("topBottomStretchCap", GetBackground()->GetTopBottomStretchCap());
            }
            // spriteModification
            if (baseBackground->GetModification() != GetBackground()->GetModification())
            {
                node->Set("spriteModification", GetBackground()->GetModification());
            }

            // margins.
            const UIControlBackground::UIMargins* margins = GetBackground()->GetMargins();
            if (margins)
            {
                node->Set("margins", margins->AsVector4());
            }
        }
        return node;
    }

    void UIControl::LoadFromYamlNode(const YamlNode * node, UIYamlLoader * loader)
    {
        const YamlNode * rectNode = node->Get("rect");
        if (rectNode)
        {
            Rect rect = rectNode->AsRect();
            SetRect(rect);
        }
        else
        {
            const YamlNode * positionNode = node->Get("position");
            if (positionNode)
                SetPosition(positionNode->AsVector2());

            const YamlNode * sizeNode = node->Get("size");
            if (sizeNode)
                SetSize(sizeNode->AsVector2());
        }

        const YamlNode * pivotNode = node->Get("pivot");
        if (pivotNode)
        {
            DVASSERT(pivotNode->GetType() == YamlNode::TYPE_ARRAY);
            SetPivotPoint(pivotNode->AsPoint());
        }

        const YamlNode * angleNode = node->Get("angle");
        if (angleNode)
        {
            SetAngle(angleNode->AsFloat());
        }

        const YamlNode * enabledNode = node->Get("enabled");
        if (enabledNode)
        {
            SetDisabled(!enabledNode->AsBool());
        }

        const YamlNode * clipNode = node->Get("clip");
        if (clipNode)
        {
            SetClipContents(clipNode->AsBool());
        }

        const YamlNode * inputNode = node->Get("noInput");
        if (inputNode)
        {
            SetInputEnabled(!inputNode->AsBool(), false);
        }

        const YamlNode * tagNode = node->Get("tag");
        if (tagNode)
        {
            SetTag(tagNode->AsInt32());
        }

        const YamlNode * initialStateNode = node->Get("initialState");
        if (initialStateNode)
        {
            int32 newInitialState = initialStateNode->AsInt32();
            SetInitialState(newInitialState);
            SetState(newInitialState);
        }

        const YamlNode * leftAlignNode = node->Get("leftAlign");
        if (leftAlignNode)
        {
            float32 leftAlign = leftAlignNode->AsFloat();
            SetLeftAlignEnabled(true, false);
            SetLeftAlign(leftAlign, false);
        }

        const YamlNode * hcenterAlignNode = node->Get("hcenterAlign");
        if (hcenterAlignNode)
        {
            float32 hcenterAlign = hcenterAlignNode->AsFloat();
            SetHCenterAlignEnabled(true, false);
            SetHCenterAlign(hcenterAlign, false);
        }

        const YamlNode * rightAlignNode = node->Get("rightAlign");
        if (rightAlignNode)
        {
            float32 rightAlign = rightAlignNode->AsFloat();
            SetRightAlignEnabled(true, false);
            SetRightAlign(rightAlign, false);
        }

        const YamlNode * topAlignNode = node->Get("topAlign");
        if (topAlignNode)
        {
            float32 topAlign = topAlignNode->AsFloat();
            SetTopAlignEnabled(true, false);
            SetTopAlign(topAlign, false);
        }

        const YamlNode * vcenterAlignNode = node->Get("vcenterAlign");
        if (vcenterAlignNode)
        {
            float32 vcenterAlign = vcenterAlignNode->AsFloat();
            SetVCenterAlignEnabled(true, false);
            SetVCenterAlign(vcenterAlign, false);
        }

        const YamlNode * bottomAlignNode = node->Get("bottomAlign");
        if (bottomAlignNode)
        {
            float32 bottomAlign = bottomAlignNode->AsFloat();
            SetBottomAlignEnabled(true, false);
            SetBottomAlign(bottomAlign, false);
        }

        const YamlNode * visibleNode = node->Get("visible");
        const YamlNode * recursiveVisibleNode = node->Get("recursiveVisible");
        bool visibilityFlag = true;
        if(visibleNode)
        {
            visibilityFlag = loader->GetBoolFromYamlNode(visibleNode, true);
        }
        if (recursiveVisibleNode)
        {
            visibilityFlag &= loader->GetBoolFromYamlNode(recursiveVisibleNode, true);
        }

        SetVisible(visibilityFlag);

        const YamlNode * drawTypeNode = node->Get("drawType");
        if (drawTypeNode)
        {
            GetBackground()->SetDrawType((UIControlBackground::eDrawType)loader->GetDrawTypeFromNode(drawTypeNode));
        }

        const YamlNode * spriteNode = node->Get("sprite");
        if (spriteNode)
        {
            GetBackground()->SetSprite(spriteNode->AsString(), GetFrame());
        }

        const YamlNode * frameNode = node->Get("frame");
        if (frameNode)
        {
            GetBackground()->SetFrame(frameNode->AsInt32());
        }

        const YamlNode * colorNode = node->Get("color");
        if (colorNode)
        {
            GetBackground()->SetColor(loader->GetColorFromYamlNode(colorNode));
        }

        const YamlNode * perPixelAccuracyTypeNode = node->Get("perPixelAccuracy");
        if(perPixelAccuracyTypeNode)
        {
            GetBackground()->SetPerPixelAccuracyType((UIControlBackground::ePerPixelAccuracyType)loader->GetPerPixelAccuracyTypeFromNode(perPixelAccuracyTypeNode));
        }

        const YamlNode * colorInheritNode = node->Get("colorInherit");
        if (colorInheritNode)
        {
            GetBackground()->SetColorInheritType((UIControlBackground::eColorInheritType)loader->GetColorInheritTypeFromNode(colorInheritNode));
        }

        const YamlNode * alignNode = node->Get("align");
        if (alignNode)
        {
            GetBackground()->SetAlign(loader->GetAlignFromYamlNode(alignNode));
        }

        const YamlNode * leftRightStretchCapNode = node->Get("leftRightStretchCap");
        if (leftRightStretchCapNode)
        {
            GetBackground()->SetLeftRightStretchCap(leftRightStretchCapNode->AsFloat());
        }

        const YamlNode * topBottomStretchCapNode = node->Get("topBottomStretchCap");
        if (topBottomStretchCapNode)
        {
            GetBackground()->SetTopBottomStretchCap(topBottomStretchCapNode->AsFloat());
        }

        const YamlNode * spriteModificationNode = node->Get("spriteModification");
        if (spriteModificationNode)
        {
            GetBackground()->SetModification(spriteModificationNode->AsInt32());
        }
        
        const YamlNode * marginsNode = node->Get("margins");
        if (marginsNode)
        {
            UIControlBackground::UIMargins margins(marginsNode->AsVector4());
            GetBackground()->SetMargins(&margins);
        }
    }

    Animation * UIControl::WaitAnimation(float32 time, int32 track)
    {
        Animation * animation = new Animation(this, time, Interpolation::LINEAR);
        animation->Start(track);
        return animation;
    }

    Animation * UIControl::PositionAnimation(const Vector2 & _position, float32 time, Interpolation::FuncType interpolationFunc, int32 track)
    {
        LinearAnimation<Vector2> * animation = new LinearAnimation<Vector2>(this, &relativePosition, _position, time, interpolationFunc);
        animation->Start(track);
        return animation;
    }

    Animation * UIControl::SizeAnimation(const Vector2 & _size, float32 time, Interpolation::FuncType interpolationFunc, int32 track)
    {
        LinearAnimation<Vector2> * animation = new LinearAnimation<Vector2>(this, &size, _size, time, interpolationFunc);
        animation->Start(track);
        return animation;
    }

    Animation * UIControl::ScaleAnimation(const Vector2 & newScale, float32 time, Interpolation::FuncType interpolationFunc, int32 track)
    {
        LinearAnimation<Vector2> * animation = new LinearAnimation<Vector2>(this, &scale, newScale, time, interpolationFunc);
        animation->Start(track);
        return animation;
    }

    Animation * UIControl::AngleAnimation(float32 newAngle, float32 time, Interpolation::FuncType interpolationFunc /*= Interpolation::LINEAR*/, int32 track /*= 0*/)
    {
        LinearAnimation<float32> * animation = new LinearAnimation<float32>(this, &angle, newAngle, time, interpolationFunc);
        animation->Start(track);
        return animation;
    }


    Animation * UIControl::MoveAnimation(const Rect & rect, float time, Interpolation::FuncType interpolationFunc, int32 track)
    {
        TwoVector2LinearAnimation *animation = new TwoVector2LinearAnimation(this
                , &relativePosition, Vector2(rect.x + pivotPoint.x, rect.y + pivotPoint.y)
                , &size, Vector2(rect.dx, rect.dy), time, interpolationFunc);
        animation->Start(track);
        return animation;
    }

    Animation * UIControl::ScaledRectAnimation(const Rect & rect, float32 time, Interpolation::FuncType interpolationFunc, int32 track)
    {
        Vector2 finalScale(rect.dx / size.x, rect.dy / size.y);

        TwoVector2LinearAnimation *animation = new TwoVector2LinearAnimation(this
                , &relativePosition, Vector2(rect.x + pivotPoint.x * finalScale.x, rect.y + pivotPoint.y * finalScale.y)
                , &scale, finalScale, time, interpolationFunc);
        animation->Start(track);
        return animation;
    }

    Animation * UIControl::ScaledSizeAnimation(const Vector2 & newSize, float32 time, Interpolation::FuncType interpolationFunc, int32 track)
    {
        Vector2 finalScale(newSize.x / size.x, newSize.y / size.y);
        LinearAnimation<Vector2> * animation = new LinearAnimation<Vector2>(this, &scale, finalScale, time, interpolationFunc);
        animation->Start(track);
        return animation;
    }


    void UIControl::TouchableAnimationCallback(BaseObject * caller, void * param, void *callerData)
    {
        bool * params = (bool*)param;
        SetInputEnabled(params[0], params[1]);
        delete[]params;
    }

    Animation * UIControl::TouchableAnimation(bool touchable, bool hierarhic/* = true*/, int32 track/* = 0*/)
    {
        //TODO: change to bool animation - Dizz
        Animation * animation = new Animation(this, 0.01f, Interpolation::LINEAR);
        bool * params = new bool[2];
        params[0] = touchable;
        params[1] = hierarhic;
        animation->AddEvent(Animation::EVENT_ANIMATION_START, Message(this, &UIControl::TouchableAnimationCallback, (void*)params));
        animation->Start(track);
        return animation;
    }

    void UIControl::DisabledAnimationCallback(BaseObject * caller, void * param, void *callerData)
    {
        bool * params = (bool*)param;
        SetDisabled(params[0], params[1]);
        delete[]params;
    }

    Animation * UIControl::DisabledAnimation(bool disabled, bool hierarhic/* = true*/, int32 track/* = 0*/)
    {
        //TODO: change to bool animation - Dizz
        Animation * animation = new Animation(this, 0.01f, Interpolation::LINEAR);
        bool * params = new bool[2];
        params[0] = disabled;
        params[1] = hierarhic;
        animation->AddEvent(Animation::EVENT_ANIMATION_START, Message(this, &UIControl::DisabledAnimationCallback, (void*)params));
        animation->Start(track);
        return animation;
    }

    void UIControl::VisibleAnimationCallback( BaseObject * caller, void * param, void *callerData )
    {
        bool visible = ( pointer_size(param) > 0 );
        SetVisible(visible);
    }

    Animation * UIControl::VisibleAnimation(bool visible, int32 track/* = 0*/)
    {
        Animation * animation = new Animation(this, 0.01f, Interpolation::LINEAR);
        animation->AddEvent(Animation::EVENT_ANIMATION_START, Message(this, &UIControl::VisibleAnimationCallback, (void*)(pointer_size)visible));
        animation->Start(track);
        return animation;
    }

    void UIControl::RemoveControlAnimationCallback(BaseObject * caller, void * param, void *callerData)
    {
        if(parent)
        {
            parent->RemoveControl(this);
        }
    }

    Animation * UIControl::RemoveControlAnimation(int32 track)
    {
        Animation * animation = new Animation(this, 0.01f, Interpolation::LINEAR);
        animation->AddEvent(Animation::EVENT_ANIMATION_START, Message(this, &UIControl::RemoveControlAnimationCallback));
        animation->Start(track);
        return animation;
    }

    Animation * UIControl::ColorAnimation(const Color & finalColor, float32 time, Interpolation::FuncType interpolationFunc, int32 track)
    {
        LinearAnimation<Color> * animation = new LinearAnimation<Color>(this, &background->color, finalColor, time, interpolationFunc);
        animation->Start(track);
        return animation;
    }

    void UIControl::OnAllAnimationsFinished()
    {
        PerformEvent(UIControl::EVENT_ALL_ANIMATIONS_FINISHED);
    }
	
    void UIControl::SetDebugDraw(bool _debugDrawEnabled, bool hierarchic/* = false*/)
    {
        debugDrawEnabled = _debugDrawEnabled;
        if(hierarchic)
        {
            List<UIControl*>::iterator it = childs.begin();
            for(; it != childs.end(); ++it)
            {
                (*it)->SetDebugDraw(debugDrawEnabled, hierarchic);
            }
        }
    }

    void UIControl::SetDebugDrawColor(const Color& color)
    {
        debugDrawColor = color;
    }

    const Color &UIControl::GetDebugDrawColor() const
    {
        return debugDrawColor;
    }

    void UIControl::SetDrawPivotPointMode(eDebugDrawPivotMode mode, bool hierarchic /*=false*/)
    {
        drawPivotPointMode = mode;
        if (hierarchic)
        {
            List<UIControl*>::iterator it = childs.begin();
            for(; it != childs.end(); ++it)
            {
                (*it)->SetDrawPivotPointMode(mode, hierarchic);
            }
        }
    }

    bool UIControl::IsLostFocusAllowed( UIControl *newFocus )
    {
        return true;
    }

    void UIControl::SystemOnFocusLost(UIControl *newFocus)
    {
        PerformEvent(EVENT_FOCUS_LOST);
        OnFocusLost(newFocus);
    }

    void UIControl::SystemOnFocused()
    {
        PerformEvent(EVENT_FOCUS_SET);
        OnFocused();
    }

    void UIControl::OnFocusLost(UIControl *newFocus)
    {
    }

    void UIControl::OnFocused()
    {
    }

    void UIControl::SetSizeFromBg(bool pivotToCenter)
    {
        SetSize(GetBackground()->GetSprite()->GetSize());

        if (pivotToCenter)
        {
            SetPivot(Vector2(0.5f, 0.5f));
        }
    }

    void UIControl::CalculateAlignSettings()
    {
        if (!parent)
            return;

        const Rect &rect = GetRect();
        const Vector2 &parentSize = parent->GetSize();

        if (GetLeftAlignEnabled() || GetHCenterAlignEnabled() || GetRightAlignEnabled())
        {
            GetAlignDataByAxisData(rect.dx, rect.x, parentSize.x,
                GetLeftAlignEnabled(), GetHCenterAlignEnabled(), GetRightAlignEnabled(),
                leftAlign, hcenterAlign, rightAlign);

            SetLeftAlign(leftAlign);
            SetHCenterAlign(hcenterAlign);
            SetRightAlign(rightAlign);
        }

        if (GetTopAlignEnabled() || GetVCenterAlignEnabled() || GetBottomAlignEnabled())
        {
            GetAlignDataByAxisData(rect.dy, rect.y, parentSize.y,
                GetTopAlignEnabled(), GetVCenterAlignEnabled(), GetBottomAlignEnabled(),
                topAlign, vcenterAlign, bottomAlign);

            SetTopAlign(topAlign);
            SetVCenterAlign(vcenterAlign);
            SetBottomAlign(bottomAlign);
        }
    }

    void UIControl::ApplyAlignSettings()
    {
        if (!parent)
            return;

        const Rect &rect = GetRect();
        const Vector2 &parentSize = parent->GetSize();

        Rect newRect = rect;

        if (GetLeftAlignEnabled() || GetHCenterAlignEnabled() || GetRightAlignEnabled())
        {
            GetAxisDataByAlignData(rect.dx, parentSize.x,
                GetLeftAlignEnabled(), leftAlign,
                GetHCenterAlignEnabled(), hcenterAlign,
                GetRightAlignEnabled(), rightAlign,
                newRect.x, newRect.dx);

        }
        if (GetTopAlignEnabled() || GetVCenterAlignEnabled() || GetBottomAlignEnabled())
        {
            GetAxisDataByAlignData(rect.dy, parentSize.y,
                GetTopAlignEnabled(), topAlign,
                GetVCenterAlignEnabled(), vcenterAlign,
                GetBottomAlignEnabled(), bottomAlign,
                newRect.y, newRect.dy);
        }

        if (rect == newRect)
            return;

        SetSize(newRect.GetSize());

        SetPosition(newRect.GetPosition() + GetPivotPoint());
    }

    void UIControl::GetAxisDataByAlignData(float32 size, float32 parentSize,
        bool firstSideAlignEnabled, float32 firstSideAlign,
        bool centerAlignEnabled, float32 centerAlign,
        bool secondSideAlignEnabled, float32 secondSideAlign,
        float32 &newPos, float32 &newSize)
    {
        if (firstSideAlignEnabled && secondSideAlignEnabled)
        {
            newPos = firstSideAlign;
            newSize = parentSize - (firstSideAlign + secondSideAlign);
        }
        else if (firstSideAlignEnabled && centerAlignEnabled)
        {
            newPos = firstSideAlign;
            newSize = parentSize / 2.0f - (firstSideAlign - centerAlign);
        }
        else if (centerAlignEnabled && secondSideAlignEnabled)
        {
            newPos = parentSize / 2.0f + centerAlign;
            newSize = parentSize / 2.0f - (centerAlign + secondSideAlign);
        }
        else if (firstSideAlignEnabled)
        {
            newPos = firstSideAlign;
            newSize = size;
        }
        else if (secondSideAlignEnabled)
        {
            newPos = parentSize - (size + secondSideAlign);
            newSize = size;
        }
        else if (centerAlignEnabled)
        {
            newPos = (parentSize - size) / 2.0f + centerAlign;
            newSize = size;
        }
    }

    void UIControl::GetAlignDataByAxisData(float32 size, float32 pos, float32 parentSize, bool firstSideAlignEnabled, bool centerAlignEnabled, bool secondSideAlignEnabled, float32 &firstSideAlign, float32 &centerAlign, float32 &secondSideAlign)
    {
        if (firstSideAlignEnabled && secondSideAlignEnabled)
        {
            firstSideAlign = pos;
            secondSideAlign = parentSize - (pos + size);
        }
        else if (firstSideAlignEnabled && centerAlignEnabled)
        {
            firstSideAlign = pos;
            centerAlign = pos + size - parentSize / 2.0f;
        }
        else if (centerAlignEnabled && secondSideAlignEnabled)
        {
            centerAlign = pos - parentSize / 2.0f;
            secondSideAlign = parentSize - (pos + size);
        }
        else if (firstSideAlignEnabled)
        {
            firstSideAlign = pos;
        }
        else if (secondSideAlignEnabled)
        {
            secondSideAlign = parentSize - (pos + size);
        }
        else if (centerAlignEnabled)
        {
            centerAlign = pos - parentSize / 2.0f + size / 2.0f;
        }
    }

    void UIControl::ApplyAlignSettingsForChildren()
    {
        UpdateChildrenLayout();
    }

    const String & UIControl::GetControlClassName() const
    {
        return GetClassName();
    }

    const String &UIControl::GetCustomControlClassName() const
    {
        return customControlType;
    }

    void UIControl::SetCustomControlClassName(const String& value)
    {
        customControlType = value;
    }

    void UIControl::ResetCustomControlClassName()
    {
        customControlType = String();
    }

    void UIControl::SetPreferredNodeType(YamlNode* node, const String& nodeTypeName)
    {
        // Do we have Custom Control name? If yes, use it as type and passed one
        // as the "Base Type"
        bool hasCustomControl = !GetCustomControlClassName().empty();
        if (hasCustomControl)
        {
            node->Set("type", GetCustomControlClassName());
            node->Set("baseType", nodeTypeName);
        }
        else
        {
            // The type coincides with the node type name passed, no base type exists.
            node->Set("type", nodeTypeName);
        }
    }

    int32 UIControl::GetInitialState() const
    {
        return initialState;
    }

    void UIControl::SetInitialState(int32 newState)
    {
        initialState = newState;
    }

    void UIControl::RegisterInputProcessor()
    {
        inputProcessorsCount++;
        if (parent)
        {
            parent->RegisterInputProcessor();
        }
    }

    void UIControl::RegisterInputProcessors(int32 processorsCount)
    {
        inputProcessorsCount += processorsCount;
        if (parent)
        {
            parent->RegisterInputProcessors(processorsCount);
        }
    }

    void UIControl::UnregisterInputProcessor()
    {
        inputProcessorsCount--;
        DVASSERT(inputProcessorsCount >= 0);
        if (parent)
        {
            parent->UnregisterInputProcessor();
        }
    }
    void UIControl::UnregisterInputProcessors(int32 processorsCount)
    {
        inputProcessorsCount -= processorsCount;
        DVASSERT(inputProcessorsCount >= 0);
        if (parent)
        {
            parent->UnregisterInputProcessors(processorsCount);
        }
    }

    void UIControl::DumpInputs(int32 depthLevel)
    {
        String outStr;
        for (int32 i = 0; i < depthLevel; i++)
        {
            outStr += "| ";
        }
        outStr += "\\-";
        outStr += name;
        if (inputProcessorsCount > 0)
        {
            outStr += " ";
            outStr += Format("%d", inputProcessorsCount);
        }

        if (inputEnabled)
        {
            outStr += " ***";
        }
        Logger::Info("%s", outStr.c_str());
        List<UIControl*>::iterator it = childs.begin();
        for(; it != childs.end(); ++it)
        {
            (*it)->DumpInputs(depthLevel + 1);
        }
    }    

    int32 UIControl::GetBackgroundComponentsCount() const
    {
        return 1;
    }
    
    UIControlBackground *UIControl::GetBackgroundComponent(int32 index) const
    {
        DVASSERT(index == 0);
        return background;
    }
    
    UIControlBackground *UIControl::CreateBackgroundComponent(int32 index) const
    {
        DVASSERT(index == 0);
        return new UIControlBackground();
    }
    
    void UIControl::SetBackgroundComponent(int32 index, UIControlBackground *bg)
    {
        DVASSERT(index == 0);
        SetBackground(bg);
    }
    
    String UIControl::GetBackgroundComponentName(int32 index) const
    {
        DVASSERT(index == 0);
        return "Background";
    }
    
    int32 UIControl::GetInternalControlsCount() const
    {
        return 0;
    }
    
    UIControl *UIControl::GetInternalControl(int32 index) const
    {
        DVASSERT(false);
        return NULL;
    }
    
    UIControl *UIControl::CreateInternalControl(int32 index) const
    {
        DVASSERT(false);
        return NULL;
    }
    
    void UIControl::SetInternalControl(int32 index, UIControl *control)
    {
        DVASSERT(false);
    }

    String UIControl::GetInternalControlName(int32 index) const
    {
        DVASSERT(false);
        return "";
    }

    String UIControl::GetInternalControlDescriptions() const
    {
        DVASSERT(false);
        return "";
    }

    void UIControl::UpdateLayout()
    {
        ApplyAlignSettings();
    }

    void UIControl::UpdateChildrenLayout()
    {
        const List<UIControl*>& realChildren = this->GetChildren();//YZ recalculate size for all controls
        for(List<UIControl*>::const_iterator iter = realChildren.begin(); iter != realChildren.end(); ++iter)
        {
            UIControl* child = (*iter);
            if (child)
            {
                child->UpdateLayout();
            }
        }
    }

    /* Components */
    bool CotrolComponentLessPredicate(Component * left, Component * right)
    {
        return left->GetType() < right->GetType();
    }

    void UIControl::AddComponent(Component * component)
    {
        DynamicTypeCheck<UIComponent*>(component)->SetControl(this);
        components.push_back(component);
        std::stable_sort(components.begin(), components.end(), CotrolComponentLessPredicate);
        UpdateFamily();
    }

    void UIControl::DetachComponent(const Vector<Component *>::iterator & it)
    {
        UIComponent * c = DynamicTypeCheck<UIComponent*>(*it);
        components.erase(it);
        UpdateFamily();
        c->SetControl(0);
    }

    Component * UIControl::GetComponent(uint32 componentType, uint32 index) const
    {
        Component * ret = 0;
        uint32 maxCount = family->GetComponentsCount(componentType);
        if (index < maxCount)
        {
            ret = components[family->GetComponentIndex(componentType, index)];
        }
        return ret;
    }

    Component * UIControl::GetOrCreateComponent(uint32 componentType, uint32 index)
    {
        Component * ret = GetComponent(componentType, index);
        if (!ret)
        {
            ret = Component::CreateByType(componentType);
            AddComponent(ret);
        }

        return ret;
    }

    inline void UIControl::UpdateFamily()
    {
        family = EntityFamily::GetOrCreate(components);
    }

    inline void UIControl::RemoveAllComponents()
    {
        while (!components.empty())
        {
            RemoveComponent(--components.end());
        }
    }

    void UIControl::RemoveComponent(const Vector<Component *>::iterator & it)
    {
        if (it != components.end())
        {
            UIComponent * c = DynamicTypeCheck<UIComponent*>(*it);
            DetachComponent(it);
            SafeDelete(c);
        }
    }

    void UIControl::RemoveComponent(uint32 componentType, uint32 index)
    {
        Component * c = GetComponent(componentType, index);
        if (c)
        {
            RemoveComponent(c);
        }
    }

    void UIControl::RemoveComponent(Component * component)
    {
        DetachComponent(component);
        SafeDelete(component);
    }

    void UIControl::DetachComponent(Component * component)
    {
        DVASSERT(component);
        auto it = std::find(components.begin(), components.end(), component);
        DetachComponent(it);
    }
    /* Components */

}
