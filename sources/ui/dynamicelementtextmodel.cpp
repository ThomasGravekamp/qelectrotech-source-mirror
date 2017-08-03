/*
	Copyright 2006-2017 The QElectroTech Team
	This file is part of QElectroTech.

	QElectroTech is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 2 of the License, or
	(at your option) any later version.

	QElectroTech is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with QElectroTech. If not, see <http://www.gnu.org/licenses/>.
*/
#include "dynamicelementtextmodel.h"
#include "dynamicelementtextitem.h"
#include <QStandardItem>
#include <QHash>
#include <QColorDialog>
#include <QModelIndex>
#include <QComboBox>
#include <QUndoCommand>
#include "QPropertyUndoCommand/qpropertyundocommand.h"

DynamicElementTextModel::DynamicElementTextModel(QObject *parent) :
QStandardItemModel(parent)
{
    setColumnCount(2);
    setHeaderData(0, Qt::Horizontal, tr("Propriété"), Qt::DisplayRole);
    setHeaderData(1, Qt::Horizontal, tr("Valeur"), Qt::DisplayRole);
    
    connect(this, &DynamicElementTextModel::itemChanged, this, &DynamicElementTextModel::dataEdited);
}

DynamicElementTextModel::~DynamicElementTextModel()
{
		//Connection is not destroy automaticaly,
		//because was not connected to a slot, but a lambda
	for(DynamicElementTextItem *deti : m_hash_text_connect.keys())
		setConnection(deti, false);
}

/**
 * @brief DynamicElementTextModel::addText
 * @param deti
 */
void DynamicElementTextModel::addText(DynamicElementTextItem *deti)
{
    if(m_texts_list.keys().contains(deti))
        return;
    
    QList <QStandardItem *> qsi_list;
    
	QStandardItem *qsi = new QStandardItem(deti->toPlainText());
    qsi->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
	
	
        //Source of text
    QStandardItem *src = new QStandardItem(tr("Source du texte"));
    src->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    
    QStandardItem *srca = new QStandardItem(deti->textFrom() == DynamicElementTextItem::UserText ? tr("Texte utilisateur") : tr("Information de l'élément"));
    srca->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable);
    srca->setData(textFrom, Qt::UserRole+1);
    
    qsi_list << src << srca;
    qsi->appendRow(qsi_list);
	
		//User text
	QStandardItem *usr = new QStandardItem(tr("Texte"));
    usr->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    
    QStandardItem *usra = new QStandardItem(deti->toPlainText());
    usra->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable);
    usra->setData(DynamicElementTextModel::userText, Qt::UserRole+1);
   
	qsi_list.clear();
    qsi_list << usr << usra;
    src->appendRow(qsi_list);
	
		//Info text
	QStandardItem *info = new QStandardItem(tr("Information"));
    info->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    
    QStandardItem *infoa = new QStandardItem(deti->toPlainText());
    infoa->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable);
    infoa->setData(DynamicElementTextModel::infoText, Qt::UserRole+1);
    
	qsi_list.clear();
    qsi_list << info << infoa;
    src->appendRow(qsi_list);

    
        //Size
	QStandardItem *size = new QStandardItem(tr("Taille"));
    size->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    
    QStandardItem *siza = new QStandardItem();
    siza->setData(deti->fontSize(), Qt::EditRole);
    siza->setData(DynamicElementTextModel::size, Qt::UserRole+1);
    siza->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable);
    
    qsi_list.clear();
    qsi_list << size << siza;
	qsi->appendRow(qsi_list);
    
        //Tagg
    QStandardItem *tagg = new QStandardItem(tr("Tagg"));
    tagg->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    
    QStandardItem *tagga = new QStandardItem(deti->tagg());
    tagga->setData(DynamicElementTextModel::tagg, Qt::UserRole+1);
    tagga->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable);
    
    qsi_list.clear();
    qsi_list << tagg << tagga;
	qsi->appendRow(qsi_list);
    
        //Color
    QStandardItem *color = new QStandardItem(tr("Couleur"));
    color->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    
    QStandardItem *colora = new QStandardItem;
    colora->setData(deti->color(), Qt::ForegroundRole);
    colora->setData(deti->color(), Qt::EditRole);
    colora->setData(DynamicElementTextModel::color, Qt::UserRole+1);
    colora->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable);
    
    qsi_list.clear();
    qsi_list << color << colora;
	qsi->appendRow(qsi_list);
    
	qsi_list.clear();
	QStandardItem *empty_qsi = new QStandardItem(0);
	empty_qsi->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
	qsi_list << qsi << empty_qsi;
    this->appendRow(qsi_list);
	
    m_texts_list.insert(deti, qsi);
	blockSignals(true);
	enableSourceText(deti, deti->textFrom());
	blockSignals(false);
	setConnection(deti, true);
}

/**
 * @brief DynamicElementTextModel::removeText
 * @param deti
 */
void DynamicElementTextModel::removeText(DynamicElementTextItem *deti)
{
	if (!m_texts_list.contains(deti))
        return;
    
    QModelIndex text_index = m_texts_list.value(deti)->index();
    this->removeRow(text_index.row(), text_index.parent());
    m_texts_list.remove(deti);
	setConnection(deti, false);
}

/**
 * @brief DynamicElementTextModel::textFromIndex
 * @param index
 * @return the text associated with @index. Return value can be nullptr
 * @Index can be a child of an index associated with a text
 */
DynamicElementTextItem *DynamicElementTextModel::textFromIndex(const QModelIndex &index) const
{
    if(!index.isValid())
        return nullptr;
    
    if (QStandardItem *item = itemFromIndex(index))
        return textFromItem(item);
    else
        return nullptr;
}

/**
 * @brief DynamicElementTextModel::textFromItem
 * @param item
 * @return the text associated with @item. Return value can be nullptr
 * @item can be a child of an item associated with a text
 */
DynamicElementTextItem *DynamicElementTextModel::textFromItem(QStandardItem *item) const
{
	QStandardItem *text_item = item;
	while (text_item->parent())
		text_item = text_item->parent();
	
	if (m_texts_list.values().contains(text_item))
		return m_texts_list.key(text_item);
	else
		return nullptr;
}

/**
 * @brief DynamicElementTextModel::undoForEditedText
 * @param deti
 * @return A QUndoCommand that describe all changes made for @deti.
 * Each change made for @deti is append as a child of the returned QUndoCommand.
 * In other word, if the returned QUndoCommand have no child, that mean there is no change.
 */
QUndoCommand *DynamicElementTextModel::undoForEditedText(DynamicElementTextItem *deti) const
{
	QUndoCommand *undo = new QUndoCommand(tr("Éditer un texte d'élément"));
	
	if (!m_texts_list.contains(deti))
		return undo;
	
	QStandardItem *text_qsi = m_texts_list.value(deti);
	
	QString from = text_qsi->child(0,1)->data(Qt::DisplayRole).toString();
	if ((from == tr("Texte utilisateur")) && (deti->textFrom() != DynamicElementTextItem::UserText))
		new QPropertyUndoCommand(deti, "textFrom", QVariant(deti->textFrom()), QVariant(DynamicElementTextItem::UserText), undo);
	else if ((from == tr("Information de l'élément")) && (deti->textFrom() != DynamicElementTextItem::ElementInfo))
		new QPropertyUndoCommand(deti, "textFrom", QVariant(deti->textFrom()), QVariant(DynamicElementTextItem::ElementInfo), undo);
	
	QString text = text_qsi->child(0,0)->child(0,1)->data(Qt::DisplayRole).toString();
	if (text != deti->text())
		new QPropertyUndoCommand(deti, "text", QVariant(deti->text()), QVariant(text), undo);
	
	int fs = text_qsi->child(1,1)->data(Qt::EditRole).toInt();
	if (fs != deti->fontSize()) 
		new QPropertyUndoCommand(deti, "fontSize", QVariant(deti->fontSize()), QVariant(fs), undo);
	
	QString tagg = text_qsi->child(2,1)->data(Qt::DisplayRole).toString();
	if(tagg != deti->tagg())
		new QPropertyUndoCommand(deti, "tagg", QVariant(deti->tagg()), QVariant(tagg), undo);
	
	QColor color = text_qsi->child(3,1)->data(Qt::EditRole).value<QColor>();
	if(color != deti->color())
		new QPropertyUndoCommand(deti, "color", QVariant(deti->color()), QVariant(color), undo);
	
	return undo;
}

/**
 * @brief DynamicElementTextModel::enableSourceText
 * Enable the good field, according to the current source of text, for the edited text @deti
 * @param deti
 * @param tf
 */
void DynamicElementTextModel::enableSourceText(DynamicElementTextItem *deti, DynamicElementTextItem::TextFrom tf)
{
	if (!m_texts_list.contains(deti))
		return;
	
	QStandardItem *qsi = m_texts_list.value(deti)->child(0,0);
	
	bool usr = true, info = false;
	if(tf == DynamicElementTextItem::ElementInfo) {
		usr = false; info = true;}
	
		//User text
	qsi->child(0,0)->setEnabled(usr);
	qsi->child(0,1)->setEnabled(usr);
		//Info text
	qsi->child(1,0)->setEnabled(info);
	qsi->child(1,1)->setEnabled(info);
}

void DynamicElementTextModel::dataEdited(QStandardItem *qsi)
{
	DynamicElementTextItem *deti = textFromItem(qsi);
	if (!deti)
		return;
	
	if (qsi->data().toInt() == textFrom)
	{
		QString from = qsi->data(Qt::DisplayRole).toString();
		if (from == tr("Texte utilisateur"))
			enableSourceText(deti, DynamicElementTextItem::UserText);
		else
			enableSourceText(deti, DynamicElementTextItem::ElementInfo);
	}
	else if (qsi->data().toInt() == userText)
	{
		QString text = qsi->data(Qt::DisplayRole).toString();
		m_texts_list.value(deti)->setData(text, Qt::DisplayRole);
	}
}

/**
 * @brief DynamicElementTextModel::setConnection
 * Set up the connection for @deti to keep up to date the data of this model and the text.
 * Is notably use with the use of QUndoCommand.
 * @param deti - text to setup connection
 * @param set - true = set connection - false unset connection
 */
void DynamicElementTextModel::setConnection(DynamicElementTextItem *deti, bool set)
{
	if(set)
	{
		if(m_hash_text_connect.keys().contains(deti))
			return;
		
		QList<QMetaObject::Connection> connection_list;
		connection_list << connect(deti, &DynamicElementTextItem::colorChanged,    [deti,this](){this->updateDataFromText(deti, color);});
		connection_list << connect(deti, &DynamicElementTextItem::fontSizeChanged, [deti,this](){this->updateDataFromText(deti, size);});
		connection_list << connect(deti, &DynamicElementTextItem::taggChanged,     [deti,this](){this->updateDataFromText(deti, tagg);});
		connection_list << connect(deti, &DynamicElementTextItem::textChanged,     [deti,this](){this->updateDataFromText(deti, userText);});
		connection_list << connect(deti, &DynamicElementTextItem::TextFromChanged, [deti,this](){this->updateDataFromText(deti, textFrom);});
		
		m_hash_text_connect.insert(deti, connection_list);
	}
	else
	{
		if(!m_hash_text_connect.keys().contains(deti))
			return;
		
		for (QMetaObject::Connection con : m_hash_text_connect.value(deti))
			disconnect(con);
		
		m_hash_text_connect.remove(deti);
	}
}

void DynamicElementTextModel::updateDataFromText(DynamicElementTextItem *deti, ValueType type)
{
	QStandardItem *qsi = m_texts_list.value(deti);
	if (!qsi)
		return;
	
	switch (type)
	{
		case textFrom:
			qsi->child(0,1)->setData(deti->textFrom() == DynamicElementTextItem::UserText ? tr("Texte utilisateur") : tr("Information de l'élément"), Qt::DisplayRole);
			break;
		case userText:
		{
			QStandardItem *qsia = qsi->child(0,0);
			qsia->child(0,1)->setData(deti->toPlainText(), Qt::DisplayRole);
			qsi->setData(deti->toPlainText(), Qt::DisplayRole);
			break;
		}
		case infoText:
			break;
		case size:
			qsi->child(1,1)->setData(deti->fontSize(), Qt::EditRole);
			break;
		case tagg:
			qsi->child(2,1)->setData(deti->tagg(), Qt::DisplayRole);
			break;
		case color:
		{
			qsi->child(3,1)->setData(deti->color(), Qt::EditRole);
			qsi->child(3,1)->setData(deti->color(), Qt::ForegroundRole);
			break;
		}
	}
}


/***************************************************
 * A little delegate only for add a combobox and a color dialog,
 * for use with the model
 ***************************************************/

DynamicTextItemDelegate::DynamicTextItemDelegate(QObject *parent) :
    QStyledItemDelegate(parent)
{}

QWidget *DynamicTextItemDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	switch (index.data(Qt::UserRole+1).toInt())
	{
		case DynamicElementTextModel::textFrom:
		{
			QComboBox *qcb = new QComboBox(parent);
			qcb->addItem(tr("Texte utilisateur"));
			qcb->addItem(tr("Information de l'élément"));
			return qcb;
		}
		case DynamicElementTextModel::color:
		{
			QColorDialog *cd = new QColorDialog(index.data(Qt::EditRole).value<QColor>());
			return cd;
		}
	}
	return QStyledItemDelegate::createEditor(parent, option, index);
}

void DynamicTextItemDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
	if (index.isValid())
	{        
		if (QStandardItemModel *qsim = dynamic_cast<QStandardItemModel *>(model))
		{
			QStandardItem *qsi = qsim->itemFromIndex(index);
			if(qsi)
			{
				if(QColorDialog *cd = dynamic_cast<QColorDialog *> (editor))
				{
					qsi->setData(cd->selectedColor(), Qt::EditRole);
					qsi->setData(cd->selectedColor(), Qt::ForegroundRole);
					return;
				}
			}
			
		}
	}
	
	QStyledItemDelegate::setModelData(editor, model, index);
}