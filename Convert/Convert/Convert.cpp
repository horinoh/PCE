// Convert.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>

#pragma warning(push)
#pragma warning(disable:4819)
#include <opencv2/opencv.hpp>
#pragma warning(pop)

#include <vector>
#include <fstream>
#include <filesystem>
#include <algorithm>
#include <string_view>
#include <charconv>

#ifdef _DEBUG
#pragma comment(lib, "opencv_world454d.lib")
#else
#pragma comment(lib, "opencv_world454.lib")
#endif

static void Preview(const cv::Mat Image) 
{
	cv::imshow("", Image);
	cv::waitKey(0);
}
static void Preview(const cv::Mat& Image, const cv::Size& Size) 
{
	cv::Mat Resized;
	cv::resize(Image, Resized, Size, 0, 0, cv::INTER_NEAREST);
	Preview(Resized);
}
static void DrawGrid(const cv::Mat& Image, const cv::Size& GridSize) 
{
	const auto Color = cv::Scalar(0, 0, 0);
	const auto SizeY = Image.size().height / GridSize.height;
	for (auto i = 0; i < SizeY; ++i) {
		cv::line(Image, cv::Point(0, i * GridSize.height), cv::Point(Image.size().width, i * GridSize.height), Color);
	}
	const auto SizeX = Image.size().width / GridSize.width;
	for (auto i = 0; i < SizeX; ++i) {
		cv::line(Image, cv::Point(i * GridSize.width, 0), cv::Point(i * GridSize.width, Image.size().height), Color);
	}
}
static void ColorReduction(cv::Mat& Dst, const cv::Mat& Image, const uint32_t ColorCount)
{
	//!< 1行の行列となるように変形
	cv::Mat Points;
	Image.convertTo(Points, CV_32FC3);
	Points = Points.reshape(3, Image.rows * Image.cols);

	//!< k-means クラスタリング
	cv::Mat_<int> Clusters(Points.size(), CV_32SC1);
	cv::Mat Centers;
	cv::kmeans(Points, ColorCount, Clusters, cv::TermCriteria(cv::TermCriteria::Type::EPS | cv::TermCriteria::Type::MAX_ITER, 10, 1.0), 1, cv::KmeansFlags::KMEANS_PP_CENTERS, Centers);

	//!< 各ピクセル値を属するクラスタの中心値で置き換え
	Dst = cv::Mat(Image.size(), Image.type());
	//cv::Mat Tmp(Image.size(), Image.type());
	auto It = Dst.begin<cv::Vec3b>();
	for (auto i = 0; It != Dst.end<cv::Vec3b>(); ++It, ++i) {
		const auto Color = Centers.at<cv::Vec3f>(Clusters(i), 0);
		(*It)[0] = cv::saturate_cast<uchar>(Color[0]); //!< B
		(*It)[1] = cv::saturate_cast<uchar>(Color[1]); //!< G
		(*It)[2] = cv::saturate_cast<uchar>(Color[2]); //!< R
	}

	//Dst = Tmp.clone();
}


class Converter
{
public:
	Converter(const cv::Mat& Img) : Image(Img) {}

	virtual Converter& Create() { return *this; }

	virtual const Converter& OutputPalette(std::string_view Path) const { return *this; }
	virtual const Converter& OutputPattern(std::string_view Path) const { return *this; }

	virtual const Converter& Restore() const { return *this; }

protected:
	const cv::Mat& Image;
};

#pragma region PCE
using PCEPalette = std::vector<uint16_t>;
template <uint32_t W, uint32_t H>
class PCEPattern
{
public:
	uint32_t PaletteIndex;
	std::array<uint32_t, W * H> ColorIndices;
};
template<uint32_t W, uint32_t H>
class ConverterPCE : public Converter
{
private:
	using Super = Converter;
public:
	//!< 0000 000G GGRR RBBB : 9 ビットカラー
	static uint16_t ToPCEColor(const cv::Vec3b& Color) { return ((Color[1] >> 5) << 6) | ((Color[2] >> 5) << 3) | (Color[0] >> 5); }
	//!< Vec3b(B, G, R)
	static cv::Vec3b FromPCEColor(const uint16_t& Color) { return cv::Vec3b((Color & 0x7) << 5, ((Color & (0x7 << 6)) >> 6) << 5, ((Color & (0x7 << 3)) >> 3) << 5); }

	ConverterPCE(const cv::Mat& Img) : Super(Img), MapSize(Image.cols / W, Image.rows / H) {}

	virtual Converter& Create() override {
#pragma region MAP
		std::vector<cv::Mat> CCPatterns; //!< Color Component Patterns
		Maps.clear();
		for (auto i = 0; i < MapSize.height; ++i) {
			for (auto j = 0; j < MapSize.width; ++j) {
				const auto NoFlip = Image(cv::Rect(j * W, i * H, W, H));
				cv::Mat VFlip, HFlip, VHFlip;
				cv::flip(NoFlip, VFlip, 0);
				cv::flip(NoFlip, HFlip, 1);
				cv::flip(NoFlip, VHFlip, -1);

				const auto It = std::ranges::find_if(CCPatterns, [&](const cv::Mat& rhs) {
					cv::Mat Result;
					cv::bitwise_xor(NoFlip, rhs, Result);
					if (0 == cv::sum(Result)[0]) { return true; }

					cv::bitwise_xor(VFlip, rhs, Result);
					if (0 == cv::sum(Result)[0]) { return true; }

					cv::bitwise_xor(HFlip, rhs, Result);
					if (0 == cv::sum(Result)[0]) { return true; }

					cv::bitwise_xor(VHFlip, rhs, Result);
					if (0 == cv::sum(Result)[0]) { return true; }

					return false;
					});
				if (end(CCPatterns) != It) {
					Maps.emplace_back(static_cast<uint32_t>(std::distance(begin(CCPatterns), It)));
				}
				else {
					Maps.emplace_back(static_cast<uint32_t>(size(CCPatterns)));
					CCPatterns.emplace_back(NoFlip);
				}
			}
		}
#pragma endregion

#pragma region PALETTE
		std::vector<uint32_t> PaletteIndices;
		Palettes.clear();
		for (auto p : CCPatterns) {
			//!< パターン毎にパレットを作る
			PCEPalette Pal;
			for (auto i = 0; i < p.rows; ++i) {
				for (auto j = 0; j < p.cols; ++j) {
					const auto Color = ToPCEColor(p.ptr<cv::Vec3b>(i)[j]);
					if (end(Pal) == std::ranges::find(Pal, Color)) {
						Pal.emplace_back(Color);
					}
				}
			}
			//!< PCE ではパレットに 16 色まで (先頭に透明色が入ることになるのでここでは 15 色までに収まっていないといけない)
			if (size(Pal) > 15) {
				std::cerr << "Color Exceed" << std::endl;
			}
			//!< 同一パレット検出のためソートしておく
			std::ranges::sort(Pal);

			//!< パターンが使用するパレットインデックス (既存に存在すればパレットは追加しない)
			const auto It = std::ranges::find(Palettes, Pal);
			if (end(Palettes) == It) {
				PaletteIndices.emplace_back(static_cast<uint32_t>(size(Palettes)));
				Palettes.emplace_back(Pal);
			}
			else {
				PaletteIndices.emplace_back(static_cast<uint32_t>(std::distance(begin(Palettes), It)));
			}
		}

#pragma region OPTIMIZE_PALETTE
		//!< パレットをまとめる
		while ([&]() {
			for (auto i = 0; i < size(Palettes); ++i) {
				for (auto j = i + 1; j < size(Palettes); ++j) {
					auto& lhs = Palettes[i];
					auto& rhs = Palettes[j];
					if (!empty(lhs) && !empty(rhs)) {
						PCEPalette UnionSet;
						std::ranges::set_union(lhs, rhs, std::back_inserter(UnionSet));
						if (16 > size(UnionSet)) {
							lhs.assign(begin(UnionSet), end(UnionSet));
							rhs.clear();
							std::ranges::replace(PaletteIndices, j, i);
							return true;
						}
					}
				}
			}
			return false;
		}()) {}
		//!< 空になったパレットは消す
		const auto [Begin, End] = std::ranges::remove_if(Palettes, [](const PCEPalette& rhs) { return empty(rhs); });
		Palettes.erase(Begin, End);
#pragma endregion

		//!< PCE ではパレット数 16 まで (BG、スプライトそれぞれ 16)
		if (size(Palettes) > 16) {
			std::cerr << "Palette Exceed" << std::endl;
		}
		else {
			std::cout << "Palette count = " << size(Palettes) << std::endl;
		}
#pragma endregion

#pragma region PATTERN
		//!< 使用パレット番号と、インデックスカラー表現のパターンを格納
		Patterns.clear();
		for (auto p = 0; p < size(CCPatterns); ++p) {
			const auto& Pat = CCPatterns[p];
			Patterns.emplace_back();
			Patterns.back().PaletteIndex = PaletteIndices[p];
			const auto& Pal = Palettes[Patterns.back().PaletteIndex];
			for (auto i = 0; i < Pat.rows; ++i) {
				for (auto j = 0; j < Pat.cols; ++j) {
					Patterns.back().ColorIndices[i * W + j] = static_cast<uint32_t>(std::distance(begin(Pal), std::ranges::find(Pal, ToPCEColor(Pat.ptr<cv::Vec3b>(i)[j]))));
				}
			}
		}
#pragma endregion
		
		Restore();
		
		return *this;
	}

	virtual const Converter& OutputPalette(std::string_view Path) const override {
		std::ofstream Out(data(Path), std::ios::binary | std::ios::out);
		if (!Out.bad()) {
			for (auto i : Palettes) {
				const uint16_t Color = 0;
				Out.write(reinterpret_cast<const char*>(&Color), sizeof(Color)); //!< 先頭に透明色(ここでは0)を出力
				Out.write(reinterpret_cast<const char*>(data(i)), size(i) * sizeof(i[0]));
				for (auto j = size(i); j < 16; j++) {
					Out.write(reinterpret_cast<const char*>(&Color), sizeof(Color)); //!< 空き要素分(ここでは0)を出力
				}
			}
			Out.close();
		}
		return *this;
	}

	//!< 復元して表示してみる (チェック用)
	virtual const Converter& Restore() const override {
		cv::Mat Res(Image.size(), Image.type());
		for (auto m = 0; m < size(Maps); ++m) {
			const auto& Pat = Patterns[Maps[m]];
			const auto& Pal = Palettes[Pat.PaletteIndex];
			cv::Mat Tile(cv::Size(W, H), Image.type());
			for (auto i = 0; i < H; ++i) {
				for (auto j = 0; j < W; ++j) {
					Tile.ptr<cv::Vec3b>(i)[j] = FromPCEColor(Pal[Pat.ColorIndices[i * W + j]]);					
				}
			}
			Tile.copyTo(Res(cv::Rect((m % MapSize.width) * W, (m / MapSize.width) * H, W, H)));
		}

		DrawGrid(Res, cv::Size(W, H));
		Preview(Res);

		return *this;
	}

protected:
	cv::Size MapSize;
	std::vector<PCEPalette> Palettes;
	std::vector<PCEPattern<W, H>> Patterns;
	std::vector<uint16_t> Maps;
};

//!< BGImage : スクロールさせない静止画向き
template<uint32_t W = 8, uint32_t H = 8>
class ConverterPCEImage : public ConverterPCE<W, H>
{
private:
	using Super = ConverterPCE<W, H>;
public:
	ConverterPCEImage(const cv::Mat& Img) : Super(Img) {}

	virtual ConverterPCEImage& Create() override { Super::Create(); return *this; }

	virtual const ConverterPCEImage& OutputPattern(std::string_view Path) const override {
		std::ofstream Out(data(Path), std::ios::binary | std::ios::out);
		if (!Out.bad()) {
			for (auto p : this->Patterns) {
				for (auto i = 0; i < H; ++i) {
					uint16_t Plane01 = 0, Plane23 = 0;
					for (auto j = 0; j < W; ++j) {
						const auto ColIdx = p.ColorIndices[i * W + j] + 1; //!< 先頭の透明色を考慮して + 1
						const auto ShiftL = 7 - j;
						const auto ShiftU = ShiftL + 8;
						Plane01 |= ((ColIdx & 1) ? 1 : 0) << ShiftL;
						Plane01 |= ((ColIdx & 2) ? 1 : 0) << ShiftU;
						Plane23 |= ((ColIdx & 4) ? 1 : 0) << ShiftL;
						Plane23 |= ((ColIdx & 8) ? 1 : 0) << ShiftU;
					}
					Out.write(reinterpret_cast<const char*>(&Plane01), sizeof(Plane01));
					Out.write(reinterpret_cast<const char*>(&Plane23), sizeof(Plane23));
				}
			}
			Out.close();
		}
		return *this;
	}
	//!< BAT はパターン番号とパレット番号からなるマップ
	virtual const ConverterPCEImage& OutputBAT(std::string_view Path) const {
		std::ofstream Out(data(Path), std::ios::binary | std::ios::out);
		if (!Out.bad()) {
			for (auto m = 0; m < size(this->Maps); ++m) {
				const auto PatIdx = this->Maps[m];
				const auto PalIdx = this->Patterns[PatIdx].PaletteIndex; 
				const uint16_t BATEntry = (PalIdx << 12) | PatIdx + 256;;
				Out.write(reinterpret_cast<const char*>(&BATEntry), sizeof(BATEntry));
			}
			Out.close();
		}
		std::cout << "BAT size = " << this->MapSize.width << " x " << this->MapSize.height << std::endl;
		return *this;
	}
	
	const ConverterPCEImage& Output(std::string_view PalettePath, std::string_view PatternPath, std::string_view BATPath) const {
		Super::OutputPalette(PalettePath);
		OutputPattern(PatternPath);
		OutputBAT(BATPath);
		return *this;
	}
};

//!< BG : スクロールさせる背景
template<uint32_t W = 16, uint32_t H = 16>
class ConverterPCEBG : public ConverterPCE<W, H>
{
private:
	using Super = ConverterPCE<W, H>;
public:
	ConverterPCEBG(const cv::Mat& Img) : Super(Img) {}

	virtual ConverterPCEBG& Create() override { Super::Create(); return *this; }

	virtual const ConverterPCEBG& OutputPattern(std::string_view Path) const override {
		std::ofstream Out(data(Path), std::ios::binary | std::ios::out);
		if (!Out.bad()) {
			for (auto p : this->Patterns) {
				for (auto i = 0; i < H; ++i) {
					uint16_t LPlane01 = 0, LPlane23 = 0;
					uint16_t RPlane01 = 0, RPlane23 = 0;
					for (auto j = 0; j < W; ++j) {
						const auto ColIdx = p.ColorIndices[i * W + j] + 1; //!< 先頭の透明色を考慮して + 1
						const auto ShiftL = 7 - (j % 8);
						const auto ShiftU = ShiftL + 8;
						if (j > 7) {
							RPlane01 |= ((ColIdx & 1) ? 1 : 0) << ShiftL;
							RPlane01 |= ((ColIdx & 2) ? 1 : 0) << ShiftU;
							RPlane23 |= ((ColIdx & 4) ? 1 : 0) << ShiftL;
							RPlane23 |= ((ColIdx & 8) ? 1 : 0) << ShiftU;
						}
						else {
							LPlane01 |= ((ColIdx & 1) ? 1 : 0) << ShiftL;
							LPlane01 |= ((ColIdx & 2) ? 1 : 0) << ShiftU;
							LPlane23 |= ((ColIdx & 4) ? 1 : 0) << ShiftL;
							LPlane23 |= ((ColIdx & 8) ? 1 : 0) << ShiftU;
						}
					}
					Out.write(reinterpret_cast<const char*>(&LPlane01), sizeof(LPlane01));
					Out.write(reinterpret_cast<const char*>(&LPlane23), sizeof(LPlane23));
					Out.write(reinterpret_cast<const char*>(&RPlane01), sizeof(RPlane01));
					Out.write(reinterpret_cast<const char*>(&RPlane23), sizeof(RPlane23));
				}
			}
			Out.close();
		}
		std::cout << "Pattern count = " << size(this->Patterns) << std::endl;
		return *this;
	}
	virtual const ConverterPCEBG& OutputPatternPalette(std::string_view Path) const {
		std::ofstream Out(data(Path), std::ios::binary | std::ios::out);
		if (!Out.bad()) {
			for (auto p : this->Patterns) {
				const uint8_t PalIdx = p.PaletteIndex << 4; //!< パターン毎のパレットインデックス (4 ビットシフトする必要がある)
				Out.write(reinterpret_cast<const char*>(&PalIdx), sizeof(PalIdx));
			}
			Out.close();
		}
		return *this;
	}
	virtual const ConverterPCEBG& OutputMap(std::string_view Path) const {
		std::ofstream Out(data(Path), std::ios::binary | std::ios::out);
		if (!Out.bad()) {
			for (auto m = 0; m < size(this->Maps); ++m) {
				const uint8_t PatIdx = static_cast<uint8_t>(this->Maps[m]);
				Out.write(reinterpret_cast<const char*>(&PatIdx), sizeof(PatIdx));
			}
			Out.close();
		}
		std::cout << "Map size = " << this->MapSize.width << " x " << this->MapSize.height << std::endl;
		return *this;
	}

	const ConverterPCEBG& Output(std::string_view PalettePath, std::string_view PatternPath, std::string_view PatternPalettePath, std::string_view MapPath) const {
		Super::OutputPalette(PalettePath);
		OutputPattern(PatternPath);
		OutputPatternPalette(PatternPalettePath);
		OutputMap(MapPath);
		return *this;
	}
};

//!< Sprite : 
template<uint32_t W = 16, uint32_t H = 16>
class ConverterPCESprite : public ConverterPCE<W, H>
{
private:
	using Super = ConverterPCE<W, H>;
public:
	ConverterPCESprite(const cv::Mat& Img) : Super(Img) {}

	virtual ConverterPCESprite& Create() override { Super::Create(); return *this; }

	virtual const ConverterPCESprite& OutputPattern(std::string_view Path) const override {
		std::ofstream Out(data(Path), std::ios::binary | std::ios::out);
		if (!Out.bad()) {
			for (auto p : this->Patterns) {
				for (auto i = 0; i < H; ++i) {
					uint16_t Plane0 = 0, Plane1 = 0, Plane2 = 0, Plane3 = 0;
					for (auto j = 0; j < W; ++j) {
						const auto ColIdx = p.ColorIndices[i * W + j] + 1; //!< 先頭の透明色を考慮して + 1
						const auto Shift = 15 - j;
						Plane0 |= ((ColIdx & 1) ? 1 : 0) << Shift;
						Plane1 |= ((ColIdx & 2) ? 1 : 0) << Shift;
						Plane2 |= ((ColIdx & 4) ? 1 : 0) << Shift;
						Plane3 |= ((ColIdx & 8) ? 1 : 0) << Shift;
					}
					Out.write(reinterpret_cast<const char*>(&Plane0), sizeof(Plane0));
					Out.write(reinterpret_cast<const char*>(&Plane1), sizeof(Plane1));
					Out.write(reinterpret_cast<const char*>(&Plane2), sizeof(Plane2));
					Out.write(reinterpret_cast<const char*>(&Plane3), sizeof(Plane3));
				}
			}
			std::cout << "Sprite size = " << W << " x " << H << " x " << size(this->Patterns) << std::endl;
			Out.close();
		}
		return *this;
	}

	const ConverterPCESprite& Output(std::string_view PalettePath, std::string_view PatternPath) const {
		Super::OutputPattern(PalettePath);
		OutputPattern(PatternPath);
		return *this;
	}
};
#pragma endregion //!< PCE

#if 0
class PatternFC : public PatternBase
{
private:
	using Super = PatternBase;
public:
	PatternFC(const cv::Mat& Img) : Super(Img) {}

	virtual PatternBase& CreatePalette() override {
		Palettes.clear();
		//!< パレットの先頭は 透明色 or 背景色
		Palettes.emplace_back(0);
		for (auto i = 0; i < Image.rows; ++i) {
			for (auto j = 0; j < Image.cols; ++j) {
				const auto Color = Image.ptr<cv::Vec3b>(i)[j];
				if (end(Palettes) == std::ranges::find(Palettes, Color)) {
					Palettes.emplace_back(Color);
				}
			}
		}
		std::cout << "Palette Count = " << size(Palettes) << std::endl;
		while (size(Palettes) < 4) { Palettes.emplace_back(0); }
		return *this;
	}
	virtual PatternBase& CreatePattern() override {
		CreatePalette();

		IndexColors.clear();
		for (auto i = 0; i < Image.rows; ++i) {
			for (auto j = 0; j < Image.cols; ++j) {
				const auto Color = Image.ptr<cv::Vec3b>(i)[j];
				const auto It = std::ranges::find(Palettes, Color);
				if (end(Palettes) != It) {
					IndexColors.emplace_back(static_cast<uint32_t>(std::distance(begin(Palettes), It)));
				}
			}
		}

		Plane0.clear();
		Plane1.clear();
		for (auto i = 0; i < size(IndexColors) >> 3; ++i) {
			Plane0.emplace_back(0);
			Plane1.emplace_back(0);
			for (auto j = 0; j < 8; ++j) {
				const auto Index = IndexColors[(i << 3) + j];
				const auto Shift = 8 - j;
				Plane0.back() |= ((Index & 1) ? 1 : 0) << Shift;
				Plane1.back() |= ((Index & 2) ? 1 : 0) << Shift;
			}
		}
		return *this;
	}

	virtual const PatternBase& OutputPalette(std::string_view Path) const override {
		//std::ofstream Out(data(Path), std::ios::out);
		//if (!Out.bad()) {
		//	Out.close();
		//}
		return *this;
	}
	virtual const PatternBase& OutputPattern(std::string_view Path) const override {
		std::ofstream Out(data(Path), std::ios::out);
		if (!Out.bad()) {
			Out << "\t";
			for (auto i : Plane0) { Out << "0x" << std::hex << std::setw(4) << std::right << std::setfill('0') << i << ", "; }
			Out << std::endl;

			Out << "\t";
			for (auto i : Plane1) { Out << "0x" << std::hex << std::setw(4) << std::right << std::setfill('0') << i << ", "; }
			Out.close();
		}
		return *this;
	}

	virtual const PatternBase& Restore() const override {
		cv::Mat RestoreImage(Image.size(), Image.type());
		int k = 0;
		for (auto i = 0; i < Image.rows; ++i) {
			for (auto j = 0; j < Image.cols; ++j) {
				RestoreImage.ptr<cv::Vec3b>(i)[j] = Palettes[IndexColors[k++]];
			}
		}
		Preview(RestoreImage, cv::Size(256, 256));
		return *this;
	}

protected:
	std::vector<cv::Vec3b> Palettes;
	std::vector<uint16_t> IndexColors;

	std::vector<uint16_t> Plane0;
	std::vector<uint16_t> Plane1;
};
#endif

static void ProcessPalette(std::string_view Name, std::string_view File)
{
	if (!empty(File)) {
		auto Image = cv::imread(data(File));
		ConverterPCEImage<>(Image).Create().OutputPalette(std::string(Name) + ".bin");
		//ConverterPCEBG<>(Image).Create().OutputPalette(std::string(Name) + ".bin");
	}
}
static void ProcessTileSet(std::string_view Name, std::string_view File, [[maybe_unused]] std::string_view Compression, [[maybe_unused]] std::string_view Option)
{
	if (!empty(File)) {
		auto Image = cv::imread(data(File));
		ConverterPCEImage<>(Image).Create().OutputPattern(std::string(Name) + ".bin");
		//ConverterPCEBG<>(Image).Create().OutputPattern(std::string(Name) + ".bin");
		//ConverterPCEBG<>(Image).Create().OutputPatternPalette(std::string(Name) + ".pal.bin");
	}
}
static void ProcessMap(std::string_view Name, std::string_view File, std::string_view TileSet, [[maybe_unused]] std::string_view Compression, [[maybe_unused]] const uint32_t Mapbase)
{
	if (!empty(File)) {
		auto Image = cv::imread(data(File));
		ConverterPCEImage<>(Image).Create().OutputBAT(std::string(Name) + ".bin");
		//ConverterPCEBG<>(Image).Create().OutputMap(std::string(Name) + ".bin");
	}
}
static void ProcessSprite(std::string_view Name, std::string_view File, const uint32_t Width, const uint32_t Height, [[maybe_unused]] std::string_view Compression, [[maybe_unused]] const uint32_t Time, [[maybe_unused]] std::string_view Collision, [[maybe_unused]] std::string_view Option, [[maybe_unused]] const uint32_t Iteration)
{
	if (!empty(File)) {
		auto Image = cv::imread(data(File));

#pragma region PCE
		switch (Width << 3) {
		case 16:
			switch (Height << 3) {
			case 16: 
				ConverterPCESprite<16, 16>(Image).Create();
				break;
			case 32:
				ConverterPCESprite<16, 32>(Image).Create();
				break;
			case 64:
				ConverterPCESprite<16, 64>(Image).Create();
				break;
			default: break;
			}
			break;
		case 32:
			switch (Height << 3) {
			case 16:
				ConverterPCESprite<32, 16>(Image).Create();
				break;
			case 32:
				ConverterPCESprite<32, 32>(Image).Create();
				break;
			case 64:
				ConverterPCESprite<32, 64>(Image).Create();
				break;
			default: break;
			}
			break;
		default: break;
		}
#pragma endregion
		ConverterPCESprite<48, 48>(Image).Create().Output("Palette.bin", "Pattern.bin");
	}
}
int main(int argc, char* argv[])
{
	//!< ターゲットフォルダ (Target folder)
	std::string_view Path = ".";
	for (const auto& i : std::filesystem::directory_iterator(Path)) {
		if (!i.is_directory()) {
			//!< .res ファイルを探す (Search for .res files)
			if (i.path().has_extension() && ".res" == i.path().extension().string()) {
				std::ifstream In(i.path().filename().string(), std::ios::in);
				if (!In.fail()) {
					//!< 行を読み込む (Read line)
					std::string Line;
					while (std::getline(In, Line)) {
						//!< 項目を読み込む (Read items)
						std::vector<std::string> Items;
						std::stringstream SS(Line);
						std::string Item;
						while (std::getline(SS, Item, ' ')) {
							Items.emplace_back(Item);
						}

						if (!empty(Items)) {
							//!< 画像ファイル名から " を取り除く (Remove " from image file name)
							std::erase(Items[2], '"');

							if ("PALETTE" == Items[0]) {
								ProcessPalette(Items[1], Items[2]);
							}
							if ("TILESET" == Items[0]) {
								ProcessTileSet(Items[1], Items[2], size(Items) > 3 ? Items[3] : "", size(Items) > 4 ? Items[4] : "");
							}
							if ("MAP" == Items[0]) {
								uint32_t MapBase = 0;
								if (size(Items) > 5) {
									auto [ptr, ec] = std::from_chars(data(Items[5]), data(Items[5]) + size(Items[5]), MapBase);
									if (std::errc() != ec) {}
								}
								ProcessMap(Items[1], Items[2], Items[3], size(Items) > 4 ? Items[4] : "", MapBase);
							}
							if ("SPRITE" == Items[0]) {
								uint32_t Width = 0;
								auto [ptr0, ec0] = std::from_chars(data(Items[3]), data(Items[3]) + size(Items[3]), Width);
								if (std::errc() != ec0) {}

								uint32_t Height = 0;
								auto [ptr1, ec1] = std::from_chars(data(Items[4]), data(Items[4]) + size(Items[4]), Height);
								if (std::errc() != ec1) {}

								uint32_t Time = 0;
								if (size(Items) > 6) {
									auto [ptr, ec] = std::from_chars(data(Items[6]), data(Items[6]) + size(Items[6]), Time);
									if (std::errc() != ec) {}
								}

								uint32_t Iteration = 500000;
								if (size(Items) > 9) {
									auto [ptr, ec] = std::from_chars(data(Items[9]), data(Items[9]) + size(Items[9]), Iteration);
									if (std::errc() != ec) {}
								}

								ProcessSprite(Items[1], Items[2], Width, Height, size(Items) > 5 ? Items[5] : "", Time, size(Items) > 7 ? Items[7] : "", size(Items) > 8 ? Items[8] : "", Iteration);
							}
						}
					}
					In.close();
				}
			}
		}
	}

	//!< リサイズと減色は予めやってもらう体とする
#if false
	//!< リサイズ
	//cv::resize(Image, Image, cv::Size(16, 16), 0, 0, cv::INTER_NEAREST);
	cv::resize(Image, Image, cv::Size(8, 8), 0, 0, cv::INTER_NEAREST);

	//!< 減色
	{
		//!< 何色に減色するか
		const int ColorCount = 16;

		//!< 1行の行列となるように変形
		cv::Mat Points;
		Image.convertTo(Points, CV_32FC3);
		Points = Points.reshape(3, Image.rows * Image.cols);

		//!< k-means クラスタリング
		cv::Mat_<int> Clusters(Points.size(), CV_32SC1);
		cv::Mat Centers;
		cv::kmeans(Points, ColorCount, Clusters, cv::TermCriteria(cv::TermCriteria::Type::EPS | cv::TermCriteria::Type::MAX_ITER, 10, 1.0), 1, cv::KmeansFlags::KMEANS_PP_CENTERS, Centers);

		//!< 各ピクセル値を属するクラスタの中心値で置き換え
		cv::Mat Tmp(Image.size(), Image.type());
		auto It = Tmp.begin<cv::Vec3b>();
		for (auto i = 0; It != Tmp.end<cv::Vec3b>(); ++It, ++i) {
			const auto Color = Centers.at<cv::Vec3f>(Clusters(i), 0);
			(*It)[0] = cv::saturate_cast<uchar>(Color[0]); //!< B
			(*It)[1] = cv::saturate_cast<uchar>(Color[1]); //!< G
			(*It)[2] = cv::saturate_cast<uchar>(Color[2]); //!< R
		}

		Image = Tmp.clone();
	}
	//!< リサイズ、減色後の結果を(拡大表示で)プレビュー 
	Preview(Image, cv::Size(256, 256));
#endif
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
