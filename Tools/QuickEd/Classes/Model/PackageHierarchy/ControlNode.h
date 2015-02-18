#ifndef __UI_EDITOR_CONTROL_NODE__
#define __UI_EDITOR_CONTROL_NODE__

#include "PackageBaseNode.h"
#include "ControlsContainerNode.h"

#include "Model/ControlProperties/PropertiesRoot.h"

class PackageSerializer;
class PackageNode;
class ControlPrototype;
class PackageRef;

class ControlNode : public ControlsContainerNode
{
public:
    enum eCreationType
    {
        CREATED_FROM_CLASS,
        CREATED_FROM_PROTOTYPE,
        CREATED_FROM_PROTOTYPE_CHILD
    };
    
private:
    ControlNode(DAVA::UIControl *control, PropertiesRoot *propertiesRoot, eCreationType creationType);
    virtual ~ControlNode();

public:
    static ControlNode *CreateFromControl(DAVA::UIControl *control);
    
    static ControlNode *CreateFromPrototype(ControlNode *sourceNode, PackageRef *nodePackage);
    static ControlNode *CreateFromPrototypeChild(ControlNode *sourceNode, PackageRef *nodePackage);
    
private:
    static ControlNode *CreateFromPrototypeImpl(ControlNode *sourceNode, PackageRef *nodePackage, bool root);

public:
    ControlNode *Clone();
    
    void Add(ControlNode *node) override;
    void InsertAtIndex(int index, ControlNode *node) override;
    void Remove(ControlNode *node) override;
    int GetCount() const override;
    ControlNode *Get(int index) const override;
    ControlNode *FindByName(const DAVA::String &name) const;
    
    virtual DAVA::String GetName() const;
    DAVA::UIControl *GetControl() const;
    ControlPrototype *GetPrototype() const;
    const DAVA::Vector<ControlNode*> &GetInstances() const;

    int GetFlags() const override;
    void SetReadOnly();
    
    eCreationType GetCreationType() const { return creationType; }

    PropertiesRoot *GetPropertiesRoot() const {return propertiesRoot; }
    BaseProperty *GetPropertyByPath(const DAVA::Vector<DAVA::String> &path);


    void MarkAsRemoved();
    void MarkAsAlive();

    void Serialize(PackageSerializer *serializer, PackageRef *currentPackage) const;

private:
    void CollectPrototypeChildrenWithChanges(DAVA::Vector<ControlNode*> &out) const;
    bool HasNonPrototypeChildren() const;
    
private:
    void AddControlToInstances(ControlNode *control);
    void RemoveControlFromInstances(ControlNode *control);

private:
    DAVA::UIControl *control;
    PropertiesRoot *propertiesRoot;
    DAVA::Vector<ControlNode*>nodes;
    
    ControlPrototype *prototype;
    DAVA::Vector<ControlNode*> instances; // week

    eCreationType creationType;
    
    bool readOnly;
    
};


#endif // __UI_EDITOR_CONTROL_NODE__
