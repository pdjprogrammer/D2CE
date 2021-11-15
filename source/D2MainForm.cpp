/*
    Diablo II Character Editor
    Copyright (C) 2000-2003  Burton Tsang
    Copyright (C) 2021 Walter Couto

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
//---------------------------------------------------------------------------

#include "pch.h"
#include "framework.h"
#include "D2Editor.h"
#include "D2MainForm.h"
#include "D2SkillTreeForm.h"
#include "D2LevelInfoForm.h"
#include "D2WaypointsForm.h"
#include "D2QuestsForm.h"
#include "D2GemsForm.h"
#include "D2MercenaryForm.h"
#include "D2ItemsForm.h"
#include "d2ce\ExperienceConstants.h"
#include "d2ce\Constants.h"
#include "afxdialogex.h"
#include "resource.h"
#include <regex>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace
{
    void ScaleImage(CDC* pDC, CBitmap& image, const CRect& rect)
    {
        BITMAP bmp;
        image.GetBitmap(&bmp);

        // select the source CBitmap in a memory DC;
        CDC memSrcDc;
        memSrcDc.CreateCompatibleDC(pDC);
        memSrcDc.SelectObject(&image); //now bitmap is an instance of CBitmap class

        // Create your new CBitmap with the new desired size and select it into a destination memory DC
        CDC memDestDC;
        CBitmap image2;
        memDestDC.CreateCompatibleDC(pDC);
        image2.CreateCompatibleBitmap(&memSrcDc, rect.Width(), rect.Height());
        memDestDC.SelectObject(&image2);

        // StretchBlt from src to dest
        memDestDC.StretchBlt(0, 0, rect.Width(), rect.Height(), &memSrcDc, 0, 0, bmp.bmWidth - 1, bmp.bmHeight - 1, SRCCOPY);

        HGDIOBJ hbitmap_detach = image.Detach();
        if (hbitmap_detach)
        {
            DeleteObject(hbitmap_detach);
        }

        image.Attach(image2.Detach());
    }

    void SetScaledButtonImage(CDC* pDC, CButton& button, CBitmap& image)
    {
        CRect buttonRect;
        button.GetClientRect(&buttonRect);
        buttonRect.bottom += 4;
        buttonRect.right += 10;
        ScaleImage(pDC, image, buttonRect);
        button.SetBitmap(image);
    }

    static CD2LevelInfoForm s_levelInfo;
    CString ExtractFilePath(LPCTSTR fullPath)
    {
        if (fullPath == nullptr)
        {
            return _T("");
        }

        CString temp = (LPCTSTR)fullPath;
        if (temp.IsEmpty())
        {
            return _T("");
        }

        std::filesystem::path p = fullPath;
        return p.remove_filename().c_str();
    }

    CString ExtractFileName(LPCTSTR fullPath)
    {
        if (fullPath == nullptr)
        {
            return _T("");
        }

        CString temp = (LPCTSTR)fullPath;
        if (temp.IsEmpty())
        {
            return _T("");
        }

        std::filesystem::path p = fullPath;
        return p.filename().c_str();
    }

    CString ExtractFileExt(LPCTSTR fullPath)
    {
        if (fullPath == nullptr)
        {
            return _T("");
        }

        CString temp = (LPCTSTR)fullPath;
        if (temp.IsEmpty())
        {
            return _T("");
        }

        std::filesystem::path p = fullPath;
        if (!p.has_extension())
        {
            return _T("");
        }

        return p.extension().c_str();
    }

    CString ChangeFileExt(LPCTSTR fullPath, LPCTSTR ext)
    {
        if (fullPath == nullptr)
        {
            return _T("");
        }

        CString temp = (LPCTSTR)fullPath;
        if (temp.IsEmpty())
        {
            return _T("");
        }

        std::filesystem::path p = fullPath;
        if (ext == nullptr)
        {
            // remove the extension
            return p.replace_extension().c_str();
        }

        return p.replace_extension(ext).c_str();
    }

    CString RemoveFileExtFromPath(LPCTSTR fullPath)
    {
        if (fullPath == nullptr)
        {
            return _T("");
        }

        CString temp = (LPCTSTR)fullPath;
        if (temp.IsEmpty())
        {
            return _T("");
        }

        std::filesystem::path p = fullPath;
        return p.replace_extension().c_str();
    }

    bool FileExists(LPCTSTR fullPath)
    {
        if (fullPath == nullptr)
        {
            return false;
        }

        CString temp = (LPCTSTR)fullPath;
        if (temp.IsEmpty())
        {
            return false;
        }

        std::filesystem::path p = fullPath;
        return std::filesystem::exists(p);
    }
}

//---------------------------------------------------------------------------
// CAboutDlg dialog used for App About
class CAboutDlg : public CDialogEx
{
public:
    CAboutDlg(d2ce::EnumCharVersion version);

    // Dialog Data
    enum { IDD = IDD_ABOUTBOX };

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    static CString GetModuleVersionInfoString(LPCTSTR lpszVariable, const std::vector<std::uint8_t>& pbData, LPCTSTR languageString)
    {
        if (pbData.empty() || lpszVariable == nullptr || languageString == nullptr)
        {
            return _T("");
        }

        CString variable = lpszVariable;
        std::uint32_t  cbValue = 0;
        CString sVersion;
        if (variable.CompareNoCase(_T("FileVersion")) == 0)
        {
            VS_FIXEDFILEINFO* fi = NULL;
            UINT uLen = 0;
            VerQueryValue(&pbData[0], _T("\\"), (LPVOID*)&fi, &uLen);
            sVersion.Format(_T("%d.%d.%d"), HIWORD(fi->dwFileVersionMS), LOWORD(fi->dwFileVersionMS), HIWORD(fi->dwFileVersionLS));
        }
        else if (variable.CompareNoCase(_T("ProductVersion")) == 0)
        {
            VS_FIXEDFILEINFO* fi = NULL;
            UINT uLen = 0;
            VerQueryValue(&pbData[0], _T("\\"), (LPVOID*)&fi, &uLen);
            sVersion.Format(_T("%d.%d.%d"), HIWORD(fi->dwProductVersionMS), LOWORD(fi->dwProductVersionMS), HIWORD(fi->dwProductVersionLS));
        }
        else
        {
            CString sFieldPath(languageString);
            sFieldPath += variable;
            LPTSTR lpszValue = nullptr;
            VerQueryValue(&pbData[0], sFieldPath, (VOID**)&lpszValue, &cbValue);
            if (lpszValue != nullptr)
            {
                sVersion = lpszValue;
                sVersion.Trim();
            }
        }

        return sVersion;
    }

    // Implementation
protected:
    DECLARE_MESSAGE_MAP()

protected:
    CString ProductName = _T("Diablo II: Resurrected Character Editor");
    CString ProductNameAndVersion = _T("Diablo II: Resurrected Character Editor, Version 2.0.1");
    CString LegalCopyright = _T("Copyright (c) 2021 By Walter Couto\nCopyright (c) 2000-2003 By Burton Tsang");
    d2ce::EnumCharVersion FileVersion = d2ce::APP_CHAR_VERSION;
public:
    virtual BOOL OnInitDialog();
};

//---------------------------------------------------------------------------
CAboutDlg::CAboutDlg(d2ce::EnumCharVersion version) : CDialogEx(CAboutDlg::IDD), FileVersion(version)
{
    TCHAR appName[MAX_PATH];
    ::GetModuleFileName(NULL, appName, MAX_PATH);
    CString sFileName = appName;
    if (!sFileName.IsEmpty())
    {
        CString languageString;
        std::vector<std::uint8_t> pbData;

        DWORD dwHandle = 0;	// unused parameter for some APIs,must be 0

        // Get the size of the Version info block
        DWORD cbBuf = ::GetFileVersionInfoSize(sFileName, &dwHandle);

        pbData.resize(cbBuf, 0);

        // Get the file version info
        dwHandle = 0;	// unused parameter for some APIs,must be 0
        BOOL bRet = ::GetFileVersionInfo(sFileName, dwHandle, cbBuf, &pbData[0]);
        if (bRet)
        {
            DWORD dwTranslation = 0;
            DWORD* pdwTranslation = nullptr;
            UINT  cbValue = 0;

            bRet = VerQueryValue(&pbData[0], _T("\\VarFileInfo\\Translation"), (void**)&pdwTranslation, &cbValue);
            if (bRet)
            {
                dwTranslation = *pdwTranslation;
            }

            TCHAR buffer[256];
            _stprintf_s(buffer, _countof(buffer), _T("%04x%04x"), LOWORD(dwTranslation), HIWORD(dwTranslation));

            languageString = _T("\\StringFileInfo\\");
            languageString += buffer;
            languageString += _T("\\");
        }

        UINT uID = FileVersion <= d2ce::EnumCharVersion::v110 ? IDS_OLD_APP_TITLE : AFX_IDS_APP_TITLE;
        if (ProductName.LoadString(uID) == 0)
        {
            ProductName = GetModuleVersionInfoString(_T("ProductName"), pbData, languageString);
        }

        CString sProductVersion = GetModuleVersionInfoString(_T("ProductVersion"), pbData, languageString);
        ProductNameAndVersion.Format(_T("%s, Version %s"), (LPCTSTR)ProductName, (LPCTSTR)sProductVersion);
        LegalCopyright = GetModuleVersionInfoString(_T("LegalCopyright"), pbData, languageString);
    }
}
//---------------------------------------------------------------------------
void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
    __super::DoDataExchange(pDX);
    DDX_Text(pDX, IDC_ABOUT_VERSION, ProductNameAndVersion);
    DDX_Text(pDX, IDC_APP_COPYRIGHT, LegalCopyright);
}
//---------------------------------------------------------------------------
BOOL CAboutDlg::OnInitDialog()
{
    __super::OnInitDialog();

    if (!ProductName.IsEmpty())
    {
        CString appTitle("About ");
        appTitle += ProductName;
        SetWindowText(appTitle);
    }

    return TRUE;  // return TRUE unless you set the focus to a control
                  // EXCEPTION: OCX Property Pages should return FALSE
}
//---------------------------------------------------------------------------
BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()

//---------------------------------------------------------------------------
// CCharNameEdit
CCharNameEdit::CCharNameEdit()
{
}
//---------------------------------------------------------------------------
CCharNameEdit::~CCharNameEdit()
{
}

//---------------------------------------------------------------------------
BEGIN_MESSAGE_MAP(CCharNameEdit, CEdit)
    //{{AFX_MSG_MAP(CCharNameEdit)
    ON_WM_CHAR()
    ON_WM_KILLFOCUS()
    //}}AFX_MSG_MAP
    ON_MESSAGE(WM_PASTE, OnPaste)
    ON_MESSAGE(WM_SETTEXT, OnSetText)
END_MESSAGE_MAP()

//---------------------------------------------------------------------------
void CCharNameEdit::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
    if (isalpha(nChar) || nChar == '_' || nChar == '-')
    {
        if (nChar == '_' || nChar == '-')
        {
            CString sWindowText;
            GetWindowText(sWindowText);
            if (sWindowText.IsEmpty())
            {
                return;
            }

            for (int i = 1; i < sWindowText.GetLength(); i++)
            {
                if (sWindowText[i] == '_' || sWindowText[i] == '-')
                {
                    // don't bother renaming if more than 1 underscore
                    return;
                }
            }
        }
        __super::OnChar(nChar, nRepCnt, nFlags);
    }
    else if (nChar == VK_BACK)
    {
        __super::OnChar(nChar, nRepCnt, nFlags);
    }
}
//---------------------------------------------------------------------------
void CCharNameEdit::OnKillFocus(CWnd* pNewWnd)
{
    __super::OnKillFocus(pNewWnd);

    CString sWindowText;
    GetWindowText(sWindowText);
    CString strText = GetValidText(sWindowText);
    if (strText != sWindowText)
    {
        SetWindowText(strText);
    }
}
//---------------------------------------------------------------------------
// Pastes the text from the clipboard onto the current selection.
LRESULT CCharNameEdit::OnPaste(WPARAM, LPARAM)
{
    int nStart, nEnd;
    GetSel(nStart, nEnd);

    Default();

    CString sWindowText;
    GetWindowText(sWindowText);
    CString strText = GetValidText(sWindowText);
    if (strText != sWindowText)
    {
        SetWindowText(strText);
        SetSel(nStart, nEnd);
    }

    return 0;
}
//---------------------------------------------------------------------------
// Handles the WM_SETTEXT message to ensure that text (set via SetWindowText) is valid.
LRESULT CCharNameEdit::OnSetText(WPARAM /*wParam*/, LPARAM lParam)
{
    LRESULT nResult = Default();

    CString strText = GetValidText((LPCTSTR)lParam);
    if (strText != (LPCTSTR)lParam)
    {
        SetWindowText(strText);
    }

    return nResult;
}
//---------------------------------------------------------------------------
// Returns the control's value in a valid format.
CString CCharNameEdit::GetValidText(LPCTSTR value) const
{
    if (value == nullptr)
    {
        return _T("");
    }

    CString strNewText;
    CString strText(value);
    UINT maxSize = GetLimitText();

    // Remove any invalid characters from the number
    for (UINT iPos = 0, nNewLen = 0, NumberOfUnderscores = 0, nLen = strText.GetLength(); iPos < nLen && nNewLen < maxSize; ++iPos)
    {
        TCHAR c = strText[iPos];

        if (isalpha(c))
        {
            strNewText += c;
            ++nNewLen;
        }
        else if ((c == '_' || c == '-') && nNewLen != 0 && NumberOfUnderscores < 1)
        {
            strNewText += c;
            ++nNewLen;
            ++NumberOfUnderscores;
        }
    }

    return strNewText.TrimRight(_T("_-"));
}
//---------------------------------------------------------------------------
// CStatsLeftImage
CStatsLeftImage::CStatsLeftImage()
{
    m_bitmap.Attach(::LoadBitmap(AfxGetResourceHandle(), MAKEINTRESOURCE(IDB_STATS_LEFT_BITMAP)));
}
//---------------------------------------------------------------------------
CStatsLeftImage::~CStatsLeftImage()
{
}

//---------------------------------------------------------------------------
BEGIN_MESSAGE_MAP(CStatsLeftImage, CStatic)
    //{{AFX_MSG_MAP(CStatsLeftImage)
    ON_WM_ERASEBKGND()
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CStatsLeftImage::PreSubclassWindow()
{
    __super::PreSubclassWindow();
    ModifyStyle(0, SS_OWNERDRAW);
}

void CStatsLeftImage::DrawItem(LPDRAWITEMSTRUCT /*lpDrawItemStruct*/)
{
}

BOOL CStatsLeftImage::OnEraseBkgnd(CDC* pDC)
{
    CRect rect;
    GetClientRect(rect);

    // If no bitmap selected, simply erase the background as per normal and return
    if (!m_bitmap.GetSafeHandle())
    {
        CBrush backBrush(::GetSysColor(COLOR_3DFACE)); // (this is meant for dialogs)
        CBrush* pOldBrush = pDC->SelectObject(&backBrush);

        pDC->PatBlt(rect.left, rect.top, rect.Width(), rect.Height(), PATCOPY);
        pDC->SelectObject(pOldBrush);

        return TRUE;
    }

    // We have a bitmap - draw it.
    BITMAP bmp;
    m_bitmap.GetBitmap(&bmp);

    // Create compatible memory DC using the controls DC
    CDC dcMem;
    VERIFY(dcMem.CreateCompatibleDC(pDC));

    // Select bitmap into memory DC.
    CBitmap* pBmpOld = dcMem.SelectObject(&m_bitmap);

    // StretchBlt bitmap onto static's client area
    pDC->StretchBlt(rect.left, rect.top, rect.Width(), rect.Height(), &dcMem, 0, 0, bmp.bmWidth - 1, bmp.bmHeight - 1, SRCCOPY);

    dcMem.SelectObject(pBmpOld);

    return TRUE;
}
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// CD2MainForm dialog
IMPLEMENT_DYNAMIC(CD2MainForm, CDialogEx)
//---------------------------------------------------------------------------
CD2MainForm::CD2MainForm(WORD fontSize, CWnd* pParent /*=nullptr*/)
    : CDialogEx()
{
    m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

    m_nIDTracking = 0;
    m_nIDLastMessage = 0;

    SetStartDir();

    BackupChar = AfxGetApp()->GetProfileInt(SettingsSection, BackupCharacterOption, 1) != 0 ? true : false;

    UINT nIDTemplate = CD2MainForm::IDD;
    m_pParentWnd = pParent;
    m_nIDHelp = nIDTemplate;

    LPCTSTR lpszTemplateName = MAKEINTRESOURCE(nIDTemplate);
    HINSTANCE hInst = AfxFindResourceHandle(lpszTemplateName, RT_DIALOG);
    HRSRC hResource = ::FindResource(hInst, lpszTemplateName, RT_DIALOG);
    if (hResource == NULL)
    {
        m_lpszTemplateName = lpszTemplateName;
        return;
    }

    DWORD dwszDialogTemplate = SizeofResource(hInst, hResource);
    HGLOBAL hTemplate = LoadResource(hInst, hResource);
    LPCDLGTEMPLATE lpTemplate = static_cast<LPCDLGTEMPLATE>(::LockResource(hTemplate));
    if (lpTemplate == nullptr || dwszDialogTemplate == 0)
    {
        m_lpszTemplateName = lpszTemplateName;
        return;
    }

    // check if we have may contain font information
    const WORD* pwPtr = reinterpret_cast<const WORD*>(lpTemplate);

    // dlgVer
    size_t idx = 0;
    if (pwPtr[idx] != 1)
    {
        m_lpszTemplateName = lpszTemplateName;
        return;
    }

    // signature
    ++idx;
    if (pwPtr[idx] != 0xFFFF)
    {
        m_lpszTemplateName = lpszTemplateName;
        return;
    }

    // skip over helpID

    // skip over exStyle
    idx += 2;

    // style
    idx += 2;
    DWORD style = ((DWORD)pwPtr[idx] << 16) | pwPtr[idx + 1];

    // skip over cDlgItems
    idx += 2;

    // skip over  x
    idx += 2;

    // skip over  y
    idx += 2;

    // skip over  cx
    idx += 2;

    // skip over  cx
    idx += 2;

    // Now skip over extended information until we get to font size
    // Skip over menu and class
    for (size_t i = 0; i < 2; ++i)
    {
        if (pwPtr[idx] == 0)
        {
            // no other elements
            ++idx;
        }
        else if (pwPtr[idx] == 0xFFFF)
        {
            //Next element is menu ID
            idx += 2;
        }
        else
        {
            //Unicode ASIIZ string
            WORD z = 0;
            do
            {
                z = pwPtr[idx];
                ++idx;
            } while (z != 0);
        }
    }

    // we are at style
    if ((style & (DS_SETFONT | DS_SHELLFONT)) == 0)
    {
        // no font information
        m_lpszTemplateName = lpszTemplateName;
        return;
    }

    WORD templateFontSize = pwPtr[idx];
    if (templateFontSize == fontSize)
    {
        // Font size already correct
        m_lpszTemplateName = lpszTemplateName;
        return;
    }

    m_vecTemplate.resize(dwszDialogTemplate, 0);
    std::memcpy(&m_vecTemplate[0], lpTemplate, dwszDialogTemplate);
    m_vecTemplate[idx] = fontSize;
    m_lpDialogTemplate = (LPCDLGTEMPLATE)&m_vecTemplate[0];

    // look for any initialization data
    HRSRC hDlgInit = ::FindResource(hInst, lpszTemplateName, RT_DLGINIT);
    if (hDlgInit == NULL)
    {
        return;
    }

    // load it
    HGLOBAL hDlgInitResource = ::LoadResource(hInst, hDlgInit);
    if (hDlgInitResource == NULL)
    {
        return;
    }

    m_lpDialogInit = ::LockResource(hDlgInitResource);
}

CD2MainForm::CD2MainForm(CWnd* pParent /*=nullptr*/)
    : CDialogEx(CD2MainForm::IDD, pParent)
{
    m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

    m_nIDTracking = 0;
    m_nIDLastMessage = 0;

    SetStartDir();

    BackupChar = AfxGetApp()->GetProfileInt(SettingsSection, BackupCharacterOption, 1) != 0 ? true : false;
}
//---------------------------------------------------------------------------
void CD2MainForm::DoDataExchange(CDataExchange* pDX)
{
    __super::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_CHAR_NAME, CharName);
    DDX_Control(pDX, IDC_HARDCORE_CHECK, CharStatusHardcore);
    DDX_Control(pDX, IDC_RESURRECTED_CHECK, CharStatusResurrected);
    DDX_Control(pDX, IDC_EXPANSION_CHECK, CharStatusExpansion);
    DDX_Control(pDX, IDC_LADDER_CHECK, CharStatusLadder);
    DDX_Control(pDX, IDC_CHAR_CLASS_CMB, CharClass);
    DDX_Control(pDX, IDC_DIFFICULTY, Difficulty);
    DDX_Control(pDX, IDC_STARTING_ACT_CMB, StartingAct);
    DDX_Control(pDX, IDC_CHAR_LEVEL, CharLevel);
    DDX_Control(pDX, IDC_CHAR_STRENGTH, CharStrength);
    DDX_Control(pDX, IDC_STRENGTH_PLUS_BTN, StrengthPlusButton);
    DDX_Control(pDX, IDC_CHAR_ENERGY, CharEnergy);
    DDX_Control(pDX, IDC_ENERGY_PLUS_BTN, EnergyPlusButton);
    DDX_Control(pDX, IDC_CHAR_DEXTERITY, CharDexterity);
    DDX_Control(pDX, IDC_DEXTERITY_PLUS_BTN, DexterityPlusButton);
    DDX_Control(pDX, IDC_CHAR_VITALITY, CharVitality);
    DDX_Control(pDX, IDC_VITALITY_PLUS_BTN, VitalityPlusButton);
    DDX_Control(pDX, IDC_CUR_LIFE, CurLife);
    DDX_Control(pDX, IDC_MAX_LIFE, MaxLife);
    DDX_Control(pDX, IDC_CUR_MANA, CurMana);
    DDX_Control(pDX, IDC_MAX_MANA, MaxMana);
    DDX_Control(pDX, IDC_CUR_STAMINA, CurStamina);
    DDX_Control(pDX, IDC_MAX_STAMINA, MaxStamina);
    DDX_Control(pDX, IDC_CHAR_EXPERIENCE, Experience);
    DDX_Control(pDX, IDC_CHAR_NEXT_EXPERIENCE, NextExperience);
    DDX_Control(pDX, IDC_STATS_LEFT, StatsLeft);
    DDX_Control(pDX, IDC_STATS_LEFT_IMAGE, StatsLeftCtrl);
    DDX_Control(pDX, IDC_GOLD_IN_BELT, GoldInBelt);
    DDX_Control(pDX, IDC_GOLD_IN_STASH, GoldInStash);
    DDX_Control(pDX, IDC_CHAR_TITLE_CMB, CharTitle);
    DDX_Control(pDX, IDC_EDIT_SKILLS_BTN, EditSkillsButton);
    DDX_Control(pDX, IDC_MAX_ALL_BTN, MaxAllButton);
    DDX_Control(pDX, IDC_QUESTS_BTN, QuestsButton);
    DDX_Control(pDX, IDC_WAYPOINTS_BTN, WaypointsButton);
    DDX_Control(pDX, IDC_QUIT_BTN, QuitButton);
    DDX_Control(pDX, IDC_BACKGROUND_BOX, BackgroundBox);

}
//---------------------------------------------------------------------------
BOOL CD2MainForm::PreTranslateMessage(MSG* pMsg)
{
    CWnd* pWndFocus = GetFocus();
    if (pWndFocus != NULL && IsChild(pWndFocus))
    {
        UINT message = pMsg->message;
        if ((message == WM_MOUSEMOVE || message == WM_NCMOUSEMOVE ||
            message == WM_LBUTTONUP || message == WM_RBUTTONUP ||
            message == WM_MBUTTONUP) &&
            (GetKeyState(VK_LBUTTON) >= 0 && GetKeyState(VK_RBUTTON) >= 0 &&
                GetKeyState(VK_MBUTTON) >= 0))
        {
            CheckToolTipCtrl();
        }

        if (pMsg->message == WM_KEYDOWN)
        {
            if (pMsg->wParam == VK_RETURN)
            {
                TCHAR szClass[10];
                if (GetClassName(pWndFocus->m_hWnd, szClass, 10) &&
                    (lstrcmpi(szClass, _T("EDIT")) == 0))
                {
                    // pressing the ENTER key will take the focus to the next control
                    pMsg->wParam = VK_TAB;
                }
            }
        }
    }

    return __super::PreTranslateMessage(pMsg);
}

//---------------------------------------------------------------------------
BEGIN_MESSAGE_MAP(CD2MainForm, CDialogEx)
    ON_WM_SYSCOMMAND()
    ON_WM_PAINT()
    ON_WM_QUERYDRAGICON()
    ON_WM_CTLCOLOR()
    ON_MESSAGE(WM_SETMESSAGESTRING, OnSetMessageString)
    ON_MESSAGE(WM_POPMESSAGESTRING, OnPopMessageString)
    ON_WM_MENUSELECT()
    ON_WM_MOUSEMOVE()
    ON_WM_INITMENUPOPUP()
    ON_COMMAND(ID_VIEW_SKILL_TREE, &CD2MainForm::OnViewSkillTree)
    ON_UPDATE_COMMAND_UI(ID_VIEW_SKILL_TREE, &CD2MainForm::OnUpdateViewSkillTree)
    ON_BN_CLICKED(IDC_EDIT_SKILLS_BTN, &CD2MainForm::OnBnClickedEditSkillsBtn)
    ON_COMMAND(ID_OPTIONS_MAX_EVERYTHING, &CD2MainForm::OnOptionsMaxEverything)
    ON_UPDATE_COMMAND_UI(ID_OPTIONS_MAX_EVERYTHING, &CD2MainForm::OnUpdateOptionsMaxEverything)
    ON_BN_CLICKED(IDC_MAX_ALL_BTN, &CD2MainForm::OnBnClickedMaxAllBtn)
    ON_COMMAND(ID_VIEW_QUESTS, &CD2MainForm::OnViewQuests)
    ON_UPDATE_COMMAND_UI(ID_VIEW_QUESTS, &CD2MainForm::OnUpdateViewQuests)
    ON_BN_CLICKED(IDC_QUESTS_BTN, &CD2MainForm::OnBnClickedQuestsBtn)
    ON_COMMAND(ID_OPTIONS_UPGRADE_GEMS, &CD2MainForm::OnOptionsUpgradeGems)
    ON_UPDATE_COMMAND_UI(ID_OPTIONS_UPGRADE_GEMS, &CD2MainForm::OnUpdateOptionsUpgradeGems)
    ON_COMMAND(ID_VIEW_WAYPOINTS, &CD2MainForm::OnViewWaypoints)
    ON_UPDATE_COMMAND_UI(ID_VIEW_WAYPOINTS, &CD2MainForm::OnUpdateViewWaypoints)
    ON_BN_CLICKED(IDC_WAYPOINTS_BTN, &CD2MainForm::OnBnClickedWaypointsBtn)
    ON_EN_SETFOCUS(IDC_CHAR_NAME, &CD2MainForm::OnEnSetfocusCharName)
    ON_EN_CHANGE(IDC_CHAR_NAME, &CD2MainForm::OnEnChangeCharName)
    ON_EN_KILLFOCUS(IDC_CHAR_NAME, &CD2MainForm::OnEnKillfocusCharName)
    ON_EN_SETFOCUS(IDC_CHAR_LEVEL, &CD2MainForm::OnEnSetfocusCharLevel)
    ON_EN_CHANGE(IDC_CHAR_LEVEL, &CD2MainForm::OnEnChangeCharLevel)
    ON_EN_KILLFOCUS(IDC_CHAR_LEVEL, &CD2MainForm::OnEnKillfocusCharLevel)
    ON_EN_SETFOCUS(IDC_CHAR_STRENGTH, &CD2MainForm::OnEnSetfocusCharStrength)
    ON_EN_CHANGE(IDC_CHAR_STRENGTH, &CD2MainForm::OnEnChangeCharStrength)
    ON_EN_KILLFOCUS(IDC_CHAR_STRENGTH, &CD2MainForm::OnEnKillfocusCharStrength)
    ON_BN_CLICKED(IDC_STRENGTH_PLUS_BTN, &CD2MainForm::OnBnClickedStrengthPlus)
    ON_EN_SETFOCUS(IDC_CHAR_DEXTERITY, &CD2MainForm::OnEnSetfocusCharDexterity)
    ON_EN_CHANGE(IDC_CHAR_DEXTERITY, &CD2MainForm::OnEnChangeCharDexterity)
    ON_EN_KILLFOCUS(IDC_CHAR_DEXTERITY, &CD2MainForm::OnEnKillfocusCharDexterity)
    ON_BN_CLICKED(IDC_DEXTERITY_PLUS_BTN, &CD2MainForm::OnBnClickedDexterityPlus)
    ON_EN_SETFOCUS(IDC_CHAR_VITALITY, &CD2MainForm::OnEnSetfocusCharVitality)
    ON_EN_CHANGE(IDC_CHAR_VITALITY, &CD2MainForm::OnEnChangeCharVitality)
    ON_EN_KILLFOCUS(IDC_CHAR_VITALITY, &CD2MainForm::OnEnKillfocusCharVitality)
    ON_BN_CLICKED(IDC_VITALITY_PLUS_BTN, &CD2MainForm::OnBnClickedVitalityPlus)
    ON_EN_SETFOCUS(IDC_CHAR_ENERGY, &CD2MainForm::OnEnSetfocusCharEnergy)
    ON_EN_CHANGE(IDC_CHAR_ENERGY, &CD2MainForm::OnEnChangeCharEnergy)
    ON_EN_KILLFOCUS(IDC_CHAR_ENERGY, &CD2MainForm::OnEnKillfocusCharEnergy)
    ON_BN_CLICKED(IDC_ENERGY_PLUS_BTN, &CD2MainForm::OnBnClickedEnergyPlus)
    ON_EN_SETFOCUS(IDC_CHAR_EXPERIENCE, &CD2MainForm::OnEnSetfocusCharExperience)
    ON_EN_CHANGE(IDC_CHAR_EXPERIENCE, &CD2MainForm::OnEnChangeCharExperience)
    ON_EN_KILLFOCUS(IDC_CHAR_EXPERIENCE, &CD2MainForm::OnEnKillfocusCharExperience)
    ON_EN_SETFOCUS(IDC_CUR_LIFE, &CD2MainForm::OnEnSetfocusCurLife)
    ON_EN_CHANGE(IDC_CUR_LIFE, &CD2MainForm::OnEnChangeCurLife)
    ON_EN_KILLFOCUS(IDC_CUR_LIFE, &CD2MainForm::OnEnKillfocusCurLife)
    ON_EN_SETFOCUS(IDC_MAX_LIFE, &CD2MainForm::OnEnSetfocusMaxLife)
    ON_EN_CHANGE(IDC_MAX_LIFE, &CD2MainForm::OnEnChangeMaxLife)
    ON_EN_KILLFOCUS(IDC_MAX_LIFE, &CD2MainForm::OnEnKillfocusMaxLife)
    ON_EN_SETFOCUS(IDC_CUR_MANA, &CD2MainForm::OnEnSetfocusCurMana)
    ON_EN_CHANGE(IDC_CUR_MANA, &CD2MainForm::OnEnChangeCurMana)
    ON_EN_KILLFOCUS(IDC_CUR_MANA, &CD2MainForm::OnEnKillfocusCurMana)
    ON_EN_SETFOCUS(IDC_MAX_MANA, &CD2MainForm::OnEnSetfocusMaxMana)
    ON_EN_CHANGE(IDC_MAX_MANA, &CD2MainForm::OnEnChangeMaxMana)
    ON_EN_KILLFOCUS(IDC_MAX_MANA, &CD2MainForm::OnEnKillfocusMaxMana)
    ON_EN_SETFOCUS(IDC_CUR_STAMINA, &CD2MainForm::OnEnSetfocusCurStamina)
    ON_EN_CHANGE(IDC_CUR_STAMINA, &CD2MainForm::OnEnChangeCurStamina)
    ON_EN_KILLFOCUS(IDC_CUR_STAMINA, &CD2MainForm::OnEnKillfocusCurStamina)
    ON_EN_SETFOCUS(IDC_MAX_STAMINA, &CD2MainForm::OnEnSetfocusMaxStamina)
    ON_EN_CHANGE(IDC_MAX_STAMINA, &CD2MainForm::OnEnChangeMaxStamina)
    ON_EN_KILLFOCUS(IDC_MAX_STAMINA, &CD2MainForm::OnEnKillfocusMaxStamina)
    ON_EN_SETFOCUS(IDC_GOLD_IN_BELT, &CD2MainForm::OnEnSetfocusGoldInBelt)
    ON_EN_CHANGE(IDC_GOLD_IN_BELT, &CD2MainForm::OnEnChangeGoldInBelt)
    ON_EN_KILLFOCUS(IDC_GOLD_IN_BELT, &CD2MainForm::OnEnKillfocusGoldInBelt)
    ON_EN_SETFOCUS(IDC_GOLD_IN_STASH, &CD2MainForm::OnEnSetfocusGoldInStash)
    ON_EN_CHANGE(IDC_GOLD_IN_STASH, &CD2MainForm::OnEnChangeGoldInStash)
    ON_EN_KILLFOCUS(IDC_GOLD_IN_STASH, &CD2MainForm::OnEnKillfocusGoldInStash)
    ON_CBN_SELCHANGE(IDC_CHAR_CLASS_CMB, &CD2MainForm::OnCbnSelchangeCharClassCmb)
    ON_BN_CLICKED(IDC_HARDCORE_CHECK, &CD2MainForm::OnBnClickedHardcoreCheck)
    ON_BN_CLICKED(IDC_RESURRECTED_CHECK, &CD2MainForm::OnBnClickedResurrectedCheck)
    ON_BN_CLICKED(IDC_EXPANSION_CHECK, &CD2MainForm::OnBnClickedExpansionCheck)
    ON_BN_CLICKED(IDC_LADDER_CHECK, &CD2MainForm::OnBnClickedLadderCheck)
    ON_CBN_SELCHANGE(IDC_CHAR_TITLE_CMB, &CD2MainForm::OnCbnSelchangeCharTitleCmb)
    ON_CBN_SELCHANGE(IDC_STARTING_ACT_CMB, &CD2MainForm::OnCbnSelchangeStartingActCmb)
    ON_COMMAND(ID_FILE_SAVE, &CD2MainForm::OnFileSave)
    ON_UPDATE_COMMAND_UI(ID_FILE_SAVE, &CD2MainForm::OnUpdateFileSave)
    ON_COMMAND(ID_FILE_CLOSE, &CD2MainForm::OnFileClose)
    ON_UPDATE_COMMAND_UI(ID_FILE_CLOSE, &CD2MainForm::OnUpdateFileClose)
    ON_COMMAND(ID_FILE_OPEN, &CD2MainForm::OnFileOpen)
    ON_MESSAGE(WM_OPEN_DLG_FILE, &CD2MainForm::OnMRUFileOpen)
    ON_COMMAND(ID_FILE_EXPORT_AS_JSON, &CD2MainForm::OnFileExportAsJson)
    ON_UPDATE_COMMAND_UI(ID_FILE_EXPORT_AS_JSON, &CD2MainForm::OnUpdateFileExportAsJson)
    ON_COMMAND(ID_VIEW_REFRESH, &CD2MainForm::OnViewRefresh)
    ON_UPDATE_COMMAND_UI(ID_VIEW_REFRESH, &CD2MainForm::OnUpdateViewRefresh)
    ON_CBN_SELCHANGE(IDC_DIFFICULTY, &CD2MainForm::OnCbnSelchangeDifficultyCmb)
    ON_BN_CLICKED(IDC_QUIT_BTN, &CD2MainForm::OnAppExit)
    ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTW, 0, 0xFFFF, OnToolTipText)
    ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTA, 0, 0xFFFF, OnToolTipText)
    ON_COMMAND(ID_OPTIONS_BACKUP_CHAR, &CD2MainForm::OnOptionsBackupChar)
    ON_UPDATE_COMMAND_UI(ID_OPTIONS_BACKUP_CHAR, &CD2MainForm::OnUpdateOptionsBackupChar)
    ON_COMMAND(ID_OPTIONS_RESTORE_CHAR, &CD2MainForm::OnOptionsRestoreChar)
    ON_UPDATE_COMMAND_UI(ID_OPTIONS_RESTORE_CHAR, &CD2MainForm::OnUpdateOptionsRestoreChar)
    ON_COMMAND(ID_OPTIONS_CHECK_CHAR, &CD2MainForm::OnOptionsCheckChar)
    ON_UPDATE_COMMAND_UI(ID_OPTIONS_CHECK_CHAR, &CD2MainForm::OnUpdateOptionsCheckChar)
    ON_COMMAND(ID_APP_ABOUT, &CD2MainForm::OnAppAbout)
    ON_COMMAND(ID_APP_EXIT, &CD2MainForm::OnAppExit)
    ON_COMMAND(ID_VIEW_LEVEL_REQ, &CD2MainForm::OnViewLevelReq)
    ON_COMMAND(ID_OPTIONS_UPGRADE_POTIONS, &CD2MainForm::OnOptionsUpgradePotions)
    ON_UPDATE_COMMAND_UI(ID_OPTIONS_UPGRADE_POTIONS, &CD2MainForm::OnUpdateOptionsUpgradePotions)
    ON_COMMAND(ID_OPTIONS_UPGRADE_REJUVENATION, &CD2MainForm::OnOptionsUpgradeRejuvenation)
    ON_UPDATE_COMMAND_UI(ID_OPTIONS_UPGRADE_REJUVENATION, &CD2MainForm::OnUpdateOptionsUpgradeRejuvenation)
    ON_COMMAND(ID_OPTIONS_GPS_CONVERTOR, &CD2MainForm::OnOptionsGpsConvertor)
    ON_UPDATE_COMMAND_UI(ID_OPTIONS_GPS_CONVERTOR, &CD2MainForm::OnUpdateOptionsGpsConvertor)
    ON_COMMAND(ID_OPTIONS_MAXFILLSTACKABLES, &CD2MainForm::OnOptionsMaxfillstackables)
    ON_UPDATE_COMMAND_UI(ID_OPTIONS_MAXFILLSTACKABLES, &CD2MainForm::OnUpdateOptionsMaxfillstackables)
    ON_COMMAND(ID_OPTIONS_FIXALLITEMS, &CD2MainForm::OnOptionsFixallitems)
    ON_UPDATE_COMMAND_UI(ID_OPTIONS_FIXALLITEMS, &CD2MainForm::OnUpdateOptionsFixallitems)
    ON_COMMAND(ID_OPTIONS_MAXDURABILITYFORALLITEMS, &CD2MainForm::OnOptionsMaxdurabilityforallitems)
    ON_UPDATE_COMMAND_UI(ID_OPTIONS_MAXDURABILITYFORALLITEMS, &CD2MainForm::OnUpdateOptionsMaxdurabilityforallitems)
    ON_COMMAND(ID_OPTIONS_RESET_STATS, &CD2MainForm::OnOptionsResetStats)
    ON_UPDATE_COMMAND_UI(ID_OPTIONS_RESET_STATS, &CD2MainForm::OnUpdateOptionsResetStats)
    ON_COMMAND(ID_VIEW_MERCENARY, &CD2MainForm::OnViewMercenary)
    ON_UPDATE_COMMAND_UI(ID_VIEW_MERCENARY, &CD2MainForm::OnUpdateViewMercenary)
    ON_COMMAND(ID_VIEW_ITEMS, &CD2MainForm::OnViewItems)
    ON_UPDATE_COMMAND_UI(ID_VIEW_ITEMS, &CD2MainForm::OnUpdateViewItems)
END_MESSAGE_MAP()

//---------------------------------------------------------------------------
// CD2MainForm message handlers

static UINT indicators[] =
{
    ID_SEPARATOR           // status line indicator
};

//---------------------------------------------------------------------------
BOOL CD2MainForm::OnInitDialog()
{
    __super::OnInitDialog();

    EnableToolTips(TRUE);

    CheckToolTipCtrl();

    CString appTitle;
    if (appTitle.LoadString(AFX_IDS_APP_TITLE) != 0)
    {
        SetWindowText(appTitle);
    }

    BlackBrush.CreateSolidBrush(RGB(0, 0, 0)); // black background color

    // Add "About..." menu item to system menu.

    // IDM_ABOUTBOX must be in the system command range.
    ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
    ASSERT(IDM_ABOUTBOX < 0xF000);

    CMenu* pSysMenu = GetSystemMenu(FALSE);
    if (pSysMenu != nullptr)
    {
        BOOL bNameValid;
        CString strAboutMenu;
        bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
        ASSERT(bNameValid);
        if (!strAboutMenu.IsEmpty())
        {
            pSysMenu->AppendMenu(MF_SEPARATOR);
            pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
        }
    }

    // Set the icon for this dialog.  The framework does this automatically
    //  when the application's main window is not a dialog
    SetIcon(m_hIcon, TRUE);			// Set big icon
    SetIcon(m_hIcon, FALSE);		// Set small icon

    // TODO: Add extra initialization here
    if (!StatusBar.Create(this) ||
        !StatusBar.SetIndicators(indicators,
            sizeof(indicators) / sizeof(UINT)))
    {
        TRACE0("Failed to create status bar\n");
    }
    OnSetMessageString(AFX_IDS_IDLEMESSAGE);


    CDC* pDC = GetDC();
    CRect rect;
    BackgroundBox.GetClientRect(&rect);
    BackgroundImage.Attach(::LoadBitmap(AfxGetResourceHandle(), MAKEINTRESOURCE(IDB_CHAR_BITMAP)));
    ScaleImage(pDC, BackgroundImage, rect);
    SetBackgroundImage(BackgroundImage, BACKGR_TOPLEFT, TRUE, FALSE);

    QuestsImage.Attach(::LoadBitmap(AfxGetResourceHandle(), MAKEINTRESOURCE(IDB_QUEST_BITMAP)));
    SetScaledButtonImage(pDC, QuestsButton, QuestsImage);

    EditSkillsImage.Attach(::LoadBitmap(AfxGetResourceHandle(), MAKEINTRESOURCE(IDB_SKILL_BITMAP)));
    SetScaledButtonImage(pDC, EditSkillsButton, EditSkillsImage);

    QuitImage.Attach(::LoadBitmap(AfxGetResourceHandle(), MAKEINTRESOURCE(IDB_EXIT_BITMAP)));
    SetScaledButtonImage(pDC, QuitButton, QuitImage);

    WaypointsImage.Attach(::LoadBitmap(AfxGetResourceHandle(), MAKEINTRESOURCE(IDB_WAYPOINT_BITMAP)));
    SetScaledButtonImage(pDC, WaypointsButton, WaypointsImage);

    MaxAllImage.Attach(::LoadBitmap(AfxGetResourceHandle(), MAKEINTRESOURCE(IDB_STATS_BITMAP)));
    SetScaledButtonImage(pDC, MaxAllButton, MaxAllImage);
    StrengthPlusButton.SetBitmap(MaxAllImage);
    EnergyPlusButton.SetBitmap(MaxAllImage);
    DexterityPlusButton.SetBitmap(MaxAllImage);
    VitalityPlusButton.SetBitmap(MaxAllImage);

    //------------------
    // Create bold font:
    //------------------
    CFont* pWndFont = CharName.GetFont();
    LOGFONT lf;
    pWndFont->GetLogFont(&lf);
    lf.lfWeight = FW_BOLD;
    m_boldFont.CreateFontIndirect(&lf);

    //This is where we actually draw it on the screen
    RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST,
        ID_SEPARATOR);

    // Set text limite on edit boxes
    CharName.SetLimitText(15);
    CharLevel.SetLimitText(2);
    CharStrength.SetLimitText(5);
    CharEnergy.SetLimitText(5);
    CharDexterity.SetLimitText(5);
    CharVitality.SetLimitText(5);
    CurLife.SetLimitText(4);
    MaxLife.SetLimitText(4);
    CurMana.SetLimitText(4);
    MaxMana.SetLimitText(4);
    CurStamina.SetLimitText(4);
    MaxStamina.SetLimitText(4);
    Experience.SetLimitText(10);
    GoldInBelt.SetLimitText(6);
    GoldInStash.SetLimitText(7);

    EnableCharInfoBox(FALSE);
    return TRUE;  // return TRUE  unless you set the focus to a control
}
//---------------------------------------------------------------------------
void CD2MainForm::OnSysCommand(UINT nID, LPARAM lParam)
{
    if ((nID & 0xFFF0) == IDM_ABOUTBOX)
    {
        CAboutDlg dlgAbout(CharInfo.getVersion());
        dlgAbout.DoModal();
        return;
    }

    if ((nID & 0xFFF0) == SC_CLOSE)
    {
        if (CharInfo.is_open() && (DoFileCloseAction() == IDCANCEL))
        {
            return;
        }

        if (::IsWindow(s_levelInfo.GetSafeHwnd()))
        {
            s_levelInfo.DestroyWindow();
        }
    }

    __super::OnSysCommand(nID, lParam);
}
//---------------------------------------------------------------------------
void CD2MainForm::OnAppAbout()
{
    SendMessage(WM_SYSCOMMAND, IDM_ABOUTBOX, 0);
}
//---------------------------------------------------------------------------
// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.
void CD2MainForm::OnPaint()
{
    if (IsIconic())
    {
        CPaintDC dc(this); // device context for painting

        SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

        // Center icon in client rectangle
        int cxIcon = GetSystemMetrics(SM_CXICON);
        int cyIcon = GetSystemMetrics(SM_CYICON);
        CRect rect;
        GetClientRect(&rect);
        int x = (rect.Width() - cxIcon + 1) / 2;
        int y = (rect.Height() - cyIcon + 1) / 2;

        // Draw the icon
        dc.DrawIcon(x, y, m_hIcon);
    }
    else
    {
        __super::OnPaint();
    }
}
//---------------------------------------------------------------------------
// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CD2MainForm::OnQueryDragIcon()
{
    return static_cast<HCURSOR>(m_hIcon);
}
//---------------------------------------------------------------------------
HBRUSH CD2MainForm::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
    if (CTLCOLOR_EDIT == nCtlColor || CTLCOLOR_LISTBOX == nCtlColor ||
        (pWnd->IsKindOf(RUNTIME_CLASS(CEdit)) || pWnd->IsKindOf(RUNTIME_CLASS(CComboBox))))
    {
        pDC->SetTextColor(RGB(255, 255, 255));
        pDC->SetBkColor(RGB(0, 0, 0));
        pDC->SetBkMode(TRANSPARENT);
        if (ctrlEditted.find(pWnd->GetDlgCtrlID()) != ctrlEditted.end())
        {
            pDC->SelectObject(&m_boldFont);
        }

        return (HBRUSH)BlackBrush;
    }

    switch (pWnd->GetDlgCtrlID())
    {
    case IDC_HARDCORE_CHECK:
    case IDC_RESURRECTED_CHECK:
    case IDC_EXPANSION_CHECK:
    case IDC_LADDER_CHECK:
        pDC->SetTextColor(RGB(255, 255, 255));
        pDC->SetBkColor(RGB(0, 0, 0));
        pDC->SetBkMode(TRANSPARENT);
        if (ctrlEditted.find(pWnd->GetDlgCtrlID()) != ctrlEditted.end())
        {
            pDC->SelectObject(&m_boldFont);
        }

        return (HBRUSH)BlackBrush;
    }

    if (CTLCOLOR_STATIC == nCtlColor)
    {
        switch (pWnd->GetDlgCtrlID())
        {
        case IDC_HARDCORE_CHECK:
        case IDC_RESURRECTED_CHECK:
        case IDC_EXPANSION_CHECK:
        case IDC_LADDER_CHECK:
            pDC->SetBkMode(TRANSPARENT);
            return reinterpret_cast<HBRUSH>(::GetStockObject(NULL_BRUSH));
        }

        HBRUSH hbr = __super::OnCtlColor(pDC, pWnd, nCtlColor);
        pDC->SetTextColor(RGB(255, 255, 255));
        pDC->SetBkColor(RGB(0, 0, 0));
        return hbr;
    }

    return __super::OnCtlColor(pDC, pWnd, nCtlColor);;

}
//---------------------------------------------------------------------------
// OnSetMessageString updates the status bar text.
//
// This code is based on CFrameWnd::OnSetMessageString.  We assume
// a string ID is always passed in wParam.
LRESULT CD2MainForm::OnSetMessageString(WPARAM wParam, LPARAM /*lParam*/)
{
    UINT nIDMsg = (UINT)wParam;
    CString strMsg;

    if (nIDMsg)
    {
        if ((nIDMsg == AFX_IDS_IDLEMESSAGE) && !CharInfo.is_open())
        {
            StatusBar.SetWindowText(_T("Click on File, Open or press CTRL-O to start"));
        }
        else if (strMsg.LoadString(nIDMsg) != 0)
        {
            CString strStatusText;
            AfxExtractSubString(strStatusText, strMsg, 0, '\n');
            switch (nIDMsg)
            {
            case IDC_GOLD_IN_BELT:
                strStatusText.Format(strStatusText, CharInfo.getMaxGoldInBelt());
                break;
            case IDC_GOLD_IN_STASH:
                strStatusText.Format(strStatusText, CharInfo.getMaxGoldInStash());
                break;
            }
            StatusBar.SetWindowText(strStatusText);
        }
        else
        {
            if (!CharInfo.is_open())
            {
                StatusBar.SetWindowText(_T("Click on File, Open or press CTRL-O to start"));
            }
            else if (strMsg.LoadString(AFX_IDS_IDLEMESSAGE) != 0)
            {
                StatusBar.SetWindowText(strMsg);
            }
            TRACE1("Warning: no message line prompt for ID %x%04X\n", nIDMsg);
        }
    }

    UINT nIDLast = m_nIDTracking;
    m_nIDTracking = nIDMsg;
    m_nIDLastMessage = nIDMsg;
    return nIDLast;
}
//---------------------------------------------------------------------------
// Resets status bar message string.  This code is based on
// CFrameWnd::OnPopMessageString
LRESULT CD2MainForm::OnPopMessageString(WPARAM wParam, LPARAM lParam)
{
    if (m_nFlags & WF_NOPOPMSG)
        return 0;

    return SendMessage(WM_SETMESSAGESTRING, wParam, lParam);
}
//---------------------------------------------------------------------------
void CD2MainForm::OnMenuSelect(UINT nItemID, UINT nFlags, HMENU hSysMenu)
{
    __super::OnMenuSelect(nItemID, nFlags, hSysMenu);

    if (nFlags == 0xFFFF)
    {
        m_nIDTracking = AFX_IDS_IDLEMESSAGE;
        OnSetMessageString(m_nIDTracking);
        ASSERT(m_nIDTracking == m_nIDLastMessage);
    }
    else if (nItemID == 0 || nFlags & (MF_SEPARATOR | MF_POPUP | MF_MENUBREAK | MF_MENUBARBREAK))
    {
        m_nIDTracking = 0;
    }
    else if (nItemID >= 0xF000 && nItemID < 0xF1F0)
    {
        m_nIDTracking = ID_COMMAND_FROM_SC(nItemID);
        ASSERT(m_nIDTracking >= AFX_IDS_SCFIRST && m_nIDTracking < AFX_IDS_SCFIRST + 31);
    }
    else
    {
        m_nIDTracking = nItemID;
        OnSetMessageString(m_nIDTracking);
    }
}
//---------------------------------------------------------------------------
void CD2MainForm::OnMouseMove(UINT nFlags, CPoint point)
{
    if (!LastRect.IsRectEmpty() && !LastRect.PtInRect(point))
    {
        OnSetMessageString(AFX_IDS_IDLEMESSAGE);
        LastRect.SetRectEmpty();
    }

    __super::OnMouseMove(nFlags, point);
}
//---------------------------------------------------------------------------
void CD2MainForm::OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu)
{
    __super::OnInitMenuPopup(pPopupMenu, nIndex, bSysMenu);
    if (bSysMenu)
        return;     // don't support system menu

    ASSERT(pPopupMenu != NULL);
    // Check the enabled state of various menu items.

    OnSetMessageString(AFX_IDS_IDLEMESSAGE);
    CCmdUI state;
    state.m_pMenu = pPopupMenu;
    ASSERT(state.m_pOther == NULL);
    ASSERT(state.m_pParentMenu == NULL);

    // Determine if menu is popup in top-level menu and set m_pOther to
    // it if so (m_pParentMenu == NULL indicates that it is secondary popup).
    HMENU hParentMenu;
    if (AfxGetThreadState()->m_hTrackingMenu == pPopupMenu->m_hMenu)
        state.m_pParentMenu = pPopupMenu;    // Parent == child for tracking popup.
    else if ((hParentMenu = ::GetMenu(m_hWnd)) != NULL)
    {
        CWnd* pParent = this;
        // Child windows don't have menus--need to go to the top!
        if (pParent != NULL &&
            (hParentMenu = ::GetMenu(pParent->m_hWnd)) != NULL)
        {
            int nIndexMax = ::GetMenuItemCount(hParentMenu);
            for (int nIndex2 = 0; nIndex2 < nIndexMax; nIndex2++)
            {
                if (::GetSubMenu(hParentMenu, nIndex2) == pPopupMenu->m_hMenu)
                {
                    // When popup is found, m_pParentMenu is containing menu.
                    state.m_pParentMenu = CMenu::FromHandle(hParentMenu);
                    break;
                }
            }
        }
    }

    state.m_nIndexMax = pPopupMenu->GetMenuItemCount();
    for (state.m_nIndex = 0; state.m_nIndex < state.m_nIndexMax;
        state.m_nIndex++)
    {
        state.m_nID = pPopupMenu->GetMenuItemID(state.m_nIndex);
        if (state.m_nID == 0)
            continue; // Menu separator or invalid cmd - ignore it.

        ASSERT(state.m_pOther == NULL);
        ASSERT(state.m_pMenu != NULL);
        if (state.m_nID == (UINT)-1)
        {
            // Possibly a popup menu, route to first item of that popup.
            state.m_pSubMenu = pPopupMenu->GetSubMenu(state.m_nIndex);
            if (state.m_pSubMenu == NULL ||
                (state.m_nID = state.m_pSubMenu->GetMenuItemID(0)) == 0 ||
                state.m_nID == (UINT)-1)
            {
                continue;       // First item of popup can't be routed to.
            }
            state.DoUpdate(this, TRUE);   // Popups are never auto disabled.
        }
        else
        {
            // Normal menu item.
            // Auto enable/disable if frame window has m_bAutoMenuEnable
            // set and command is _not_ a system command.
            state.m_pSubMenu = NULL;
            state.DoUpdate(this, FALSE);
        }

        // Adjust for menu deletions and additions.
        UINT nCount = pPopupMenu->GetMenuItemCount();
        if (nCount < state.m_nIndexMax)
        {
            state.m_nIndex -= (state.m_nIndexMax - nCount);
            while (state.m_nIndex < nCount &&
                pPopupMenu->GetMenuItemID(state.m_nIndex) == state.m_nID)
            {
                state.m_nIndex++;
            }
        }
        state.m_nIndexMax = nCount;
    }
}
//---------------------------------------------------------------------------
BOOL CD2MainForm::OnToolTipText(UINT, NMHDR* pNMHDR, LRESULT* pResult)
{
    ASSERT(pNMHDR->code == TTN_NEEDTEXTA || pNMHDR->code == TTN_NEEDTEXTW);

    TOOLTIPTEXTA* pTTTA = (TOOLTIPTEXTA*)pNMHDR;
    TOOLTIPTEXTW* pTTTW = (TOOLTIPTEXTW*)pNMHDR;
    TCHAR szFullText[256];
    CString strTipText;
    CString strPromtpText;
    UINT_PTR nID = pNMHDR->idFrom;

    CRect rect;
    if (pNMHDR->code == TTN_NEEDTEXTA && (pTTTA->uFlags & TTF_IDISHWND) ||
        pNMHDR->code == TTN_NEEDTEXTW && (pTTTW->uFlags & TTF_IDISHWND))
    {
        ::GetWindowRect((HWND)nID, &rect);
        ScreenToClient(&rect);
        nID = ((UINT_PTR)(DWORD)::GetDlgCtrlID((HWND)nID));
    }

    if ((nID != 0) && (nID != ID_VIEW_STATUS_BAR))
    {
        AfxLoadString((UINT)nID, szFullText);
        AfxExtractSubString(strTipText, szFullText, 1, '\n');
        AfxExtractSubString(strPromtpText, szFullText, 0, '\n');
        switch (nID)
        {
        case IDC_GOLD_IN_BELT:
            strTipText.Format(strTipText, CharInfo.getMaxGoldInBelt());
            strPromtpText.Format(strPromtpText, CharInfo.getMaxGoldInBelt());
            break;
        case IDC_GOLD_IN_STASH:
            strTipText.Format(strTipText, CharInfo.getMaxGoldInStash());
            strPromtpText.Format(strPromtpText, CharInfo.getMaxGoldInStash());
            break;
        case IDC_CHAR_STRENGTH:
            strTipText.Format(strTipText, CharInfo.getMinStrength());
            strPromtpText.Format(strPromtpText, CharInfo.getMinStrength());
            break;
        case IDC_CHAR_ENERGY:
            strTipText.Format(strTipText, CharInfo.getMinEnergy());
            strPromtpText.Format(strPromtpText, CharInfo.getMinEnergy());
            break;
        case IDC_CHAR_DEXTERITY:
            strTipText.Format(strTipText, CharInfo.getMinDexterity());
            strPromtpText.Format(strPromtpText, CharInfo.getMinDexterity());
            break;
        case IDC_CHAR_VITALITY:
            strTipText.Format(strTipText, CharInfo.getMinVitality());
            strPromtpText.Format(strPromtpText, CharInfo.getMinVitality());
            break;
        case IDC_MAX_LIFE:
            strTipText.Format(strTipText, CharInfo.getMaxHitPoints() >> 8);
            strPromtpText.Format(strPromtpText, CharInfo.getMaxHitPoints() >> 8);
            break;
        case IDC_MAX_MANA:
            strTipText.Format(strTipText, CharInfo.getMaxMana() >> 8);
            strPromtpText.Format(strPromtpText, CharInfo.getMaxMana() >> 8);
            break;
        case IDC_MAX_STAMINA:
            strTipText.Format(strTipText, CharInfo.getMaxStamina() >> 8);
            strPromtpText.Format(strPromtpText, CharInfo.getMaxStamina() >> 8);
            break;
        }

        if (!strPromtpText.IsEmpty())
        {
            StatusBar.SetWindowText(strPromtpText);
            LastRect = rect;
        }
    }
#ifndef _UNICODE
    if (pNMHDR->code == TTN_NEEDTEXTA)
        lstrcpyn(pTTTA->szText, strTipText, (sizeof(pTTTA->szText) / sizeof(pTTTA->szText[0])));
    else
        _mbstowcsz(pTTTW->szText, strTipText, (sizeof(pTTTW->szText) / sizeof(pTTTW->szText[0])));
#else
    if (pNMHDR->code == TTN_NEEDTEXTA)
        _wcstombsz(pTTTA->szText, strTipText, (sizeof(pTTTA->szText) / sizeof(pTTTA->szText[0])));
    else
        lstrcpyn(pTTTW->szText, strTipText, (sizeof(pTTTW->szText) / sizeof(pTTTW->szText[0])));
#endif
    * pResult = 0;

    ::SetWindowPos(pNMHDR->hwndFrom, HWND_TOP, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOMOVE);

    return TRUE;
}
//---------------------------------------------------------------------------
void CD2MainForm::OnOptionsCheckChar()
{
    if (!CharInfo.is_open())
    {
        return;
    }

    bool statChanged = false;
    bool bFoundIssue = false;

    d2ce::CharStats cs;
    CharInfo.fillCharacterStats(cs);

    // does a valid level range check
    if (cs.Level < 1 || cs.Level > d2ce::NUM_OF_LEVELS)
    {
        bFoundIssue = true;
        if (AfxMessageBox(_T("\"Level\" amount exceeds the recommended maximum limit.\n")
            _T("Would you like the amount changed to the recommended maximum limit?"),
            MB_ICONQUESTION | MB_YESNO) == IDYES)
        {
            cs.Level = d2ce::NUM_OF_LEVELS;
            CharInfo.updateCharacterStats(cs);
            UpdateCharInfo();
            statChanged = true;
        }
        else
        {
            // Can't proceed with an invalid level
            StatusBar.SetWindowText(_T("Character stats have been checked"));
            return;
        }
    }

    // does a level-experience check
    std::uint32_t expLevel = d2ce::NUM_OF_LEVELS;
    std::uint32_t value = (std::uint32_t)ToInt(&Experience);
    // find the correct level
    while ((expLevel > 1) && (value < d2ce::MinExpRequired[expLevel - 1]))
    {
        --expLevel;
    }

    if (expLevel > cs.Level)
    {
        bFoundIssue = true;
        if (AfxMessageBox(_T("Your character's level is too low for the experience achieved.\n")
            _T("Would you like the amount changed to match your character's experience?"),
            MB_ICONQUESTION | MB_YESNO) == IDYES)
        {
            cs.Level = expLevel;
            CharInfo.updateCharacterStats(cs);
            UpdateCharInfo();
            statChanged = true;
        }
    }
    else if (expLevel < cs.Level)
    {
        bFoundIssue = true;
        if (AfxMessageBox(_T("Insufficient experience based on current level.\n")
            _T("Would you like experience changed to match your character's level?"),
            MB_ICONQUESTION | MB_YESNO) == IDYES)
        {
            cs.Experience = d2ce::MinExpRequired[cs.Level - 1];
            CharInfo.updateCharacterStats(cs);
            UpdateCharInfo();
            statChanged = true;
        }
    }

    // does a level-gold check
    if (ToInt(&GoldInBelt) > cs.getMaxGoldInBelt())
    {
        bFoundIssue = true;
        if (AfxMessageBox(_T("\"Gold In Belt\" amount exceeds the maximum limit.\n")
            _T("Would you like the amount changed to match your character's level?"),
            MB_ICONQUESTION | MB_YESNO) == IDYES)
        {
            cs.GoldInBelt = cs.getMaxGoldInBelt();
            CharInfo.updateCharacterStats(cs);
            UpdateCharInfo();
            statChanged = true;
        }
    }

    if (ToInt(&GoldInStash) > cs.getMaxGoldInStash())
    {
        bFoundIssue = true;
        if (AfxMessageBox(_T("\"Gold In Stash\" amount exceeds the maximum limit.\n")
            _T("Would you like the amount changed to match your character's level?"),
            MB_ICONQUESTION | MB_YESNO) == IDYES)
        {
            cs.GoldInStash = cs.getMaxGoldInStash();
            CharInfo.updateCharacterStats(cs);
            UpdateCharInfo();
            statChanged = true;
        }
    }

    if (ToInt(&CharVitality) < cs.Vitality)
    {
        bFoundIssue = true;
        if (AfxMessageBox(_T("Insufficient \"Vitality\" based on the character's class.\n")
            _T("Would you like the amount changed to match your character's class?"),
            MB_ICONQUESTION | MB_YESNO) == IDYES)
        {
            CharInfo.updateCharacterStats(cs);
            UpdateCharInfo();
            statChanged = true;
        }
    }

    if (ToInt(&CharEnergy) < cs.Energy)
    {
        bFoundIssue = true;
        if (AfxMessageBox(_T("Insufficient \"Energy\" based for the character's class.\n")
            _T("Would you like the amount changed to match your character's class?"),
            MB_ICONQUESTION | MB_YESNO) == IDYES)
        {
            CharInfo.updateCharacterStats(cs);
            UpdateCharInfo();
            statChanged = true;
        }
    }

    if (ToInt(&CharDexterity) < cs.Dexterity)
    {
        bFoundIssue = true;
        if (AfxMessageBox(_T("Insufficient \"Dexterity\" based for the character's class.\n")
            _T("Would you like the amount changed to match your character's class?"),
            MB_ICONQUESTION | MB_YESNO) == IDYES)
        {
            CharInfo.updateCharacterStats(cs);
            UpdateCharInfo();
            statChanged = true;
        }
    }

    if (ToInt(&CharStrength) < cs.Strength)
    {
        bFoundIssue = true;
        if (AfxMessageBox(_T("Insufficient \"Strength\" based for the character's class.\n")
            _T("Would you like the amount changed to match your character's class?"),
            MB_ICONQUESTION | MB_YESNO) == IDYES)
        {
            CharInfo.updateCharacterStats(cs);
            UpdateCharInfo();
            statChanged = true;
        }
    }

    if (ToInt(&MaxLife) < cs.MaxLife)
    {
        bFoundIssue = true;
        if (AfxMessageBox(_T("Insufficient \"Hit Points\" based on current level, class and stats.\n")
            _T("Would you like the amount changed to match your character's level, class and stats?"),
            MB_ICONQUESTION | MB_YESNO) == IDYES)
        {
            CharInfo.updateCharacterStats(cs);
            UpdateCharInfo();
            statChanged = true;
        }
    }

    if (ToInt(&MaxStamina) < cs.MaxStamina)
    {
        bFoundIssue = true;
        if (AfxMessageBox(_T("Insufficient \"Stamina\" based on current level and stats.\n")
            _T("Would you like the amount changed to match your character's level and stats?"),
            MB_ICONQUESTION | MB_YESNO) == IDYES)
        {
            CharInfo.updateCharacterStats(cs);
            UpdateCharInfo();
            statChanged = true;
        }
    }

    if (ToInt(&MaxMana) < cs.MaxMana)
    {
        bFoundIssue = true;
        if (AfxMessageBox(_T("Insufficient \"Mana\" based on current level and stats.\n")
            _T("Would you like the amount changed to match your character's level and stats?"),
            MB_ICONQUESTION | MB_YESNO) == IDYES)
        {
            CharInfo.updateCharacterStats(cs);
            UpdateCharInfo();
            statChanged = true;
        }
    }

    // Check Stat points
    if ((CharInfo.getStatPointsUsed() >= 4 * d2ce::MAX_BASICSTATS) && (cs.StatsLeft > 0))
    {
        bFoundIssue = true;
        if (AfxMessageBox(_T("No more stat points can be assigned but the amount left to assign is non-zero.\n")
            _T("Would you like the amount of stat points remaining to be set to zero?"),
            MB_ICONQUESTION | MB_YESNO) == IDYES)
        {
            cs.StatsLeft = 0;
            Cs.StatsLeft = 0;
            CharInfo.updateCharacterStats(cs);
            UpdateCharInfo();
            StatsLeftChanged = true;
            statChanged = true;
        }
    }

    std::uint32_t totalStatPoints = CharInfo.getTotalStatPoints();
    std::uint32_t earnedStatPoints = CharInfo.getTotalStartStatPoints() + CharInfo.getStatPointsEarned();
    if (totalStatPoints < earnedStatPoints)
    {
        bFoundIssue = true;
        if (AfxMessageBox(_T("Insufficient \"Total Stat Points\" based on current level and quests completed.\n")
            _T("Would you like the amount changed to match your character's level and quests completed?"),
            MB_ICONQUESTION | MB_YESNO) == IDYES)
        {
            cs.StatsLeft = std::min(earnedStatPoints - CharInfo.getStatPointsUsed(), d2ce::MAX_BASICSTATS);
            Cs.StatsLeft = cs.StatsLeft;
            CharInfo.updateCharacterStats(cs);
            UpdateCharInfo();
            StatsLeftChanged = true;
            statChanged = true;
        }
    }
    else if (totalStatPoints > earnedStatPoints)
    {
        expLevel = CharInfo.getLevelFromTotalStatPoints();
        if (expLevel > cs.Level)
        {
            bFoundIssue = true;
            if (expLevel > d2ce::NUM_OF_LEVELS)
            {
                // stats do not make sense
                if (AfxMessageBox(_T("\"Total Stat Points\" is higher then what can be achieved in the game.\n")
                    _T("Would you like to reset your stat points inorder to reallocate them?"),
                    MB_ICONQUESTION | MB_YESNO) == IDYES)
                {
                    CharInfo.resetStats();
                    CharInfo.fillCharacterStats(cs);
                    Cs.StatsLeft = cs.StatsLeft;
                    Cs.SkillChoices = cs.SkillChoices;
                    StatsLeftChanged = true;
                    SkillChoicesChanged = true;
                    statChanged = true;
                }
            }
            else
            {
                if (AfxMessageBox(_T("Your character's level is too low for the value of \"Total Stat Points\".\n")
                    _T("Would you like the amount changed to match your character's \"Total Stat Points\"?"),
                    MB_ICONQUESTION | MB_YESNO) == IDYES)
                {
                    cs.Level = expLevel;
                    CharInfo.updateCharacterStats(cs);
                    UpdateCharInfo();
                    statChanged = true;
                }
            }
        }
    }

    // Check Skill points
    if ((cs.SkillChoices > 0) && CharInfo.areSkillsMaxed())
    {
        bFoundIssue = true;
        if (AfxMessageBox(_T("No more skill points can be assigned but the amount left to assign is non-zero.\n")
            _T("Would you like the amount of skill points remaining to be set to zero?"),
            MB_ICONQUESTION | MB_YESNO) == IDYES)
        {
            cs.SkillChoices = 0;
            Cs.SkillChoices = 0;
            CharInfo.updateCharacterStats(cs);
            UpdateCharInfo();
            SkillChoicesChanged = true;
            statChanged = true;
        }
    }

    std::uint32_t totalPoints = CharInfo.getTotalSkillPoints();
    std::uint32_t earnedPoints = CharInfo.getSkillPointsEarned();
    if (totalPoints < earnedPoints)
    {
        bFoundIssue = true;
        if (AfxMessageBox(_T("Insufficient \"Total Skill Points\" based on current level and quests completed.\n")
            _T("Would you like the amount changed to match your character's level and quests completed?"),
            MB_ICONQUESTION | MB_YESNO) == IDYES)
        {
            cs.SkillChoices = std::min(earnedPoints - CharInfo.getSkillPointsUsed(), d2ce::MAX_SKILL_CHOICES);
            Cs.SkillChoices = cs.SkillChoices;
            SkillChoicesChanged = true;
            CharInfo.updateCharacterStats(cs);
            UpdateCharInfo();
            statChanged = true;
        }
    }
    else if (totalPoints > earnedPoints)
    {
        expLevel = CharInfo.getLevelFromTotalSkillPoints();
        if (expLevel > cs.Level)
        {
            bFoundIssue = true;
            if (expLevel > d2ce::NUM_OF_LEVELS)
            {
                // stats do not make sense
                if (AfxMessageBox(_T("\"Total Skill Points\" is higher then what can be achieved in the game.\n")
                    _T("Would you like to reset your skill points inorder to reallocate them?"),
                    MB_ICONQUESTION | MB_YESNO) == IDYES)
                {
                    CharInfo.resetStats();
                    CharInfo.fillCharacterStats(cs);
                    Cs.StatsLeft = cs.StatsLeft;
                    Cs.SkillChoices = cs.SkillChoices;
                    StatsLeftChanged = true;
                    SkillChoicesChanged = true;
                    statChanged = true;
                }
            }
            else
            {
                if (AfxMessageBox(_T("Your character's level is too low for the value of \"Total Skill Points\".\n")
                    _T("Would you like the amount changed to match your character's \"Total Skill Points\"?"),
                    MB_ICONQUESTION | MB_YESNO) == IDYES)
                {
                    cs.Level = expLevel;
                    CharInfo.updateCharacterStats(cs);
                    UpdateCharInfo();
                    statChanged = true;
                }
            }
        }
    }

    // Check Mercenary level
    if (CharInfo.getMercenaryInfo().isHired())
    {
        std::uint32_t mercLevel = CharInfo.getMercenaryInfo().getLevel();
        if (mercLevel > cs.Level)
        {
            bFoundIssue = true;
            if (AfxMessageBox(_T("Your character's level is too low for the value of your Mercenary's level.\n")
                _T("Would you like the amount changed to match your Mercenary's level?"),
                MB_ICONQUESTION | MB_YESNO) == IDYES)
            {
                cs.Level = mercLevel;
                CharInfo.updateCharacterStats(cs);
                UpdateCharInfo();
                statChanged = true;
            }
        }
    }

    // Check Hardcore character
    bool deadStatChanged = false;
    if (CheckIsHardcoreDead(deadStatChanged))
    {
        bFoundIssue = true;
        if (deadStatChanged)
        {
            statChanged = true;
        }
    }

    // Check file size
    if (CheckFileSize())
    {
        bFoundIssue = true;
    }

    if (bFoundIssue)
    {
        StatusBar.SetWindowText(_T("Character stats have been checked"));
        if (statChanged)
        {
            CheckStatsLeft();
            StatsChanged();
        }
    }
    else
    {
        CString msg(_T("No discrepancies found"));
        StatusBar.SetWindowText(msg);
        AfxMessageBox(msg, MB_OK | MB_ICONINFORMATION);
    }
}

void CD2MainForm::OnUpdateOptionsCheckChar(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(CharInfo.is_open() ? TRUE : FALSE);
}
//---------------------------------------------------------------------------
bool CD2MainForm::CheckIsHardcoreDead(bool& bStatChanged)
{
    bStatChanged = false;
    bool bFoundIssue = false;
    if (!CharInfo.is_open())
    {
        return bFoundIssue;
    }

    if (CharInfo.isHardcoreCharacter() && CharInfo.isDeadCharacter())
    {
        bFoundIssue = true;
        if (AfxMessageBox(_T("Your \"Hardcore\" character is dead and is no longer playable!\n")
            _T("Would you like to resurrect this character?"),
            MB_ICONQUESTION | MB_YESNO) == IDYES)
        {
            CharInfo.setIsDeadCharacter(false);
            bStatChanged = true;
            StatusChanged = true;
        }
    }

    return bFoundIssue;
}
//---------------------------------------------------------------------------
bool CD2MainForm::CheckFileSize()
{
    bool bFoundIssue = false;
    if (!CharInfo.is_open())
    {
        return bFoundIssue;
    }

    if (CharInfo.getFileSize() > d2ce::MAX_FILE_SIZE)
    {
        bFoundIssue = true;
        AfxMessageBox(_T("Your character's save file size is too large!\n")
            _T("You will need to remove/modify your characater's items to reduce the size of the file before using it in the game."),
            MB_OK | MB_ICONWARNING);
    }

    return bFoundIssue;
}
//---------------------------------------------------------------------------
void CD2MainForm::ClearAllBoolVars()
{
    Editted = false;
    ItemsChanged = false;
    StatusChanged = false;
    StatsLeftChanged = false;
    SkillChoicesChanged = false;
    WaypointsChanged = false;
    QuestsChanged = false;
    MercChanged = false;
    UpdateAppTitle();
}
//---------------------------------------------------------------------------
void CD2MainForm::OnCbnSelchangeCharClassCmb()
{
    UpdateTitleDisplay();
    d2ce::BasicStats bs;
    CharInfo.fillBasicStats(bs);
    if (static_cast<d2ce::EnumCharClass>(CharClass.GetCurSel()) != bs.Class)
    {
        ctrlEditted.insert(CharClass.GetDlgCtrlID());
        bs.Class = static_cast<d2ce::EnumCharClass>(CharClass.GetCurSel());
        CharInfo.updateBasicStats(bs);
        UpdateTitleDisplay();
        UpdateCharInfo();
        StatsChanged();
    }
}
//---------------------------------------------------------------------------
void CD2MainForm::OnBnClickedHardcoreCheck()
{
    UpdateData(TRUE);
    d2ce::BasicStats bs;
    CharInfo.fillBasicStats(bs);
    if (getCharacterStatus() != bs.Status)
    {
        bs.Status = getCharacterStatus();
        StatusChanged = true;
        CharInfo.updateBasicStats(bs);
        UpdateTitleDisplay();
        UpdateCharInfo();
        StatsChanged();
    }
}
//---------------------------------------------------------------------------
void CD2MainForm::OnBnClickedResurrectedCheck()
{
    UpdateData(TRUE);
    d2ce::BasicStats bs;
    CharInfo.fillBasicStats(bs);
    if (getCharacterStatus() != bs.Status)
    {
        bs.Status = getCharacterStatus();
        StatusChanged = true;
        CharInfo.updateBasicStats(bs);
        UpdateCharInfo();
        StatsChanged();
    }
}
//---------------------------------------------------------------------------
void CD2MainForm::OnBnClickedExpansionCheck()
{
    UpdateData(TRUE);
    d2ce::BasicStats bs;
    CharInfo.fillBasicStats(bs);
    if (getCharacterStatus() != bs.Status)
    {
        bs.Status = getCharacterStatus();
        StatusChanged = true;
        CharInfo.updateBasicStats(bs);
        UpdateTitleDisplay();
        UpdateClassDisplay();
        UpdateStartingActDisplay();
        UpdateCharInfo();
        StatsChanged();
    }
}
//---------------------------------------------------------------------------
void CD2MainForm::OnBnClickedLadderCheck()
{
    UpdateData(TRUE);
    d2ce::BasicStats bs;
    CharInfo.fillBasicStats(bs);
    if (getCharacterStatus() != bs.Status)
    {
        bs.Status = getCharacterStatus();
        StatusChanged = true;
        CharInfo.updateBasicStats(bs);
        UpdateCharInfo();
        StatsChanged();
    }
}
//---------------------------------------------------------------------------
void CD2MainForm::OnCbnSelchangeCharTitleCmb()
{
    d2ce::BasicStats bs;
    CharInfo.fillBasicStats(bs);
    if (getCharacterTitle() != bs.Title)
    {
        bs.Title = getCharacterTitle();
        ctrlEditted.insert(CharTitle.GetDlgCtrlID());
        CharInfo.updateBasicStats(bs);
        UpdateCharInfo();
        StatsChanged();
    }
}
//---------------------------------------------------------------------------
void CD2MainForm::OnCbnSelchangeStartingActCmb()
{
    d2ce::EnumAct startingAct = static_cast<d2ce::EnumAct>(StartingAct.GetCurSel());
    d2ce::BasicStats bs;
    CharInfo.fillBasicStats(bs);
    if (bs.StartingAct != startingAct)
    {
        bs.StartingAct = startingAct;
        ctrlEditted.insert(StartingAct.GetDlgCtrlID());
        CharInfo.updateBasicStats(bs);
        UpdateCharInfo();
        StatsChanged();
    }
}
//---------------------------------------------------------------------------
void CD2MainForm::OnCbnSelchangeDifficultyCmb()
{
    d2ce::BasicStats bs;
    CharInfo.fillBasicStats(bs);
    if (bs.DifficultyLastPlayed != getDifficultyLastPlayed())
    {
        bs.DifficultyLastPlayed = getDifficultyLastPlayed();
        ctrlEditted.insert(Difficulty.GetDlgCtrlID());
        CharInfo.updateBasicStats(bs);
        UpdateCharInfo();
        StatsChanged();
    }
}
//---------------------------------------------------------------------------
/*
   Fills in all the components to display the character's stats
*/
void CD2MainForm::DisplayCharInfo()
{
    CharTitle.SetCurSel(-1);
    SetupBasicStats();

    hasUpgradableRejuvenations = CharInfo.anyUpgradableRejuvenations();
    if (!hasUpgradableRejuvenations)
    {
        // implies no potions or all Full Rejuvenation potions so nothing to upgrade
        hasUpgradablePotions = false;
    }
    else
    {
        hasUpgradablePotions = CharInfo.anyUpgradablePotions();
    }

    hasUpgradableGems = CharInfo.anyUpgradableGems();

    d2ce::CharStats cs;
    CharInfo.fillCharacterStats(cs);

    SetInt(&CharLevel, cs.Level);

    SetInt(&CharStrength, cs.Strength);
    SetInt(&CharEnergy, cs.Energy);
    SetInt(&CharDexterity, cs.Dexterity);
    SetInt(&CharVitality, cs.Vitality);

    SetInt(&CurLife, cs.CurLife);
    SetInt(&MaxLife, cs.MaxLife);
    SetInt(&CurMana, cs.CurMana);
    SetInt(&MaxMana, cs.MaxMana);
    SetInt(&CurStamina, cs.CurStamina);
    SetInt(&MaxStamina, cs.MaxStamina);

    SetInt(&Experience, cs.Experience);
    SetInt(&GoldInBelt, cs.GoldInBelt);
    SetInt(&GoldInStash, cs.GoldInStash);
    CheckStatsLeft();

    CharStatusLadder.EnableWindow(CharInfo.getVersion() >= d2ce::EnumCharVersion::v110 ? TRUE : FALSE);
    s_levelInfo.ResetVersion(CharInfo.getVersion());
}
//---------------------------------------------------------------------------
void CD2MainForm::UpdateCharInfo()
{
    bool statsChanged = false;

    d2ce::BasicStats bs;
    CharInfo.fillBasicStats(bs);

    auto strValue = ToStdString(&CharName);
    if (_stricmp(strValue.c_str(), CharInfo.getName()) != 0)
    {
        SetText(&CharName, CharInfo.getName());
        if (_stricmp(CharInfo.getName(), Bs.Name) == 0)
        {
            auto iter = ctrlEditted.find(CharName.GetDlgCtrlID());
            if (iter != ctrlEditted.end())
            {
                ctrlEditted.erase(iter);
            }
        }
        else
        {
            ctrlEditted.insert(CharName.GetDlgCtrlID());
            statsChanged = true;
        }
    }

    if (getCharacterClass() != bs.Class)
    {
        UpdateClassDisplay();
    }

    if (CharStatusHardcore.GetCheck() != (bs.isHardcoreCharacter() ? 1 : 0))
    {
        CharStatusHardcore.SetCheck(bs.isHardcoreCharacter() ? 1 : 0);
        if (CharStatusExpansion.GetCheck() != (bs.isExpansionCharacter() ? 1 : 0))
        {
            CharStatusExpansion.SetCheck(bs.isExpansionCharacter() ? 1 : 0);
            UpdateClassDisplay();
            UpdateStartingActDisplay();
        }
        UpdateTitleDisplay();
        StatusChanged = true;
        statsChanged = true;
    }
    else if (CharStatusExpansion.GetCheck() != (bs.isExpansionCharacter() ? 1 : 0))
    {
        CharStatusExpansion.SetCheck(bs.isExpansionCharacter() ? 1 : 0);
        UpdateTitleDisplay();
        UpdateClassDisplay();
        UpdateStartingActDisplay();
        StatusChanged = true;
        statsChanged = true;
    }

    if (CharStatusResurrected.GetCheck() != (bs.isResurrectedCharacter() ? 1 : 0))
    {
        CharStatusResurrected.SetCheck(bs.isResurrectedCharacter() ? 1 : 0);
        StatusChanged = true;
        statsChanged = true;
    }

    if (CharStatusLadder.GetCheck() != (bs.isLadderCharacter() ? 1 : 0))
    {
        CharStatusResurrected.SetCheck(CharInfo.isResurrectedCharacter() ? 1 : 0);
        StatusChanged = true;
        statsChanged = true;
    }

    if (getCharacterTitle() != bs.Title)
    {
        CharTitle.SetCurSel((bs.Title.bits() & 0x0C) >> 2);
        if (bs.Title == Bs.Title)
        {
            auto iter = ctrlEditted.find(CharTitle.GetDlgCtrlID());
            if (iter != ctrlEditted.end())
            {
                ctrlEditted.erase(iter);
            }
        }
        else
        {
            ctrlEditted.insert(CharTitle.GetDlgCtrlID());
            statsChanged = true;
        }
    }

    d2ce::CharStats cs;
    CharInfo.fillCharacterStats(cs);

    std::uint32_t value = ToInt(&CharLevel);
    if (value != cs.Level)
    {
        SetInt(&CharLevel, cs.Level);
        if (value == Cs.Level)
        {
            auto iter = ctrlEditted.find(CharLevel.GetDlgCtrlID());
            if (iter != ctrlEditted.end())
            {
                ctrlEditted.erase(iter);
            }
        }
        else
        {
            ctrlEditted.insert(CharLevel.GetDlgCtrlID());
            statsChanged = true;
        }
    }

    value = ToInt(&CharStrength);
    if (value != cs.Strength)
    {
        SetInt(&CharStrength, cs.Strength);
        if (value == Cs.Strength)
        {
            auto iter = ctrlEditted.find(CharStrength.GetDlgCtrlID());
            if (iter != ctrlEditted.end())
            {
                ctrlEditted.erase(iter);
            }
        }
        else
        {
            ctrlEditted.insert(CharStrength.GetDlgCtrlID());
            statsChanged = true;
        }
    }

    value = ToInt(&CharEnergy);
    if (value != cs.Energy)
    {
        SetInt(&CharEnergy, cs.Energy);
        if (value == Cs.Energy)
        {
            auto iter = ctrlEditted.find(CharEnergy.GetDlgCtrlID());
            if (iter != ctrlEditted.end())
            {
                ctrlEditted.erase(iter);
            }
        }
        else
        {
            ctrlEditted.insert(CharEnergy.GetDlgCtrlID());
            statsChanged = true;
        }
    }

    value = ToInt(&CharDexterity);
    if (value != cs.Dexterity)
    {
        SetInt(&CharDexterity, cs.Dexterity);
        if (value == Cs.Dexterity)
        {
            auto iter = ctrlEditted.find(CharDexterity.GetDlgCtrlID());
            if (iter != ctrlEditted.end())
            {
                ctrlEditted.erase(iter);
            }
        }
        else
        {
            ctrlEditted.insert(CharDexterity.GetDlgCtrlID());
            statsChanged = true;
        }
    }

    value = ToInt(&CharVitality);
    if (value != cs.Vitality)
    {
        SetInt(&CharVitality, cs.Vitality);
        if (value == Cs.Vitality)
        {
            auto iter = ctrlEditted.find(CharVitality.GetDlgCtrlID());
            if (iter != ctrlEditted.end())
            {
                ctrlEditted.erase(iter);
            }
        }
        else
        {
            ctrlEditted.insert(CharVitality.GetDlgCtrlID());
            statsChanged = true;
        }
    }

    value = ToInt(&Experience);
    if (value != cs.Experience)
    {
        SetInt(&Experience, cs.Experience);
        if (value == Cs.Experience)
        {
            auto iter = ctrlEditted.find(Experience.GetDlgCtrlID());
            if (iter != ctrlEditted.end())
            {
                ctrlEditted.erase(iter);
            }
        }
        else
        {
            ctrlEditted.insert(Experience.GetDlgCtrlID());
            statsChanged = true;
        }
    }

    value = ToInt(&GoldInBelt);
    if (value != cs.GoldInBelt)
    {
        SetInt(&GoldInBelt, cs.GoldInBelt);
        if (value == Cs.GoldInBelt)
        {
            auto iter = ctrlEditted.find(GoldInBelt.GetDlgCtrlID());
            if (iter != ctrlEditted.end())
            {
                ctrlEditted.erase(iter);
            }
        }
        else
        {
            ctrlEditted.insert(GoldInBelt.GetDlgCtrlID());
            statsChanged = true;
        }
    }

    value = ToInt(&GoldInStash);
    if (value != cs.GoldInStash)
    {
        SetInt(&GoldInStash, cs.GoldInStash);
        if (value == Cs.GoldInStash)
        {
            auto iter = ctrlEditted.find(GoldInStash.GetDlgCtrlID());
            if (iter != ctrlEditted.end())
            {
                ctrlEditted.erase(iter);
            }
        }
        else
        {
            ctrlEditted.insert(GoldInStash.GetDlgCtrlID());
            statsChanged = true;
        }
    }

    value = ToInt(&MaxLife);
    if (value != cs.MaxLife)
    {
        SetInt(&MaxLife, cs.MaxLife);
        if (value == Cs.MaxLife)
        {
            auto iter = ctrlEditted.find(MaxLife.GetDlgCtrlID());
            if (iter != ctrlEditted.end())
            {
                ctrlEditted.erase(iter);
            }
        }
        else
        {
            ctrlEditted.insert(MaxLife.GetDlgCtrlID());
            statsChanged = true;
        }
    }

    value = ToInt(&CurLife);
    if (value != cs.CurLife)
    {
        SetInt(&CurLife, cs.CurLife);
        if (value == Cs.CurLife)
        {
            auto iter = ctrlEditted.find(CurLife.GetDlgCtrlID());
            if (iter != ctrlEditted.end())
            {
                ctrlEditted.erase(iter);
            }
        }
        else
        {
            ctrlEditted.insert(CurLife.GetDlgCtrlID());
            statsChanged = true;
        }
    }

    value = ToInt(&MaxStamina);
    if (value != cs.MaxStamina)
    {
        SetInt(&MaxStamina, cs.MaxStamina);
        if (value == Cs.MaxStamina)
        {
            auto iter = ctrlEditted.find(MaxStamina.GetDlgCtrlID());
            if (iter != ctrlEditted.end())
            {
                ctrlEditted.erase(iter);
            }
        }
        else
        {
            ctrlEditted.insert(MaxStamina.GetDlgCtrlID());
            statsChanged = true;
        }
    }

    value = ToInt(&CurStamina);
    if (value != cs.CurStamina)
    {
        SetInt(&CurStamina, cs.CurStamina);
        if (value == Cs.CurStamina)
        {
            auto iter = ctrlEditted.find(CurStamina.GetDlgCtrlID());
            if (iter != ctrlEditted.end())
            {
                ctrlEditted.erase(iter);
            }
        }
        else
        {
            ctrlEditted.insert(CurStamina.GetDlgCtrlID());
            statsChanged = true;
        }
    }

    value = ToInt(&MaxMana);
    if (value != cs.MaxMana)
    {
        SetInt(&MaxMana, cs.MaxMana);
        if (value == Cs.MaxMana)
        {
            auto iter = ctrlEditted.find(MaxMana.GetDlgCtrlID());
            if (iter != ctrlEditted.end())
            {
                ctrlEditted.erase(iter);
            }
        }
        else
        {
            ctrlEditted.insert(MaxMana.GetDlgCtrlID());
            statsChanged = true;
        }
    }

    value = ToInt(&CurMana);
    if (value != cs.CurMana)
    {
        SetInt(&CurMana, cs.CurMana);
        if (value == Cs.CurMana)
        {
            auto iter = ctrlEditted.find(CurMana.GetDlgCtrlID());
            if (iter != ctrlEditted.end())
            {
                ctrlEditted.erase(iter);
            }
        }
        else
        {
            ctrlEditted.insert(CurMana.GetDlgCtrlID());
            statsChanged = true;
        }
    }

    if (Cs.StatsLeft != cs.StatsLeft)
    {
        Cs.StatsLeft = cs.StatsLeft;
        StatsLeftChanged = true;
        statsChanged = true;
    }

    if (Cs.SkillChoices != cs.SkillChoices)
    {
        Cs.SkillChoices = cs.SkillChoices;
        SkillChoicesChanged = true;
        statsChanged = true;
    }

    if (statsChanged)
    {
        CheckStatsLeft();
        StatsChanged();
    }
}
//---------------------------------------------------------------------------
void CD2MainForm::EnableCharInfoBox(BOOL bEnable)
{
    CharName.EnableWindow(bEnable);
    CharStatusHardcore.EnableWindow(bEnable);
    CharStatusResurrected.EnableWindow(bEnable);
    CharStatusExpansion.EnableWindow(bEnable);
    CharStatusLadder.EnableWindow(bEnable);
    CharClass.EnableWindow(bEnable);
    Difficulty.EnableWindow(bEnable);
    StartingAct.EnableWindow(bEnable);
    CharLevel.EnableWindow(bEnable);
    CharStrength.EnableWindow(bEnable);
    CharEnergy.EnableWindow(bEnable);
    CharDexterity.EnableWindow(bEnable);
    CharVitality.EnableWindow(bEnable);
    CurLife.EnableWindow(bEnable);
    MaxLife.EnableWindow(bEnable);
    CurMana.EnableWindow(bEnable);
    MaxMana.EnableWindow(bEnable);
    CurStamina.EnableWindow(bEnable);
    MaxStamina.EnableWindow(bEnable);
    Experience.EnableWindow(bEnable);
    NextExperience.EnableWindow(bEnable);
    GoldInBelt.EnableWindow(bEnable);
    GoldInStash.EnableWindow(bEnable);
    CharTitle.EnableWindow(bEnable);
    EditSkillsButton.EnableWindow(bEnable);
    MaxAllButton.EnableWindow(bEnable);
    QuestsButton.EnableWindow(bEnable);
    WaypointsButton.EnableWindow(bEnable);

    if (!bEnable)
    {
        CharName.SetWindowText(_T(""));
        CharStatusHardcore.SetCheck(0);
        CharStatusResurrected.SetCheck(0);
        CharStatusExpansion.SetCheck(0);
        CharStatusHardcore.SetCheck(0);
        CharStatusLadder.SetCheck(0);
        CharClass.SetCurSel(-1);
        Difficulty.SetCurSel(-1);
        StartingAct.SetCurSel(-1);
        CharLevel.SetWindowText(_T(""));
        CharStrength.SetWindowText(_T(""));
        CharEnergy.SetWindowText(_T(""));
        CharDexterity.SetWindowText(_T(""));
        CharVitality.SetWindowText(_T(""));
        CurLife.SetWindowText(_T(""));
        MaxLife.SetWindowText(_T(""));
        CurMana.SetWindowText(_T(""));
        MaxMana.SetWindowText(_T(""));
        CurStamina.SetWindowText(_T(""));
        MaxStamina.SetWindowText(_T(""));
        Experience.SetWindowText(_T(""));
        GoldInBelt.SetWindowText(_T(""));
        GoldInStash.SetWindowText(_T(""));
        CharTitle.SetCurSel(-1);
        CharTitle.ResetContent();

        // Update Static so we refresh properly
        SetText(&NextExperience, _T(""));
        SetText(&StatsLeft, _T(""));

        StrengthPlusButton.EnableWindow(FALSE);
        StrengthPlusButton.ShowWindow(SW_HIDE);
        DexterityPlusButton.EnableWindow(FALSE);
        DexterityPlusButton.ShowWindow(SW_HIDE);
        VitalityPlusButton.EnableWindow(FALSE);
        VitalityPlusButton.ShowWindow(SW_HIDE);
        EnergyPlusButton.EnableWindow(FALSE);
        EnergyPlusButton.ShowWindow(SW_HIDE);
        StatsLeft.ShowWindow(SW_HIDE);
        StatsLeftCtrl.ShowWindow(SW_HIDE);
    }
}
//---------------------------------------------------------------------------
void CD2MainForm::OnFileOpen()
{
    CString	defaultDirectory = InitialDir;
    if (!curPathName.IsEmpty())
    {
        defaultDirectory = ExtractFilePath(curPathName);
        if (defaultDirectory.IsEmpty())
        {
            defaultDirectory = InitialDir;
        }
    }

    // close any open files
    if (CharInfo.is_open() && (DoFileCloseAction() == IDCANCEL))
    {
        return;
    }

    CFileDialog fileDialog(TRUE, _T("d2s"), NULL,
        OFN_HIDEREADONLY | OFN_FILEMUSTEXIST,
        _T("Diablo II Character Files (*.d2s)|*.d2s|All Files (*.*)|*.*||"), this);

    fileDialog.m_ofn.lpstrInitialDir = (LPCTSTR)defaultDirectory;
    if (fileDialog.DoModal() != IDOK)
    {
        return;
    }

    AfxGetApp()->AddToRecentFileList(fileDialog.GetPathName());

    OpenFile(fileDialog.GetPathName().GetString());
}
//---------------------------------------------------------------------------
LRESULT CD2MainForm::OnMRUFileOpen(WPARAM, LPARAM lParam)
{
    OpenFile((LPCTSTR)lParam);
    return 0;
}
//---------------------------------------------------------------------------
void CD2MainForm::OpenFile(LPCTSTR filename)
{
    // close any open files
    if (CharInfo.is_open() && (DoFileCloseAction() == IDCANCEL))
    {
        return;
    }

    CStringA newPathNameA(filename);

    // return if open not successful
    if (!CharInfo.open(newPathNameA, false))
    {
        CString errorMsg(CharInfo.getLastError().message().c_str());
        if (errorMsg.IsEmpty())
        {
            errorMsg = _T("Not a valid Diablo II save file.");
        }
        AfxMessageBox(errorMsg, MB_OK | MB_ICONERROR);

        Editted = false;
        OnFileClose();
        return;
    }

    if (static_cast<d2ce::CharacterErrc>(CharInfo.getLastError().value()) == d2ce::CharacterErrc::InvalidChecksum)
    {
        // Checksum is invalid
        if (AfxMessageBox(_T("Character File checksum is not valid.\nDo you wish to correct it now?"), MB_ICONERROR | MB_YESNO) == IDYES)
        {
            // The checksum was updated on load, so just save the file
            CharInfo.save();
        }
    }

    CheckFileSize();

    curPathName = filename;
    EnableCharInfoBox(TRUE);

    if (FileExists(ChangeFileExt(curPathName, _T(".bak"))))
    {
        hasBackupFile = true;
    }

    CharInfo.fillBasicStats(Bs);
    CharInfo.fillCharacterStats(Cs);
    DisplayCharInfo();
    ctrlEditted.clear();
    StatusBar.SetWindowText(_T("Character stats have been refreshed"));
    UpdateAppTitle();
}
//---------------------------------------------------------------------------
void CD2MainForm::OnFileClose()
{
    DoFileCloseAction();
}

void CD2MainForm::OnUpdateFileClose(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(CharInfo.is_open() ? TRUE : FALSE);
}
//---------------------------------------------------------------------------
int CD2MainForm::DoFileCloseAction()
{
    int ret = IDYES;
    if (Editted)
    {
        ret = AfxMessageBox(_T("Character has been modified.\nSave file?"), MB_ICONQUESTION | MB_YESNOCANCEL);
        switch (ret)
        {
        case IDYES:
            OnFileSave();
            break;
        case IDCANCEL:
            return ret;
        }
    }

    CharInfo.close();
    s_levelInfo.ResetVersion(d2ce::EnumCharVersion::v110);

    curPathName.Empty();
    Initialize();

    hasBackupFile = false;

    EnableCharInfoBox(FALSE);
    OnSetMessageString(AFX_IDS_IDLEMESSAGE);
    return ret;
}
//---------------------------------------------------------------------------
void CD2MainForm::OnFileSave()
{
    if (BackupChar)
    {
        WriteBackupFile();
    }

    if (!CharInfo.save())
    {
        CString errorMsg(CharInfo.getLastError().message().c_str());
        if (errorMsg.IsEmpty())
        {
            errorMsg = _T("Corrupted Diablo II save file discovered!");
        }

        AfxMessageBox(errorMsg, MB_OK | MB_ICONERROR);

        Editted = false;
        OnFileClose();
        return;
    }

    curPathName = CharInfo.getPathName();

    if (static_cast<d2ce::CharacterErrc>(CharInfo.getLastError().value()) == d2ce::CharacterErrc::AuxFileRenameError)
    {
        CString errorMsg(CharInfo.getLastError().message().c_str());
        if (errorMsg.IsEmpty())
        {
            errorMsg = _T("One or more auxiliary Character Files (.key, .ma*) could not be renamed!");
        }

        AfxMessageBox(errorMsg, MB_OK | MB_ICONWARNING);
    }

    CheckFileSize();

    CharInfo.fillBasicStats(Bs);
    CharInfo.fillCharacterStats(Cs);

    Initialize();
    CheckStatsLeft();

    CString msg(_T("Character stats saved"));
    StatusBar.SetWindowText(msg);
    AfxMessageBox(msg, MB_OK | MB_ICONINFORMATION);
}

void CD2MainForm::OnUpdateFileSave(CCmdUI* pCmdUI)
{
    pCmdUI->Enable((CharInfo.is_open() && Editted) ? TRUE : FALSE);
}
//---------------------------------------------------------------------------
void CD2MainForm::OnFileExportAsJson()
{
    if (!CharInfo.is_open())
    {
        return;
    }

    CString filename(CharInfo.getName());
    filename += ".json";

    CFileDialog fileDialog(FALSE, _T("json"), filename,
        OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
        _T("JSON file  (*.json)|*.json|"), this);

    if (fileDialog.DoModal() != IDOK)
    {
        return;
    }

    CStringA jsonFileName(fileDialog.GetPathName());
    std::FILE* jsonFile = NULL;
    fopen_s(&jsonFile, jsonFileName.GetString(), "wb");
    std::rewind(jsonFile);

    auto output = CharInfo.asJson();
    if (!output.empty())
    {
        std::fwrite(output.c_str(), output.size(), 1, jsonFile);
    }

    std::fclose(jsonFile);

    CString msg(_T("Character stats exported to JSON"));
    StatusBar.SetWindowText(msg);
    AfxMessageBox(msg, MB_OK | MB_ICONINFORMATION);
}

void CD2MainForm::OnUpdateFileExportAsJson(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(CharInfo.is_open() ? TRUE : FALSE);
}
//---------------------------------------------------------------------------
void CD2MainForm::OnAppExit()
{
    SendMessage(WM_SYSCOMMAND, SC_CLOSE, 0);
}
//---------------------------------------------------------------------------
void CD2MainForm::Initialize()
{
    ClearAllBoolVars();
    ctrlEditted.clear();
}
//---------------------------------------------------------------------------
void CD2MainForm::OnViewRefresh()
{
    if (Editted)
    {
        if (AfxMessageBox(_T("Character has been modified.\nAre you sure you want to refresh and lose all your changes?"), MB_YESNO | MB_ICONQUESTION) != IDYES)
        {
            return;
        }
    }

    Initialize();

    CharInfo.refresh();
    CharInfo.fillBasicStats(Bs);
    CharInfo.fillCharacterStats(Cs);
    DisplayCharInfo();

    ctrlEditted.clear();

    StatusBar.SetWindowText(_T("Character stats have been refreshed"));
}

void CD2MainForm::OnUpdateViewRefresh(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(CharInfo.is_open() ? TRUE : FALSE);
}
//-------------------------------------------------------------------------
void CD2MainForm::OnBnClickedMaxAllBtn()
{
    OnOptionsMaxEverything();
}
void CD2MainForm::OnOptionsMaxEverything()
{
    if (!CharInfo.is_open())
    {
        return;
    }

    d2ce::CharStats cs;
    CharInfo.fillCharacterStats(cs);
    cs.Level = d2ce::NUM_OF_LEVELS;
    cs.Experience = d2ce::MAX_EXPERIENCE;
    cs.GoldInBelt = cs.getMaxGoldInBelt();
    cs.GoldInStash = cs.getMaxGoldInStash();
    CharInfo.updateCharacterStats(cs);
    UpdateCharInfo();

    CharInfo.upgradeGems();
    CharInfo.upgradePotions();
    CharInfo.fillAllStackables();
    CharInfo.maxDurabilityAllItems();
    ItemsChanged = true;
    CheckStatsLeft();
    StatsChanged();
}
void CD2MainForm::OnUpdateOptionsMaxEverything(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(CharInfo.is_open() ? MaxAllButton.IsWindowEnabled() : FALSE);
}
//---------------------------------------------------------------------------
/*
   Looks in the Registry for the location of where Diablo II is installed
   and sets the Open Dialog to start in the "save" directory
*/
#define MAX_VALUE_NAME 4096
void CD2MainForm::SetStartDir()
{
    // get the 'Saved Games' shell directory and try searching there
    PWSTR saveBasePath;
    if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_SavedGames, 0, nullptr, &saveBasePath)))
    {
        std::filesystem::path savePath(saveBasePath);
        CoTaskMemFree(saveBasePath);

        // Look for Ressurected path first
        auto d2Path = savePath / _T("Diablo II Resurrected");
        if (std::filesystem::exists(d2Path))
        {
            d2Path += _T("\\");
            InitialDir = d2Path.string().c_str();
            return;
        }

        // Fall back on classic Diablo II
        d2Path = savePath / _T("Diablo II");
        if (std::filesystem::exists(d2Path))
        {
            d2Path += _T("\\");
            InitialDir = d2Path.string().c_str();
            return;
        }
    }

    // Search the Classic register position for the path to use
    HKEY regKey = 0;
    if (::RegOpenKeyEx(HKEY_CURRENT_USER, _T("SOFTWARE\\Blizzard Entertainment\\Diablo II"), 0, KEY_QUERY_VALUE, &regKey) != ERROR_SUCCESS)
    {
        if (::RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Blizzard Entertainment\\Diablo II"), 0, KEY_QUERY_VALUE, &regKey) != ERROR_SUCCESS)
        {
            InitialDir.Empty();
            return;
        }
    }

    std::vector<wchar_t> valueName(MAX_VALUE_NAME + 1, 0);
    DWORD valueNameLength = MAX_VALUE_NAME;
    DWORD valueType = 0;
    if ((RegQueryValueEx(regKey, _T("NewSavePath"), 0, &valueType, (LPBYTE)&valueName[0], &valueNameLength) == ERROR_SUCCESS) && (valueType == REG_SZ))
    {
        InitialDir = &valueName[0];
    }
    else if ((RegQueryValueEx(regKey, _T("Save Path"), 0, &valueType, (LPBYTE)&valueName[0], &valueNameLength) == ERROR_SUCCESS) && (valueType == REG_SZ))
    {
        InitialDir = &valueName[0];
    }

    RegCloseKey(regKey);
    regKey = 0;
}
//---------------------------------------------------------------------------
void CD2MainForm::SetupBasicStats()
{
    d2ce::BasicStats bs;
    CharInfo.fillBasicStats(bs);

    // display character stats
    SetText(&CharName, &bs.Name[0]);

    if (bs.isHardcoreCharacter())
    {
        if (bs.isResurrectedCharacter())
        {
            // Hardcore characters can't ever have been ressurrected
            CharInfo.setIsResurrectedCharacter(false);
            StatusChanged = true;
            Editted = true;
        }

        bool deadStatChanged = false;
        if (CheckIsHardcoreDead(deadStatChanged))
        {
            Editted = true;
        }
    }

    // set state of Status checkbox
    CharStatusHardcore.SetCheck(bs.isHardcoreCharacter() ? 1 : 0);
    CharStatusResurrected.SetCheck(bs.isResurrectedCharacter() ? 1 : 0);
    CharStatusExpansion.SetCheck(bs.isExpansionCharacter() ? 1 : 0);
    CharStatusLadder.SetCheck(bs.isLadderCharacter() ? 1 : 0);

    UpdateTitleDisplay();
    UpdateClassDisplay();
    Difficulty.SetCurSel(static_cast<std::underlying_type_t<d2ce::EnumDifficulty>>(bs.DifficultyLastPlayed));
    UpdateStartingActDisplay();
}
//---------------------------------------------------------------------------
void CD2MainForm::StatsChanged()
{
    if (!Editted && (ItemsChanged || !ctrlEditted.empty() || StatusChanged ||
        StatsLeftChanged || SkillChoicesChanged || WaypointsChanged || QuestsChanged ||
        MercChanged))
    {
        Editted = true;
        UpdateAppTitle();
    }
}
//---------------------------------------------------------------------------
void CD2MainForm::CheckStatsLeft()
{
    if (Cs.StatsLeft > 0)
    {
        if (ToInt(&CharStrength) < d2ce::MAX_BASICSTATS)
        {
            StrengthPlusButton.ShowWindow(SW_SHOWNORMAL);
            StrengthPlusButton.EnableWindow(TRUE);
        }

        if (ToInt(&CharDexterity) < d2ce::MAX_BASICSTATS)
        {
            DexterityPlusButton.ShowWindow(SW_SHOWNORMAL);
            DexterityPlusButton.EnableWindow(TRUE);
        }

        if (ToInt(&CharVitality) < d2ce::MAX_BASICSTATS)
        {
            VitalityPlusButton.ShowWindow(SW_SHOWNORMAL);
            VitalityPlusButton.EnableWindow(TRUE);
        }

        if (ToInt(&CharEnergy) < d2ce::MAX_BASICSTATS)
        {
            EnergyPlusButton.ShowWindow(SW_SHOWNORMAL);
            EnergyPlusButton.EnableWindow(TRUE);
        }

        StatsLeft.ShowWindow(SW_SHOWNORMAL);
        StatsLeftCtrl.ShowWindow(SW_SHOWNORMAL);
        SetInt(&StatsLeft, Cs.StatsLeft);
    }
    else
    {
        StrengthPlusButton.EnableWindow(FALSE);
        StrengthPlusButton.ShowWindow(SW_HIDE);
        DexterityPlusButton.EnableWindow(FALSE);
        DexterityPlusButton.ShowWindow(SW_HIDE);
        VitalityPlusButton.EnableWindow(FALSE);
        VitalityPlusButton.ShowWindow(SW_HIDE);
        VitalityPlusButton.EnableWindow(FALSE);
        EnergyPlusButton.ShowWindow(SW_HIDE);

        StatsLeft.SetWindowText(_T(""));
        StatsLeft.ShowWindow(SW_HIDE);
        StatsLeftCtrl.ShowWindow(SW_HIDE);
    }
}
//---------------------------------------------------------------------------
void CD2MainForm::UpdateAppTitle()
{
    UINT uID = AFX_IDS_APP_TITLE;
    if (!curPathName.IsEmpty())
    {
        if (CharInfo.getVersion() <= d2ce::EnumCharVersion::v110)
        {
            uID = IDS_OLD_APP_TITLE;
        }
    }

    CString newAppTitle;
    if (newAppTitle.LoadString(uID) != 0)
    {
        if (!curPathName.IsEmpty())
        {
            newAppTitle += _T(" - ") + ExtractFileName(curPathName);
            if (Editted)
            {
                newAppTitle += "*";
            }
            switch (CharInfo.getVersion())
            {
            case d2ce::EnumCharVersion::v110:
                newAppTitle += _T(" (Version 1.10-1.14d)");
                break;
            case d2ce::EnumCharVersion::v109:
                newAppTitle += _T(" (Version 1.09)");
                break;
            case d2ce::EnumCharVersion::v108:
                newAppTitle += _T(" (Version Standard 1.08)");
                break;
            case d2ce::EnumCharVersion::v107:
                newAppTitle += _T(" (Version 1.07/Expansion 1.08)");
                break;
            case d2ce::EnumCharVersion::v100:
                newAppTitle += _T(" (Version 1.00 - 1.06)");
                break;
            }
        }
        SetWindowText(newAppTitle);
    }
}
//---------------------------------------------------------------------------
void CD2MainForm::UpdateClassDisplay()
{
    // Check if we need to add before selected class
    if (CharInfo.isExpansionCharacter())
    {
        if (CharClass.GetCount() < d2ce::NUM_OF_CLASSES)
        {
            // add the expansion set characters to combo box component
            CharClass.AddString(_T("Druid"));
            CharClass.AddString(_T("Assassin"));
        }
    }

    CharClass.SetCurSel(static_cast<std::underlying_type_t<d2ce::EnumCharClass>>(CharInfo.getClass()));

    // Check if we need to remove after selecting class
    if (!CharInfo.isExpansionCharacter())
    {
        // remove expansion set classes from combo box component
        auto pos = CharClass.FindStringExact(0, _T("Druid"));
        if (pos != CB_ERR)
        {
            CharClass.DeleteString(pos);
        }

        pos = CharClass.FindStringExact(0, _T("Assassin"));
        if (pos != CB_ERR)
        {
            CharClass.DeleteString(pos);
        }
    }
}
//---------------------------------------------------------------------------
/*
   Makes sure the correct gender titles are displayed based on the
   character's class
*/
void CD2MainForm::UpdateTitleDisplay()
{
    int curSel = CharTitle.GetCurSel();
    if (curSel < 0)
    {
        curSel = (CharInfo.getTitle().bits() & 0x0C) >> 2;
    }

    CharTitle.ResetContent();

    const char** pValidTitles = nullptr;
    switch (static_cast<d2ce::EnumCharClass>(CharClass.GetCurSel()))
    {
    case d2ce::EnumCharClass::Amazon:
    case d2ce::EnumCharClass::Assassin:
    case d2ce::EnumCharClass::Sorceress: // add titles for female characters
        if (CharInfo.isExpansionCharacter())
        {
            if (CharInfo.isHardcoreCharacter())
            {
                pValidTitles = HardcoreExpansionTitle;
            }
            else
            {
                pValidTitles = FemaleExpansionTitle;
            }
        }
        else if (CharInfo.isHardcoreCharacter())
        {
            pValidTitles = FemaleHardcoreTitle;
        }
        else
        {
            pValidTitles = FemaleTitle;
        }
        break;
    case d2ce::EnumCharClass::Barbarian:
    case d2ce::EnumCharClass::Druid:
    case d2ce::EnumCharClass::Necromancer:
    case d2ce::EnumCharClass::Paladin: // add titles for male characters
    default:
        if (CharInfo.isExpansionCharacter())
        {
            if (CharInfo.isHardcoreCharacter())
            {
                pValidTitles = HardcoreExpansionTitle;
            }
            else
            {
                pValidTitles = MaleExpansionTitle;
            }
        }
        else if (CharInfo.isHardcoreCharacter())
        {
            pValidTitles = MaleHardcoreTitle;
        }
        else
        {
            pValidTitles = MaleTitle;
        }
        break;
    }

    if (pValidTitles != nullptr)
    {
        for (std::uint32_t i = 0; i < NUM_OF_TITLES; i++)
        {
            CharTitle.AddString(CString(pValidTitles[i]));
        }
    }

    CharTitle.SetCurSel(curSel);
}
//---------------------------------------------------------------------------
void CD2MainForm::UpdateStartingActDisplay()
{
    // Check if we need to add before selected starting Act
    if (CharInfo.isExpansionCharacter())
    {
        if (StartingAct.GetCount() < d2ce::NUM_OF_ACTS)
        {
            StartingAct.AddString(_T("V"));
        }
    }

    StartingAct.SetCurSel(static_cast <std::underlying_type_t<d2ce::EnumAct>>(CharInfo.getStartingAct()));

    // Check if we need to remove after selected starting Act
    if (!CharInfo.isExpansionCharacter())
    {
        auto pos = StartingAct.FindStringExact(0, _T("V"));
        if (pos != CB_ERR)
        {
            StartingAct.DeleteString(pos);
        }
    }
}
//---------------------------------------------------------------------------
void CD2MainForm::OnOptionsUpgradeGems()
{
    if (!CharInfo.is_open())
    {
        return;
    }

    auto numConverted = CharInfo.upgradeGems();
    CString msg;
    msg.Format(_T("%zd gem(s) have been upgraded to perfect state"), numConverted);
    if (numConverted > 0)
    {
        hasUpgradableGems = false;
        StatusBar.SetWindowText(msg);

        ItemsChanged = true;
        StatsChanged();
    }

    AfxMessageBox(msg, MB_ICONINFORMATION | MB_OK);
}
void CD2MainForm::OnUpdateOptionsUpgradeGems(CCmdUI* pCmdUI)
{
    pCmdUI->Enable((CharInfo.is_open() && hasUpgradableGems) ? TRUE : FALSE);
}
//---------------------------------------------------------------------------
void CD2MainForm::OnOptionsUpgradePotions()
{
    if (!CharInfo.is_open())
    {
        return;
    }

    auto numConverted = CharInfo.upgradePotions();
    CString msg;
    msg.Format(_T("%zd potion(s) have been upgraded to their highest level"), numConverted);
    if (numConverted > 0)
    {
        hasUpgradablePotions = false;
        hasUpgradableRejuvenations = CharInfo.anyUpgradableRejuvenations();
        StatusBar.SetWindowText(msg);

        ItemsChanged = true;
        StatsChanged();
    }

    AfxMessageBox(msg, MB_ICONINFORMATION | MB_OK);
}

void CD2MainForm::OnUpdateOptionsUpgradePotions(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(hasUpgradablePotions ? TRUE : FALSE);
}
//---------------------------------------------------------------------------
void CD2MainForm::OnOptionsUpgradeRejuvenation()
{
    if (!CharInfo.is_open())
    {
        return;
    }

    auto numConverted = CharInfo.upgradeRejuvenationPotions();
    CString msg;
    msg.Format(_T("%zd potion(s) have been upgraded to Full Rejuvenation potions"), numConverted);
    if (numConverted > 0)
    {
        hasUpgradableRejuvenations = false;
        hasUpgradablePotions = false;
        StatusBar.SetWindowText(msg);

        ItemsChanged = true;
        StatsChanged();
    }

    AfxMessageBox(msg, MB_ICONINFORMATION | MB_OK);
}

void CD2MainForm::OnUpdateOptionsUpgradeRejuvenation(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(hasUpgradableRejuvenations ? TRUE : FALSE);
}
//---------------------------------------------------------------------------
/*
   Makes a backup copy of the character file and overwrites any
   existing backup file
*/
void CD2MainForm::WriteBackupFile()
{
    CStringA oldPathNameA(curPathName);
    CString backupname = ChangeFileExt(curPathName, _T(".bak"));
    if (CopyFile(curPathName, backupname, false))
    {
        hasBackupFile = true;
    }
}

//---------------------------------------------------------------------------
void CD2MainForm::OnViewSkillTree()
{
    CD2SkillTreeForm dlg(*this);
    dlg.DoModal();
}

void CD2MainForm::OnUpdateViewSkillTree(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(CharInfo.is_open() ? EditSkillsButton.IsWindowEnabled() : FALSE);
}

void CD2MainForm::OnBnClickedEditSkillsBtn()
{
    OnViewSkillTree();
}
//---------------------------------------------------------------------------
void CD2MainForm::OnViewQuests()
{
    CD2QuestsForm dlg(*this);
    dlg.DoModal();
}

void CD2MainForm::OnUpdateViewQuests(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(CharInfo.is_open() ? QuestsButton.IsWindowEnabled() : FALSE);
}

void CD2MainForm::OnBnClickedQuestsBtn()
{
    OnViewQuests();
}
//---------------------------------------------------------------------------
void CD2MainForm::OnViewWaypoints()
{
    CD2WaypointsForm dlg(*this);
    dlg.DoModal();
}

void CD2MainForm::OnUpdateViewWaypoints(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(CharInfo.is_open() ? WaypointsButton.IsWindowEnabled() : FALSE);
}

void CD2MainForm::OnBnClickedWaypointsBtn()
{
    OnViewWaypoints();
}
//---------------------------------------------------------------------------
void CD2MainForm::OnEnSetfocusCharName()
{
    prev_name = ToStdString(&CharName);
}
void CD2MainForm::OnEnChangeCharName()
{
    ctrlEditted.insert(CharName.GetDlgCtrlID());
}
void CD2MainForm::OnEnKillfocusCharName()
{
    auto newName = ToStdString(&CharName);
    if (newName != prev_name)
    {
        CString newFileName = ExtractFilePath(curPathName) + _T("\\") + ToText(&CharName) + _T(".d2s");
        if (FileExists(newFileName) && Editted)
        {
            AfxMessageBox(_T("A file with that name already exists.  Please select another name."), MB_OK | MB_ICONEXCLAMATION);
            SetText(&CharName, prev_name.c_str());
            return;
        }

        ctrlEditted.insert(CharName.GetDlgCtrlID());
        d2ce::BasicStats bs;
        CharInfo.fillBasicStats(bs);
        strcpy_s(bs.Name, sizeof(bs.Name), newName.c_str());
        bs.Name[15] = 0; // must be zero
        CharInfo.updateBasicStats(bs);
        UpdateCharInfo();
        StatsChanged();
    }
}
//---------------------------------------------------------------------------
void CD2MainForm::OnEnSetfocusCharDexterity()
{
    OrigValue = ToInt(&CharDexterity);
}
void CD2MainForm::OnEnChangeCharDexterity()
{
    ctrlEditted.insert(CharDexterity.GetDlgCtrlID());
}
void CD2MainForm::OnEnKillfocusCharDexterity()
{
    d2ce::CharStats cs;
    CharInfo.fillCharacterStats(cs);
    cs.Dexterity = std::min(ToInt(&CharDexterity), d2ce::MAX_BASICSTATS);
    CharInfo.updateCharacterStats(cs);
    UpdateCharInfo();
}
void CD2MainForm::OnBnClickedDexterityPlus()
{
    std::uint32_t value = ToInt(&CharDexterity);
    if (value < d2ce::MAX_BASICSTATS && Cs.StatsLeft > 0)
    {
        ++value;
        --Cs.StatsLeft;
        StatsLeftChanged = true;
        d2ce::CharStats cs;
        CharInfo.fillCharacterStats(cs);
        cs.Energy = std::min(value, d2ce::MAX_BASICSTATS);
        cs.StatsLeft = Cs.StatsLeft;
        CharInfo.updateCharacterStats(cs);
        UpdateCharInfo();
        return;
    }

    // should not happen
    CheckStatsLeft();
}
//---------------------------------------------------------------------------
void CD2MainForm::OnEnChangeCharEnergy()
{
    ctrlEditted.insert(CharEnergy.GetDlgCtrlID());
}
void CD2MainForm::OnEnSetfocusCharEnergy()
{
    OrigValue = ToInt(&CharEnergy);
}
void CD2MainForm::OnEnKillfocusCharEnergy()
{
    d2ce::CharStats cs;
    CharInfo.fillCharacterStats(cs);
    cs.Energy = std::min(ToInt(&CharEnergy), d2ce::MAX_BASICSTATS);
    CharInfo.updateCharacterStats(cs);
    UpdateCharInfo();
}
//---------------------------------------------------------------------------
void CD2MainForm::OnBnClickedEnergyPlus()
{
    std::uint32_t value = ToInt(&CharEnergy);
    if (value < d2ce::MAX_BASICSTATS && Cs.StatsLeft > 0)
    {
        ++value;
        --Cs.StatsLeft;
        StatsLeftChanged = true;
        d2ce::CharStats cs;
        CharInfo.fillCharacterStats(cs);
        cs.Energy = std::min(value, d2ce::MAX_BASICSTATS);
        cs.StatsLeft = Cs.StatsLeft;
        CharInfo.updateCharacterStats(cs);
        UpdateCharInfo();
        return;
    }

    // should not happen
    CheckStatsLeft();
}
//---------------------------------------------------------------------------
void CD2MainForm::OnEnSetfocusCharLevel()
{
    OrigValue = ToInt(&CharLevel);
}
void CD2MainForm::OnEnChangeCharLevel()
{
    ctrlEditted.insert(CharLevel.GetDlgCtrlID());
}
void CD2MainForm::OnEnKillfocusCharLevel()
{
    d2ce::CharStats cs;
    CharInfo.fillCharacterStats(cs);
    std::uint32_t level = std::min(std::max(ToInt(&CharLevel), std::uint32_t(1)), d2ce::NUM_OF_LEVELS);
    if (level != cs.Level)
    {
        cs.Level = level;
        ctrlEditted.insert(CharLevel.GetDlgCtrlID());
        CharInfo.updateCharacterStats(cs);
        UpdateCharInfo();
    }
}
//---------------------------------------------------------------------------
void CD2MainForm::OnEnSetfocusCharStrength()
{
    OrigValue = ToInt(&CharStrength);
}
void CD2MainForm::OnEnChangeCharStrength()
{
    ctrlEditted.insert(CharStrength.GetDlgCtrlID());
}
void CD2MainForm::OnEnKillfocusCharStrength()
{
    d2ce::CharStats cs;
    CharInfo.fillCharacterStats(cs);
    cs.Strength = std::min(ToInt(&CharStrength), d2ce::MAX_BASICSTATS);
    CharInfo.updateCharacterStats(cs);
    UpdateCharInfo();
}
//---------------------------------------------------------------------------
void CD2MainForm::OnBnClickedStrengthPlus()
{
    std::uint32_t value = ToInt(&CharStrength);
    if (value < d2ce::MAX_BASICSTATS && Cs.StatsLeft > 0)
    {
        ++value;
        --Cs.StatsLeft;
        StatsLeftChanged = true;
        d2ce::CharStats cs;
        CharInfo.fillCharacterStats(cs);
        cs.Strength = std::min(value, d2ce::MAX_BASICSTATS);
        cs.StatsLeft = Cs.StatsLeft;
        CharInfo.updateCharacterStats(cs);
        UpdateCharInfo();
        return;
    }

    // should not happen
    CheckStatsLeft();
}
//---------------------------------------------------------------------------
void CD2MainForm::OnEnSetfocusCharVitality()
{
    OrigValue = ToInt(&CharVitality);
}
void CD2MainForm::OnEnChangeCharVitality()
{
    ctrlEditted.insert(CharVitality.GetDlgCtrlID());
}
void CD2MainForm::OnEnKillfocusCharVitality()
{
    d2ce::CharStats cs;
    CharInfo.fillCharacterStats(cs);
    cs.Vitality = std::min(ToInt(&CharVitality), d2ce::MAX_BASICSTATS);
    CharInfo.updateCharacterStats(cs);
    UpdateCharInfo();
}
void CD2MainForm::OnBnClickedVitalityPlus()
{
    std::uint32_t value = ToInt(&CharVitality);
    if (value < d2ce::MAX_BASICSTATS && Cs.StatsLeft > 0)
    {
        ++value;
        --Cs.StatsLeft;
        StatsLeftChanged = true;
        d2ce::CharStats cs;
        CharInfo.fillCharacterStats(cs);
        cs.Vitality = std::min(value, d2ce::MAX_BASICSTATS);
        cs.StatsLeft = Cs.StatsLeft;
        CharInfo.updateCharacterStats(cs);
        UpdateCharInfo();
        return;
    }

    // should not happen
    CheckStatsLeft();
}
//---------------------------------------------------------------------------
void CD2MainForm::OnEnSetfocusCurLife()
{
    OrigValue = ToInt(&CurLife);
}
void CD2MainForm::OnEnChangeCurLife()
{
    ctrlEditted.insert(CurLife.GetDlgCtrlID());
}
void CD2MainForm::OnEnKillfocusCurLife()
{
    d2ce::CharStats cs;
    CharInfo.fillCharacterStats(cs);
    cs.CurLife = std::min(ToInt(&CurLife), d2ce::MAX_LMS);
    CharInfo.updateCharacterStats(cs);
    UpdateCharInfo();
}
//---------------------------------------------------------------------------
void CD2MainForm::OnEnSetfocusMaxLife()
{
    OrigValue = ToInt(&MaxLife);
}
void CD2MainForm::OnEnChangeMaxLife()
{
    ctrlEditted.insert(MaxLife.GetDlgCtrlID());
}
void CD2MainForm::OnEnKillfocusMaxLife()
{
    d2ce::CharStats cs;
    CharInfo.fillCharacterStats(cs);
    cs.MaxLife = std::min(ToInt(&MaxLife), d2ce::MAX_LMS);
    CharInfo.updateCharacterStats(cs);
    UpdateCharInfo();
}
//---------------------------------------------------------------------------
void CD2MainForm::OnEnSetfocusCurMana()
{
    OrigValue = ToInt(&CurMana);
}
void CD2MainForm::OnEnChangeCurMana()
{
    ctrlEditted.insert(CurMana.GetDlgCtrlID());
}
void CD2MainForm::OnEnKillfocusCurMana()
{
    d2ce::CharStats cs;
    CharInfo.fillCharacterStats(cs);
    cs.CurMana = std::min(ToInt(&CurMana), d2ce::MAX_LMS);
    CharInfo.updateCharacterStats(cs);
    UpdateCharInfo();
}
//---------------------------------------------------------------------------
void CD2MainForm::OnEnSetfocusMaxMana()
{
    OrigValue = ToInt(&CurLife);
}
void CD2MainForm::OnEnChangeMaxMana()
{
    ctrlEditted.insert(MaxMana.GetDlgCtrlID());
}
void CD2MainForm::OnEnKillfocusMaxMana()
{
    d2ce::CharStats cs;
    CharInfo.fillCharacterStats(cs);
    cs.MaxMana = std::min(ToInt(&MaxMana), d2ce::MAX_LMS);
    CharInfo.updateCharacterStats(cs);
    UpdateCharInfo();
}
//---------------------------------------------------------------------------
void CD2MainForm::OnEnSetfocusCurStamina()
{
    OrigValue = ToInt(&CurStamina);
}
void CD2MainForm::OnEnChangeCurStamina()
{
    ctrlEditted.insert(CurStamina.GetDlgCtrlID());
}
void CD2MainForm::OnEnKillfocusCurStamina()
{
    d2ce::CharStats cs;
    CharInfo.fillCharacterStats(cs);
    cs.CurStamina = std::min(ToInt(&CurStamina), d2ce::MAX_LMS);
    CharInfo.updateCharacterStats(cs);
    UpdateCharInfo();
}
//---------------------------------------------------------------------------
void CD2MainForm::OnEnSetfocusMaxStamina()
{
    OrigValue = ToInt(&MaxStamina);
}
void CD2MainForm::OnEnChangeMaxStamina()
{
    ctrlEditted.insert(MaxStamina.GetDlgCtrlID());
}
void CD2MainForm::OnEnKillfocusMaxStamina()
{
    d2ce::CharStats cs;
    CharInfo.fillCharacterStats(cs);
    cs.MaxStamina = std::min(ToInt(&MaxStamina), d2ce::MAX_LMS);
    CharInfo.updateCharacterStats(cs);
    UpdateCharInfo();
}
//---------------------------------------------------------------------------
void CD2MainForm::OnEnSetfocusCharExperience()
{
    OrigValue = ToInt(&Experience);
}
void CD2MainForm::OnEnChangeCharExperience()
{
    ctrlEditted.insert(Experience.GetDlgCtrlID());
}
void CD2MainForm::OnEnKillfocusCharExperience()
{
    d2ce::CharStats cs;
    CharInfo.fillCharacterStats(cs);
    cs.Experience = std::min(ToInt(&Experience), d2ce::MAX_EXPERIENCE);
    CharInfo.updateCharacterStats(cs);
    UpdateCharInfo();
}
//---------------------------------------------------------------------------
void CD2MainForm::OnEnSetfocusGoldInBelt()
{
    OrigValue = ToInt(&GoldInBelt);
}
void CD2MainForm::OnEnChangeGoldInBelt()
{
    ctrlEditted.insert(GoldInBelt.GetDlgCtrlID());
}
void CD2MainForm::OnEnKillfocusGoldInBelt()
{
    d2ce::CharStats cs;
    CharInfo.fillCharacterStats(cs);
    cs.GoldInBelt = ToInt(&GoldInBelt);;
    CharInfo.updateCharacterStats(cs);
    UpdateCharInfo();
}
//---------------------------------------------------------------------------
void CD2MainForm::OnEnSetfocusGoldInStash()
{
    OrigValue = ToInt(&GoldInStash);
}
void CD2MainForm::OnEnChangeGoldInStash()
{
    ctrlEditted.insert(GoldInStash.GetDlgCtrlID());
}
void CD2MainForm::OnEnKillfocusGoldInStash()
{
    d2ce::CharStats cs;
    CharInfo.fillCharacterStats(cs);
    cs.GoldInStash = ToInt(&GoldInStash);
    CharInfo.updateCharacterStats(cs);
    UpdateCharInfo();
}
//---------------------------------------------------------------------------
std::string CD2MainForm::ToStdString(const CWnd* Sender) const
{
    return (LPCSTR)CStringA(ToText(Sender));
}
//---------------------------------------------------------------------------
CString CD2MainForm::ToText(const CWnd* Sender) const
{
    if (Sender->IsKindOf(RUNTIME_CLASS(CEdit)) || Sender->IsKindOf(RUNTIME_CLASS(CStatic)))
    {
        CString sWindowText;
        Sender->GetWindowText(sWindowText);
        return sWindowText;
    }

    if (Sender->IsKindOf(RUNTIME_CLASS(CComboBox)))
    {
        CString sWindowText;
        ((CComboBox*)Sender)->GetLBText(((CComboBox*)Sender)->GetCurSel(), sWindowText);
        return sWindowText;
    }

    return CString();
}
//---------------------------------------------------------------------------
CStringA CD2MainForm::ToTextA(const CWnd* Sender) const
{
    return CStringA(ToText(Sender));
}
//---------------------------------------------------------------------------
void CD2MainForm::SetText(CWnd* Sender, const char* newValue)
{
    if (Sender->IsKindOf(RUNTIME_CLASS(CEdit)) || Sender->IsKindOf(RUNTIME_CLASS(CStatic)))
    {
        CString sWindowText(newValue);
        Sender->SetWindowText(sWindowText);
        if (Sender->IsKindOf(RUNTIME_CLASS(CStatic)))
        {
            CRect rect;
            Sender->GetWindowRect(&rect);
            ScreenToClient(&rect);
            InvalidateRect(&rect);
            UpdateWindow();
        }
    }
}
//---------------------------------------------------------------------------
void CD2MainForm::SetText(CWnd* Sender, const wchar_t* newValue)
{
    if (Sender->IsKindOf(RUNTIME_CLASS(CEdit)) || Sender->IsKindOf(RUNTIME_CLASS(CStatic)))
    {
        CString sWindowText(newValue);
        Sender->SetWindowText(sWindowText);
        if (Sender->IsKindOf(RUNTIME_CLASS(CStatic)))
        {
            CRect rect;
            Sender->GetWindowRect(&rect);
            ScreenToClient(&rect);
            InvalidateRect(&rect);
            UpdateWindow();
        }
    }
}
//---------------------------------------------------------------------------
std::uint32_t CD2MainForm::ToInt(const CWnd* Sender) const
{
    if (Sender->IsKindOf(RUNTIME_CLASS(CEdit)) || Sender->IsKindOf(RUNTIME_CLASS(CStatic)))
    {
        std::string sWindowText(ToStdString(Sender));
        char* ptr = nullptr;
        std::uint32_t value = std::strtoul(sWindowText.c_str(), &ptr, 10);
        switch (Sender->GetDlgCtrlID())
        {
            // shift right 8 bits for actual value
            // Only update these values if the value had changed as
            // you may loose the lower 8 bits from the original value
        case IDC_CUR_LIFE:
            if (value == (Cs.CurLife >> 8))
            {
                return Cs.CurLife;
            }
            value <<= 8;
            break;
        case IDC_MAX_LIFE:
            if (value == (Cs.MaxLife >> 8))
            {
                return Cs.MaxLife;
            }
            value <<= 8;
            break;
        case IDC_CUR_MANA:
            if (value == (Cs.CurMana >> 8))
            {
                return Cs.CurMana;
            }
            value <<= 8;
            break;
        case IDC_MAX_MANA:
            if (value == (Cs.MaxMana >> 8))
            {
                return Cs.MaxMana;
            }
            value <<= 8;
            break;
        case IDC_CUR_STAMINA:
            if (value == (Cs.CurStamina >> 8))
            {
                return Cs.CurStamina;
            }
            value <<= 8;
            break;
        case IDC_MAX_STAMINA:
            if (value == (Cs.MaxStamina >> 8))
            {
                return Cs.MaxStamina;
            }
            value <<= 8;
            break;
        }

        return value;
    }

    if (Sender->IsKindOf(RUNTIME_CLASS(CComboBox)))
    {
        return ((CComboBox*)Sender)->GetCurSel();
    }

    return 0;
}
//---------------------------------------------------------------------------
void CD2MainForm::SetInt(CWnd* Sender, std::uint32_t newValue)
{
    if (Sender->IsKindOf(RUNTIME_CLASS(CEdit)) || Sender->IsKindOf(RUNTIME_CLASS(CStatic)))
    {
        switch (Sender->GetDlgCtrlID())
        {
        case IDC_CUR_LIFE:
        case IDC_MAX_LIFE:
        case IDC_CUR_MANA:
        case IDC_MAX_MANA:
        case IDC_CUR_STAMINA:
        case IDC_MAX_STAMINA:
            newValue >>= 8; // shift right 8 bits for actual value
            break;
        case IDC_CHAR_LEVEL:
            if (newValue + 1 <= d2ce::NUM_OF_LEVELS)
            {
                SetInt(&NextExperience, d2ce::MinExpRequired[newValue]);
            }
            else
            {
                SetText(&NextExperience, _T("NONE"));
            }
            break;
        }

        CString sWindowText;
        sWindowText.Format(_T("%lu"), newValue);
        Sender->SetWindowText(sWindowText);
        if (Sender->IsKindOf(RUNTIME_CLASS(CStatic)))
        {
            CRect rect;
            Sender->GetWindowRect(&rect);
            ScreenToClient(&rect);
            InvalidateRect(&rect);
            UpdateWindow();
        }
        return;
    }

    if (Sender->IsKindOf(RUNTIME_CLASS(CComboBox)))
    {
        ((CComboBox*)Sender)->SetCurSel(newValue);
        return;
    }
}
//---------------------------------------------------------------------------
void CD2MainForm::CheckToolTipCtrl()
{
    auto pToolTip = AfxGetModuleThreadState()->m_pToolTip;
    if (pToolTip != NULL && (pToolTip->GetOwner() != this || DYNAMIC_DOWNCAST(CMFCToolTipCtrl, pToolTip) == NULL))
    {
        pToolTip->DestroyWindow();
        delete pToolTip;
        AfxGetModuleThreadState()->m_pToolTip = NULL;
        pToolTip = NULL;
    }

    if (pToolTip == NULL)
    {
        CMFCToolTipInfo ttParams;
        ttParams.m_bVislManagerTheme = TRUE;
        pToolTip = new CMFCToolTipCtrl(&ttParams);
        if (pToolTip->Create(this, TTS_ALWAYSTIP))
        {
            pToolTip->SendMessage(TTM_ACTIVATE, FALSE);
            AfxGetModuleThreadState()->m_pToolTip = pToolTip;
        }
    }
}
//---------------------------------------------------------------------------
void CD2MainForm::OnOptionsBackupChar()
{
    BackupChar = !BackupChar;
    AfxGetApp()->WriteProfileInt(SettingsSection, BackupCharacterOption, BackupChar ? 1 : 0);
}

void CD2MainForm::OnUpdateOptionsBackupChar(CCmdUI* pCmdUI)
{
    pCmdUI->SetCheck(BackupChar ? TRUE : FALSE);
}
//---------------------------------------------------------------------------
void CD2MainForm::OnOptionsRestoreChar()
{
    if (!hasBackupFile)
    {
        return;
    }

    hasBackupFile = false;
    CStringA origNameA(ExtractFileName(RemoveFileExtFromPath(curPathName)));
    CStringA oldPathNameA(curPathName);
    CStringA backupname(ChangeFileExt(curPathName, _T(".bak")));

    Editted = false;
    DoFileCloseAction();

    // rename temp file to character file
    if (std::remove((LPCSTR)oldPathNameA))
    {
        CString msg;
        msg.Format(_T("Failed to delete existing character file: %s"), curPathName.GetString());
        AfxMessageBox(msg, MB_OK | MB_ICONERROR);

        // just reopen it again
    }
    else if (std::rename((LPCSTR)backupname, (LPCSTR)oldPathNameA))
    {
        CStringA msg;
        msg.Format("Failed to rename backup character file: %s", backupname.GetString());
        AfxMessageBox(CString(msg), MB_OK | MB_ICONERROR);
        return;
    }

    // return if open not successful
    if (!CharInfo.open(oldPathNameA))
    {
        CString errorMsg(CharInfo.getLastError().message().c_str());
        if (errorMsg.IsEmpty())
        {
            errorMsg = _T("Not a valid Diablo II save file.");
        }
        AfxMessageBox(errorMsg, MB_OK | MB_ICONERROR);

        Editted = false;
        OnFileClose();
        return;
    }

    Initialize();
    EnableCharInfoBox(TRUE);

    CharInfo.fillBasicStats(Bs);
    CharInfo.fillCharacterStats(Cs);
    DisplayCharInfo();
    ctrlEditted.clear();
    CString msg(_T("Character stats have been restored"));
    StatusBar.SetWindowText(msg);
    UpdateAppTitle();
    AfxMessageBox(msg, MB_OK | MB_ICONINFORMATION);
}

void CD2MainForm::OnUpdateOptionsRestoreChar(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(CharInfo.is_open() && hasBackupFile ? TRUE : FALSE);
}
//---------------------------------------------------------------------------
void CD2MainForm::OnViewLevelReq()
{
    s_levelInfo.Show(this);
}
//---------------------------------------------------------------------------
void CD2MainForm::OnOptionsGpsConvertor()
{
    CD2GemsForm dlg(*this);
    dlg.DoModal();
}

void CD2MainForm::OnUpdateOptionsGpsConvertor(CCmdUI* pCmdUI)
{
    pCmdUI->Enable((CharInfo.is_open() && (CharInfo.getNumberOfGPSs() != 0)) ? TRUE : FALSE);
}
//---------------------------------------------------------------------------
void CD2MainForm::OnOptionsMaxfillstackables()
{
    auto numConverted = CharInfo.fillAllStackables();
    CString msg;
    msg.Format(_T("%zd stackable item(s) have been fully filled"), numConverted);
    if (numConverted > 0)
    {
        StatusBar.SetWindowText(msg);

        ItemsChanged = true;
        StatsChanged();
    }

    AfxMessageBox(msg, MB_ICONINFORMATION | MB_OK);
}

void CD2MainForm::OnUpdateOptionsMaxfillstackables(CCmdUI* pCmdUI)
{
    pCmdUI->Enable((CharInfo.is_open() && (CharInfo.getNumberOfStackables() != 0)) ? TRUE : FALSE);
}
//---------------------------------------------------------------------------
void CD2MainForm::OnOptionsFixallitems()
{
    auto numConverted = CharInfo.fixAllItems();
    CString msg;
    msg.Format(_T("%zd item(s) have been fixed"), numConverted);
    if (numConverted > 0)
    {
        StatusBar.SetWindowText(msg);

        ItemsChanged = true;
        StatsChanged();
    }

    AfxMessageBox(msg, MB_ICONINFORMATION | MB_OK);
}

void CD2MainForm::OnUpdateOptionsFixallitems(CCmdUI* pCmdUI)
{
    pCmdUI->Enable((CharInfo.is_open() && (CharInfo.getNumberOfArmor() != 0 || CharInfo.getNumberOfWeapons() != 0)) ? TRUE : FALSE);
}
//---------------------------------------------------------------------------
void CD2MainForm::OnOptionsMaxdurabilityforallitems()
{
    auto numConverted = CharInfo.maxDurabilityAllItems();
    CString msg;
    msg.Format(_T("%zd item(s) have been given the highest durability value"), numConverted);
    if (numConverted > 0)
    {
        StatusBar.SetWindowText(msg);

        ItemsChanged = true;
        StatsChanged();
    }

    AfxMessageBox(msg, MB_ICONINFORMATION | MB_OK);
}

void CD2MainForm::OnUpdateOptionsMaxdurabilityforallitems(CCmdUI* pCmdUI)
{
    pCmdUI->Enable((CharInfo.is_open() && (CharInfo.getNumberOfArmor() != 0 || CharInfo.getNumberOfWeapons() != 0)) ? TRUE : FALSE);
}
//---------------------------------------------------------------------------
void CD2MainForm::OnOptionsResetStats()
{
    CharInfo.resetStats();
    UpdateCharInfo();
}
//---------------------------------------------------------------------------
void CD2MainForm::OnUpdateOptionsResetStats(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(CharInfo.is_open() ? TRUE : FALSE);
}
//---------------------------------------------------------------------------
void CD2MainForm::OnViewMercenary()
{
    CD2MercenaryForm dlg(*this);
    dlg.DoModal();
    if (dlg.MercChanged())
    {
        MercChanged = true;
        StatsChanged();
    }
}
//---------------------------------------------------------------------------
void CD2MainForm::OnUpdateViewMercenary(CCmdUI* pCmdUI)
{
    pCmdUI->Enable((CharInfo.is_open() && CharInfo.getVersion() >= d2ce::EnumCharVersion::v109) ? TRUE : FALSE);
}
//---------------------------------------------------------------------------
void CD2MainForm::OnViewItems()
{
    CD2ItemsForm dlg(*this);
    dlg.DoModal();
}
//---------------------------------------------------------------------------
void CD2MainForm::OnUpdateViewItems(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(CharInfo.is_open() ? TRUE : FALSE);
}
//---------------------------------------------------------------------------
d2ce::Character& CD2MainForm::getCharacterInfo()
{
    return CharInfo;
}
//---------------------------------------------------------------------------
d2ce::EnumCharVersion CD2MainForm::getCharacterVersion() const
{
    return CharInfo.getVersion();
}
//---------------------------------------------------------------------------
bitmask::bitmask<d2ce::EnumCharStatus> CD2MainForm::getCharacterStatus() const
{
    auto status = CharInfo.getStatus();
    if (CharStatusResurrected.GetCheck() == 1)
    {
        status |= d2ce::EnumCharStatus::Resurrected;
    }
    else
    {
        status &= ~d2ce::EnumCharStatus::Resurrected;
    }

    if (CharStatusHardcore.GetCheck() == 1)
    {
        status |= d2ce::EnumCharStatus::Hardcore;
        status &= ~d2ce::EnumCharStatus::Resurrected; // can't be resurrected
    }
    else
    {
        status &= ~d2ce::EnumCharStatus::Hardcore;
    }

    if (CharStatusExpansion.GetCheck() == 1)
    {
        status |= d2ce::EnumCharStatus::Expansion;
    }
    else
    {
        status &= ~d2ce::EnumCharStatus::Expansion;
    }

    if (CharStatusLadder.GetCheck() == 1)
    {
        status |= d2ce::EnumCharStatus::Ladder;
    }
    else
    {
        status &= ~d2ce::EnumCharStatus::Ladder;
    }

    return status;
}
//---------------------------------------------------------------------------
bitmask::bitmask<d2ce::EnumCharTitle> CD2MainForm::getCharacterTitle() const
{
    bitmask::bitmask<d2ce::EnumCharTitle> title = d2ce::EnumCharTitle::None;
    switch (CharTitle.GetCurSel())
    {
    case 1:
        title = d2ce::EnumCharTitle::SirDame;
        if (isExpansionCharacter())
        {
            title |= d2ce::EnumCharTitle::Slayer;
        }
        break;

    case 2:
        title = d2ce::EnumCharTitle::LordLady;
        if (isExpansionCharacter())
        {
            title |= d2ce::EnumCharTitle::Champion;
        }
        break;

    case 3:
        title = d2ce::EnumCharTitle::BaronBaroness;
        if (isExpansionCharacter())
        {
            title |= d2ce::EnumCharTitle::MPatriarch;
        }
        break;
    }

    return title;
}
//---------------------------------------------------------------------------
d2ce::EnumCharClass CD2MainForm::getCharacterClass() const
{
    return static_cast<d2ce::EnumCharClass>(CharClass.GetCurSel());
}
//---------------------------------------------------------------------------
d2ce::EnumDifficulty CD2MainForm::getDifficultyLastPlayed() const
{
    return static_cast<d2ce::EnumDifficulty>(Difficulty.GetCurSel());
}
//---------------------------------------------------------------------------
std::uint32_t CD2MainForm::getCharacterLevel() const
{
    return CharInfo.getLevel();
}
//---------------------------------------------------------------------------
std::uint32_t CD2MainForm::getWeaponSet() const
{
    return CharInfo.getWeaponSet();
}
//---------------------------------------------------------------------------
bool CD2MainForm::isExpansionCharacter() const
{
    return CharInfo.isExpansionCharacter();
}
//---------------------------------------------------------------------------
std::uint32_t CD2MainForm::getSkillPointsEarned() const
{
    std::uint32_t curLevel = std::min(ToInt(&CharLevel), d2ce::NUM_OF_LEVELS);
    return CharInfo.getSkillPointsEarned(curLevel);
}
//---------------------------------------------------------------------------
std::uint32_t CD2MainForm::getStatPointsEarned() const
{
    std::uint32_t curLevel = std::min(ToInt(&CharLevel), d2ce::NUM_OF_LEVELS);
    return CharInfo.getStatPointsEarned(curLevel);
}
//---------------------------------------------------------------------------
d2ce::Mercenary& CD2MainForm::getMercenaryInfo()
{
    return CharInfo.getMercenaryInfo();
}
//---------------------------------------------------------------------------
const std::list<d2ce::Item>& CD2MainForm::getMercItems() const
{
    return CharInfo.getMercItems();
}
//---------------------------------------------------------------------------
const std::list<d2ce::Item>& CD2MainForm::getCorpseItems() const
{
    return CharInfo.getCorpseItems();
}
//---------------------------------------------------------------------------
bool CD2MainForm::hasGolem() const
{
    return CharInfo.hasGolem();
}
//---------------------------------------------------------------------------
const d2ce::Item& CD2MainForm::getGolemItem() const
{
    return CharInfo.getGolemItem();
}
//---------------------------------------------------------------------------
const d2ce::ActsInfo& CD2MainForm::getQuests()
{
    return CharInfo.getQuests();
}
//---------------------------------------------------------------------------
void CD2MainForm::updateQuests(const d2ce::ActsInfo& qi)
{
    CharInfo.updateQuests(qi);
    QuestsChanged = true;
    StatsChanged();
}
//---------------------------------------------------------------------------
std::uint64_t CD2MainForm::getWaypoints(d2ce::EnumDifficulty difficulty) const
{
    return CharInfo.getWaypoints(difficulty);
}
//---------------------------------------------------------------------------
void CD2MainForm::setWaypoints(d2ce::EnumDifficulty difficulty, std::uint64_t newvalue)
{
    CharInfo.setWaypoints(difficulty, newvalue);
    WaypointsChanged = true;
    StatsChanged();
}
//---------------------------------------------------------------------------
std::uint8_t(&CD2MainForm::getSkills())[d2ce::NUM_OF_SKILLS]
{
    return CharInfo.getSkills();
}
//---------------------------------------------------------------------------
void CD2MainForm::updateSkills(const std::uint8_t(&updated_skills)[d2ce::NUM_OF_SKILLS])
{
    CharInfo.updateSkills(updated_skills);
    SkillChoicesChanged = true;
    UpdateCharInfo();
    StatsChanged();
}
//---------------------------------------------------------------------------
std::uint32_t CD2MainForm::getSkillPointsUsed() const
{
    return CharInfo.getSkillPointsUsed();
}
//---------------------------------------------------------------------------
std::uint32_t CD2MainForm::getSkillChoices() const
{
    return CharInfo.getSkillChoices();
}
//---------------------------------------------------------------------------
const std::vector<std::reference_wrapper<d2ce::Item>>& CD2MainForm::getGPSs()
{
    return CharInfo.getGPSs();
}
//---------------------------------------------------------------------------
/*
   Converts the specified original gems, potions or skulls to the specified
   final gem, potion or skull.
   Returns the number of gems converted.
*/
size_t CD2MainForm::convertGPSs(const std::uint8_t(&existingGem)[4], const std::uint8_t(&desiredGem)[4])
{
    auto numConverted = CharInfo.convertGPSs(existingGem, desiredGem);
    if (numConverted > 0)
    {
        hasUpgradableRejuvenations = CharInfo.anyUpgradableRejuvenations();
        if (!hasUpgradableRejuvenations)
        {
            // implies no potions or all Full Rejuvenation potions so nothing to upgrade
            hasUpgradablePotions = false;
        }
        else
        {
            hasUpgradablePotions = CharInfo.anyUpgradablePotions();
        }

        hasUpgradableGems = CharInfo.anyUpgradableGems();

        ItemsChanged = true;
        StatsChanged();
    }

    return numConverted;
}
//---------------------------------------------------------------------------
bool CD2MainForm::updateGem(d2ce::Item& item, const std::uint8_t(&newgem)[4])
{
    if (!item.updateGem(newgem))
    {
        return false;
    }

    hasUpgradableRejuvenations = CharInfo.anyUpgradableRejuvenations();
    if (!hasUpgradableRejuvenations)
    {
        // implies no potions or all Full Rejuvenation potions so nothing to upgrade
        hasUpgradablePotions = false;
    }
    else
    {
        hasUpgradablePotions = CharInfo.anyUpgradablePotions();
    }

    hasUpgradableGems = CharInfo.anyUpgradableGems();

    ItemsChanged = true;
    StatsChanged();
    return true;
}
//---------------------------------------------------------------------------
bool CD2MainForm::upgradeGem(d2ce::Item& item)
{
    if (!item.upgradeGem())
    {
        return false;
    }

    hasUpgradableGems = CharInfo.anyUpgradableGems();

    ItemsChanged = true;
    StatsChanged();
    return true;
}
//---------------------------------------------------------------------------
bool CD2MainForm::upgradePotion(d2ce::Item& item)
{
    if (!item.upgradePotion())
    {
        return false;
    }

    hasUpgradableRejuvenations = CharInfo.anyUpgradableRejuvenations();
    if (!hasUpgradableRejuvenations)
    {
        // implies no potions or all Full Rejuvenation potions so nothing to upgrade
        hasUpgradablePotions = false;
    }
    else
    {
        hasUpgradablePotions = CharInfo.anyUpgradablePotions();
    }

    ItemsChanged = true;
    StatsChanged();
    return true;
}
//---------------------------------------------------------------------------
bool CD2MainForm::upgradeToFullRejuvenationPotion(d2ce::Item& item)
{
    if (!item.upgradeToFullRejuvenationPotion())
    {
        return false;
    }

    hasUpgradableRejuvenations = CharInfo.anyUpgradableRejuvenations();
    if (!hasUpgradableRejuvenations)
    {
        // implies no potions or all Full Rejuvenation potions so nothing to upgrade
        hasUpgradablePotions = false;
    }
    else
    {
        hasUpgradablePotions = CharInfo.anyUpgradablePotions();
    }

    ItemsChanged = true;
    StatsChanged();
    return true;
}
//---------------------------------------------------------------------------
bool CD2MainForm::getItemBitmap(const d2ce::Item& item, CBitmap& bitmap) const
{
    CStringA invFile(item.getInvFile().c_str());
    if (invFile.IsEmpty())
    {
        return false;
    }

    static std::string resourceHeader;
    if (resourceHeader.empty())
    {
        HINSTANCE hInstance = ::GetModuleHandle(NULL);
        HRSRC hres = ::FindResource(hInstance, MAKEINTRESOURCE(IDR_RESOURCE_HEADER), _T("TEXT"));
        if (hres == NULL)
        {
            return false;
        }

        DWORD dwSizeBytes = ::SizeofResource(hInstance, hres);
        if (dwSizeBytes == 0)
        {
            return false;
        }

        HGLOBAL hglobal = ::LoadResource(hInstance, hres);
        if (hglobal == NULL)
        {
            return false;
        }

        BYTE* pRes = (BYTE*)LockResource(hglobal);
        if (pRes == nullptr)
        {
            ::FreeResource(hglobal);
            return false;
        }

        resourceHeader.assign((char*)pRes, dwSizeBytes);
        ::FreeResource(hglobal);

        if (resourceHeader.empty())
        {
            return false;
        }
    }

    std::stringstream ss;
    ss << "^[\\s]*#define\\s+IDB_";
    ss << invFile.MakeUpper().GetString();
    ss << "\\s+([0-9]+)\\s*$";
    std::regex regexDefine(ss.str(), std::regex::ECMAScript | std::regex::icase);
    std::cmatch m;
    auto pStr = resourceHeader.c_str();
    if (std::regex_search(pStr, m, regexDefine))
    {
        bitmap.Attach(::LoadBitmap(AfxGetResourceHandle(), MAKEINTRESOURCE(atoi(m[1].str().c_str()))));
        return bitmap.GetSafeHandle() == NULL ? false : true;
    }

    return false;
}
//---------------------------------------------------------------------------
bool CD2MainForm::repairItem(d2ce::Item& item)
{
    bool ret = item.fixDurability();
    if (ret)
    {
        ItemsChanged = true;
        StatsChanged();
    }

    return ret;
}
//---------------------------------------------------------------------------
bool CD2MainForm::setItemMaxQuantity(d2ce::Item& item)
{
    bool ret = item.setMaxQuantity();
    if (ret)
    {
        ItemsChanged = true;
        StatsChanged();
    }

    return ret;
}
//---------------------------------------------------------------------------
bool CD2MainForm::setItemMaxDurability(d2ce::Item& item)
{
    bool ret = item.setMaxDurability();
    if (ret)
    {
        ItemsChanged = true;
        StatsChanged();
    }

    return ret;
}
//---------------------------------------------------------------------------
size_t CD2MainForm::getNumberOfEquippedItems() const
{
    return CharInfo.getNumberOfEquippedItems();
}
//---------------------------------------------------------------------------
const std::vector<std::reference_wrapper<d2ce::Item>>& CD2MainForm::getEquippedItems() const
{
    return CharInfo.getEquippedItems();
}
//---------------------------------------------------------------------------
bool CD2MainForm::getHasBeltEquipped() const
{
    return CharInfo.getHasBeltEquipped();
}
//---------------------------------------------------------------------------
size_t CD2MainForm::getMaxNumberOfItemsInBelt() const
{
    return CharInfo.getMaxNumberOfItemsInBelt();
}
//---------------------------------------------------------------------------
size_t CD2MainForm::getNumberOfItemsInBelt() const
{
    return CharInfo.getNumberOfItemsInBelt();
}
//---------------------------------------------------------------------------
const std::vector<std::reference_wrapper<d2ce::Item>>& CD2MainForm::getItemsInBelt() const
{
    return CharInfo.getItemsInBelt();
}
//---------------------------------------------------------------------------
size_t CD2MainForm::getNumberOfItemsInInventory() const
{
    return CharInfo.getNumberOfItemsInInventory();
}
//---------------------------------------------------------------------------
const std::vector<std::reference_wrapper<d2ce::Item>>& CD2MainForm::getItemsInInventory() const
{
    return CharInfo.getItemsInInventory();
}
//---------------------------------------------------------------------------
size_t CD2MainForm::getNumberOfItemsInStash() const
{
    return CharInfo.getNumberOfItemsInStash();
}
//---------------------------------------------------------------------------
const std::vector<std::reference_wrapper<d2ce::Item>>& CD2MainForm::getItemsInStash() const
{
    return CharInfo.getItemsInStash();
}
//---------------------------------------------------------------------------
bool CD2MainForm::getHasHoradricCube() const
{
    return CharInfo.getHasHoradricCube();
}
//---------------------------------------------------------------------------
size_t CD2MainForm::getNumberOfItemsInHoradricCube() const
{
    return CharInfo.getNumberOfItemsInHoradricCube();
}
//---------------------------------------------------------------------------
const std::vector<std::reference_wrapper<d2ce::Item>>& CD2MainForm::getItemsInHoradricCube() const
{
    return CharInfo.getItemsInHoradricCube();
}
//---------------------------------------------------------------------------

