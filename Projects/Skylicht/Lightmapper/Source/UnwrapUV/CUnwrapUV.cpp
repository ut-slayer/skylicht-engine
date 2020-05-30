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
#include "xatlas.h"
#include "CUnwrapUV.h"

namespace Skylicht
{
	namespace Lightmapper
	{
		static void RandomColor(uint8_t *color)
		{
			for (int i = 0; i < 3; i++)
				color[i] = uint8_t((rand() % 255 + 192) * 0.5f);
		}

		static void SetPixel(uint8_t *dest, int destWidth, int x, int y, const uint8_t *color)
		{
			uint8_t *pixel = &dest[x * 3 + y * (destWidth * 3)];
			pixel[0] = color[0];
			pixel[1] = color[1];
			pixel[2] = color[2];
		}

		// https://github.com/miloyip/line/blob/master/line_bresenham.c
		// License: public domain.
		static void RasterizeLine(uint8_t *dest, int destWidth, const int *p1, const int *p2, const uint8_t *color)
		{
			const int dx = abs(p2[0] - p1[0]), sx = p1[0] < p2[0] ? 1 : -1;
			const int dy = abs(p2[1] - p1[1]), sy = p1[1] < p2[1] ? 1 : -1;
			int err = (dx > dy ? dx : -dy) / 2;
			int current[2];
			current[0] = p1[0];
			current[1] = p1[1];
			while (SetPixel(dest, destWidth, current[0], current[1], color), current[0] != p2[0] || current[1] != p2[1])
			{
				const int e2 = err;
				if (e2 > -dx) { err -= dy; current[0] += sx; }
				if (e2 < dy) { err += dx; current[1] += sy; }
			}
		}

		/*
		https://github.com/ssloy/tinyrenderer/wiki/Lesson-2:-Triangle-rasterization-and-back-face-culling
		Copyright Dmitry V. Sokolov
		This software is provided 'as-is', without any express or implied warranty.
		In no event will the authors be held liable for any damages arising from the use of this software.
		Permission is granted to anyone to use this software for any purpose,
		including commercial applications, and to alter it and redistribute it freely,
		subject to the following restrictions:
		1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
		2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
		3. This notice may not be removed or altered from any source distribution.
		*/
		static void RasterizeTriangle(uint8_t *dest, int destWidth, const int *t0, const int *t1, const int *t2, const uint8_t *color)
		{
			if (t0[1] > t1[1]) std::swap(t0, t1);
			if (t0[1] > t2[1]) std::swap(t0, t2);
			if (t1[1] > t2[1]) std::swap(t1, t2);
			int total_height = t2[1] - t0[1];
			for (int i = 0; i < total_height; i++) {
				bool second_half = i > t1[1] - t0[1] || t1[1] == t0[1];
				int segment_height = second_half ? t2[1] - t1[1] : t1[1] - t0[1];
				float alpha = (float)i / total_height;
				float beta = (float)(i - (second_half ? t1[1] - t0[1] : 0)) / segment_height;
				int A[2], B[2];
				for (int j = 0; j < 2; j++) {
					A[j] = int(t0[j] + (t2[j] - t0[j]) * alpha);
					B[j] = int(second_half ? t1[j] + (t2[j] - t1[j]) * beta : t0[j] + (t1[j] - t0[j]) * beta);
				}
				if (A[0] > B[0]) std::swap(A, B);
				for (int j = A[0]; j <= B[0]; j++)
					SetPixel(dest, destWidth, j, t0[1] + i, color);
			}
		}

		bool ProgressCallback(xatlas::ProgressCategory::Enum category, int progress, void *userData)
		{
			const char *task[] = {
				"AddMesh",
				"ComputeCharts",
				"ParameterizeCharts",
				"PackCharts",
				"BuildOutputMeshes"
			};

			printf("Atlas: %s - progress: %d\n", task[category], progress);
			return true;
		}

		CUnwrapUV::CUnwrapUV()
		{
			m_atlas = xatlas::Create();

			xatlas::SetProgressCallback(m_atlas, ProgressCallback, this);
		}

		CUnwrapUV::~CUnwrapUV()
		{
			xatlas::Destroy(m_atlas);
		}

		bool CUnwrapUV::addMeshBuffer(IMeshBuffer *mb)
		{
			xatlas::MeshDecl meshDecl;

			IVertexBuffer *vb = mb->getVertexBuffer(0);
			IIndexBuffer *id = mb->getIndexBuffer();

			unsigned char *vtx = (unsigned char*)vb->getVertices();

			video::IVertexDescriptor* vertexDescriptor = mb->getVertexDescriptor();

			u32 vertexSize = vertexDescriptor->getVertexSize(0);
			IVertexAttribute *attibute = NULL;

			// Position
			attibute = vertexDescriptor->getAttributeBySemantic(video::EVAS_POSITION);
			if (attibute == NULL)
				return false;

			meshDecl.vertexCount = (uint32_t)vb->getVertexCount();
			meshDecl.vertexPositionData = vtx + attibute->getOffset();
			meshDecl.vertexPositionStride = vertexSize;

			attibute = vertexDescriptor->getAttributeBySemantic(video::EVAS_NORMAL);
			if (attibute != NULL)
			{
				meshDecl.vertexNormalData = vtx + attibute->getOffset();
				meshDecl.vertexNormalStride = vertexSize;
			}

			attibute = vertexDescriptor->getAttributeBySemantic(video::EVAS_TEXCOORD0);
			if (attibute != NULL)
			{
				meshDecl.vertexUvData = vtx + attibute->getOffset();
				meshDecl.vertexUvStride = vertexSize;
			}

			meshDecl.indexCount = (uint32_t)id->getIndexCount();
			meshDecl.indexData = id->getIndices();

			if (id->getType() == video::EIT_16BIT)
				meshDecl.indexFormat = xatlas::IndexFormat::UInt16;
			else
				meshDecl.indexFormat = xatlas::IndexFormat::UInt32;

			xatlas::AddMeshError::Enum error = xatlas::AddMesh(m_atlas, meshDecl);
			if (error != xatlas::AddMeshError::Success) {
				xatlas::Destroy(m_atlas);
				printf("\rError adding mesh: %s\n", xatlas::StringForEnum(error));
				return false;
			}

			return true;
		}

		bool CUnwrapUV::addMesh(CMesh *mesh)
		{
			for (u32 i = 0, n = mesh->getMeshBufferCount(); i < n; i++)
			{
				IMeshBuffer *mb = mesh->getMeshBuffer(i);
				if (addMeshBuffer(mb) == false)
				{
					return false;
				}
			}

			return true;
		}

		void CUnwrapUV::generate()
		{
			xatlas::PackOptions packOptions = xatlas::PackOptions();
			packOptions.padding = 1;
			// packOptions.texelsPerUnit = 32.0f;
			packOptions.resolution = 4096;

			xatlas::Generate(m_atlas,
				xatlas::ChartOptions(),
				xatlas::ParameterizeOptions(),
				packOptions
			);
		}

		void CUnwrapUV::writeUVToImage(const char *outputName)
		{
			if (m_atlas->width > 0 && m_atlas->height > 0)
			{
				printf("Rasterizing result...\n");

				// Dump images.
				std::vector<uint8_t> outputTrisImage, outputChartsImage;
				const uint32_t imageDataSize = m_atlas->width * m_atlas->height * 3;
				outputTrisImage.resize(m_atlas->atlasCount * imageDataSize);
				outputChartsImage.resize(m_atlas->atlasCount * imageDataSize);

				for (uint32_t i = 0; i < m_atlas->meshCount; i++)
				{
					const xatlas::Mesh &mesh = m_atlas->meshes[i];

					// Rasterize mesh triangles.
					const uint8_t white[] = { 255, 255, 255 };

					for (uint32_t j = 0; j < mesh.indexCount; j += 3)
					{
						int32_t atlasIndex = -1;
						int verts[3][2];

						for (int k = 0; k < 3; k++)
						{
							const xatlas::Vertex &v = mesh.vertexArray[mesh.indexArray[j + k]];
							atlasIndex = v.atlasIndex; // The same for every vertex in the triangle.
							verts[k][0] = int(v.uv[0]);
							verts[k][1] = int(v.uv[1]);
						}

						if (atlasIndex < 0)
							continue; // Skip triangles that weren't atlased.

						uint8_t color[3];
						RandomColor(color);

						uint8_t *imageData = &outputTrisImage[atlasIndex * imageDataSize];
						RasterizeTriangle(imageData, m_atlas->width, verts[0], verts[1], verts[2], color);
						RasterizeLine(imageData, m_atlas->width, verts[0], verts[1], white);
						RasterizeLine(imageData, m_atlas->width, verts[1], verts[2], white);
						RasterizeLine(imageData, m_atlas->width, verts[2], verts[0], white);
					}

					// Rasterize mesh charts.
					for (uint32_t j = 0; j < mesh.chartCount; j++)
					{
						const xatlas::Chart *chart = &mesh.chartArray[j];
						uint8_t color[3];
						RandomColor(color);
						for (uint32_t k = 0; k < chart->faceCount; k++)
						{
							int verts[3][2];
							for (int l = 0; l < 3; l++) {
								const xatlas::Vertex &v = mesh.vertexArray[mesh.indexArray[chart->faceArray[k] * 3 + l]];
								verts[l][0] = int(v.uv[0]);
								verts[l][1] = int(v.uv[1]);
							}
							uint8_t *imageData = &outputChartsImage[chart->atlasIndex * imageDataSize];
							RasterizeTriangle(imageData, m_atlas->width, verts[0], verts[1], verts[2], color);
							RasterizeLine(imageData, m_atlas->width, verts[0], verts[1], white);
							RasterizeLine(imageData, m_atlas->width, verts[1], verts[2], white);
							RasterizeLine(imageData, m_atlas->width, verts[2], verts[0], white);
						}
					}
				}

				for (uint32_t i = 0; i < m_atlas->atlasCount; i++) {
					char filename[256];
					IImage *img;
					void *data;

					snprintf(filename, sizeof(filename), "%s_tris%02u.png", outputName, i);
					printf("Writing '%s'...\n", filename);

					img = getVideoDriver()->createImage(video::ECF_R8G8B8, core::dimension2du(m_atlas->width, m_atlas->height));
					data = img->lock();
					memcpy(data, &outputTrisImage[i * imageDataSize], m_atlas->width * m_atlas->height * 3);
					img->unlock();
					getVideoDriver()->writeImageToFile(img, filename);

					snprintf(filename, sizeof(filename), "%s_charts%02u.png", outputName, i);
					printf("Writing '%s'...\n", filename);

					data = img->lock();
					memcpy(data, &outputChartsImage[i * imageDataSize], m_atlas->width * m_atlas->height * 3);
					img->unlock();
					getVideoDriver()->writeImageToFile(img, filename);

					img->drop();
				}
			}
		}
	}
}