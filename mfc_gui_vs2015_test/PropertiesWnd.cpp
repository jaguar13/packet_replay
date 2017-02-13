
#include "stdafx.h"

#include "PropertiesWnd.h"
#include "Resource.h"
#include "MainFrm.h"
#include "mfc_gui.h"

#include "traffic_replay.hpp"

struct pcap_devs_action_getif_t
{
	pcap_devs_action_getif_t(std::string& ip):m_ip(ip){}

	bool do_action(pcap_if_t* ifdev = 0)
	{
		if(ifdev == 0)
			return false;

		if(ifdev->addresses != 0)
		{
			for (pcap_addr* address = ifdev->addresses; address != 0; address = address->next)
			{
				if(address->addr == 0)
					continue;

				if(address->addr->sa_data == 0)
					continue;

				//ip
				if(address->addr->sa_family == AF_INET)
				{
					std::string ip(inet_ntoa(((struct sockaddr_in*)address->addr)->sin_addr));

					if(ip == m_ip)
						m_id = ifdev->name;

					continue;
				}					
			}
		}

		return false;					
	}

	std::string m_id;
	std::string m_ip;
};

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CResourceViewBar

CPropertiesWnd::CPropertiesWnd()
{
	m_SettingFile = "packet_replay_settings.txt";
	pProgramSettings = new SCL::settingsTV;	

	SCL::setting_initializer pgmInitialSettings[] =
	{
		{ "","src_ip"   , ST_STR, "0.0.0.0" , "" },
		{ "","dst_ip"   , ST_STR ,"0.0.0.0" , "" },
		{ "","cpu"      , ST_STR, "40"      , "" },
		{ "","dump_log" , ST_STR, "False"   , "" },
		{ "","frag_off" , ST_STR, "True"    , "" }
	};

	uint32_t numSettings =
		sizeof(pgmInitialSettings) / sizeof(SCL::setting_initializer);

	uint32_t rc = pProgramSettings->init(numSettings, pgmInitialSettings);
}

CPropertiesWnd::~CPropertiesWnd()
{
	if (pProgramSettings != NULL)
		delete pProgramSettings;

	pProgramSettings = NULL;
}

BEGIN_MESSAGE_MAP(CPropertiesWnd, CDockablePane)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_COMMAND(ID_EXPAND_ALL, OnExpandAllProperties)
	ON_UPDATE_COMMAND_UI(ID_EXPAND_ALL, OnUpdateExpandAllProperties)
	ON_COMMAND(ID_SORTPROPERTIES, OnSortProperties)
	ON_UPDATE_COMMAND_UI(ID_SORTPROPERTIES, OnUpdateSortProperties)
	ON_COMMAND(ID_PROPERTIES1, OnProperties1)
	ON_UPDATE_COMMAND_UI(ID_PROPERTIES1, OnUpdateProperties1)
	ON_COMMAND(ID_PROPERTIES2, OnProperties2)
	ON_UPDATE_COMMAND_UI(ID_PROPERTIES2, OnUpdateProperties2)
	ON_WM_SETFOCUS()
	ON_WM_SETTINGCHANGE()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CResourceViewBar message handlers

void CPropertiesWnd::AdjustLayout()
{
	if (GetSafeHwnd() == NULL)
	{
		return;
	}

	CRect rectClient,rectCombo;
	GetClientRect(rectClient);

	m_wndObjectCombo.GetWindowRect(&rectCombo);

	int cyCmb = rectCombo.Size().cy;
	int cyTlb = m_wndToolBar.CalcFixedLayout(FALSE, TRUE).cy;

	m_wndObjectCombo.SetWindowPos(NULL, rectClient.left, rectClient.top, rectClient.Width(), 200, SWP_NOACTIVATE | SWP_NOZORDER);
	m_wndToolBar.SetWindowPos(NULL, rectClient.left, rectClient.top + cyCmb, rectClient.Width(), cyTlb, SWP_NOACTIVATE | SWP_NOZORDER);
	m_wndPropList.SetWindowPos(NULL, rectClient.left, rectClient.top + cyCmb + cyTlb, rectClient.Width(), rectClient.Height() -(cyCmb+cyTlb), SWP_NOACTIVATE | SWP_NOZORDER);
}

int CPropertiesWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDockablePane::OnCreate(lpCreateStruct) == -1)
		return -1;

	CRect rectDummy;
	rectDummy.SetRectEmpty();

	// Create combo:
	const DWORD dwViewStyle = WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_BORDER | CBS_SORT | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;

	if (!m_wndObjectCombo.Create(dwViewStyle, rectDummy, this, 1))
	{
		TRACE0("Failed to create Properties Combo \n");
		return -1;      // fail to create
	}

	m_wndObjectCombo.AddString(_T("Application"));
	m_wndObjectCombo.AddString(_T("Properties Window"));
	m_wndObjectCombo.SetFont(CFont::FromHandle((HFONT) GetStockObject(DEFAULT_GUI_FONT)));
	m_wndObjectCombo.SetCurSel(0);

	if (!m_wndPropList.Create(WS_VISIBLE | WS_CHILD, rectDummy, this, 2))
	{
		TRACE0("Failed to create Properties Grid \n");
		return -1;      // fail to create
	}

	InitPropList();

	m_wndToolBar.Create(this, AFX_DEFAULT_TOOLBAR_STYLE, IDR_PROPERTIES);
	m_wndToolBar.LoadToolBar(IDR_PROPERTIES, 0, 0, TRUE /* Is locked */);
	m_wndToolBar.CleanUpLockedImages();
	m_wndToolBar.LoadBitmap(theApp.m_bHiColorIcons ? IDB_PROPERTIES_HC : IDR_PROPERTIES, 0, 0, TRUE /* Locked */);

	m_wndToolBar.SetPaneStyle(m_wndToolBar.GetPaneStyle() | CBRS_TOOLTIPS | CBRS_FLYBY);
	m_wndToolBar.SetPaneStyle(m_wndToolBar.GetPaneStyle() & ~(CBRS_GRIPPER | CBRS_SIZE_DYNAMIC | CBRS_BORDER_TOP | CBRS_BORDER_BOTTOM | CBRS_BORDER_LEFT | CBRS_BORDER_RIGHT));
	m_wndToolBar.SetOwner(this);

	// All commands will be routed via this control , not via the parent frame:
	m_wndToolBar.SetRouteCommandsViaFrame(FALSE);

	AdjustLayout();
	return 0;
}

void CPropertiesWnd::OnSize(UINT nType, int cx, int cy)
{
	CDockablePane::OnSize(nType, cx, cy);
	AdjustLayout();
}

void CPropertiesWnd::OnExpandAllProperties()
{
	m_wndPropList.ExpandAll();
}

void CPropertiesWnd::OnUpdateExpandAllProperties(CCmdUI* pCmdUI)
{
}

void CPropertiesWnd::OnSortProperties()
{
	m_wndPropList.SetAlphabeticMode(!m_wndPropList.IsAlphabeticMode());
}

void CPropertiesWnd::OnUpdateSortProperties(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(m_wndPropList.IsAlphabeticMode());
}

void CPropertiesWnd::OnProperties1()
{
	// TODO: Add your command handler code here
}

void CPropertiesWnd::OnUpdateProperties1(CCmdUI* /*pCmdUI*/)
{
	// TODO: Add your command update UI handler code here
}

void CPropertiesWnd::OnProperties2()
{
	// TODO: Add your command handler code here
}

void CPropertiesWnd::OnUpdateProperties2(CCmdUI* /*pCmdUI*/)
{
	// TODO: Add your command update UI handler code here
}

void CPropertiesWnd::InitPropList()
{
	SetPropListFont();

	m_wndPropList.EnableHeaderCtrl(FALSE);
	m_wndPropList.EnableDescriptionArea();
	m_wndPropList.SetVSDotNetLook();
	m_wndPropList.MarkModifiedProperties();

	
	std::string src_ip;
	std::string dst_ip;
	std::string cpu;
	std::string dump_log;	
	std::string frag_off;

	replay::pcap_devs_t devs;
	devs.get_ifs(*(const_cast<CPropertiesWnd*>(this)));

	if (LoadSettings()) 
	{
		pProgramSettings->getStrSetting("", "src_ip", src_ip);
		pProgramSettings->getStrSetting("", "dst_ip", dst_ip);
		pProgramSettings->getStrSetting("", "cpu", cpu);
		pProgramSettings->getStrSetting("", "dump_log", dump_log);
		pProgramSettings->getStrSetting("", "frag_off", frag_off);
	}
	else 
	{
		src_ip = m_if_names[0];
		dst_ip = m_if_names[0];
		cpu = "40";
		dump_log = "False";
		frag_off = "False";
	}

	if(src_ip == "0.0.0.0")
		src_ip = m_if_names[0];

	if (dst_ip == "0.0.0.0")
		dst_ip = m_if_names[0];

	CMFCPropertyGridProperty* pGroup1 = new CMFCPropertyGridProperty(_T("Application Settings"));

	if(m_if_names.size() > 0)
	{
		
		pSourceIf = new CMFCPropertyGridProperty(_T("Source"), CString(src_ip.c_str()), _T("Source interface to replay packets"));
		pSourceIf->AllowEdit(FALSE);

		for(size_t i = 0; i < m_if_names.size(); i++)
			pSourceIf->AddOption(CString(m_if_names[i].c_str()));
		
		pGroup1->AddSubItem(pSourceIf);
		
		pDestinationIf = new CMFCPropertyGridProperty(_T("Destination"), CString(dst_ip.c_str()), _T("Destination interface to replay packets"));
		pDestinationIf->AllowEdit(FALSE);
		
		for(size_t i = 0; i < m_if_names.size(); i++)
			pDestinationIf->AddOption(CString(m_if_names[i].c_str()));

		pGroup1->AddSubItem(pDestinationIf);		
	}	

	pCPUusage = new CMFCPropertyGridProperty(_T("CPU %"), CString(cpu.c_str()), _T("Control the CPU usage while repaying"));
	pCPUusage->AllowEdit(FALSE);

	for (size_t i = 10; i <= 100; i += 10) {
		CString percent;
		percent.Format(_T("%d"), i);
		pCPUusage->AddOption(percent);
	}

	pGroup1->AddSubItem(pCPUusage);

	pDumpLog = new CMFCPropertyGridProperty(_T("Dump Log"), CString(dump_log.c_str()), _T("Generate Log file"));
	pDumpLog->AllowEdit(FALSE);

	pDumpLog->AddOption(CString("True"));
	pDumpLog->AddOption(CString("False"));

	pGroup1->AddSubItem(pDumpLog);

	pDisableFrag = new CMFCPropertyGridProperty(_T("Disable Fragmentation"), CString(frag_off.c_str()), _T("When Fragementation is required then only replay the first fragment."));
	pDisableFrag->AllowEdit(FALSE);

	pDisableFrag->AddOption(CString("True"));
	pDisableFrag->AddOption(CString("False"));

	pGroup1->AddSubItem(pDisableFrag);

	m_wndPropList.AddProperty(pGroup1);
}

void CPropertiesWnd::OnSetFocus(CWnd* pOldWnd)
{
	CDockablePane::OnSetFocus(pOldWnd);
	m_wndPropList.SetFocus();
}

void CPropertiesWnd::OnSettingChange(UINT uFlags, LPCTSTR lpszSection)
{
	CDockablePane::OnSettingChange(uFlags, lpszSection);
	SetPropListFont();
}

void CPropertiesWnd::SetPropListFont()
{
	::DeleteObject(m_fntPropList.Detach());

	LOGFONT lf;
	afxGlobalData.fontRegular.GetLogFont(&lf);

	NONCLIENTMETRICS info;
	info.cbSize = sizeof(info);

	afxGlobalData.GetNonClientMetrics(info);

	lf.lfHeight = info.lfMenuFont.lfHeight;
	lf.lfWeight = info.lfMenuFont.lfWeight;
	lf.lfItalic = info.lfMenuFont.lfItalic;

	m_fntPropList.CreateFontIndirect(&lf);

	m_wndPropList.SetFont(&m_fntPropList);
}

bool CPropertiesWnd::do_action(pcap_if_t* ifdev /*= 0*/)
{
	if(ifdev == 0)
		return false;

	if(ifdev->addresses != 0)
	{
		for (pcap_addr* address = ifdev->addresses; address != 0; address = address->next)
		{
			if(address->addr == 0)
				continue;

			if(address->addr->sa_data == 0)
				continue;

			//ip
			if(address->addr->sa_family == AF_INET)
			{
				m_if_names.push_back(inet_ntoa(((struct sockaddr_in*)address->addr)->sin_addr));
				continue;
			}					
		}
	}

	return false;
}

bool CPropertiesWnd::SaveSettings() 
{
	pProgramSettings->setStrSetting("", "src_ip", GetSourceIP());
	pProgramSettings->setStrSetting("", "dst_ip", GetDestinationIP());
	pProgramSettings->setStrSetting("", "cpu", GetCPU());
	pProgramSettings->setStrSetting("", "dump_log", GetDumpLogEnable());
	pProgramSettings->setStrSetting("", "frag_off", GetFragmentationDisable());

	return pProgramSettings->saveSettingsToFile(m_SettingFile, true) == SCL::SERC_SUCCESS;	 
}

bool CPropertiesWnd::LoadSettings()
{
	bool fileSettingVersionDifferent = false;
	string importErrStr;
	SCL::SettingsImportType importType = SCL::IMPORT_ALL;
	SCL::SettingsImportErrType irc = pProgramSettings->importSettingsFromFile(
		m_SettingFile,
		importType, 
		fileSettingVersionDifferent, 
		importErrStr, 
		NULL);

	return irc == SCL::SIRC_SUCCESS;	
}

std::string CPropertiesWnd::GetSourceIf()
{
	CString val = pSourceIf->GetValue();
	std::string ip = CT2A(val);

	pcap_devs_action_getif_t if_id(ip);
	replay::pcap_devs_t devs;
	devs.get_ifs(if_id);

	return if_id.m_id;
}

std::string CPropertiesWnd::GetSourceIP()
{
	CString val = pSourceIf->GetValue();
	std::string ip = CT2A(val);
	return ip;
}

std::string CPropertiesWnd::GetDestinationIf()
{
	CString val = pDestinationIf->GetValue();
	std::string ip = CT2A(val);

	pcap_devs_action_getif_t if_id(ip);
	replay::pcap_devs_t devs;
	devs.get_ifs(if_id);

	return if_id.m_id;
}

std::string CPropertiesWnd::GetDestinationIP()
{
	CString val = pDestinationIf->GetValue();
	std::string ip = CT2A(val);

	return ip;
}

bool CPropertiesWnd::IsDumpLogEnable()
{
	CString val = pDumpLog->GetValue();
	return val.Compare(L"True") == 0;
}

std::string CPropertiesWnd::GetDumpLogEnable()
{
	CString val = pDumpLog->GetValue();
	std::string log = CT2A(val);
	return log;
}

bool CPropertiesWnd::IsFragmentationDisable()
{
	CString val = pDisableFrag->GetValue();
	return val.Compare(L"True") == 0;
}

std::string CPropertiesWnd::GetFragmentationDisable()
{
	CString val = pDisableFrag->GetValue();
	std::string num = CT2A(val);
	return num;
}

int CPropertiesWnd::CPULimit()
{
	CString val = pCPUusage->GetValue();	
	std::string num = CT2A(val);
	return  atoi(num.c_str());
}

std::string CPropertiesWnd::GetCPU()
{
	CString val = pCPUusage->GetValue();
	std::string num = CT2A(val);
	return num;
}


