// Convert.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>

#pragma warning(push)
#pragma warning(disable:4819)
#include <opencv2/opencv.hpp>
#include <opencv2/core/utils/logger.hpp>
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

static void Preview(std::string_view Title, const cv::Mat Image) 
{
	cv::imshow(data(Title), Image);
	cv::waitKey(0);
	cv::destroyWindow(data(Title));
}
static void Preview(std::string_view Title, const cv::Mat& Image, const cv::Size& Size) 
{
	cv::Mat Resized;
	cv::resize(Image, Resized, Size, 0, 0, cv::INTER_NEAREST);
	Preview(Title, Resized);
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
	//!< 1�s�̍s��ƂȂ�悤�ɕό`
	cv::Mat Points;
	Image.convertTo(Points, CV_32FC3);
	Points = Points.reshape(3, Image.rows * Image.cols);

	//!< k-means �N���X�^�����O
	cv::Mat_<int> Clusters(Points.size(), CV_32SC1);
	cv::Mat Centers;
	cv::kmeans(Points, ColorCount, Clusters, cv::TermCriteria(cv::TermCriteria::Type::EPS | cv::TermCriteria::Type::MAX_ITER, 10, 1.0), 1, cv::KmeansFlags::KMEANS_PP_CENTERS, Centers);

	//!< �e�s�N�Z���l�𑮂���N���X�^�̒��S�l�Œu������
	Dst = cv::Mat(Image.size(), Image.type());
	auto It = Dst.begin<cv::Vec3b>();
	for (auto i = 0; It != Dst.end<cv::Vec3b>(); ++It, ++i) {
		const auto Color = Centers.at<cv::Vec3f>(Clusters(i), 0);
		(*It)[0] = cv::saturate_cast<uchar>(Color[0]); //!< B
		(*It)[1] = cv::saturate_cast<uchar>(Color[1]); //!< G
		(*It)[2] = cv::saturate_cast<uchar>(Color[2]); //!< R
	}
}

class Converter
{
public:
	Converter(const cv::Mat& Img) : Image(Img) {}

	using Palette = std::vector<uint16_t>;

	template <uint32_t W, uint32_t H>
	class Pattern
	{
	public:
		uint32_t PaletteIndex;
		std::array<uint8_t, W* H> ColorIndices;
	};

	virtual uint16_t ToPlatformColor(const cv::Vec3b& Color) const { return 0; }
	virtual cv::Vec3b FromPlatformColor(const uint16_t& Color) const { return cv::Vec3b(0, 0, 0); }

	virtual Converter& Create() {
		CreateMap();
		CreatePalette();
		OptimizePalette();
		CreatePattern();
		return *this;
	}

	void SplitMap(const cv::Size& MapSize, const cv::Size& PatSize) {
		CCPatterns.clear();
		Maps.clear();
		for (auto i = 0; i < MapSize.height; ++i) {
			for (auto j = 0; j < MapSize.width; ++j) {
				const auto NoFlip = Image(cv::Rect(j * PatSize.width, i * PatSize.height, PatSize.width, PatSize.height));
				cv::Mat VFlip, HFlip, VHFlip;
				cv::flip(NoFlip, VFlip, 0);
				cv::flip(NoFlip, HFlip, 1);
				cv::flip(NoFlip, VHFlip, -1);

				//!< ���]���邱�Ƃɂ�蓯���p�^�[���ƂȂ邩?
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
					//!< #TODO ���]�����o���Ă����K�v������
					Maps.emplace_back(static_cast<uint32_t>(size(CCPatterns)));
					CCPatterns.emplace_back(NoFlip);
				}
			}
		}
	}
	virtual Converter& CreateMap() { return *this; }
	virtual Converter& CreatePalette() { return *this; }
	virtual Converter& OptimizePalette() { return *this; }
	virtual Converter& CreatePattern() { return *this; }

	virtual const Converter& OutputPalette(std::string_view Path) const { return *this; }
	virtual const Converter& OutputPattern(std::string_view Path) const { return *this; }

	virtual const Converter& RestorePalette() const { return *this; }
	virtual const Converter& RestorePattern() const { return *this; }
	virtual const Converter& RestoreMap() const { return *this; }

protected:
	const cv::Mat& Image;
	std::vector<cv::Mat> CCPatterns;
	std::vector<uint16_t> Maps;
};

#pragma region PCE
/*
* ���		: 256 x 224
* BG		: 512 x 256(6 ��ނ���)�A(15 + 1) �F x 16 �p���b�g (9 �r�b�g�J���[ = 8 x 8 x 8 = 512 �F)
* �X�v���C�g	: 64 ���A16 x 16 ��(6 ��ނ���)�A(15 + 1) �F x 16 �p���b�g (9 �r�b�g�J���[)
*/
namespace PCE
{
	template<uint8_t W, uint8_t H>
	class ConverterBase : public Converter
	{
	private:
		using Super = Converter;
	public:
		ConverterBase(const cv::Mat& Img) : Super(Img), MapSize(Image.cols / W, Image.rows / H) {}

		//!< 0000 000G GGRR RBBB : 9 �r�b�g�J���[
		virtual uint16_t ToPlatformColor(const cv::Vec3b& Color) const override { return ((Color[1] >> 5) << 6) | ((Color[2] >> 5) << 3) | (Color[0] >> 5); }
		//!< Vec3b(B, G, R)
		virtual cv::Vec3b FromPlatformColor(const uint16_t& Color) const override { return cv::Vec3b((Color & 0x7) << 5, ((Color & (0x7 << 6)) >> 6) << 5, ((Color & (0x7 << 3)) >> 3) << 5); }

		virtual ConverterBase& CreateMap() override {
			SplitMap(MapSize, cv::Size(W, H));
			return *this;
		}
		virtual ConverterBase& CreatePalette() override {
			PaletteIndices.clear();
			Palettes.clear();
			for (auto pat : CCPatterns) {
				Palette Pal;
				for (auto i = 0; i < pat.rows; ++i) {
					for (auto j = 0; j < pat.cols; ++j) {
						const auto Color = ToPlatformColor(pat.ptr<cv::Vec3b>(i)[j]);
						if (end(Pal) == std::ranges::find(Pal, Color)) {
							Pal.emplace_back(Color);
						}
					}
				}
				//!< PCE �ł̓p���b�g�� 16 �F�܂� (�擪�ɓ����F�����邱�ƂɂȂ�̂ŁA�����ł� 15 �F�܂łɎ��܂��Ă��Ȃ��Ƃ����Ȃ�)
				if (size(Pal) > 15) { std::cerr << "Color Exceed " << size(Pal) << std::endl; }
				//!< �\�[�g���Ă���
				std::ranges::sort(Pal);
				//!< �����łȂ��ꍇ�̓p���b�g�̒ǉ�
				//!< �p�^�[�����g�p����p���b�g�C���f�b�N�X���o���Ă���
				const auto It = std::ranges::find(Palettes, Pal);
				if (end(Palettes) == It) {
					PaletteIndices.emplace_back(static_cast<uint32_t>(size(Palettes)));
					Palettes.emplace_back(Pal);
				}
				else {
					PaletteIndices.emplace_back(static_cast<uint32_t>(std::distance(begin(Palettes), It)));
				}
			}
			return *this;
		}
		virtual ConverterBase& OptimizePalette() override {
			while ([&]() {
				for (auto i = 0; i < size(Palettes); ++i) {
					for (auto j = i + 1; j < size(Palettes); ++j) {
						auto& lhs = Palettes[i];
						auto& rhs = Palettes[j];
						if (!empty(lhs) && !empty(rhs)) {
							Palette UnionSet;
							std::ranges::set_union(lhs, rhs, std::back_inserter(UnionSet));
							//!< �p���b�g�̘a�W���� 15 �F�ȉ��Ɏ��܂�ꍇ�́A��̃p���b�g�ɂ܂Ƃ߂鎖���\
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
				}()) {
			}
			//!< �p���b�g�ԍ����l�߂�
			{
				auto SortUnique = PaletteIndices;
				std::ranges::sort(SortUnique);
				const auto [B, E] = std::ranges::unique(SortUnique);
				SortUnique.erase(B, E);
				for (auto i = 0; i < size(SortUnique); ++i) {
					std::ranges::replace(PaletteIndices, SortUnique[i], i);
				}
			}
			//!< ��ɂȂ����p���b�g�͏���
			{
				const auto [B, E] = std::ranges::remove_if(Palettes, [](const Palette& rhs) { return empty(rhs); });
				Palettes.erase(B, E);
			}
			return *this;
		}
		virtual ConverterBase& CreatePattern() override {
			Patterns.clear();
			for (auto p = 0; p < size(CCPatterns); ++p) {
				const auto& Pat = CCPatterns[p];
				Patterns.emplace_back();
				Patterns.back().PaletteIndex = PaletteIndices[p];
				const auto& Pal = Palettes[Patterns.back().PaletteIndex];
				for (auto i = 0; i < Pat.rows; ++i) {
					for (auto j = 0; j < Pat.cols; ++j) {
						Patterns.back().ColorIndices[i * W + j] = static_cast<uint32_t>(std::distance(begin(Pal), std::ranges::find(Pal, ToPlatformColor(Pat.ptr<cv::Vec3b>(i)[j]))));
					}
				}
			}
			return *this;
		}

		virtual const Converter& OutputPalette(std::string_view Path) const override {
			std::cout << "\tPalette count = " << size(Palettes) << std::endl;
			//!< PCE �ł̓p���b�g�� 16 �܂� (BG�A�X�v���C�g���ꂼ�� 16)
			if (size(Palettes) > 16) { std::cerr << "\tPalette Exceed 16" << std::endl; }

			std::ofstream Out(data(Path), std::ios::binary | std::ios::out);
			if (!Out.bad()) {
				for (auto i : Palettes) {
					const uint16_t Color = 0;
					Out.write(reinterpret_cast<const char*>(&Color), sizeof(Color)); //!< �擪�ɓ����F(�����ł�0)���o��
					Out.write(reinterpret_cast<const char*>(data(i)), size(i) * sizeof(i[0]));
					for (auto j = size(i); j < 16; j++) {
						Out.write(reinterpret_cast<const char*>(&Color), sizeof(Color)); //!< �󂫗v�f��(�����ł�0)���o��
					}
				}
				Out.close();
			}
			return *this;
		}

		//!< �������ĕ\�����Ă݂� (�p���b�g�`�F�b�N�p)
		virtual const Converter& RestorePalette() const override {
#ifdef _DEBUG
			cv::Mat Res(cv::Size(15, static_cast<int>(size(Palettes))), Image.type());
			for (auto i = 0; i < size(Palettes); ++i) {
				for (auto j = 0; j < 15; ++j) {
					Res.ptr<cv::Vec3b>(i)[j] = j < size(Palettes[i]) ? FromPlatformColor(Palettes[i][j]) : cv::Vec3b();
				}
			}
			Preview("Palette", Res, Res.size() * 50);
#endif
			return *this;
		}
		//!< �������ĕ\�����Ă݂� (�p�^�[���`�F�b�N�p)
		virtual const Converter& RestorePattern() const override {
#ifdef _DEBUG
			const auto PatCount = static_cast<int>(size(Patterns));
			cv::Mat Res(cv::Size(16 * W, (PatCount / 16 + 1) * H), Image.type());
			for (auto p = 0; p < size(Patterns); ++p) {
				const auto& Pat = Patterns[p];
				const auto& Pal = Palettes[Pat.PaletteIndex];
				cv::Mat Tile(cv::Size(W, H), Image.type());
				for (auto i = 0; i < H; ++i) {
					for (auto j = 0; j < W; ++j) {
						Tile.ptr<cv::Vec3b>(i)[j] = FromPlatformColor(Pal[Pat.ColorIndices[i * W + j]]);
					}
				}
				Tile.copyTo(Res(cv::Rect((p % 16) * W, (p / 16) * H, W, H)));
			}

			DrawGrid(Res, cv::Size(W, H));
			Preview("Pattern", Res, Res.size() * 5);
#endif
			return *this;
		}
		//!< �������ĕ\�����Ă݂� (�}�b�v�`�F�b�N�p)
		virtual const Converter& RestoreMap() const override {
#ifdef _DEBUG
			cv::Mat Res(Image.size(), Image.type());
			for (auto m = 0; m < size(Maps); ++m) {
				const auto& Pat = Patterns[Maps[m]];
				const auto& Pal = Palettes[Pat.PaletteIndex];
				cv::Mat Tile(cv::Size(W, H), Image.type());
				for (auto i = 0; i < H; ++i) {
					for (auto j = 0; j < W; ++j) {
						Tile.ptr<cv::Vec3b>(i)[j] = FromPlatformColor(Pal[Pat.ColorIndices[i * W + j]]);
					}
				}
				Tile.copyTo(Res(cv::Rect((m % MapSize.width) * W, (m / MapSize.width) * H, W, H)));
			}

			DrawGrid(Res, cv::Size(W, H));
			Preview("Map", Res, Res.size() * 3);
#endif
			return *this;
		}

	private:
		std::vector<uint32_t> PaletteIndices;
	protected:
		cv::Size MapSize;
		std::vector<Palette> Palettes;
		std::vector<Pattern<W, H>> Patterns;
	};

	//!< �C���[�W : �X�N���[�������Ȃ��Î~�����
	//!< 4 �v���[���ɕ����ďo�́A4 �v���[�������킹��ƃJ���[�C���f�b�N�X�����܂�
	//!< �p�^�[�� 8 x 8 ��\���̂�
	//!<	�ŏ��� u16 x 8 �� ��ʉ��� 8 �r�b�g�փv���[�� 0, 1�A���� u16 x 8 �� ��ʉ��� 8 �r�b�g�փv���[�� 2, 3
	//!<	u16[00] 1111111100000000
	//!<	u16[01] 1111111100000000
	//!<	....
	//!<	u16[14] 3333333322222222
	//!<	u16[15] 3333333322222222
	//!<  
	//!<	Background Attribute Table (BAT)
	//!<	BAT ��p���āA�g�p�p���b�g�̓p�^�[�����Ɏw�肪�\ 
	//!<    LLLLTTTT TTTTTTTT
	//!<	L : �p���b�g�ԍ�[0, 15]�AT : �p�^�[���ԍ� [0, 4095](�����[�U���g����̂�[256, 4095])
	template<uint8_t W = 8, uint8_t H = 8>
	class ImageConverter : public ConverterBase<W, H>
	{
	private:
		using Super = ConverterBase<W, H>;
	public:
		ImageConverter(const cv::Mat& Img) : Super(Img) {}

		virtual ImageConverter& Create() override { Super::Create(); return *this; }

		virtual const ImageConverter& OutputPattern(std::string_view Path) const override {
			std::cout << "\tPattern count = " << size(this->Patterns) << std::endl;
			std::ofstream Out(data(Path), std::ios::binary | std::ios::out);
			if (!Out.bad()) {
				for (auto p : this->Patterns) {
					for (auto pl = 0; pl < 2; ++pl) {
						for (auto i = 0; i < H; ++i) {
							uint16_t Plane = 0;
							for (auto j = 0; j < W; ++j) {
								const auto ColIdx = p.ColorIndices[i * W + j] + 1; //!< �擪�̓����F���l������ + 1
								const auto ShiftL = 7 - j;
								const auto ShiftU = ShiftL + 8;
								const auto MaskL = 1 << ((pl << 1) + 0);
								const auto MaskU = 1 << ((pl << 1) + 1);
								Plane |= ((ColIdx & MaskL) ? 1 : 0) << ShiftL;
								Plane |= ((ColIdx & MaskU) ? 1 : 0) << ShiftU;
							}
							Out.write(reinterpret_cast<const char*>(&Plane), sizeof(Plane));
						}
					}
				}
				Out.close();
			}
			return *this;
		}
		//!< BAT �̓p�^�[���ԍ��ƃp���b�g�ԍ�����Ȃ�}�b�v
		virtual const ImageConverter& OutputBAT(std::string_view Path) const {
			std::cout << "\tBAT size = " << this->MapSize.width << " x " << this->MapSize.height << std::endl;
			std::ofstream Out(data(Path), std::ios::binary | std::ios::out);
			if (!Out.bad()) {
				for (auto m = 0; m < size(this->Maps); ++m) {
					const auto PatIdx = this->Maps[m];
					const auto PalIdx = this->Patterns[PatIdx].PaletteIndex;
					const uint16_t BATEntry = (PalIdx << 12) | PatIdx + 256; //!< �A�v������g�p�ł���̂� 256 �ȍ~ [256, 4095]
					Out.write(reinterpret_cast<const char*>(&BATEntry), sizeof(BATEntry));
				}
				Out.close();
			}
			return *this;
		}
	};

	//!< BG : �X�N���[��������w�i
	//!< 4 �v���[���ɕ����ďo�́A4 �v���[�������킹��ƃJ���[�C���f�b�N�X�����܂�
	//!< �^�C�� 16 x 16 ����ׂă}�b�v���쐬���邱�ƂɂȂ� (8 x 8 �ł͂Ȃ� 16 x 16 �P�ʂȂ̂Œ���) 
	//!< �^�C���� 1 / 4 �����ł��� 8 x 8 ��\���̂Ɉȉ��̂悤�ɂȂ�
	//!<	�ŏ��� u16 x 8 �� ��ʉ��� 8 �r�b�g�փv���[�� 0, 1�A���� u16 x 8 �� ��ʉ��� 8 �r�b�g�փv���[�� 2, 3
	//!<	u16[00] 1111111100000000
	//!<	u16[01] 1111111100000000
	//!<	....
	//!<	u16[14] 3333333322222222
	//!<	u16[15] 3333333322222222
	//!<	
	//!<	�^�C�� 16 x 16 ��\���ɂ� 4 ��J��Ԃ��ALT, RT, LB, RB �̏�
	//!<  
	//!< �g�p�p���b�g�̓^�C�����Ɏw�肪�\�A�^�C�����������p���b�g�ԍ���ʓr�o�͂���(4 �r�b�g�V�t�g����K�v������)
	//!< 
	//!< �}�b�v�͒P���Ƀ^�C���ԍ��� uint8_t ����ƂȂ�
	template<uint32_t W = 16, uint32_t H = 16>
	class BGConverter : public ConverterBase<W, H>
	{
	private:
		using Super = ConverterBase<W, H>;
	public:
		BGConverter(const cv::Mat& Img) : Super(Img) {}

		virtual BGConverter& Create() override { Super::Create(); return *this; }

		virtual const BGConverter& OutputPattern(std::string_view Path) const override {
			std::cout << "\tPattern count = " << size(this->Patterns) << std::endl;
			std::ofstream Out(data(Path), std::ios::binary | std::ios::out);
			if (!Out.bad()) {
				for (auto p : this->Patterns) {
					const auto W2 = W >> 1;
					const auto H2 = H >> 1;
					//!< LT
					for (auto pl = 0; pl < 2; ++pl) {
						for (auto i = 0; i < H2; ++i) {
							uint16_t Plane = 0;
							for (auto j = 0; j < W2; ++j) {
								const auto ColIdx = p.ColorIndices[i * W + j] + 1; //!< �擪�̓����F���l������ + 1
								const auto ShiftL = 7 - j;
								const auto ShiftU = ShiftL + 8;
								const auto MaskL = 1 << ((pl << 1) + 0);
								const auto MaskU = 1 << ((pl << 1) + 1);
								Plane |= ((ColIdx & MaskL) ? 1 : 0) << ShiftL;
								Plane |= ((ColIdx & MaskU) ? 1 : 0) << ShiftU;
							}
							Out.write(reinterpret_cast<const char*>(&Plane), sizeof(Plane));
						}
					}
					//!< RT
					for (auto pl = 0; pl < 2; ++pl) {
						for (auto i = 0; i < H2; ++i) {
							uint16_t Plane = 0;
							for (auto j = 0; j < W2; ++j) {
								const auto ColIdx = p.ColorIndices[i * W + j + W2] + 1; 
								const auto ShiftL = 7 - j;
								const auto ShiftU = ShiftL + 8;
								const auto MaskL = 1 << ((pl << 1) + 0);
								const auto MaskU = 1 << ((pl << 1) + 1);
								Plane |= ((ColIdx & MaskL) ? 1 : 0) << ShiftL;
								Plane |= ((ColIdx & MaskU) ? 1 : 0) << ShiftU;
							}
							Out.write(reinterpret_cast<const char*>(&Plane), sizeof(Plane));
						}
					}
					//!< LB
					for (auto pl = 0; pl < 2; ++pl) {
						for (auto i = 0; i < H2; ++i) {
							uint16_t Plane = 0;
							for (auto j = 0; j < W2; ++j) {
								const auto ColIdx = p.ColorIndices[(i + H2) * W + j] + 1;
								const auto ShiftL = 7 - j;
								const auto ShiftU = ShiftL + 8;
								const auto MaskL = 1 << ((pl << 1) + 0);
								const auto MaskU = 1 << ((pl << 1) + 1);
								Plane |= ((ColIdx & MaskL) ? 1 : 0) << ShiftL;
								Plane |= ((ColIdx & MaskU) ? 1 : 0) << ShiftU;
							}
							Out.write(reinterpret_cast<const char*>(&Plane), sizeof(Plane));
						}
					}
					//!< RB
					for (auto pl = 0; pl < 2; ++pl) {
						for (auto i = 0; i < H2; ++i) {
							uint16_t Plane = 0;
							for (auto j = 0; j < W2; ++j) {
								const auto ColIdx = p.ColorIndices[(i + H2) * W + j + W2] + 1;
								const auto ShiftL = 7 - j;
								const auto ShiftU = ShiftL + 8;
								const auto MaskL = 1 << ((pl << 1) + 0);
								const auto MaskU = 1 << ((pl << 1) + 1);
								Plane |= ((ColIdx & MaskL) ? 1 : 0) << ShiftL;
								Plane |= ((ColIdx & MaskU) ? 1 : 0) << ShiftU;
							}
							Out.write(reinterpret_cast<const char*>(&Plane), sizeof(Plane));
						}
					}
				}
				Out.close();
			}
			return *this;
		}
		virtual const BGConverter& OutputPatternPalette(std::string_view Path) const {
			std::ofstream Out(data(Path), std::ios::binary | std::ios::out);
			if (!Out.bad()) {
				for (auto p : this->Patterns) {
					const uint8_t PalIdx = p.PaletteIndex << 4; //!< �^�C�����̃p���b�g�C���f�b�N�X (4 �r�b�g�V�t�g����K�v������)
					Out.write(reinterpret_cast<const char*>(&PalIdx), sizeof(PalIdx));
				}
				Out.close();
			}
			return *this;
		}
		virtual const BGConverter& OutputMap(std::string_view Path) const {
			std::cout << "\tMap size = " << this->MapSize.width << " x " << this->MapSize.height << std::endl;
			std::ofstream Out(data(Path), std::ios::binary | std::ios::out);
			if (!Out.bad()) {
				for (auto m = 0; m < size(this->Maps); ++m) {
					const uint8_t PatIdx = static_cast<uint8_t>(this->Maps[m]);
					Out.write(reinterpret_cast<const char*>(&PatIdx), sizeof(PatIdx));
				}
				Out.close();
			}
			return *this;
		}
	};

	//!< �X�v���C�g
	//!< 4 �v���[���ɕ����ďo�́A4 �v���[�������킹��ƃJ���[�C���f�b�N�X�����܂�
	template<uint8_t W = 16, uint8_t H = 16>
	class SpriteConverter : public ConverterBase<W, H>
	{
	private:
		using Super = ConverterBase<W, H>;
	public:
		SpriteConverter(const cv::Mat& Img) : Super(Img) {}

		virtual SpriteConverter& Create() override { Super::Create(); return *this; }

		virtual const SpriteConverter& OutputPattern(std::string_view Path) const override {
			std::cout << "\tPattern count = " << size(this->Patterns) << std::endl;
			std::cout << "\tSprite size = " << static_cast<uint16_t>(W) << " x " << static_cast<uint16_t>(H) << std::endl;
			std::ofstream Out(data(Path), std::ios::binary | std::ios::out);
			if (!Out.bad()) {
				for (auto pat : this->Patterns) {
					//!< �p�^�[�����̃p���b�g�C���f�b�N�X
					std::cout << "\t Palette = " << pat.PaletteIndex << std::endl;
					//std::cout << "//!< Pattern " << std::endl;
					for (auto pl = 0; pl < 4; ++pl) {
						//std::cout << "\t//!< Plane " << pl << std::endl;
						for (auto i = 0; i < H; ++i) {
							uint16_t Plane = 0;
							for (auto j = 0; j < W; ++j) {
								const auto ColIdx = pat.ColorIndices[i * W + j] + 1; //!< �擪�̓����F���l������ + 1
								const auto Shift = 15 - j;
								const auto Mask = 1 << pl;
								Plane |= ((ColIdx & Mask) ? 1 : 0) << Shift;
							}
							Out.write(reinterpret_cast<const char*>(&Plane), sizeof(Plane));
							//std::cout << "\t0x" << std::hex << std::setw(4) << std::right << std::setfill('0') << Plane << "," << std::endl;
						}
					}
				}	
				Out.close();
			}
			return *this;
		}
	};
}
#pragma endregion //!< PCE

#pragma region FC
/*
* ���		: 256 x 240
* BG		: 256 x 240 x 2 ��ʁA�p�^�[�� 8 x 8 x 256 ��A(3 + 1) �F x 4 �p���b�g (52�F��)
* �X�v���C�g	: 8x8�A64 ���A�p�^�[�� 8 x 8 x 256 ��A(3 + 1) �F x 4 �p���b�g (52�F��) 
*/
namespace FC {
#define TO_BGR(r, g, b) cv::Vec3b(b, g, r)
	//!< FC �ł͌Œ�� 64 �F���̃G���g�������� (���������F������̂Ŏ����� 52 �F)
	static const std::array ColorEntries = {
		TO_BGR(117, 117, 117),
		TO_BGR(39,  27, 143),
		TO_BGR(0,   0, 171),
		TO_BGR(71,   0, 159),
		TO_BGR(143,   0, 119),
		TO_BGR(171,   0,  19),
		TO_BGR(167,   0,   0),
		TO_BGR(127,  11,   0),
		TO_BGR(67,  47,   0),
		TO_BGR(0,  71,   0),
		TO_BGR(0,  81,   0),
		TO_BGR(0,  63,  23),
		TO_BGR(27,  63,  95),
		TO_BGR(0,   0,   0),
		TO_BGR(0,   0,   0),
		TO_BGR(0,   0,   0),
		TO_BGR(188, 188, 188),
		TO_BGR(0, 115, 239),
		TO_BGR(35,  59, 239),
		TO_BGR(131,   0, 243),
		TO_BGR(191,   0, 191),
		TO_BGR(231,   0,  91),
		TO_BGR(219,  43,   0),
		TO_BGR(203,  79,  15),
		TO_BGR(139, 115,   0),
		TO_BGR(0, 151,   0),
		TO_BGR(0, 171,   0),
		TO_BGR(0, 147,  59),
		TO_BGR(0, 131, 139),
		TO_BGR(0,   0,   0),
		TO_BGR(0,   0,   0),
		TO_BGR(0,   0,   0),
		TO_BGR(255, 255, 255),
		TO_BGR(63, 191, 255),
		TO_BGR(95, 115, 255),
		TO_BGR(167, 139, 253),
		TO_BGR(247, 123, 255),
		TO_BGR(255, 119, 183),
		TO_BGR(255, 119,  99),
		TO_BGR(255, 155,  59),
		TO_BGR(243, 191,  63),
		TO_BGR(131, 211,  19),
		TO_BGR(79, 223,  75),
		TO_BGR(88, 248, 152),
		TO_BGR(0, 235, 219),
		TO_BGR(117, 117, 117),
		TO_BGR(0,   0,   0),
		TO_BGR(0,   0,   0),
		TO_BGR(255, 255, 255),
		TO_BGR(171, 231, 255),
		TO_BGR(199, 215, 255),
		TO_BGR(215, 203, 255),
		TO_BGR(255, 199, 255),
		TO_BGR(255, 199, 219),
		TO_BGR(255, 191, 179),
		TO_BGR(255, 219, 171),
		TO_BGR(255, 231, 163),
		TO_BGR(227, 255, 163),
		TO_BGR(171, 243, 191),
		TO_BGR(179, 255, 207),
		TO_BGR(159, 255, 243),
		TO_BGR(188, 188, 188),
		TO_BGR(0,   0,   0),
		TO_BGR(0,   0,   0),
	};
#undef TO_BGR

	//!< 2 �v���[���ɕ����ďo�́A2 �v���[�������킹��ƃJ���[�C���f�b�N�X�����܂�
	//!< �p�^�[�� 8 x 8 ��\���̂�
	//!<	�ŏ��� u8 x 8 �փv���[�� 0�A���� u8 x 8 �փv���[�� 1
	//!<	u8[00] 00000000
	//!<	u8[01] 00000000
	//!<	....
	//!<	u8[14] 11111111
	//!<	u8[15] 11111111
	template<uint8_t W, uint8_t H>
	class ConverterBase : public Converter
	{
	private:
		using Super = Converter;
	public:
		ConverterBase(const cv::Mat& Img) : Super(Img), MapSize(Image.cols / W, Image.rows / H) {}

		//!< ��ԋ߂��F�̃C���f�b�N�X��Ԃ�
		virtual uint16_t ToPlatformColor(const cv::Vec3b& Color) const override {
			uint8_t Index = 0xff;
			float minDistSq = std::numeric_limits<float>::max();
			for (auto i = 0; i < size(ColorEntries); ++i) {
				const auto d = cv::Vec3f(ColorEntries[i]) - cv::Vec3f(Color);
				const auto distSq = d.dot(d);
				if (distSq < minDistSq) {
					minDistSq = distSq;
					Index = i;
				}
			}
			return Index;
		}
		virtual cv::Vec3b FromPlatformColor(const uint16_t& Index) const override {
			if (Index < size(ColorEntries)) {
				return ColorEntries[Index];
			}
			return cv::Vec3b(0, 0, 0);
		}

		virtual ConverterBase& CreateMap() override {
			SplitMap(MapSize, cv::Size(W, H));
			return *this;
		}
		virtual ConverterBase& CreatePalette() override {
			PaletteIndices.clear();
			Palettes.clear();
			for (auto pat : CCPatterns) {
				Palette Pal;
				for (auto i = 0; i < pat.rows; ++i) {
					for (auto j = 0; j < pat.cols; ++j) {
						const auto Color = ToPlatformColor(pat.ptr<cv::Vec3b>(i)[j]);
						if (end(Pal) == std::ranges::find(Pal, Color)) {
							Pal.emplace_back(Color);
						}
					}
				}
				//!< FC �ł̓p���b�g�� 4 �F�܂� (�擪�ɓ����F�����邱�ƂɂȂ�̂ŁA�����ł� 3 �F�܂łɎ��܂��Ă��Ȃ��Ƃ����Ȃ�)
				if (size(Pal) > 3) { std::cerr << "Color Exceed " << size(Pal) << std::endl; }
				//!< �\�[�g���Ă���
				std::ranges::sort(Pal);
				//!< �����łȂ��ꍇ�̓p���b�g�̒ǉ�
				//!< �p�^�[�����g�p����p���b�g�C���f�b�N�X���o���Ă���
				const auto It = std::ranges::find(Palettes, Pal);
				if (end(Palettes) == It) {
					PaletteIndices.emplace_back(static_cast<uint32_t>(size(Palettes)));
					Palettes.emplace_back(Pal);
				}
				else {
					PaletteIndices.emplace_back(static_cast<uint32_t>(std::distance(begin(Palettes), It)));
				}
			}
			return *this;
		}
		virtual ConverterBase& OptimizePalette() override {
			while ([&]() {
				for (auto i = 0; i < size(Palettes); ++i) {
					for (auto j = i + 1; j < size(Palettes); ++j) {
						auto& lhs = Palettes[i];
						auto& rhs = Palettes[j];
						if (!empty(lhs) && !empty(rhs)) {
							Palette UnionSet;
							std::ranges::set_union(lhs, rhs, std::back_inserter(UnionSet));
							//!< �p���b�g�̘a�W���� 3 �F�ȉ��Ɏ��܂�ꍇ�́A��̃p���b�g�ɂ܂Ƃ߂鎖���\
							if (4 > size(UnionSet)) {
								lhs.assign(begin(UnionSet), end(UnionSet));
								rhs.clear();
								std::ranges::replace(PaletteIndices, j, i);
								return true;
							}
						}
					}
				}
				return false;
				}()) {
			}
			//!< �p���b�g�ԍ����l�߂�
				{
					auto SortUnique = PaletteIndices;
					std::ranges::sort(SortUnique);
					const auto [B, E] = std::ranges::unique(SortUnique);
					SortUnique.erase(B, E);
					for (auto i = 0; i < size(SortUnique); ++i) {
						std::ranges::replace(PaletteIndices, SortUnique[i], i);
					}
				}
				//!< ��ɂȂ����p���b�g�͏���
				{
					const auto [B, E] = std::ranges::remove_if(Palettes, [](const Palette& rhs) { return empty(rhs); });
					Palettes.erase(B, E);
				}
				return *this;
		}
		virtual ConverterBase& CreatePattern() override {
			Patterns.clear();
			for (auto p = 0; p < size(CCPatterns); ++p) {
				const auto& Pat = CCPatterns[p];
				Patterns.emplace_back();
				Patterns.back().PaletteIndex = PaletteIndices[p];
				const auto& Pal = Palettes[Patterns.back().PaletteIndex];
				for (auto i = 0; i < Pat.rows; ++i) {
					for (auto j = 0; j < Pat.cols; ++j) {
						Patterns.back().ColorIndices[i * W + j] = static_cast<uint32_t>(std::distance(begin(Pal), std::ranges::find(Pal, ToPlatformColor(Pat.ptr<cv::Vec3b>(i)[j]))));
					}
				}
			}
			return *this;
		}

		virtual const Converter& OutputPalette(std::string_view Path) const override {
			std::cout << "\tPalette count = " << size(Palettes) << std::endl;
			//!< FC �ł̓p���b�g�� 4 �܂� (BG�A�X�v���C�g���ꂼ�� 4)
			if (size(Palettes) > 4) { std::cerr << "\tPalette Exceed 4" << std::endl; }

			std::ofstream Out(data(Path), std::ios::binary | std::ios::out);
			if (!Out.bad()) {
				for (auto i : Palettes) {
					const uint16_t Color = 0;
					Out.write(reinterpret_cast<const char*>(&Color), sizeof(Color)); //!< �擪�ɓ����F(�����ł�0)���o��
					Out.write(reinterpret_cast<const char*>(data(i)), size(i) * sizeof(i[0]));
					for (auto j = size(i); j < 4; j++) {
						Out.write(reinterpret_cast<const char*>(&Color), sizeof(Color)); //!< �󂫗v�f��(�����ł�0)���o��
					}
				}
				Out.close();
			}
			return *this;
		}

		//!< �������ĕ\�����Ă݂� (�p���b�g�`�F�b�N�p)
		virtual const Converter& RestorePalette() const override {
#ifdef _DEBUG
			cv::Mat Res(cv::Size(4, static_cast<int>(size(Palettes))), Image.type());
			for (auto i = 0; i < size(Palettes); ++i) {
				for (auto j = 0; j < 4; ++j) {
					Res.ptr<cv::Vec3b>(i)[j] = j < size(Palettes[i]) ? FromPlatformColor(Palettes[i][j]) : cv::Vec3b();
				}
			}
			Preview("Palette", Res, Res.size() * 50);
#endif
			return *this;
		}
		//!< �������ĕ\�����Ă݂� (�p�^�[���`�F�b�N�p)
		virtual const Converter& RestorePattern() const override {
#ifdef _DEBUG
			const auto PatCount = static_cast<int>(size(Patterns));
			cv::Mat Res(cv::Size(16 * W, (PatCount / 16 + 1) * H), Image.type());
			for (auto p = 0; p < size(Patterns); ++p) {
				const auto& Pat = Patterns[p];
				const auto& Pal = Palettes[Pat.PaletteIndex];
				cv::Mat Tile(cv::Size(W, H), Image.type());
				for (auto i = 0; i < H; ++i) {
					for (auto j = 0; j < W; ++j) {
						Tile.ptr<cv::Vec3b>(i)[j] = FromPlatformColor(Pal[Pat.ColorIndices[i * W + j]]);
					}
				}
				Tile.copyTo(Res(cv::Rect((p % 16) * W, (p / 16) * H, W, H)));
			}

			DrawGrid(Res, cv::Size(W, H));
			Preview("Pattern", Res, Res.size() * 5);
#endif
			return *this;
		}
		//!< �������ĕ\�����Ă݂� (�}�b�v�`�F�b�N�p)
		virtual const Converter& RestoreMap() const override {
#ifdef _DEBUG
			cv::Mat Res(Image.size(), Image.type());
			for (auto m = 0; m < size(Maps); ++m) {
				const auto& Pat = Patterns[Maps[m]];
				const auto& Pal = Palettes[Pat.PaletteIndex];
				cv::Mat Tile(cv::Size(W, H), Image.type());
				for (auto i = 0; i < H; ++i) {
					for (auto j = 0; j < W; ++j) {
						Tile.ptr<cv::Vec3b>(i)[j] = FromPlatformColor(Pal[Pat.ColorIndices[i * W + j]]);
					}
				}
				Tile.copyTo(Res(cv::Rect((m % MapSize.width) * W, (m / MapSize.width) * H, W, H)));
			}

			DrawGrid(Res, cv::Size(W, H));
			Preview("Map", Res, Res.size() * 3);
#endif
			return *this;
		}

	private:
		std::vector<uint32_t> PaletteIndices;
	protected:
		cv::Size MapSize;
		std::vector<Palette> Palettes;
		std::vector<Pattern<W, H>> Patterns;
	};
	//!< �X�v���C�g
	//!< 2 �v���[���ɕ����ďo�́A2 �v���[�������킹��ƃJ���[�C���f�b�N�X�����܂�
	template<uint8_t W = 8, uint8_t H = 8>
	class SpriteConverter : public ConverterBase<W, H>
	{
	private:
		using Super = ConverterBase<W, H>;
	public:
		SpriteConverter(const cv::Mat& Img) : Super(Img) {}

		virtual SpriteConverter& Create() override { Super::Create(); return *this; }

		virtual const SpriteConverter& OutputPattern(std::string_view Path) const override {
			std::cout << "\tPattern count = " << size(this->Patterns) << std::endl;
			std::cout << "\tSprite size = " << static_cast<uint16_t>(W) << " x " << static_cast<uint16_t>(H) << std::endl;
			std::ofstream Out(data(Path), std::ios::binary | std::ios::out);
			if (!Out.bad()) {
				for (auto pat : this->Patterns) {
					//!< �p�^�[�����̃p���b�g�C���f�b�N�X
					std::cout << "\t Palette = " << pat.PaletteIndex << std::endl;
					//std::cout << "//!< Pattern " << std::endl;
					for (auto pl = 0; pl < 2; ++pl) {
						//std::cout << "\t//!< Plane " << pl << std::endl;
						for (auto i = 0; i < H; ++i) {
							uint16_t Plane = 0;
							for (auto j = 0; j < W; ++j) {
								const auto ColIdx = pat.ColorIndices[i * W + j] + 1; //!< �擪�̓����F���l������ + 1
								const auto Shift = 7 - j;
								const auto Mask = 1 << pl;
								Plane |= ((ColIdx & Mask) ? 1 : 0) << Shift;
							}
							Out.write(reinterpret_cast<const char*>(&Plane), sizeof(Plane));
							//std::cout << "\t0x" << std::hex << std::setw(4) << std::right << std::setfill('0') << Plane << "," << std::endl;
						}
					}
				}
				Out.close();
			}
			return *this;
		}
	};
}
#pragma endregion //!< FC

#pragma region GB
/*
* ���		: 160 x 144
* BG		: 256 x 256�A4 �~�����m�N��
* �X�v���C�g	: 40 ���A4 �~�����m�N��
*/
namespace GB
{

}
#pragma endregion //!< GB

static void ProcessPalette(std::string_view Name, std::string_view File)
{
	if (!empty(File)) {
		auto Image = cv::imread(data(File));
		std::cout << "[ Output Palette ] " << Name << " (" << File << ")" << std::endl;

#pragma region PCE
#if 0
		PCE::ImageConverter<>(Image).Create().OutputPalette(std::string(Name) + ".bin").RestorePalette();
#else
		PCE::BGConverter<>(Image).Create().OutputPalette(std::string(Name) + ".bin").RestorePalette();
#endif
#pragma endregion
	}
}
static void ProcessTileSet(std::string_view Name, std::string_view File, [[maybe_unused]] std::string_view Compression, [[maybe_unused]] std::string_view Option)
{
	if (!empty(File)) {
		auto Image = cv::imread(data(File));
		std::cout << "[ Output Pattern ] " << Name << " (" << File << ")" << std::endl;
#pragma region PCE
#if 0
		//!< �C���[�W�̏ꍇ�̓p�^�[�����S���قȂ����肷��̂ŁA�}�b�v(BAT) �𕜌�����̂Ƒ債�ĕς��Ȃ�
		PCE::ImageConverter<>(Image).Create().OutputPattern(std::string(Name) + ".bin");
#else
		PCE::BGConverter<>(Image).Create().OutputPattern(std::string(Name) + ".bin").RestorePattern();
		std::cout << "[ Output PatternPalette ] " << Name << ".pal" << " (" << File << ")" << std::endl;
		PCE::BGConverter<>(Image).Create().OutputPatternPalette(std::string(Name) + ".pal.bin");
#endif
#pragma endregion
	}
}
static void ProcessMap(std::string_view Name, std::string_view File, std::string_view TileSet, [[maybe_unused]] std::string_view Compression, [[maybe_unused]] const uint32_t Mapbase)
{
	if (!empty(File)) {
		auto Image = cv::imread(data(File));

#pragma region PCE
#if 0
		std::cout << "[ Output BAT ] " << Name << " (" << File << ")" << std::endl;
		PCE::ImageConverter<>(Image).Create().OutputBAT(std::string(Name) + ".bin").RestoreMap();
#else
		std::cout << "[ Output Map ] " << Name << " (" << File << ")" << std::endl;
		PCE::BGConverter<>(Image).Create().OutputMap(std::string(Name) + ".bin").RestoreMap();
#endif
#pragma endregion
	}
}
static void ProcessSprite(std::string_view Name, std::string_view File, const uint32_t Width, const uint32_t Height, [[maybe_unused]] std::string_view Compression, [[maybe_unused]] const uint32_t Time, [[maybe_unused]] std::string_view Collision, [[maybe_unused]] std::string_view Option, [[maybe_unused]] const uint32_t Iteration)
{
	if (!empty(File)) {
		auto Image = cv::imread(data(File));

		std::cout << "[ Output Sprite ] " << Name << " (" << File << ")" << std::endl;
#pragma region PCE
		switch (Width << 3) {
		case 16:
			switch (Height << 3) {
			case 16: 
				PCE::SpriteConverter<16, 16>(Image).Create().OutputPattern(std::string(Name) + ".bin").RestorePattern();
				break;
			case 32:
				PCE::SpriteConverter<16, 32>(Image).Create().OutputPattern(std::string(Name) + ".bin").RestorePattern();
				break;
			case 64:
				PCE::SpriteConverter<16, 64>(Image).Create().OutputPattern(std::string(Name) + ".bin").RestorePattern();
				break;
			default: break;
			}
			break;
		case 32:
			switch (Height << 3) {
			case 16:
				PCE::SpriteConverter<32, 16>(Image).Create().OutputPattern(std::string(Name) + ".bin").RestorePattern();
				break;
			case 32:
				PCE::SpriteConverter<32, 32>(Image).Create().OutputPattern(std::string(Name) + ".bin").RestorePattern();
				break;
			case 64:
				PCE::SpriteConverter<32, 64>(Image).Create();
				break;
			default: break;
			}
			break;
		default: break;
		}
#pragma endregion
	}
}
int main(int argc, char* argv[])
{
	//!< LOG_LEVEL_INFO ���� gtk ����̗]�v�ȃ��O���o�Ă������̂�
	cv::utils::logging::setLogLevel(cv::utils::logging::LOG_LEVEL_WARNING);

	//!< �^�[�Q�b�g�t�H���_ (Target folder)
	std::string_view Path = ".";
	for (const auto& i : std::filesystem::directory_iterator(Path)) {
		if (!i.is_directory()) {
			//!< .res �t�@�C����T�� (Search for .res files)
			if (i.path().has_extension() && ".res" == i.path().extension().string()) {
				std::ifstream In(i.path().filename().string(), std::ios::in);
				if (!In.fail()) {
					//!< �s��ǂݍ��� (Read line)
					std::string Line;
					while (std::getline(In, Line)) {
						//!< ���ڂ�ǂݍ��� (Read items)
						std::vector<std::string> Items;
						std::stringstream SS(Line);
						std::string Item;
						while (std::getline(SS, Item, ' ')) {
							Items.emplace_back(Item);
						}

						if (!empty(Items)) {
							//!< �摜�t�@�C�������� " ����菜�� (Remove " from image file name)
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
