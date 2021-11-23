/*
!@
MIT License

Copyright (c) 2020 Skylicht Technology CO., LTD

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files
(the "Software"), to deal in the Software without restriction, including without limitation the Rights to use, copy, modify,
merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

This file is part of the "Skylicht Engine".
https://github.com/skylicht-lab/skylicht-engine
!#
*/

#pragma once

#include "SkylichtEngine.h"
#include "Editor/Space/CSpace.h"

#include "Thread/IThread.h"
#include "Thread/IMutex.h"
#include "DownloadMap.h"

#ifdef HAVE_SKYLICHT_NETWORK
#include "HttpRequest/CHttpRequest.h"
#endif

#define NUM_HTTPREQUEST	8

namespace Skylicht
{
	namespace Editor
	{
		struct SRenderMap
		{
			core::vector2d<long> From;
			core::vector2d<long> To;
			long CountX;
			long CountY;
		};

		class CSpaceGMap : public CSpace, IThreadCallback
		{
		protected:
			GUI::CBase* m_view;

			int m_zoom;
			long m_viewX;
			long m_viewY;

			SRenderMap m_renderMap;
			int m_gridSize;

			int m_materialID;

			CGlyphFont* m_fontNormal;
			CGlyphFont* m_fontLarge;

			bool m_rightPress;

			IThread* m_downloadMapThread;
			IMutex* m_lock;
			IMutex* m_lockFile;

			std::list<SImageDownload> m_queueDownload;
			std::list<SImageDownload> m_downloading;

			SImageDownload* m_imgDownloading[NUM_HTTPREQUEST];
			CHttpRequest* m_httpRequest[NUM_HTTPREQUEST];
			CHttpStream* m_httpStream[NUM_HTTPREQUEST];

			EImageMapType m_mapBGType;

			std::vector<SImageMapElement> m_mapOverlay;

		public:
			CSpaceGMap(GUI::CWindow* window, CEditor* editor);

			virtual ~CSpaceGMap();

			void clear();

			virtual void updateThread();

			virtual void onResize(float w, float h);

			virtual void onRender(GUI::CBase* base);

			virtual void onMouseMoved(GUI::CBase* base, float x, float y, float deltaX, float deltaY);

			virtual void onLeftMouseClick(GUI::CBase* base, float x, float y, bool down);

			virtual void onRightMouseClick(GUI::CBase* base, float x, float y, bool down);

			virtual void onMiddleMouseClick(GUI::CBase* base, float x, float y, bool down);

			virtual void onMouseWheeled(GUI::CBase* base, int wheel);

		protected:

			void requestDownloadMap(long x, long y, int z);

			std::string getMapLocalPath(EImageMapType type, long x, long y, int z);

			ITexture* searchMapTileset(long x, long y, int z);

			ITexture* searchMapTilesetOnLocal(long x, long y, int z);

			void updateMap();

			void renderMap();

			void renderMapBG();

			void renderGrid();

			void renderString();

			void setZoom(int z);

			void zoomIn();

			void zoomIn(long viewX, long viewY);

			void zoomOut();

			void zoomOut(long viewX, long viewY);
		};
	}
}