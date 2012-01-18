/*==================================================================================
    Copyright (c) 2008, DAVA Consulting, LLC
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the DAVA Consulting, LLC nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE DAVA CONSULTING, LLC AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL DAVA CONSULTING, LLC BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

    Revision History:
        * Created by Vitaliy Borodovsky 
=====================================================================================*/
#ifndef __DAVAENGINE_SCENENODE_H__
#define __DAVAENGINE_SCENENODE_H__

#include "Base/BaseObject.h"
#include "Base/BaseTypes.h"
#include "Base/BaseMath.h"
#include "Render/RenderBase.h"
#include "Scene3D/SceneNodeAnimationKey.h"
#include <deque>

namespace DAVA
{

class Scene;
class SceneNodeAnimation;
class SceneNodeAnimationKey;
class KeyedArchive;
class SceneFileV2;
/**
    \brief Root class of 3D scene hierarchy. 
 
 */
class SceneNode : public BaseObject
{
public:	
	SceneNode(Scene * scene = 0);
	virtual ~SceneNode();
	
    
    virtual void SetScene(Scene * _scene);
    
	// working with childs
	virtual void	AddNode(SceneNode * node);
	virtual void	RemoveNode(SceneNode * node);
	virtual SceneNode * GetChild(int32 index);
	virtual int32 GetChildrenCount();
	virtual void	RemoveAllChildren();
    
	virtual bool FindNodesByNamePart(const String & namePart, List<SceneNode *> &outNodeList);
    
    /**
        \brief Find node by it's name inside this scene node.
        \param[in] name name of object you want to find. 
        \returns pointer to the object if object with such name available, 0 in case if it's not exists.
     */
	virtual SceneNode *	FindByName(const String & name);
    /**
        \brief Set name of this particular node.
        \param[in] new name for this node
     */
    virtual void SetName(const String & name);

    /**
        \brief Get name of this particular node.
        \returns name of this node
     */
    inline const String & GetName();

    /**
        \brief Get full name of this node from root. This function is slow because it go up by hierarchy and make full node name.
        \returns this node full name from root. Example [MaxScene->camera->instance0]
     */
    String GetFullName();
    
    /**
        \brief Set tag for this object.
        Tag can be used to identify object, or find it. You can mark objects with same properies by tag, and later find them using tag criteria. 
     */
    inline void SetTag(int32 _tag);
    
    /**
        \brief Return tag for this object
        \returns tag for this object
     */
    inline const int32 GetTag(); 

	
	// virtual updates
	virtual void	Update(float32 timeElapsed);
	virtual void	Draw();
	
	// properties
	inline void SetVisible(bool isVisible);
	inline SceneNode * GetParent();
	
	// extract data from current node to use it in animations
	void ExtractCurrentNodeKeyForAnimation(SceneNodeAnimationKey & resultKey);
	
    inline const Matrix4 & GetLocalTransform(); 

    /**
     \brief Accamulates local transform from the requested parent to this node.
     */
    Matrix4 AccamulateLocalTransform(SceneNode *fromParent);

    /**
     \brief This method means that you always modify geted matrix. 
        If you dont want to modify matrix call GetLocalTransform().
     */
    inline Matrix4 & ModifyLocalTransform(); 
    inline const Matrix4 & GetWorldTransform();
    inline const Matrix4 & GetDefaultLocalTransform(); 
    
    inline void SetLocalTransform(const Matrix4 & newMatrix);
    inline void SetDefaultLocalTransform(const Matrix4 & newMatrix);
    inline void InvalidateLocalTransform();
    
    /*
        Go down by hierarchy and bake all transforms
     */
    virtual void BakeTransforms();
    
	enum
    {
        // NODE_STATIC = 0x1,  // this flag means that node is always static and we do not need to update it's worldTransform
        // NODE_DYNAMIC = 0x2, // node automatically become dynamic when we update it's local matrix
        NODE_WORLD_MATRIX_ACTUAL = 0x4, // if this flag set this means we do not need to rebuild worldMatrix
    };
	
	// animations 
	void ExecuteAnimation(SceneNodeAnimation * animation);	
	void DetachAnimation(SceneNodeAnimation * animation);
	virtual void StopAllAnimations(bool recursive = true);
	void RestoreOriginalTransforms();

	
    virtual SceneNode* Clone(SceneNode *dstNode = NULL);
	
    // Do not use variables 
    std::deque<SceneNodeAnimation *> nodeAnimations;

	enum
	{
		DEBUG_DRAW_NONE = 0,
		DEBUG_DRAW_AABBOX = 1,              
		DEBUG_DRAW_LOCAL_AXIS = 2,
		DEBUG_DRAW_AABOX_CORNERS = 4,
		DEBUG_DRAW_LIGHT_NODE = 8,
        DEBUG_DRAW_NORMALS = 16,
        DEBUG_DRAW_ALL = 0xFFFFFFFF,
	};
	/**
        \brief function to enable or disable debug drawing for particular node.
        By default it's not recursive. Some objects may support flags only partially.
        For example if node do not have bounding box flag DEBUG_DRAW_AABBOX will not produce any output
        These flags are mostly for debug purposes and we do not guarantee that logic of the debug rendering will remain unchanged between 
        framework versions.
     
        \param[in] debugFlags flags to be set
        \param[in] isRecursive do you want to set flags recursively
     
     */
	void SetDebugFlags(uint32 debugFlags, bool isRecursive = false);  
    /**
        \brief function returns debug flags of specific node
        \returns flags of this specific scene node
     */
    uint32 GetDebugFlags() const;
    
    inline Scene *GetScene() const;
	
    void SetSolid(bool isSolid);
    bool GetSolid();
    
	inline void SetUserData(BaseObject * newData)
	{
		SafeRelease(userData);
		userData = newData;
		SafeRetain(userData);
	}

	inline BaseObject * GetUserData(void)
	{
		return userData;
	}
	
//	bool isSolidNode;
    
    //Returns maximum Bounding Box as WorlTransformedBox

    /**
        \brief function returns maximum bounding box of scene in world coordinates.
        \returns bounding box
     */
    virtual AABBox3 GetWTMaximumBoundingBox();
    
    
    /**
        \brief virtual function to save node to KeyedArchive
     */
    virtual void Save(KeyedArchive * archive, SceneFileV2 * sceneFileV2);
    
    /**
        \brief virtual function to load node to KeyedArchive
     */
	virtual void Load(KeyedArchive * archive, SceneFileV2 * sceneFileV2);
    
    /**
        \brief Function to get node description for debug printing
     */
    virtual String GetDebugDescription();
    
    KeyedArchive *GetCustomProperties();
    
protected:

    String RecursiveBuildFullName(SceneNode * node, SceneNode * endNode);
    
//    virtual SceneNode* CopyDataTo(SceneNode *dstNode);
	void SetParent(SceneNode * node);
	BaseObject * userData;

	Scene * scene;
	SceneNode * parent;
	std::vector<SceneNode*> children;
	std::deque<SceneNode*> removedCache;
	bool visible;
    bool inUpdate;

	String	name;
	int32	tag;

    uint32 flags;
    uint32 debugFlags;

	Matrix4 worldTransform;

    KeyedArchive *customProperties;
    
private:
    Matrix4 localTransform;
    Matrix4 defaultLocalTransform;
    
};

inline void SceneNode::SetVisible(bool isVisible)
{
	visible = isVisible;
}
	
inline SceneNode * SceneNode::GetParent()
{
	return parent;
}
    
inline const String & SceneNode::GetName()
{
    return name;
}

inline const int32 SceneNode::GetTag() 
{ 
    return tag; 
}
    
inline const Matrix4 & SceneNode::GetLocalTransform() 
{ 
    return localTransform; 
}; 

inline const Matrix4 & SceneNode::GetWorldTransform() 
{ 
    return worldTransform; 
};
    
inline const Matrix4 & SceneNode::GetDefaultLocalTransform()
{
    return defaultLocalTransform;
}
    
inline Matrix4 & SceneNode::ModifyLocalTransform()
{
    flags &= ~NODE_WORLD_MATRIX_ACTUAL;
    return localTransform;
}

inline void SceneNode::SetLocalTransform(const Matrix4 & newMatrix)
{
    localTransform = newMatrix;
    flags &= ~NODE_WORLD_MATRIX_ACTUAL;
}
    
inline void SceneNode::InvalidateLocalTransform()
{
    flags &= ~NODE_WORLD_MATRIX_ACTUAL;
}

    
inline void SceneNode::SetDefaultLocalTransform(const Matrix4 & newMatrix)
{
    defaultLocalTransform = newMatrix;
}
    
inline void SceneNode::SetTag(int32 _tag)
{
    tag = _tag;
}
    
inline uint32 SceneNode::GetDebugFlags() const
{
    return debugFlags;
}
    
inline Scene *SceneNode::GetScene() const
{
    return scene;
}


};

#endif // __DAVAENGINE_SCENENODE_H__





