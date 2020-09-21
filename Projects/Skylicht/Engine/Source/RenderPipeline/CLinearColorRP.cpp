/*
!@
MIT License

Copyright (c) 2020 Skylicht Technology CO., LTD

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files
(the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify,
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

#include "pch.h"
#include "CLinearColorRP.h"
#include "Material/Shader/CShaderManager.h"

namespace Skylicht
{
	CLinearColorRP::CLinearColorRP()
	{

	}

	CLinearColorRP::~CLinearColorRP()
	{
		IVideoDriver *driver = getVideoDriver();
	}

	void CLinearColorRP::initRender(int w, int h)
	{
		IVideoDriver *driver = getVideoDriver();
		CShaderManager *shaderMgr = CShaderManager::getInstance();

		// init size of framebuffer
		m_size = core::dimension2du((u32)w, (u32)h);

		// init final pass shader
		m_finalPass.MaterialType = shaderMgr->getShaderIDByName("TextureLinearRGB");
	}

	void CLinearColorRP::render(ITexture *target, CCamera *camera, CEntityManager *entityManager, const core::recti& viewport)
	{
		if (camera == NULL)
			return;

		onNext(target, camera, entityManager, viewport);
	}

	void CLinearColorRP::postProcessing(ITexture *finalTarget, ITexture *color, ITexture *normal, ITexture *position, const core::recti& viewport)
	{
		IVideoDriver *driver = getVideoDriver();

		driver->setRenderTarget(finalTarget, false, false);

		float renderW = (float)m_size.Width;
		float renderH = (float)m_size.Height;

		if (viewport.getWidth() > 0 && viewport.getHeight() > 0)
		{
			driver->setViewPort(viewport);
			renderW = (float)viewport.getWidth();
			renderH = (float)viewport.getHeight();
		}

		m_finalPass.setTexture(0, color);

		beginRender2D(renderW, renderH);
		renderBufferToTarget(0.0f, 0.0f, renderW, renderH, m_finalPass);
	}
}