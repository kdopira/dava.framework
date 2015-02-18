#ifndef __QUICKED_PACKAGE_COMMAND_EXECUTOR_H__
#define __QUICKED_PACKAGE_COMMAND_EXECUTOR_H__

#include "Base/BaseObject.h"

class ControlNode;
class PackageControlsNode;
class PackageNode;
class BaseProperty;
class ControlsContainerNode;

class PackageCommandExecutor : public DAVA::BaseObject
{
public:
    PackageCommandExecutor();
    virtual ~PackageCommandExecutor();
    
    virtual void AddImportedPackageIntoPackage(PackageControlsNode *importedPackageControls, PackageNode *package) = 0;
    virtual void ChangeProperty(ControlNode *node, BaseProperty *property, const DAVA::VariantType &value) = 0;
    virtual void ResetProperty(ControlNode *node, BaseProperty *property) = 0;

    virtual void InsertControl(ControlNode *control, ControlsContainerNode *package, DAVA::int32 destIndex) = 0;
    virtual void CopyControls(const DAVA::Vector<ControlNode*> &nodes, ControlsContainerNode *dest, DAVA::int32 destIndex) = 0;
    virtual void MoveControls(const DAVA::Vector<ControlNode*> &nodes, ControlsContainerNode *dest, DAVA::int32 destIndex) = 0;
    virtual void RemoveControl(ControlNode* node) = 0;
    virtual void RemoveControls(const DAVA::Vector<ControlNode*> &nodes) = 0;
};

class DefaultPackageCommandExecutor : public PackageCommandExecutor
{
public:
    DefaultPackageCommandExecutor();
    virtual ~DefaultPackageCommandExecutor();

    void AddImportedPackageIntoPackage(PackageControlsNode *importedPackageControls, PackageNode *package) override;
    void ChangeProperty(ControlNode *node, BaseProperty *property, const DAVA::VariantType &value) override;
    void ResetProperty(ControlNode *node, BaseProperty *property) override;
    
    void InsertControl(ControlNode *control, ControlsContainerNode *dest, DAVA::int32 destIndex) override;
    void CopyControls(const DAVA::Vector<ControlNode*> &nodes, ControlsContainerNode *dest, DAVA::int32 destIndex) override;
    void MoveControls(const DAVA::Vector<ControlNode*> &nodes, ControlsContainerNode *dest, DAVA::int32 destIndex) override;
    void RemoveControl(ControlNode* node) override;
    void RemoveControls(const DAVA::Vector<ControlNode*> &nodes) override;

};

#endif // __QUICKED_PACKAGE_COMMAND_EXECUTOR_H__
