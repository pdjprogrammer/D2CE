/*
    Diablo II Character Editor
    Copyright (C) 2000-2003  Burton Tsang
    Copyright (C) 2021 Walter Couto

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

//---------------------------------------------------------------------------
class CD2MercenaryForm : public CDialogEx
{
	DECLARE_DYNAMIC(CD2MercenaryForm)

public:
    CD2MercenaryForm(CD2MainForm& form);
	virtual ~CD2MercenaryForm();

    bool MercChanged() const;

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_MERC_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    // Implementation
protected:
    // Generated message map functions
    virtual BOOL OnInitDialog();

    afx_msg void OnEnChangeMercLevel();
    afx_msg void OnEnKillfocusMercLevel();
    afx_msg void OnEnChangeMercExperience();
    afx_msg void OnEnKillfocusMercExperience();
    afx_msg void OnCbnSelchangeDifficulty();
    afx_msg void OnCbnSelchangeAttribute();
    afx_msg void OnCbnSelchangeMercClass();
    afx_msg void OnCbnSelchangeMercName();

	DECLARE_MESSAGE_MAP()

private:
    CButton MercHired;
    CButton MercDead;
    CComboBox MercName;
    d2ce::EnumMercenaryClass CurMercNameClass = d2ce::EnumMercenaryClass::None;
    CButton CharStatusHired;
    CButton CharStatusDead;
    CComboBox MercClass;
    CComboBox Difficulty;
    CComboBox Attribute;
    d2ce::EnumMercenaryClass CurAttributeClass = d2ce::EnumMercenaryClass::None;
    CStatic AttributeDash;
    CEdit MercLevel;
    CEdit MercStrength;
    CEdit MercDexterity;
    CEdit MercLife;
    CEdit Experience;

    std::set<UINT> ctrlEditted;

    CFont m_boldFont;

    CD2MainForm& MainForm;
    d2ce::Mercenary& Merc;
    d2ce::MercInfo OrigMerc;

    void DisplayMercInfo();
    void EnableMercInfoBox();
    void UpdateMercNames();
    void UpdateAttributes();
    void UpdateModified();

    std::string ToStdString(const CWnd* Sender) const;
    CString ToText(const CWnd* Sender) const;
    CStringA ToTextA(const CWnd* Sender) const;
    void SetText(CWnd* Sender, const char* newValue);
    void SetText(CWnd* Sender, const wchar_t* newValue);
    std::uint32_t ToInt(const CWnd* Sender) const;
    void SetInt(CWnd* Sender, std::uint32_t newValue);

public:
    afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
    afx_msg void OnBnClickedOk();
    afx_msg void OnBnClickedCancel();
    afx_msg void OnClickedMercHired();
    afx_msg void OnClickedResurrectedCheck();
};
//---------------------------------------------------------------------------
