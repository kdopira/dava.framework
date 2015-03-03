#ifndef __QUICKED_PACKAGE_MODEL_H__
#define __QUICKED_PACKAGE_MODEL_H__

#include <QAbstractItemModel>
#include <QMimeData>
#include <QStringList>
#include <QUndoStack>
#include "DAVAEngine.h"

class PackageNode;
class ControlNode;
class PackageBaseNode;
class PackageControlsNode;
class ControlsContainerNode;
class QtModelPackageCommandExecutor;

class PackageModel : public QAbstractItemModel
{
    Q_OBJECT
    
public:
    PackageModel(PackageNode *root, QtModelPackageCommandExecutor *commandExecutor, QObject *parent = 0);
    virtual ~PackageModel();
    
    void emitNodeChanged(PackageBaseNode *node);

    QModelIndex indexByNode(PackageBaseNode *node) const;
    virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    virtual QModelIndex parent(const QModelIndex &child) const override;
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const  override;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const  override;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

    virtual Qt::ItemFlags flags(const QModelIndex &index) const override;

    virtual Qt::DropActions supportedDropActions() const override;
    virtual QStringList mimeTypes() const override;
    virtual QMimeData *mimeData(const QModelIndexList &indexes) const override;
    virtual bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) override;
    
    void InsertControlNode(ControlNode *node, ControlsContainerNode *dest, int row);
    void RemoveControlNode(ControlNode *node, ControlsContainerNode *parent);
    void InsertImportedPackage(PackageControlsNode *node, PackageNode *dest, int destRow);
    void RemoveImportedPackage(PackageControlsNode *node, PackageNode *parent);
    
private:
    PackageNode *root;
    QtModelPackageCommandExecutor *commandExecutor;
};

#endif // __QUICKED_PACKAGE_MODEL_H__
