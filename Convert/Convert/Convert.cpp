// Convert.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>

#pragma warning(push)
#pragma warning(disable:4819)
#include <opencv2/opencv.hpp>
#pragma warning(pop)

#include <vector>
#include <fstream>
#include <string_view>

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

	virtual const PatternBase& OutputPalette([[maybe_unused]] std::string_view Path) const { return *this; }
	virtual const PatternBase& OutputPattern([[maybe_unused]] std::string_view Path) const { return *this; }
	virtual const PatternBase& OutputBAT([[maybe_unused]] std::string_view Path) const { return *this; }

	virtual const PatternBase& Restore() const { return *this; }

protected:
	const cv::Mat& Image;
};
class Pattern : public PatternBase
{
private:
	using Super = PatternBase;
public:	
	Pattern(const cv::Mat& Img) : Super(Img) {}
	
	virtual PatternBase& CreatePalette() override {
		Palettes.clear();
		for (auto i = 0; i < Image.rows; ++i) {
			for (auto j = 0; j < Image.cols; ++j) {
				const auto Color = Image.ptr<cv::Vec3b>(i)[j];
				if (end(Palettes) == std::find(begin(Palettes), end(Palettes), Color)) {
					Palettes.emplace_back(Color);
				}
			}
		}
		return *this;
	}
	virtual PatternBase& CreatePattern() override {
		IndexColors.clear();
		for (auto i = 0; i < Image.rows; ++i) {
			for (auto j = 0; j < Image.cols; ++j) {
				const auto Color = Image.ptr<cv::Vec3b>(i)[j];
				const auto It = std::find(begin(Palettes), end(Palettes), Color);
				if (end(Palettes) != It) {
					IndexColors.emplace_back(static_cast<uint32_t>(std::distance(begin(Palettes), It)));
				}
			}
		}
		return *this;
	}

	//!< 復元して表示してみる (チェック用)
	virtual const PatternBase& Restore() const override {
		cv::Mat RestoreImage(Image.size(), Image.type());
		int k = 0;
		for (auto i = 0; i < Image.rows; ++i) {
			for (auto j = 0; j < Image.cols; ++j) {
				RestoreImage.ptr<cv::Vec3b>(i)[j] = Palettes[IndexColors[k++]];
			}
		}
		//!< 拡大表示
		cv::resize(RestoreImage, RestoreImage, cv::Size(256, 256), 0, 0, cv::INTER_NEAREST);
		cv::imshow("Restore", RestoreImage);
		cv::waitKey(0);
		return *this;
	}

protected:
	std::vector<cv::Vec3b> Palettes;
	std::vector<uint32_t> IndexColors;
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
				if (end(Palettes) == std::find(begin(Palettes), end(Palettes), Color)) {
					Palettes.emplace_back(Color);
				}
			}
		}
		std::cout << "Palette Count = " << size(Palettes) << std::endl;
		while (size(Palettes) < 16) { Palettes.emplace_back(GetPaletteTopColor()); }
		return *this;
	}
	virtual PatternBase& CreatePattern() override {
		IndexColors.clear();
		for (auto i = 0; i < Image.rows; ++i) {
			for (auto j = 0; j < Image.cols; ++j) {
				const auto Color = ToPCEColor(Image.ptr<cv::Vec3b>(i)[j]);
				const auto It = std::find(begin(Palettes), end(Palettes), Color);
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
		cv::resize(RestoreImage, RestoreImage, cv::Size(256, 256), 0, 0, cv::INTER_NEAREST);
		cv::imshow("Restore PCE", RestoreImage);
		cv::waitKey(0);
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
		cv::resize(RestoreImage, RestoreImage, cv::Size(256, 256), 0, 0, cv::INTER_NEAREST);
		cv::imshow("Restore PCE SP", RestoreImage);
		cv::waitKey(0);
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
				if (end(Palettes) == std::find(begin(Palettes), end(Palettes), Color)) {
					Palettes.emplace_back(Color);
				}
			}
		}
		std::cout << "Palette Count = " << size(Palettes) << std::endl;
		while (size(Palettes) < 4) { Palettes.emplace_back(0); }
		return *this;
	}
	virtual PatternBase& CreatePattern() override {
		IndexColors.clear();
		for (auto i = 0; i < Image.rows; ++i) {
			for (auto j = 0; j < Image.cols; ++j) {
				const auto Color = Image.ptr<cv::Vec3b>(i)[j];
				const auto It = std::find(begin(Palettes), end(Palettes), Color);
				if (end(Palettes) != It) {
					IndexColors.emplace_back(static_cast<uint32_t>(std::distance(begin(Palettes), It)));
				}
			}
		}

		Plane0.clear();
		Plane1.clear();
		for (auto i = 0; i < size(IndexColors) / 8; ++i) {
			Plane0.emplace_back(0);
			Plane1.emplace_back(0);
			for (auto j = 0; j < 8; ++j) {
				const auto Index = IndexColors[i * 8 + j];
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
		cv::resize(RestoreImage, RestoreImage, cv::Size(256, 256), 0, 0, cv::INTER_NEAREST);
		cv::imshow("Restore FC", RestoreImage);
		cv::waitKey(0);
		return *this;
	}

protected:
	std::vector<cv::Vec3b> Palettes;
	std::vector<uint16_t> IndexColors;

	std::vector<uint16_t> Plane0;
	std::vector<uint16_t> Plane1;
};

int main(int argc, char* argv[])
{
	auto Image = cv::imread("sprite.png");
	if (1 < argc) {
		Image = cv::imread(argv[1]);
	}
	cv::imshow("Original", Image);
	cv::waitKey(0);

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
	{
		cv::Mat Dst;
		cv::resize(Image, Dst, cv::Size(256, 256), 0, 0, cv::INTER_NEAREST);
		cv::imshow("Resize & color reduction", Dst);
		cv::waitKey(0);
	}

	//!< パレットとインデックスカラー画像
	//IndexColorImage ICI(Image);
	//ICI.CreatePalette().CreatePattern().Restore();

#if 0
	//!< パレットとインデックスカラー画像 (PCE SP)
	IndexColorImagePCESprite PCESP(Image);
	PCESP.
		//SetPaletteTopColor(cv::Vec3b(255, 255, 255)).
		CreatePalette().CreatePattern().Restore().OutputPalette("../../SpriteBin/SP_Palette.bin").OutputPattern("../../SpriteBin/SP_Pattern.bin");
#else
	//!< パレットとインデックスカラー画像 (PCE BG)
	PatternPCEBG PCEBG(Image);
	PCEBG.
		//SetPaletteTopColor(cv::Vec3b(255, 255, 255)).
		CreatePalette().CreatePattern().Restore().OutputPalette("../../SpriteBin/BG_Palette.bin").OutputPattern("../../SpriteBin/BG_Pattern.bin").OutputBAT("../../SpriteBin/BG_BAT.bin");
#endif

	PatternFC FC(Image);
	FC.CreatePalette().CreatePattern().Restore()./*OutputPalette("../../SpriteBin/FC_Palette.h").*/OutputPattern("../../SpriteBin/FC_Pattern.h");
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
