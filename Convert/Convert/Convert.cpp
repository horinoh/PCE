// Convert.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>

#pragma warning(push)
#pragma warning(disable:4819)
#include <opencv2/opencv.hpp>
#pragma warning(pop)

#include <vector>
//#include <ranges>
#include <fstream>

#ifdef _DEBUG
#pragma comment(lib, "opencv_world454d.lib")
#else
#pragma comment(lib, "opencv_world454.lib")
#endif

int main(int argc, char *argv[])
{
	//auto Image = cv::imread("kueken7_rgb8.jpg");

	if (1 < argc) {
		auto Image = cv::imread(argv[1]);

		//!< リサイズ
		{
			cv::resize(Image, Image, cv::Size(256, 256));
		}

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

		//!< パレット
		std::vector<cv::Vec3b> Palette;
		for (auto i = 0; i < Image.rows; ++i) {
			for (auto j = 0; j < Image.cols; ++j) {
				const auto Color = Image.ptr<cv::Vec3b>(j)[i];
				if (end(Palette) == std::find(begin(Palette), end(Palette), Color)) {
					Palette.emplace_back(Color);
				}
			}
		}
		//!< PCEパレット (GGGRRRBBB)
		//!< 24bitカラー -> 9bitカラー へ丸められる為、近い色が同じになる場合があり、エントリが想定よりも少なくなる可能性がある
		std::vector<uint16_t> PCEPalette;
		for (auto i : Palette) {
			const auto Color = ((i[1] * 7 / 255) << 6) | ((i[2] * 7 / 255) << 3) | (i[0] * 7 / 255);
			if (end(PCEPalette) == std::find(begin(PCEPalette), end(PCEPalette), Color)) {
				PCEPalette.emplace_back(Color);
			}
		}
		//!< PCEパレットの出力
		{
			std::ofstream Out("PCEPalette.bin", std::ios::binary | std::ios::out);
			if (!Out.bad()) {
				Out.write(reinterpret_cast<const char*>(data(PCEPalette)), size(PCEPalette) * sizeof(PCEPalette[0]));
				Out.close();
			}
		}

		//!< インデックスカラー画像
		std::vector<uint32_t> IndexColorImage;
		for (auto i = 0; i < Image.rows; ++i) {
			for (auto j = 0; j < Image.cols; ++j) {
				const auto Color = Image.ptr<cv::Vec3b>(j)[i];
				const auto It = std::find(begin(Palette), end(Palette), Color);
				if (end(Palette) != It) {
					IndexColorImage.emplace_back(static_cast<uint32_t>(std::distance(begin(Palette), It)));
				}
			}
		}
		//!< PCEプレーン	 
		std::vector<uint16_t> PCEPlane0;
		std::vector<uint16_t> PCEPlane1;
		std::vector<uint16_t> PCEPlane2;
		std::vector<uint16_t> PCEPlane3;
		for (auto i = 0; i < size(IndexColorImage) / 16; ++i) {
			PCEPlane0.emplace_back(0);
			PCEPlane1.emplace_back(0);
			PCEPlane2.emplace_back(0);
			PCEPlane3.emplace_back(0);
			for (auto j = 0; j < 16; ++j) {
				const auto Index = IndexColorImage[i * 16 + j];
				const auto Shift = 15 - j;
				PCEPlane0.back() |= (Index & 1) << Shift;
				PCEPlane1.back() |= (Index & 2) << Shift;
				PCEPlane2.back() |= (Index & 4) << Shift;
				PCEPlane3.back() |= (Index & 8) << Shift;
			}
		}
		//!< PCEパターンの出力
		{
			std::ofstream Out("PCEPattern.bin", std::ios::binary | std::ios::out);
			if (!Out.bad()) {
				Out.write(reinterpret_cast<const char*>(data(PCEPlane0)), size(PCEPlane0) * sizeof(PCEPlane0[0]));
				Out.write(reinterpret_cast<const char*>(data(PCEPlane1)), size(PCEPlane1) * sizeof(PCEPlane1[0]));
				Out.write(reinterpret_cast<const char*>(data(PCEPlane2)), size(PCEPlane2) * sizeof(PCEPlane2[0]));
				Out.write(reinterpret_cast<const char*>(data(PCEPlane3)), size(PCEPlane3) * sizeof(PCEPlane3[0]));
				Out.close();
			}
		}

		cv::imshow("resize & color reduction", Image);
		cv::waitKey(0);

#if true
		//!< パレットとインデックスカラーから復元(チェック用)
		{
			cv::Mat RestoreImage(Image.size(), Image.type());
			int k = 0;
			for (auto i = 0; i < Image.rows; ++i) {
				for (auto j = 0; j < Image.cols; ++j) {
					RestoreImage.ptr<cv::Vec3b>(j)[i] = Palette[IndexColorImage[k++]];
				}
			}
			cv::imshow("restore", RestoreImage);
			cv::waitKey(0);
		}
#endif
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
