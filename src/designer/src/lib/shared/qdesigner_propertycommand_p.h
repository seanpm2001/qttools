// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of Qt Designer.  This header
// file may change from version to version without notice, or even be removed.
//
// We mean it.
//

#ifndef QDESIGNER_PROPERTYCOMMAND_H
#define QDESIGNER_PROPERTYCOMMAND_H

#include "qdesigner_formwindowcommand_p.h"

#include <QtCore/qvariant.h>
#include <QtCore/qhash.h>
#include <QtCore/qlist.h>
#include <QtCore/qpair.h>
#include <QtCore/qpointer.h>
#include <QtCore/qsharedpointer.h>

QT_BEGIN_NAMESPACE

class QDesignerFormWindowInterface;
class QDesignerPropertySheetExtension;
class QDesignerIntegration;

namespace qdesigner_internal {

enum SpecialProperty {
        SP_None, SP_ObjectName, SP_LayoutName, SP_SpacerName,SP_WindowTitle,
        SP_MinimumSize, SP_MaximumSize, SP_Geometry, SP_Icon, SP_CurrentTabName, SP_CurrentItemName, SP_CurrentPageName,
        SP_AutoDefault, SP_Alignment, SP_Shortcut, SP_Orientation
};

//Determine special property
enum SpecialProperty getSpecialProperty(const QString& propertyName);

// A helper class for applying properties to objects.
// Can be used for Set commands (setValue(), restoreOldValue()) or
// Reset Commands     restoreDefaultValue(), restoreOldValue()).
//
class QDESIGNER_SHARED_EXPORT PropertyHelper {
    Q_DISABLE_COPY(PropertyHelper)
public:
    // A pair of Value and changed flag
    using Value = QPair<QVariant, bool>;

    enum ObjectType {OT_Object, OT_FreeAction, OT_AssociatedAction, OT_Widget};

    PropertyHelper(QObject* object,
                   SpecialProperty specialProperty,
                   QDesignerPropertySheetExtension *sheet,
                   int index);
    virtual ~PropertyHelper() = default;

    QObject *object() const { return m_object; }
    SpecialProperty specialProperty() const { return m_specialProperty; }
    // set a new value. Can be overwritten to perform a transformation (see
    // handling of Arrow key move in FormWindow class).
    virtual Value setValue(QDesignerFormWindowInterface *fw, const QVariant &value,
                           bool changed, quint64 subPropertyMask);

    // restore old value
    Value restoreOldValue(QDesignerFormWindowInterface *fw);
    // set default value
    Value restoreDefaultValue(QDesignerFormWindowInterface *fw);

    inline QVariant oldValue() const
    { return m_oldValue.first; }

    inline void setOldValue(const QVariant &oldValue)
    { m_oldValue.first = oldValue; }

    // required updates for this property (bit mask)
    enum UpdateMask { UpdatePropertyEditor=1, UpdateObjectInspector=2 };
    unsigned updateMask() const;

    // can be merged into one command (that is, object and name match)
    bool canMerge(const PropertyHelper &other) const;
    QDesignerIntegration *integration(QDesignerFormWindowInterface *fw) const;

    static void triggerActionChanged(QAction *a);

private:
    // Apply the value and update. Returns corrected value
    Value applyValue(QDesignerFormWindowInterface *fw, const QVariant &oldValue, Value newValue);

    static void checkApplyWidgetValue(QDesignerFormWindowInterface *fw, QWidget* w,
                                      SpecialProperty specialProperty, QVariant &v);

    void updateObject(QDesignerFormWindowInterface *fw, const QVariant &oldValue, const QVariant &newValue);
    QVariant findDefaultValue(QDesignerFormWindowInterface *fw) const;
    void ensureUniqueObjectName(QDesignerFormWindowInterface *fw, QObject *object) const;
    SpecialProperty m_specialProperty;

    QPointer<QObject> m_object;
    ObjectType m_objectType;
    QPointer<QWidget> m_parentWidget;

    QDesignerPropertySheetExtension *m_propertySheet;
    int m_index;

    Value m_oldValue;
};

// Base class for commands that can be applied to several widgets

class QDESIGNER_SHARED_EXPORT PropertyListCommand : public QDesignerFormWindowCommand {
public:
    explicit PropertyListCommand(QDesignerFormWindowInterface *formWindow, QUndoCommand *parent = nullptr);

    QObject* object(int index = 0) const;

    QVariant oldValue(int index = 0) const;

    void setOldValue(const QVariant &oldValue, int index = 0);

    // Calls restoreDefaultValue() and update()
    void undo() override;

protected:
    using PropertyHelperPtr = std::unique_ptr<PropertyHelper>;
    using PropertyHelperList = std::vector<PropertyHelperPtr>;

    // add an object
    bool add(QObject *object, const QString &propertyName);

    // Init from a list and make sure referenceObject is added first to obtain the right property group
    bool initList(const QObjectList &list, const QString &apropertyName, QObject *referenceObject = nullptr);

    // set a new value, return update mask
    unsigned setValue(const QVariant &value, bool changed, quint64 subPropertyMask);

    // restore old value,  return update mask
    unsigned  restoreOldValue();
    // set default value,  return update mask
    unsigned  restoreDefaultValue();

    // update designer
    void update(unsigned updateMask);

    // check if lists are aequivalent for command merging (same widgets and props)
    bool canMergeLists(const PropertyHelperList& other) const;

    PropertyHelperList& propertyHelperList() { return m_propertyHelperList; }
    const PropertyHelperList& propertyHelperList() const { return m_propertyHelperList; }

    const QString propertyName() const;
    SpecialProperty specialProperty() const;

    // Helper struct describing a property used for checking whether
    // properties of different widgets are equivalent
    struct PropertyDescription {
    public:
        PropertyDescription() = default;
        PropertyDescription(const QString &propertyName, QDesignerPropertySheetExtension *propertySheet, int index);
        bool equals(const PropertyDescription &p) const;
        void debug() const;

        QString m_propertyName;
        QString m_propertyGroup;
        int m_propertyType = QMetaType::UnknownType;
        SpecialProperty m_specialProperty = SP_None;
    };
    const PropertyDescription &propertyDescription() const { return  m_propertyDescription; }

protected:
    virtual std::unique_ptr<PropertyHelper>
    createPropertyHelper(QObject *o, SpecialProperty sp,
                         QDesignerPropertySheetExtension *sheet, int sheetIndex) const;

private:
    PropertyDescription m_propertyDescription;
    PropertyHelperList m_propertyHelperList;
};

class QDESIGNER_SHARED_EXPORT SetPropertyCommand: public PropertyListCommand
{

public:
    explicit SetPropertyCommand(QDesignerFormWindowInterface *formWindow, QUndoCommand *parent = nullptr);

    bool init(QObject *object, const QString &propertyName, const QVariant &newValue);
    bool init(const QObjectList &list, const QString &propertyName, const QVariant &newValue,
              QObject *referenceObject = nullptr, bool enableSubPropertyHandling = true);


    inline QVariant newValue() const
    { return m_newValue; }

    inline void setNewValue(const QVariant &newValue)
    { m_newValue = newValue; }

    int id() const override;
    bool mergeWith(const QUndoCommand *other) override;

    void redo() override;

protected:
    virtual QVariant mergeValue(const QVariant &newValue);

private:
    quint64 subPropertyMask(const QVariant &newValue, QObject *referenceObject);
    void setDescription();
    QVariant m_newValue;
    quint64 m_subPropertyMask;
};

class QDESIGNER_SHARED_EXPORT ResetPropertyCommand: public PropertyListCommand
{

public:
    explicit ResetPropertyCommand(QDesignerFormWindowInterface *formWindow);

    bool init(QObject *object, const QString &propertyName);
    bool init(const QObjectList &list, const QString &propertyName, QObject *referenceObject = nullptr);

    void redo() override;

protected:
    bool mergeWith(const QUndoCommand *) override { return false; }

private:
    void setDescription();
    QString m_propertyName;
};


class QDESIGNER_SHARED_EXPORT AddDynamicPropertyCommand: public QDesignerFormWindowCommand
{

public:
    explicit AddDynamicPropertyCommand(QDesignerFormWindowInterface *formWindow);

    bool init(const QObjectList &selection, QObject *current, const QString &propertyName, const QVariant &value);

    void redo() override;
    void undo() override;
private:
    void setDescription();
    QString m_propertyName;
    QObjectList m_selection;
    QVariant m_value;
};

class QDESIGNER_SHARED_EXPORT RemoveDynamicPropertyCommand: public QDesignerFormWindowCommand
{

public:
    explicit RemoveDynamicPropertyCommand(QDesignerFormWindowInterface *formWindow);

    bool init(const QObjectList &selection, QObject *current, const QString &propertyName);

    void redo() override;
    void undo() override;
private:
    void setDescription();
    QString m_propertyName;
    QHash<QObject *, QPair<QVariant, bool> > m_objectToValueAndChanged;
};

} // namespace qdesigner_internal

QT_END_NAMESPACE

#endif // QDESIGNER_PROPERTYCOMMAND_H
