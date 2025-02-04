/*
    Diablo II Character Editor
    Copyright (C) 2021-2023 Walter Couto

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; If not, see <http://www.gnu.org/licenses/>.
*/
//---------------------------------------------------------------------------
#pragma once

//---------------------------------------------------------------------------
#include "D2MainForm.h"
#include "D2ItemsForm.h"
#include "D2SharedStashForm.h"
#include "D2ItemToolTipCtrl.h"
#include "D2TreeCtrl.h"
#include <list>

//---------------------------------------------------------------------------
class CD2NewItemForm : public CDialogEx, public CD2ItemTooltipCallback
{
    friend class CD2MagicalAffixesForm;
    friend class CD2RareAffixesForm;
    friend class CD2RunewordForm;
    friend class CD2EarAttributesForm;
	DECLARE_DYNAMIC(CD2NewItemForm)

public:
	CD2NewItemForm(CD2MainForm& form);   // standard constructor
    CD2NewItemForm(CD2ItemsForm& form);
    CD2NewItemForm(CD2SharedStashForm& form);
	virtual ~CD2NewItemForm();

    // Dialog Data
	enum { IDD = IDD_NEW_ITEM_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    afx_msg void OnBnClickedOk();
    afx_msg void OnTvnSelchangedItemtree(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnNMDblclkItemtree(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnBnClickedEtherealCheck();
    afx_msg void OnCbnSelchangeMonstersCombo();
    afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);

	DECLARE_MESSAGE_MAP()

public:
    virtual BOOL OnInitDialog();

    const d2ce::Item* GetCreatedItem();

private:
    void InitTree();

    const d2ce::Item* GetSelectedItem() const override;

protected:
    const d2ce::Character* GetCharacterInfo() const override;

protected:
    CD2ItemInfoStatic ItemTooltipBox;
    CD2MainForm& MainForm;
    CD2ItemsForm* ItemsFormPtr = nullptr;
    CD2SharedStashForm* SharedStashFormPtr = nullptr;
    mutable CD2TreeCtrl ItemTree;
    const d2ce::Item* CreatedItem = nullptr;
    CButton Ethereal;
    CStatic QualityStatic;
    CComboBox Quality;
    CStatic MonstersStatic;
    CComboBox Monsters;
    CStatic SocketsStatic;
    CEdit SocketsEdit;
    CSpinButtonCtrl SocketsSpinner;

    std::map<HTREEITEM, d2ce::AvailableItemType> AvailableItemTypes;
    mutable std::list<d2ce::Item> AvailableItems;
    std::map<std::uint16_t, int> MonsterIDToIndexMap;
};
//---------------------------------------------------------------------------
