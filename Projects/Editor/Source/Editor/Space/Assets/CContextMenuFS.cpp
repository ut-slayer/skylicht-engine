/*
!@
MIT License

CopyRight (c) 2020 Skylicht Technology CO., LTD

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files
(the "Software"), to deal in the Software without restriction, including without limitation the Rights to use, copy, modify,
merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRight HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

This file is part of the "Skylicht Engine".
https://github.com/skylicht-lab/skylicht-engine
!#
*/

#include "pch.h"
#include "CContextMenuFS.h"
#include "Utils/CStringImp.h"
#include "GUI/Input/CInput.h"
#include "GUI/Clipboard/CClipboard.h"

namespace Skylicht
{
	namespace Editor
	{
		CContextMenuFS::CContextMenuFS(GUI::CCanvas* canvas, GUI::CTreeControl* tree, GUI::CListBox* list, CListFSController* listFSController) :
			m_canvas(canvas),
			m_treeFS(tree),
			m_listFS(list),
			m_listFSController(listFSController),
			m_msgBox(NULL)
		{
			m_contextMenu = new GUI::CMenu(canvas);
			m_contextMenu->setHidden(true);
			m_contextMenu->OnCommand = BIND_LISTENER(&CContextMenuFS::OnCommand, this);

			m_open = m_contextMenu->addItem(L"Open");
			m_contextMenu->addItem(L"Show in Explorer");
			m_contextMenu->addSeparator();
			m_contextMenu->addItem(L"Delete", GUI::ESystemIcon::Trash);
			m_contextMenu->addItem(L"Rename", L"F2");
			m_contextMenu->addItem(L"Copy path", GUI::ESystemIcon::Copy, L"SHIFT + C");
			m_contextMenu->addItem(L"Duplicate", GUI::ESystemIcon::Duplicate, L"CTRL + D");

			m_treeFS->addAccelerator("SHIFT + C", BIND_LISTENER(&CContextMenuFS::OnCopyPath, this));
			m_listFS->addAccelerator("SHIFT + C", BIND_LISTENER(&CContextMenuFS::OnCopyPath, this));

			m_treeFS->OnItemContextMenu = BIND_LISTENER(&CContextMenuFS::OnTreeContextMenu, this);
			m_listFS->OnItemContextMenu = BIND_LISTENER(&CContextMenuFS::OnListContextMenu, this);

			m_assetManager = CAssetManager::getInstance();
		}

		CContextMenuFS::~CContextMenuFS()
		{

		}

		void CContextMenuFS::OnTreeContextMenu(GUI::CBase* row)
		{
			GUI::CTreeRowItem* rowItem = dynamic_cast<GUI::CTreeRowItem*>(row);
			if (rowItem != NULL)
			{
				GUI::CTreeNode* node = rowItem->getNode();
				if (node != NULL)
				{
					m_ownerControl = node->getRoot();
					m_selected = node;
					m_selectedPath = node->getTagString();
					m_open->setHidden(true);
					m_contextMenu->open(GUI::CInput::getInput()->getMousePosition());
				}
			}
		}

		void CContextMenuFS::OnListContextMenu(GUI::CBase* row)
		{
			GUI::CListRowItem* rowItem = dynamic_cast<GUI::CListRowItem*>(row);
			if (rowItem != NULL)
			{
				m_ownerControl = rowItem->getListBox();
				m_selected = rowItem;
				m_selectedPath = rowItem->getTagString();
				m_open->setHidden(false);
				m_contextMenu->open(GUI::CInput::getInput()->getMousePosition());
			}
		}

		void CContextMenuFS::OnCommand(GUI::CBase* item)
		{
			GUI::CMenuItem* menuItem = dynamic_cast<GUI::CMenuItem*>(item);
			const std::wstring& label = menuItem->getLabel();
			if (label == L"Open")
			{
				if (m_ownerControl == m_listFS)
					m_listFSController->OnFileOpen(m_selected);
			}
			else if (label == L"Show in Explorer")
			{
#if defined(WIN32)
				char path[512] = { 0 };
				CStringImp::replaceText(path, m_selectedPath.c_str(), "/", "\\");
				ShellExecuteA(NULL, "open", path, NULL, NULL, SW_SHOWDEFAULT);
#elif defined(__APPLE__)
				// QProcess::execute("/usr/bin/osascript", { "-e", "tell application \"Finder\" to reveal POSIX file \"" + path + "\"" });
				// QProcess::execute("/usr/bin/osascript", { "-e", "tell application \"Finder\" to activate" });

				char cmd[1024] = { 0 };
				sprintf(cmd, "osascript -e 'tell app \"Finder\" to open POSIX file \"%s\"'", m_selectedPath.c_str());
				system(cmd);
#endif
			}
			else if (label == L"Delete")
			{
				// m_msgBox = new GUI::CMessageBox(m_canvas);

				if (m_assetManager->deleteAsset(m_selectedPath.c_str()))
					m_listFSController->refresh();
			}
			else if (label == L"Rename")
			{

			}
			else if (label == L"Copy path")
			{
				wchar_t* text = new wchar_t[m_selectedPath.size() + 1];
				CStringImp::convertUTF8ToUnicode(m_selectedPath.c_str(), text);
				GUI::CClipboard::get()->copyTextToClipboard(text);
				delete[]text;
			}
			else if (label == L"Duplicate")
			{

			}
		}

		void CContextMenuFS::OnCopyPath(GUI::CBase* item)
		{
			wchar_t* text = NULL;

			if (m_listFS->isFocussed())
			{
				GUI::CListRowItem* row = m_listFS->getSelected();
				std::string path = row->getTagString();
				if (path.empty() == false)
				{
					text = new wchar_t[path.size() + 1];
					CStringImp::convertUTF8ToUnicode(path.c_str(), text);
				}
			}
			else if (m_treeFS->isFocussed())
			{
				GUI::CTreeNode* node = m_treeFS->getChildSelected();
				std::string path = node->getTagString();
				if (path.empty() == false)
				{
					text = new wchar_t[path.size() + 1];
					CStringImp::convertUTF8ToUnicode(path.c_str(), text);
				}
			}

			if (text != NULL)
			{
				GUI::CClipboard::get()->copyTextToClipboard(text);
				delete[]text;
			}
		}
	}
}