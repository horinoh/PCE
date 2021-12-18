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

class IndexColorBase
{
public:
	IndexColorBase(const cv::Mat& Img) : Image(Img) {}

	virtual IndexColorBase& CreatePalette() { return *this; }
	virtual IndexColorBase& CreateIndexColor() { return *this; }
	virtual IndexColorBase& OutputPalette([[maybe_unused]] std::string_view Path) { return *this; }
	virtual IndexColorBase& OutputIndexColor([[maybe_unused]] std::string_view Path) { return *this; }
	virtual IndexColorBase& Restore() { return *this; }

protected:
	const cv::Mat& Image;
};
class IndexColorImage : public IndexColorBase
{
private:
	using Super = IndexColorBase;
public:	
	IndexColorImage(const cv::Mat& Img) : Super(Img) {}
	virtual IndexColorBase& CreatePalette() override {
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
	virtual IndexColorBase& CreateIndexColor() override {
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
	//!< インデックスカラーから復元して表示してみる (チェック用)
	virtual IndexColorBase& Restore() override {
		cv::Mat RestoreImage(Image.size(), Image.type());
		int k = 0;
		for (auto i = 0; i < Image.rows; ++i) {
			for (auto j = 0; j < Image.cols; ++j) {
				RestoreImage.ptr<cv::Vec3b>(i)[j] = Palettes[IndexColors[k++]];
			}
		}
		//!< 大きくして表示
		cv::resize(RestoreImage, RestoreImage, cv::Size(512, 512), 0, 0, cv::INTER_NEAREST);
		cv::imshow("Restore", RestoreImage);
		cv::waitKey(0);
		return *this;
	}
protected:
	std::vector<cv::Vec3b> Palettes;
	std::vector<uint32_t> IndexColors;
};

class IndexColorImagePCE : public IndexColorBase
{
private:
	using Super = IndexColorBase;
public:
	IndexColorImagePCE(const cv::Mat& Img) : Super(Img) {}
	//!< 0000 000G GGRR RBBB
	static uint16_t ToPCEColor(const cv::Vec3b& Color) { return ((Color[1] * 7 / 255) << 6) | ((Color[2] * 7 / 255) << 3) | (Color[0] * 7 / 255); }
	//!< Vec3b(B, G, R)
	static cv::Vec3b FromPCEColor(const uint16_t& Color) { return cv::Vec3b((Color & 0x7) * 255 / 7, ((Color & (0x7 << 6)) >> 6) * 255 / 7, ((Color & (0x7 << 3)) >> 3) * 255 / 7); }
	//!< 24bitカラー -> 9bitカラー へ丸められる為、近い色だと同じ色になってしまう場合があり、想定よりもエントリが少なくなる可能性がある
	virtual IndexColorBase& CreatePalette() override {
		Palettes.clear();
		Palettes.emplace_back(0);
		for (auto i = 0; i < Image.rows; ++i) {
			for (auto j = 0; j < Image.cols; ++j) {
				const auto Color = ToPCEColor(Image.ptr<cv::Vec3b>(i)[j]);
				if (end(Palettes) == std::find(begin(Palettes), end(Palettes), Color)) {
					Palettes.emplace_back(Color);
				}
			}
		}
		std::cout << "Palette Count = " << size(Palettes) << std::endl;
		while (size(Palettes) < 16) { Palettes.emplace_back(0); }
		return *this;
	}
	virtual IndexColorBase& CreateIndexColor() override {
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
	virtual IndexColorBase& OutputPalette(std::string_view Path) override {
		std::ofstream Out(data(Path), std::ios::binary | std::ios::out);
		if (!Out.bad()) {
			Out.write(reinterpret_cast<const char*>(data(Palettes)), size(Palettes) * sizeof(Palettes[0]));
			Out.close();
		}
		return *this;
	}
	virtual IndexColorBase& OutputIndexColor(std::string_view Path) override {
		std::ofstream Out(data(Path), std::ios::binary | std::ios::out);
		if (!Out.bad()) {
			Out.close();
		}
		return *this;
	}
	virtual IndexColorBase& Restore() override {
		cv::Mat RestoreImage(Image.size(), Image.type());
		int k = 0;
		for (auto i = 0; i < Image.rows; ++i) {
			for (auto j = 0; j < Image.cols; ++j) {
				RestoreImage.ptr<cv::Vec3b>(i)[j] = FromPCEColor(Palettes[IndexColors[k++]]);
			}
		}
		cv::resize(RestoreImage, RestoreImage, cv::Size(512, 512), 0, 0, cv::INTER_NEAREST);
		cv::imshow("Restore PCE", RestoreImage);
		cv::waitKey(0);
		return *this;
	}
protected:
	std::vector<uint16_t> Palettes;
	std::vector<uint16_t> IndexColors;
};


//!< スプライトのサイズ : SZ_16x16, SZ_16x32, SZ_16x64, SZ_32x16, SZ_32x32, SZ_32x64
class IndexColorImagePCESprite : public IndexColorImagePCE
{
private:
	using Super = IndexColorImagePCE;
public:
	IndexColorImagePCESprite(const cv::Mat& Img) : Super(Img) {}
	virtual IndexColorBase& CreateIndexColor() override {
		Super::CreateIndexColor();
		
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
				Plane0.back() |= (Index & 1) << Shift;
				Plane1.back() |= (Index & 2) << Shift;
				Plane2.back() |= (Index & 4) << Shift;
				Plane3.back() |= (Index & 8) << Shift;
			}
		}
		return *this;
	}
	virtual IndexColorBase& OutputIndexColor(std::string_view Path) override {
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
	
protected:
	std::vector<uint16_t> Plane0;
	std::vector<uint16_t> Plane1;
	std::vector<uint16_t> Plane2;
	std::vector<uint16_t> Plane3;
};

int main(int argc, char *argv[])
{
	auto Image = cv::imread("mychara.png");
	{
		//if (1 < argc) {
			//auto Image = cv::imread(argv[1]);

		cv::imshow("Original", Image);
		cv::waitKey(0);

#if true
		//!< リサイズ
		cv::resize(Image, Image, cv::Size(16, 16), 0, 0, cv::INTER_NEAREST);

		//!< 減色
		{
			//!< 何色にするか
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
#endif

		//!< パレットとインデックスカラー画像
		IndexColorImage ICI(Image);
		ICI.CreatePalette().CreateIndexColor().Restore();

		//!< パレットとインデックスカラー画像(PCE)
		IndexColorImagePCESprite ICIPCE(Image);
		ICIPCE.CreatePalette().CreateIndexColor().Restore().OutputPalette("SPR_Palette.bin").OutputIndexColor("SPR_Pattern.bin");

		cv::resize(Image, Image, cv::Size(512, 512), 0, 0, cv::INTER_NEAREST);
		cv::imshow("resize & color reduction", Image);
		cv::waitKey(0);
	}
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
