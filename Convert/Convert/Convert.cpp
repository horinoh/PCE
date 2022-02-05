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

class PatternBase
{
public:
	PatternBase(const cv::Mat& Img) : Image(Img) {}

	virtual PatternBase& CreatePalette() { return *this; }
	virtual PatternBase& CreatePattern() { return *this; }
	virtual PatternBase& Create() { return *this; }

	virtual const PatternBase& OutputPalette([[maybe_unused]] std::string_view Path) const { return *this; }
	virtual const PatternBase& OutputPattern([[maybe_unused]] std::string_view Path) const { return *this; }
	virtual const PatternBase& OutputBAT([[maybe_unused]] std::string_view Path) const { return *this; }

	virtual const PatternBase& Restore() const { return *this; }

	static void Preview(const cv::Mat Image) {
		cv::imshow("", Image);
		cv::waitKey(0);
	}
	static void Preview(const cv::Mat Image, const cv::Size& Size) {
		cv::Mat Resized;
		cv::resize(Image, Resized, Size, 0, 0, cv::INTER_NEAREST);
		Preview(Resized);
	}

protected:
	const cv::Mat& Image;
};

template <uint32_t W, uint32_t H>
using PatternI = std::array<uint32_t, W * H>;

template<uint32_t W = 8, uint32_t H = 8>
class Pattern : public PatternBase
{
private:
	using Super = PatternBase;
public:	
	Pattern(const cv::Mat& Img) : Super(Img), MapSize(Image.cols / W, Image.rows / H) {}
	
	virtual PatternBase& Create() override {
#pragma region PALETTE
		Palettes.clear();
		for (auto i = 0; i < Image.rows; ++i) {
			for (auto j = 0; j < Image.cols; ++j) {
				const auto Color = Image.ptr<cv::Vec3b>(i)[j];
				if (end(Palettes) == std::ranges::find(Palettes, Color)) {
					Palettes.emplace_back(Color);
				}
			}
		}
#pragma endregion

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
		
#pragma region PATTERN
		Patterns.clear();
		for (auto ccp : CCPatterns) {
			Patterns.emplace_back();
			for (auto i = 0; i < ccp.rows; ++i) {
				for (auto j = 0; j < ccp.cols; ++j) {
					const auto It = std::ranges::find(Palettes, ccp.ptr<cv::Vec3b>(i)[j]);
					if (end(Palettes) != It) {
						Patterns.back()[i * W + j] = (static_cast<uint32_t>(std::distance(begin(Palettes), It)));
					}
				}
			}
		}
#pragma endregion
		Restore();
		return *this;
	}

	//!< 復元して表示してみる (チェック用)
	virtual const PatternBase& Restore() const override {
		cv::Mat Res(Image.size(), Image.type());
		for (auto m = 0; m < size(Maps); ++m) {
			cv::Mat Tile(cv::Size(W, H), Image.type());
			for (auto i = 0; i < H; ++i) {
				for (auto j = 0; j < W; ++j) {
					Tile.ptr<cv::Vec3b>(i)[j] = Palettes[Patterns[Maps[m]][i * W + j]];
				}
			}
			Tile.copyTo(Res(cv::Rect((m % MapSize.width) * W, (m / MapSize.width) * H, W, H)));
		}
		Preview(Res);

		return *this;
	}

protected:
	cv::Size MapSize;
	std::vector<cv::Vec3b> Palettes;
	std::vector<PatternI<W, H>> Patterns;
	std::vector<uint32_t> Maps;
};

class Converter
{
public:
	Converter(const cv::Mat& Img) : Image(Img) {}

	virtual Converter& Create() { return *this; }

	virtual const Converter& Restore() const { return *this; }

	static void Preview(const cv::Mat Image) {
		cv::imshow("", Image);
		cv::waitKey(0);
	}
	static void Preview(const cv::Mat Image, const cv::Size& Size) {
		cv::Mat Resized;
		cv::resize(Image, Resized, Size, 0, 0, cv::INTER_NEAREST);
		Preview(Resized);
	}

protected:
	const cv::Mat& Image;
};

using PCEPalette = std::vector<uint16_t>;
template <uint32_t W, uint32_t H>
class PCEPattern
{
public:
	uint32_t PaletteIndex;
	std::array<uint32_t, W * H> ColorIndices;
};

template<uint32_t W = 8, uint32_t H = 8>
class ConverterPCEImage : public Converter
{
private:
	using Super = Converter;
public:
	//!< 0000 000G GGRR RBBB : 9 ビットカラー
	static uint16_t ToPCEColor(const cv::Vec3b& Color) { return ((Color[1] >> 5) << 6) | ((Color[2] >> 5) << 3) | (Color[0] >> 5); }
	//!< Vec3b(B, G, R)
	static cv::Vec3b FromPCEColor(const uint16_t& Color) { return cv::Vec3b((Color & 0x7) << 5, ((Color & (0x7 << 6)) >> 6) << 5, ((Color & (0x7 << 3)) >> 3) << 5); }

	ConverterPCEImage(const cv::Mat& Img) : Super(Img), MapSize(Image.cols / W, Image.rows / H) {}

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
			Pal.emplace_back(0); //!< 先頭に透明カラーを追加 (ここでは 0 を透明とする)
			for (auto i = 0; i < p.rows; ++i) {
				for (auto j = 0; j < p.cols; ++j) {
					const auto Color = ToPCEColor(p.ptr<cv::Vec3b>(i)[j]);
					if (end(Pal) == std::ranges::find(Pal, Color)) {
						Pal.emplace_back(Color);
					}
				}
			}
			//!< PCE ではパレットに 16 色まで(透明を含む)
			if (size(Pal) > 16) {
				std::cerr << "Pal Exceed" << std::endl;
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
#pragma endregion

#pragma region PATTERN
		//!< 使用パレット番号と、インデックスカラー表現のパターンを格納
		Patterns.clear();
		int ii = 0;
		for (auto p : CCPatterns) {
			Patterns.emplace_back();
			Patterns.back().PaletteIndex = PaletteIndices[ii++/*std::distance(begin(CCPatterns), p)*/];
			const auto& Pal = Palettes[Patterns.back().PaletteIndex];
			for (auto i = 0; i < p.rows; ++i) {
				for (auto j = 0; j < p.cols; ++j) {
					Patterns.back().ColorIndices[i * W + j] = static_cast<uint32_t>(std::distance(begin(Pal), std::ranges::find(Pal, ToPCEColor(p.ptr<cv::Vec3b>(i)[j]))));
				}
			}
		}
#pragma endregion
		
		Restore();
		
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
		Preview(Res);

		return *this;
	}

protected:
	cv::Size MapSize;
	std::vector<PCEPalette> Palettes;
	std::vector<PCEPattern<W, H>> Patterns;
	std::vector<uint32_t> Maps;
};

class PatternPCE : public PatternBase
{
private:
	using Super = PatternBase;
public:
	PatternPCE(const cv::Mat& Img) : Super(Img) {}

	//!< 0000 000G GGRR RBBB
	static uint16_t ToPCEColor(const cv::Vec3b& Color) { return ((Color[1] >> 5) << 6) | ((Color[2] >> 5) << 3) | (Color[0] >> 5); }
	//!< Vec3b(B, G, R)
	static cv::Vec3b FromPCEColor(const uint16_t& Color) { return cv::Vec3b((Color & 0x7) << 5, ((Color & (0x7 << 6)) >> 6) << 5, ((Color & (0x7 << 3)) >> 3) << 5); }
	
	//!< パレットの先頭色 (透明色 or 背景色)
	PatternBase& SetPaletteTopColor(const cv::Vec3b& Color) { PaletteTopColor = ToPCEColor(Color); return *this; }
	PatternBase& SetPaletteTopColor(const uint16_t Color) { PaletteTopColor = Color; return *this; }
	virtual uint16_t GetPaletteTopColor() const { return PaletteTopColor; }

	//!< 24bitカラー -> 9bitカラー へ丸められる為、近い色だと同じ色になってしまう場合があり、想定よりもエントリが少なくなる可能性がある
	virtual PatternBase& CreatePalette() override {
		Palettes.clear();
		//!< パレットの先頭は 透明色 or 背景色
		Palettes.emplace_back(GetPaletteTopColor());
		for (auto i = 0; i < Image.rows; ++i) {
			for (auto j = 0; j < Image.cols; ++j) {
				const auto Color = ToPCEColor(Image.ptr<cv::Vec3b>(i)[j]);
				if (end(Palettes) == std::ranges::find(Palettes, Color)) {
					Palettes.emplace_back(Color);
				}
			}
		}
		std::cout << "Palette Count = " << size(Palettes) << std::endl;
		while (size(Palettes) < 16) { Palettes.emplace_back(GetPaletteTopColor()); }
		return *this;
	}
	virtual PatternBase& CreatePattern() override {
		CreatePalette();

		IndexColors.clear();
		for (auto i = 0; i < Image.rows; ++i) {
			for (auto j = 0; j < Image.cols; ++j) {
				const auto Color = ToPCEColor(Image.ptr<cv::Vec3b>(i)[j]);
				const auto It = std::ranges::find(Palettes, Color);
				if (end(Palettes) != It) {
					IndexColors.emplace_back(static_cast<uint32_t>(std::distance(begin(Palettes), It)));
				}
			}
		}
		return *this;
	}

	virtual const PatternBase& OutputPalette(std::string_view Path) const override {
		std::ofstream Out(data(Path), std::ios::binary | std::ios::out);
		if (!Out.bad()) {
			Out.write(reinterpret_cast<const char*>(data(Palettes)), size(Palettes) * sizeof(Palettes[0]));
			Out.close();
		}
		return *this;
	}
	virtual const PatternBase& OutputPattern(std::string_view Path) const override {
		std::ofstream Out(data(Path), std::ios::binary | std::ios::out);
		if (!Out.bad()) {
			Out.close();
		}
		return *this;
	}

	virtual const PatternBase& Restore() const override {
		cv::Mat RestoreImage(Image.size(), Image.type());
		int k = 0;
		for (auto i = 0; i < Image.rows; ++i) {
			for (auto j = 0; j < Image.cols; ++j) {
				RestoreImage.ptr<cv::Vec3b>(i)[j] = FromPCEColor(Palettes[IndexColors[k++]]);
			}
		}
		Preview(RestoreImage, cv::Size(256, 256));
		return *this;
	}

protected:
	uint16_t PaletteTopColor = ToPCEColor(cv::Vec3b(0, 0, 0));
	std::vector<uint16_t> Palettes;
	std::vector<uint16_t> IndexColors;
};

//!< スプライトのサイズバリエーション : SZ_16x16, SZ_16x32, SZ_16x64, SZ_32x16, SZ_32x32, SZ_32x64
class PatternPCESP : public PatternPCE
{
private:
	using Super = PatternPCE;
public:
	PatternPCESP(const cv::Mat& Img) : Super(Img) {}

	//!< 4つのプレーンに分けて出力
	//!< プレーン0 : インデックス & 1
	//!< プレーン1 : インデックス & 2
	//!< プレーン2 : インデックス & 4
	//!< プレーン3 : インデックス & 8
	virtual PatternBase& CreatePattern() override {
		Super::CreatePattern();

		Plane0.clear();
		Plane1.clear();
		Plane2.clear();
		Plane3.clear();
		for (auto i = 0; i < size(IndexColors) / 16; ++i) {
			Plane0.emplace_back(0);
			Plane1.emplace_back(0);
			Plane2.emplace_back(0);
			Plane3.emplace_back(0);
			for (auto j = 0; j < 16; ++j) {
				const auto Index = IndexColors[i * 16 + j];
				const auto Shift = 15 - j;
				Plane0.back() |= ((Index & 1) ? 1 : 0) << Shift;
				Plane1.back() |= ((Index & 2) ? 1 : 0) << Shift;
				Plane2.back() |= ((Index & 4) ? 1 : 0) << Shift;
				Plane3.back() |= ((Index & 8) ? 1 : 0) << Shift;
			}
		}
		return *this;
	}

	virtual const PatternBase& OutputPattern(std::string_view Path) const override {
		std::ofstream Out(data(Path), std::ios::binary | std::ios::out);
		if (!Out.bad()) {
			Out.write(reinterpret_cast<const char*>(data(Plane0)), size(Plane0) * sizeof(Plane0[0]));
			Out.write(reinterpret_cast<const char*>(data(Plane1)), size(Plane1) * sizeof(Plane1[0]));
			Out.write(reinterpret_cast<const char*>(data(Plane2)), size(Plane2) * sizeof(Plane2[0]));
			Out.write(reinterpret_cast<const char*>(data(Plane3)), size(Plane3) * sizeof(Plane3[0]));
			Out.close();
		}
		return *this; 
	}

	//!< 復元して表示してみる (チェック用)
	virtual const PatternBase& Restore() const override {
		cv::Mat RestoreImage(Image.size(), Image.type());
		for (auto i = 0; i < size(IndexColors) / 16; ++i) {
			for (auto j = 0; j < 16; ++j) {
				const auto Mask = 1 << (15 - j);
				const auto Index = ((Plane0[i] & Mask) ? 1 : 0) | ((Plane1[i] & Mask) ? 2 : 0) | ((Plane2[i] & Mask) ? 4 : 0) | ((Plane3[i] & Mask) ? 8 : 0);
				RestoreImage.ptr<cv::Vec3b>(i)[j] = FromPCEColor(Palettes[Index]);
			}
		}
		Preview(RestoreImage, cv::Size(256, 256));
		return *this;
	}

protected:
	std::vector<uint16_t> Plane0;
	std::vector<uint16_t> Plane1;
	std::vector<uint16_t> Plane2;
	std::vector<uint16_t> Plane3;
};

class PatternPCEBG : public PatternPCE
{
private:
	using Super = PatternPCE;
public:
	PatternPCEBG(const cv::Mat& Img) : Super(Img) {}

	virtual PatternBase& CreatePattern() override {
		Super::CreatePattern();
		return *this;
	}

	virtual const PatternBase& OutputPattern(std::string_view Path) const override {
		std::ofstream Out(data(Path), std::ios::binary | std::ios::out);
		if (!Out.bad()) {
			Out.close();
		}
		return *this;
	}
	//!< BAT (Background Attribute Table)
	virtual const PatternBase& OutputBAT(std::string_view Path) const {
		std::ofstream Out(data(Path), std::ios::binary | std::ios::out);
		if (!Out.bad()) {
			Out.close();
		}
		return *this;
	}

	//!< 復元して表示してみる (チェック用)
	virtual const PatternBase& Restore() const override {
		return *this;
	}

protected:
};

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

static void ProcessPalette(std::string_view Name, std::string_view File)
{
	if (!empty(File)) {
		auto Image = cv::imread(data(File));
		ConverterPCEImage<>(Image).Create();
		//PatternBase::Preview(Image);
	}
}
static void ProcessTileSet(std::string_view Name, std::string_view File, std::string_view Compression, std::string_view Option)
{
	if (!empty(File)) {
		auto Image = cv::imread(data(File));
		ConverterPCEImage<>(Image).Create();
		//PatternBase::Preview(Image);
	}
}
static void ProcessMap(std::string_view Name, std::string_view File, std::string_view TileSet, std::string_view Compression, const uint32_t Mapbase)
{
}
static void ProcessSprite(std::string_view Name, std::string_view File, const uint32_t Width, const uint32_t Height, std::string_view Compression, const uint32_t Time, std::string_view Collision, std::string_view Opt, const uint32_t Iteration)
{
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

	//auto Image = cv::imread("sprite.png");
	//if (1 < argc) {
	//	Image = cv::imread(argv[1]);
	//}
	//Preview(Image, cv::Size(256, 256));

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

#if 0
	//!< パレットとインデックスカラー画像 (PCE SP)
	IndexColorImagePCESprite PCESP(Image);
	PCESP.
		//SetPaletteTopColor(cv::Vec3b(255, 255, 255)).
		CreatePattern().Restore().OutputPalette("../../SpriteBin/SP_Palette.bin").OutputPattern("../../SpriteBin/SP_Pattern.bin");
#else
	//!< パレットとインデックスカラー画像 (PCE BG)
	//PatternPCEBG PCEBG(Image);
	//PCEBG.
	//	//SetPaletteTopColor(cv::Vec3b(255, 255, 255)).
	//	CreatePattern().Restore().OutputPalette("../../SpriteBin/BG_Palette.bin").OutputPattern("../../SpriteBin/BG_Pattern.bin").OutputBAT("../../SpriteBin/BG_BAT.bin");
#endif

	//PatternFC FC(Image);
	//FC.CreatePattern().Restore()./*OutputPalette("../../SpriteBin/FC_Palette.h").*/OutputPattern("../../SpriteBin/FC_Pattern.h");
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
