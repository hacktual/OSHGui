#include "Graphics.hpp"
#include "Image.hpp"
#include <algorithm>

#define _USE_MATH_DEFINES
#include <math.h>

namespace OSHGui
{
	namespace Drawing
	{
		std::vector<PointF> circlePoints;
		//---------------------------------------------------------------------------
		//Constructor
		//---------------------------------------------------------------------------
		Graphics::Graphics(GeometryBuffer &buffer)
			: buffer(buffer)
		{
			buffer.SetClippingActive(false);

			if (circlePoints.empty())
			{
				circlePoints.resize(33);

				const auto pi2 = static_cast<float>(M_PI) * 2.0f;
				for (auto i = 0; i <= 32; i++)
				{
					const auto angle = static_cast<float>(i) / 32.0f * pi2;
					circlePoints[i] = PointF(std::cos(angle), std::sin(angle));
				}
			}
		}
		//---------------------------------------------------------------------------
		Graphics::~Graphics()
		{
			
		}
		//---------------------------------------------------------------------------
		void Graphics::SetClip(const RectangleF &clip)
		{
			buffer.SetClippingRegion(clip);
			buffer.SetClippingActive(true);
		}
		//---------------------------------------------------------------------------
		//Runtime Functions
		//---------------------------------------------------------------------------
		void Graphics::Clear()
		{
			buffer.Reset();
		}
		//---------------------------------------------------------------------------
		void Graphics::Rotate(const PointF &pivot, const Vector &angles)
		{
			buffer.SetPivot(Vector(pivot.X, pivot.Y, 0.0f));
			buffer.SetRotation(Quaternion::EulerAnglesDegrees(angles.x, angles.y, angles.z));
		}
		//---------------------------------------------------------------------------
		void Graphics::DrawLine(const Color &color, const PointF &from, const PointF &to,float thickness)
		{
			auto delta = to - from;
			const auto length = std::sqrt(delta.X * delta.X + delta.Y * delta.Y);

			if (length > 0.0f)
			{
				delta.X /= length;
				delta.Y /= length;
			}

			const PointF offset(thickness * delta.Y, thickness * -delta.X);
			std::vector<PointF> vertices = {
				PointF(from.X, from.Y),
				PointF(to.X, to.Y),
				PointF(to.X - offset.X, to.Y - offset.Y),
				PointF(from.X - offset.X, from.Y - offset.Y),
			};

			FillPolygon(vertices, color);
		}
		//---------------------------------------------------------------------------
		void Graphics::DrawRectangle(const Color &color, const PointF &origin, const SizeF &size, float rounding, float thickness)
		{
			DrawRectangle(color, origin.X, origin.Y, size.Width, size.Height, rounding, thickness);
		}
		//---------------------------------------------------------------------------
		void Graphics::DrawRectangle(const Color &color, const RectangleF &rectangle, float rounding, float thickness)
		{
			DrawRectangle(color, rectangle.GetLeft(), rectangle.GetTop(), rectangle.GetWidth(), rectangle.GetHeight(), rounding, thickness);
		}
		//---------------------------------------------------------------------------
		void Graphics::DrawRectangle(const Color &color, float x, float y, float width, float height, float rounding, float thickness)
		{
			if (rounding > 0.0f)
			{
				DrawCircle(color, x + rounding, y + rounding, rounding, thickness, 0.5f, 0.75f);
				DrawCircle(color, x + width - rounding, y + rounding, rounding, thickness, 0.75f);
				DrawCircle(color, x + width - rounding, y + height - rounding, rounding, thickness, 0.0f, 0.25f);
				DrawCircle(color, x + rounding, y + height - rounding, rounding, thickness, 0.25f, 0.5f);

				DrawLine(color, PointF(x + rounding, y), PointF(x + width - rounding, y), thickness);
				DrawLine(color, PointF(x + width, y + rounding), PointF(x + width, y + height - rounding), thickness);
				DrawLine(color, PointF(x + width - rounding, y + height), PointF(x + rounding, y + height), thickness);
				DrawLine(color, PointF(x, y + height - rounding), PointF(x, y + rounding), thickness);
			}
			else
			{
				FillRectangle(color, x, y, width - thickness, thickness);
				FillRectangle(color, x + width - thickness, y, thickness, height - thickness);
				FillRectangle(color, x + thickness, y + height - thickness, width - thickness, thickness);
				FillRectangle(color, x, y + thickness, thickness, height - thickness);
			}
		}
		//---------------------------------------------------------------------------
		void Graphics::FillRectangle(const Color &color, const PointF &origin, const SizeF &size, float rounding)
		{
			FillRectangle(color, origin.X, origin.Y, size.Width, size.Height, rounding);
		}
		//---------------------------------------------------------------------------
		void Graphics::FillRectangle(const Color &color, const RectangleF &rectangle, float rounding)
		{
			FillRectangle(color, rectangle.GetLeft(), rectangle.GetTop(), rectangle.GetWidth(), rectangle.GetHeight(), rounding);
		}
		//---------------------------------------------------------------------------
		void Graphics::FillRectangle(const Color &color, float x, float y, float width, float height, float rounding)
		{
			if (rounding > 0.0f)
			{
				FillCircle(color, x + rounding, y + rounding, rounding, 0.5f, 0.75f);
				FillCircle(color, x + width - rounding, y + rounding, rounding, 0.75f);
				FillCircle(color, x + width - rounding, y + height - rounding, rounding, 0.0f, 0.25f);
				FillCircle(color, x + rounding, y + height - rounding, rounding, 0.25f, 0.5f);

				FillRectangle(color, x, y + rounding, rounding, height - rounding * 2.0f);
				FillRectangle(color, x + rounding, y, width - rounding * 2.0f, height);
				FillRectangle(color, x + width - rounding, y + rounding, rounding, height - rounding * 2.0f);
			}
			else
			{
				Vertex vertices[] = {
					{ Vector(x, y, 0.0f), color },
					{ Vector(x + width, y, 0.0f), color },
					{ Vector(x, y + height, 0.0f), color },
					{ Vector(x + width, y + height, 0.0f), color },
					{ Vector(x, y + height, 0.0f), color },
					{ Vector(x + width, y, 0.0f), color }
				};
				buffer.AppendGeometry(vertices, 6);
			}
		}
		//---------------------------------------------------------------------------
		void Graphics::FillRectangleGradient(const ColorRectangle &colors, const PointF &origin, const SizeF &size)
		{
			FillRectangleGradient(colors, origin.X, origin.Y, size.Width, size.Height);
		}
		//---------------------------------------------------------------------------
		void Graphics::FillRectangleGradient(const ColorRectangle &colors, const RectangleF &rectangle)
		{
			FillRectangleGradient(colors, rectangle.GetLeft(), rectangle.GetTop(), rectangle.GetWidth(), rectangle.GetHeight());
		}
		//---------------------------------------------------------------------------
		void Graphics::FillRectangleGradient(const ColorRectangle &colors, float x, float y, float width, float height)
		{
			Vertex vertices[] = {
				{ Vector(x, y, 0.0f), colors.TopLeft },
				{ Vector(x + width, y, 0.0f), colors.TopRight },
				{ Vector(x, y + height, 0.0f), colors.BottomLeft },
				{ Vector(x + width, y + height, 0.0f), colors.BottomRight },
				{ Vector(x, y + height, 0.0f), colors.BottomLeft },
				{ Vector(x + width, y, 0.0f), colors.TopRight }
			};
			buffer.AppendGeometry(vertices, 6);
		}
		//---------------------------------------------------------------------------
		void Graphics::FillPolygon(const std::vector<PointF> &points, const Color &color)
		{
			if (points.empty())
			{
				return;
			}

			const auto size = static_cast<int>(points.size());
			std::vector<PointF> normals(size);

			for (auto i = size - 1, j = 0; j < size; i = j++)
			{
				auto delta = points[j] - points[i];
				const auto length = std::sqrt(delta.X * delta.X + delta.Y * delta.Y);

				if (length > 0.0f)
				{
					delta.X /= length;
					delta.Y /= length;
				}

				normals[i].X = delta.Y;
				normals[i].Y = -delta.X;
			}

			std::vector<Vertex> vertices(size * 2);
			const Color transparent(0.0f, color.GetRed(), color.GetGreen(), color.GetBlue());

			for (int i = size - 1, j = 0; j < size; i = j++)
			{
				PointF delta((normals[i].X + normals[j].X) * 0.5f, (normals[i].Y + normals[j].Y) * 0.5f);
				const auto length = std::sqrt(delta.X * delta.X + delta.Y * delta.Y);

				if (length > 0.0f)
				{
					delta.X /= length;
					delta.Y /= length;
				}

				delta.X *= 0.5f;
				delta.Y *= 0.5f;

				vertices[j * 2] = { Vector(points[j].X - delta.X, points[j].Y - delta.Y, 0.0f), color };
				vertices[j * 2 + 1] = { Vector(points[j].X + delta.X, points[j].Y + delta.Y, 0.0f), transparent };
			}

			std::vector<Vertex> polygon(size * 3);
			std::vector<Vertex> fringe(size * 6);

			for (auto i = 0; i < size; ++i)
			{
				if (i > 1)
				{
					polygon[i * 3] = vertices[0];
					polygon[i * 3 + 1] = vertices[i * 2];
					polygon[i * 3 + 2] = vertices[(i - 1) * 2];
				}

				fringe[i * 6] = vertices[i * 2];
				fringe[i * 6 + 1] = vertices[i * 2 + 1];
				fringe[i * 6 + 2] = vertices[(i + 1) % size * 2];
				fringe[i * 6 + 3] = vertices[(i + 1) % size * 2];
				fringe[i * 6 + 4] = vertices[i * 2 + 1];
				fringe[i * 6 + 5] = vertices[(i + 1) % size * 2 + 1];
			}

			polygon.insert(polygon.end(), fringe.begin(), fringe.end());
			buffer.AppendGeometry(polygon.data(), polygon.size());
		}
		//---------------------------------------------------------------------------
		void Graphics::FillCircle(const Color &color, const PointF &origin, float radius, float start, float end)
		{
			FillCircle(color, origin.X, origin.Y, radius, start, end);
		}
		//---------------------------------------------------------------------------
		void Graphics::FillCircle(const Color &color, float x, float y, float radius, float start, float end)
		{
			std::vector<PointF> points(2 + static_cast<int>((end - start) * 32.0f));
			points[0] = PointF(x, y);

			for (auto i = static_cast<int>(start * 32.0f), j = 1; i <= static_cast<int>(end * 32.0f); ++i)
			{
				points[j++] = PointF(x + circlePoints[i].X * radius, y + circlePoints[i].Y * radius);
			}

			FillPolygon(points, color);
		}
		//---------------------------------------------------------------------------
		void Graphics::FillEllipse(const Color &color, const PointF &origin, const SizeF &size)
		{
			FillEllipse(color, origin.X, origin.Y, size.Width, size.Height);
		}
		//---------------------------------------------------------------------------
		void Graphics::FillEllipse(const Color &color, const RectangleF &region)
		{
			FillEllipse(color, region.GetLeft(), region.GetTop(), region.GetWidth(), region.GetHeight());
		}
		//---------------------------------------------------------------------------
		void Graphics::FillEllipse(const Color &color, float _x, float _y, float width, float height)
		{
			const auto a = width / 2;
			const auto b = height / 2;
			const auto xc = _x;
			const auto yc = _y;
			int x = 0;
			int y = b;
			const auto a2 = a * a;
			const auto b2 = b * b;
			auto xp = 1;
			auto yp = y;

			while (b2 * x < a2 * y)
			{
				++x;
				if ((b2 * x * x + a2 * (y - 0.5f) * (y - 0.5f) - a2 * b2) >= 0)
				{
					y--;
				}
				if (x == 1 && y != yp)
				{
					FillRectangle(color, xc, yc + yp - 1, 1, 1);
					FillRectangle(color, xc, yc - yp, 1, 1);
				}
				if (y != yp)
				{
					FillRectangle(color, xc - x + 1, yc - yp, 2 * x - 1, 1);
					FillRectangle(color, xc - x + 1, yc + yp, 2 * x - 1, 1);
					yp = y;
					xp = x;
				}

				if (b2 * x >= a2 * y)
				{
					FillRectangle(color, xc - x, yc - yp, 2 * x + 1, 1);
					FillRectangle(color, xc - x, yc + yp, 2 * x + 1, 1);
				}
			}

			xp = x;
			yp = y;
			auto divHeight = 1;

			while (y != 0)
			{
				y--;
				if ((b2 * (x + 0.5f) * (x + 0.5f) + a2 * y * y - a2 * b2) <= 0)
				{
					x++;
				}

				if (x != xp)
				{
					divHeight = yp - y;

					FillRectangle(color, xc - xp, yc - yp, 2 * xp + 1, divHeight);
					FillRectangle(color, xc - xp, yc + y + 1, 2 * xp + 1, divHeight);

					xp = x;
					yp = y;
				}

				if (y == 0)
				{
					divHeight = yp - y + 1;

					FillRectangle(color, xc - xp, yc - yp, 2 * x + 1, divHeight);
					FillRectangle(color, xc - xp, yc + y, 2 * x + 1, divHeight);
				}
			}
		}

		void Graphics::DrawString(const Misc::AnsiString &text, const FontPtr &font, const Color &color, const PointF &origin)
		{
			font->DrawText(buffer, text, origin, nullptr, color);

			buffer.SetActiveTexture(nullptr);
		}

		void Graphics::DrawString(const Misc::AnsiString &text, const FontPtr &font, const Color &color, float x, float y)
		{
			DrawString(text, font, color, PointF(x, y));
		}

		void Graphics::DrawImage(const ImagePtr &image, const ColorRectangle &color, const PointF &origin)
		{
			DrawImage(image, color, RectangleF(origin, image->GetSize()));
		}

		void Graphics::DrawImage(const std::shared_ptr<Image> &image, const ColorRectangle &color, const PointF &origin, const RectangleF &clip)
		{
			DrawImage(image, color, RectangleF(origin, image->GetSize()), clip);
		}

		void Graphics::DrawImage(const ImagePtr &image, const ColorRectangle &color, const RectangleF &area)
		{
			image->Render(buffer, area, nullptr, color);

			buffer.SetActiveTexture(nullptr);
		}

		void Graphics::DrawImage(const std::shared_ptr<Image> &image, const ColorRectangle &color, const RectangleF &area, const RectangleF &clip)
		{
			image->Render(buffer, area, &clip.OffsetEx(area.GetLocation()), color);
		}

		void Graphics::DrawTriangle(const Color& color, const PointF &origin1, const PointF &origin2, const PointF &origin3)
		{
			DrawLine(color, origin1, origin2);
			DrawLine(color, origin2, origin3);
			DrawLine(color, origin3, origin1);
		}

		void Graphics::FillTriangle(const Color &color, const PointF &origin1, const PointF &origin2, const PointF &origin3)
		{
			FillPolygon({ origin1, origin2, origin3 }, color);
		}

		void Graphics::DrawTriangleGradient(const Color &color1, const Color &color2, const Color &color3, const PointF &origin1, const PointF &origin2, const PointF &origin3)
		{
			buffer.SetVertexDrawMode(VertexDrawMode::LineList);

			Vertex vertices[] = {
				{ Vector(origin1.X, origin1.Y, 0.0f), color1 },
				{ Vector(origin2.X, origin2.Y, 0.0f), color2 },
				{ Vector(origin2.X, origin2.Y, 0.0f), color2 },
				{ Vector(origin3.X, origin3.Y, 0.0f), color3 },
				{ Vector(origin3.X, origin3.Y, 0.0f), color3 },
				{ Vector(origin1.X, origin1.Y, 0.0f), color1 },
			};
			buffer.AppendGeometry(vertices, 6);

			buffer.SetVertexDrawMode(VertexDrawMode::TriangleList);
		}

		void Graphics::FillTriangleGradient(const Color &color1, const Color &color2, const Color &color3, const PointF &origin1, const PointF &origin2, const PointF &origin3)
		{
			Vertex vertices[] = {
				{ Vector(origin1.X, origin1.Y, 0.0f), color1 },
				{ Vector(origin2.X, origin2.Y, 0.0f), color2 },
				{ Vector(origin3.X, origin3.Y, 0.0f), color3 },
			};
			buffer.AppendGeometry(vertices, 3);
		}
		void Graphics::DrawCircle(const Color &color, const PointF &origin, float radius, float thickness, float start, float end)
		{
			DrawCircle(color, origin.X, origin.Y, thickness, radius, start, end);
		}

		void Graphics::DrawCircle(const Color &color, float x, float y, float radius, float thickness, float start, float end)
		{
			for (auto i = static_cast<int>(start * 32.0f), j = i + 1; j <= static_cast<int>(end * 32.0f); i = j++)
			{
				DrawLine(color, PointF(x + circlePoints[i].X * radius, y + circlePoints[i].Y * radius), PointF(x + circlePoints[j].X * radius, y + circlePoints[j].Y * radius), thickness);
			}
		}

		void Graphics::FillCircleGradient(const Color &colorOut, const Color &colorIn, const PointF &origin, float radius, float start, float end)
		{
			FillCircleGradient(colorIn, colorOut, origin.X, origin.Y, radius, start, end);
		}

		void Graphics::FillCircleGradient(const Color &colorOut, const Color &colorIn, float x, float y, float radius, float start, float end)
		{
			const PointF center(x, y);
			for (auto i = static_cast<int>(start * 32.0f), j = i + 1; j <= static_cast<int>(end * 32.0f); i = j++)
			{
				FillTriangleGradient(colorOut, colorIn, colorOut, PointF(x + circlePoints[i].X * radius, y + circlePoints[i].Y * radius), center, PointF(x + circlePoints[j].X * radius, y + circlePoints[j].Y * radius));
			}
		}

		void Graphics::DrawShadow(const Color &color, const PointF &origin, const SizeF &size, float amount)
		{
			DrawShadow(color, origin.X, origin.Y, size.Width, size.Height, amount);
		}

		void Graphics::DrawShadow(const Color &color, const RectangleF &rectangle, float amount)
		{
			DrawShadow(color, rectangle.GetLeft(), rectangle.GetTop(), rectangle.GetWidth(), rectangle.GetHeight(), amount);
		}

		void Graphics::DrawShadow(const Color &color, float x, float y, float width, float height, float amount)
		{
			const auto shadow = Color(0.0f, color.GetRed(), color.GetGreen(), color.GetBlue());
			FillCircleGradient(shadow, color, x, y, amount, 0.5f, 0.75f);
			FillCircleGradient(shadow, color, x + width, y, amount, 0.75f);
			FillCircleGradient(shadow, color, x + width, y + height, amount, 0.0f, 0.25f);
			FillCircleGradient(shadow, color, x, y + height, amount, 0.25f, 0.5f);

			FillRectangleGradient(ColorRectangle(shadow, color, shadow, color), x - amount, y, amount, height);
			FillRectangleGradient(ColorRectangle(shadow, shadow, color, color), x, y - amount, width, amount);
			FillRectangleGradient(ColorRectangle(color, shadow, color, shadow), x + width, y, amount, height);
			FillRectangleGradient(ColorRectangle(color, color, shadow, shadow), x, y + height, width, amount);
		}
	}
}
